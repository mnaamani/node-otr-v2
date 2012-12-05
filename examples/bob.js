var libotr = require('otr3');
var dgram = require('dgram');
var keys_dir = __dirname+"/keys";
var ALLOW_NEW_RELATIONSHIPS = true;

var bob = new libotr.User({name:'bob',keys:keys_dir+'/bob.keys',fingerprints:keys_dir+'/bob.fp'});
var ALICE = bob.ConnContext("bob@telechat.org","telechat","ALICE");
var otrchan = new libotr.OTRChannel(bob, ALICE, {    
    policy:59,
    MTU:1450,
    secret:"SECRET",
    secrets:{'question-1':'secret-1','question-2':'secret-2'}
    //secret:"WRONGSECRET",
    //secrets:{'question-1':'wrongsecret-1','question-2':'wrongsecret-2'}
});

var interval;

var server = dgram.createSocket("udp4", function(msg,rinfo){
    otrchan.recv(msg);
});

otrchan.on("inject_message",function(msg){
    var buff = new Buffer(msg, "utf8");
    server.send(buff, 0, buff.length, 5000, "127.0.0.1");
});

server.bind(5001, '127.0.0.1');

otrchan.connect();

var msgcount = 0;
interval = setInterval(function(){
    if(otrchan.isEncrypted() && otrchan.isAuthenticated() ){
        
        otrchan.send("Hello I'm Bob! -- "+msgcount++)
    }
},1000);

otrchan.on("message",function(msg){
    if(this.isAuthenticated()) console.log("<< ",msg);
    else console.log("ignoring: <<",msg);
});

otrchan.on("display_otr_message",function(msg){
    console.error("[OTR]",msg);
});
otrchan.on("log_message",function(msg){
    console.error("[LOG]",msg);
});
otrchan.on("new_fingerprint",function(fp){
    console.log(this.context.username,"'s New Fingerprint:",fp);
    if(ALLOW_NEW_RELATIONSHIPS) return;
    console.log("No New Peers Accepted.. Aborting.");
    this.close();
});
otrchan.on("gone_secure",function(){    
    console.log("Connection Encrypted.");
    if(this.context.trust!="smp" && ALLOW_NEW_RELATIONSHIPS ){
        console.log("Waiting for Remote Peer to Authenticate...");                     
        //this.start_smp(); //if we want to establish new trust relationship
    }else {
        if(this.context.trust!="smp"){
            console.log("Only Previously Authenticated Peers Allowed! Abandoning Session");
            this.close();            
            return;
        }
        console.log("Peer Authenticated [In Previous Session]");
    }
});
otrchan.on("still_secure",function(){
    console.log("Secure Connection Re-Established");
});
otrchan.on("remote_disconnected",function(){
    console.log("Remote Peer Disconnected. Ending Session.");
    this.close();
});

otrchan.on("gone_insecure",function(){
    console.log("Connection INSECURE!");
    this.close();
});

otrchan.on("smp_request",function(question){
    console.log("Responding to SMP Authentication");
    if(question){
      console.log("Question=",question);
      if(this.parameters && this.parameters.secrets && this.parameters.secrets[question]){
          this.respond_smp(this.parameters.secrets[question]);
      }else{
          console.log("We don't have secret to match the incoming Question challenge");
          this.close();
      }
    }else{
      this.respond_smp();
    }
});

otrchan.on("smp_complete",function(){
    console.log("SMP_COMPLETE");
    if(this.context.trust=="smp") {
        //we initiated the smp authentication and smp completed successfully
        console.log("Peer Authenticated.");
    }else{
        //remote end initiated smp authentication.. it successeded now its our turn..
        console.log("Authenticating..."); 
        this.start_smp();
    }
});

otrchan.on("smp_failed",function(){
    console.log("Peer Authentication Failed!");
    this.close();
});

otrchan.on("smp_aborted",function(){
    //this generally happens if both ends try to init smp at same time..    
    console.log("SMP_ABORTED");
    return;
    /*
    var chan=this;    
    setTimeout(function(){
        if(chan.context.trust!="smp") {
            console.log("Authenticaing.. (retry)");
            chan.start_smp_question('question-1');
        }
    },Math.random()*8000);
    */
});
