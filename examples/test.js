var libotr = require('otr3');
console.log("libotr version:",libotr.version());

var verbose = true;//TODO: read command line param -v / --verbose 

if(verbose){
    libotr.debugOn();
}

var keys_dir = __dirname + "/keys";
//create user alice, specify encryption keys and fingerprints files
var alice = new libotr.User({name:'alice',keys:keys_dir+'/alice.keys',fingerprints:keys_dir+'/alice.fp'});

//create a connection context used to identify communication with Bob
var BOB = alice.ConnContext("alice@telechat.org","telechat","BOB");

//setup an OTR communication channel with Bob
var otrchan_a = new libotr.OTRChannel(alice, BOB,{policy:libotr.POLICY("ALWAYS"),secret:'s3cr37'});

var bob = new libotr.User({name:'bob',keys:keys_dir+'/bob.keys',fingerprints:keys_dir+'/bob.fp'});
var ALICE = bob.ConnContext("bob@telechat.org","telechat","ALICE");
var otrchan_b = new libotr.OTRChannel(bob, ALICE,{policy:libotr.POLICY("ALWAYS")});

console.log("Alice's OTR fingerprint:",alice.state.fingerprint("alice@telechat.org","telechat"));
console.log("Bob's OTR fingerprint:",bob.state.fingerprint("bob@telechat.org","telechat"));

console.log(alice.accounts());
console.log(bob.accounts());

//dump the OTR channel objects
console.log(otrchan_a);
console.log(otrchan_b);
    
//simulate a network connection between two parties
otrchan_a.on("inject_message",function(msg){
	otrchan_b.recv(msg);
});
otrchan_b.on("inject_message",function(msg){
	otrchan_a.recv(msg);
});


//output incoming messages to console
otrchan_a.on("message",function(msg){
    if(this.isEncrypted()) {
        console.log('encrypted: Bob->Alice: ', msg);        
    }else{
        //policy is set to ALWAYS so we should not get any unencrypted messages!
        console.log('not-encrypted!!!: Bob->Alice: ',msg);
    }
});

//output incoming messages to console
otrchan_b.on("message",function(msg){    
    if(this.isEncrypted()) {
        console.log('encrypted: Alice->Bob: ', msg);
    }else{
        //policy is set to ALWAYS so we should not get any unencrypted messages!
        console.log('not-encrypted!!!: Alice->Bob: ',msg);
    }
});

//will get fired because we are manually closing otrchan_b
otrchan_b.on("shutdown",function(){
    console.log("Bob's channel shutting down.");
});

//because otrchan_b was closed otrchan_a get a remote_disconnect event.
otrchan_a.on("remote_disconnected",function(){
    console.log("Bob disconnected");
    exit_test(true);
});

//connection is encrypted..
otrchan_a.on("gone_secure",function(){
    //if fingerprint not previously authenticated, do it now
    if(this.context.trust!="smp"){
        console.log("Alice initiating SMP authentication to verify keys...");
        this.start_smp();
    }
});

otrchan_b.on("smp_request",function(){
    console.log("Bob responding to SMP request.");
    this.respond_smp('s3cr37');
});

otrchan_a.on("create_privkey",function(account,protocol){
    console.log("Alice doesn't have a key for",account,protocol," creating one now..");
    alice.generateKey(account,protocol,function(err){
        if(err){
            console.log(err);
        }
        console.log("Key Generation Done.");
        //restart the OTR conversation
        otrchan_a.connect();        
    });
    console.log("handled create_privkey");
});
otrchan_b.on("create_privkey",function(account,protocol){
    console.log("Bob  doesn't have a key for",account,protocol," creating one now..");
    bob.generateKey(account,protocol,function(err){
        if(err){
            console.log(err);
        }
        console.log("Key Generation Done.");
        //restart the OTR conversation
        otrchan_b.connect();
    });
    console.log("handled create_privkey");
});


otrchan_a.connect();

var loop = setInterval(function(){
    if(otrchan_a.isEncrypted() && otrchan_a.isAuthenticated()){
        if(loop) clearInterval(loop);
        otrchan_b.close();
    }
},1000);

function exit_test(TEST_PASSED){
    if(TEST_PASSED){ console.log("== TEST PASSED ==\n"); } else { console.log("== TEST FAILED ==\n"); }
    process.exit();
}

function dumpConnContext(chan,msg){
    console.log(msg,
        chan.context.username,
        chan.context.protocol,
        chan.context.accountname,
        chan.context.fingerprint,
        chan.context.protocol_version,
        chan.context.msgstate,
        chan.context.trust,
        chan.context.smstate);
}
