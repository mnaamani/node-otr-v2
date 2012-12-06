/*
 *  Off-the-Record Messaging bindings for nodejs
 *  Copyright (C) 2012  Mokhtar Naamani,
 *                      <mokhtar.naamani@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of version 2 of the GNU General Public License as
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
var debug_mode = false;
exports.debugOn = function(){
    debug_mode = true;
};
exports.debugOff = function(){
    debug_mode = false;
};
function debug(msg){
    if(debug_mode) console.error(msg);
}

//load node C++ native module
var otr=require("./build/Release/otrnat");

if(otr.version()!="3.2.1"){
	console.error("Warning. You are not using the latest version of libotr on your system.");
}

var util = require('util');
var events = require('events');

//low level - wrappers to C API
exports.version = otr.version;
exports.UserState = otr.UserState;
exports.ConnContext = otr.ConnContext;
//discourage direct use of otr.MessageAppOps - keep it internal
//exports.MessageAppOps = otr.MessageAppOps;

//high level - javascript API
exports.User = User;
exports.OTRChannel = OTRChannel;
exports.POLICY = POLICY;

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

User.prototype.generateKey = function(accountname,protocol,callback){
    this.state.generateKey(this.keys,accountname,protocol,callback);
}

User.prototype.ConnContext = function(accountname, protocol, recipient){    
    return new otr.ConnContext(this.state,accountname,protocol,recipient);
}

User.prototype.writeFingerprints = function(){
    console.error("Writing Fingerprints to:",this.fingerprints);
    this.state.writeFingerprintsSync(this.fingerprints);
}

function OTRChannel(user, context, parameters){
    events.EventEmitter.call(this);
    
    this.user = user;
    this.context = context;
    this.parameters = parameters;
    this.ops = new otr.MessageAppOps( OtrEventHandler(this) );
    
}
OTRChannel.prototype.connect = function(){
    return this.send("");
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
    debug(o.EVENT);
    switch(o.EVENT){
        //custom events - raised by ops.messageReceiving() 
        case "smp_request":         emit(o.EVENT,o.question);break;
        case "smp_complete":        emit(o.EVENT);break;
        case "smp_failed":          emit(o.EVENT);break;
        case "smp_aborted":         emit(o.EVENT);break;
        case "remote_disconnected": emit(o.EVENT);break;
        //message operations
        case "gone_secure":         emit(o.EVENT);break;
        case "gone_insecure":       emit(o.EVENT);break;
        case "still_secure":        emit(o.EVENT);break;
        case "update_context_list": emit(o.EVENT);break;
        case "inject_message":      emit(o.EVENT,o.message);break;
        case "create_privkey":      emit(o.EVENT);break;
        case "display_otr_message": emit(o.EVENT,o.message);break;
        case "notify":              emit(o.EVENT,o.title,o.primary,o.secondary);break;
        case "log_message":         emit(o.EVENT,o.message);break;
        case "new_fingerprint":     emit(o.EVENT,o.fingerprint);break;
        case "write_fingerprints":
            otrChannel.user.writeFingerprints();
            emit(o.EVENT);//event handled raise event anyway
            break;
        case "is_logged_in":        return 1;break;//assume remote buddy is logged in.
        case "policy":
            //if no policy specified, default to DEFAULT policy
            if(!otrChannel.parameters) return POLICY("DEFAULT");
            if(typeof otrChannel.parameters.policy == 'number' ){
                return otrChannel.parameters.policy;
            }
            return POLICY("DEFAULT");
        case "max_message_size":
            //fragment size : default no fragmentation
            if(!otrChannel.parameters) return 0;
            return otrChannel.parameters.MTU || 0;
        default:
            //test that we handled all the operations
            debug("Unhandled Message Operation:",o.EVENT);
            return;
    }
 });
}

/* --- libotr-3.2.1/src/proto.h   */
var _policy = {
    'NEVER':0x00,
    'ALLOW_V1': 0x01,
    'ALLOW_V2': 0x02,
    'REQUIRE_ENCRYPTION': 0x04,
    'SEND_WHITESPACE_TAG': 0x08,
    'WHITESPACE_START_AKE': 0x10,
    'ERROR_START_AKE': 0x20
};

_policy['VERSION_MASK'] = _policy['ALLOW_V1']|_policy['ALLOW_V2'];
_policy['OPPORTUNISTIC'] =  _policy['ALLOW_V1']|_policy['ALLOW_V2']|_policy['SEND_WHITESPACE_TAG']|_policy['WHITESPACE_START_AKE']|_policy['ERROR_START_AKE'];
_policy['MANUAL'] = _policy['ALLOW_V1']|_policy['ALLOW_V2'];
_policy['ALWAYS'] = _policy['ALLOW_V1']|_policy['ALLOW_V2']|_policy['REQUIRE_ENCRYPTION']|_policy['WHITESPACE_START_AKE']|_policy['ERROR_START_AKE'];
_policy['DEFAULT'] = _policy['OPPORTUNISTIC']

function POLICY(p){  
    return _policy[p];
};
