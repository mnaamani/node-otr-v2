var otr=require("./otrnat");
if(otr.version()!="3.2.1"){
	console.log("Warning. you do not have the latest version of libotr installed on your system.");
}
var util = require('util');
var events = require('events');

exports.version = otr.version;
exports.UserState = otr.UserState;
exports.ConnContext = otr.ConnContext;
exports.MessageAppOps = otr.MessageAppOps;

exports.User = User;
exports.OTRChannel = OTRChannel;

util.inherits(OTRChannel, events.EventEmitter);

function User( config ){
    this.name = config.name;    
    this.state = new otr.UserState();
    this.keys = config.keys;
    this.fingerprints = config.fingerprints;
    try{    
        this.state.readKeysSync(this.keys);
    }catch(e){ console.error("Warning Reading Keys:",e);}
    try{
        this.state.readFingerprintsSync(this.fingerprints);
    }catch(e){ console.error("Warning Reading Fingerprints:",e);}
}

User.prototype.ConnContext = function(accountname, protocol, recipient){    
    return new otr.ConnContext(this.state,accountname,protocol,recipient);
}

User.prototype.writeFingerprints = function(){
    console.error("Writing Fingerprints to:",this.fingerprints);
    this.state.writeFingerprints(this.fingerprints);
}

function OTRChannel(user, context, parameters){
    events.EventEmitter.call(this);
    
    this.user = user;
    this.context = context;
    this.parameters = parameters;
    this.ops = new otr.MessageAppOps( OtrEventHandler(this) );
    
}
OTRChannel.prototype.connect = function(){
    return this.send("?OTRv2?");
};
OTRChannel.prototype.send = function(message){
    //message can be any object that can be serialsed to a string using it's .toString() method.   
    var msgout, err;
    msgout = this.ops.messageSending(this.user.state, this.context.accountname, this.context.protocol, this.context.username, message.toString());
    if(msgout){
       err=this.ops.fragmentAndSend(this.context,msgout);
       return err;       
    }
};
OTRChannel.prototype.recv = function(message){
    //message can be any object that can be serialsed to a string using it's .toString() method.
    var msg = this.ops.messageReceiving(this.user.state, this.context.accountname, this.context.protocol, this.context.username, message.toString());
    if(msg) this.emit("message",msg);
};
OTRChannel.prototype.close = function(){
    this.ops.disconnect(this.user.state,this.context.accountname,this.context.protocol,this.context.username);
    this.emit("shutdown");
};
OTRChannel.prototype.start_smp = function(secret){
    var sec = secret || this.parameters? this.parameters.secret:undefined || undefined;
    if(!sec) throw( new Error("No Secret Provided"));
    this.ops.initSMP(this.user.state, this.context, sec);
};

OTRChannel.prototype.start_smp_question = function(question,secret){
    if(!question){
        throw(new Error("No Question Provided"));        
    }
    var sec = secret;
    if(!sec){
        sec = this.parameters ? this.parameters.secrets : undefined;
        if(!sec) throw(new Error("No Secrets Provided"));
        sec = sec[question];        
    }    
    
    if(!sec) throw(new Error("No Secret Matched for Question"));
   
    this.ops.initSMP(this.user.state, this.context, sec,question);
};

OTRChannel.prototype.respond_smp = function(secret){
    var sec = secret ? secret : undefined;
    if(!sec){
        sec = this.parameters ? this.parameters.secret : undefined;
    }
    if(!sec) throw( new Error("No Secret Provided"));    
    this.ops.respondSMP(this.user.state, this.context, sec);
};
OTRChannel.prototype.isEncrypted = function(){
    return (this.context.msgstate===1);
};
OTRChannel.prototype.isAuthenticated = function(){
    return (this.context.trust==="smp");
};

function OtrEventHandler( otrChannel ){
 function emit(){
    otrChannel.emit.apply(otrChannel,arguments);
 }
 return (function(o){
    //console.error(o.EVENT);
    switch(o.EVENT){
        case "smp_request":
            emit(o.EVENT,o.question);
            return;
        case "smp_complete":
            emit(o.EVENT);
            return;
        case "smp_failed":
            emit(o.EVENT);
            return;
        case "smp_aborted":
            emit(o.EVENT);
            return;
        case "display_otr_message":
            emit(o.EVENT,o.message);
            return;
        case "is_logged_in":
            return 1;
        case "gone_secure":
            emit(o.EVENT);
            return;
        case "gone_insecure":
            emit(o.EVENT);
            return;
        case "remote_disconnected":
            emit(o.EVENT);
            return;
        case "policy":                  
            //OTRL_POLICY_DEFAULT == OTRL_POLICY_OPPORTUNISTIC == 55
            //OTRL_POLICY_ALWAYS == 59
            if(!otrChannel.parameters) return 59;            
            return otrChannel.parameters.policy || 59;
        case "update_context_list":
            emit(o.EVENT);
            return;
        case "max_message_size":
            if(!otrChannel.parameters) return 1450;            
            return otrChannel.parameters.MTU || 1450;
        case "inject_message":
            emit(o.EVENT,o.message);
            return;
        case "create_privkey":
            emit(o.EVENT);
            return;
        case "notify":
            emit(o.EVENT,o.title,o.primary,o.secondary);
            return;
        case "log_message":
            emit(o.Event,o.message);
            return;
        case "new_fingerprint":
            emit(o.EVENT,o.fingerprint);
            return;
        case "write_fingerprints":            
            otrChannel.user.writeFingerprints();
            return;
        case "still_secure":
            emit(o.EVENT);
            return;
        default:
            console.error("UNHANDLED EVENT:",o.EVENT);
            return;
    }
 });
}
