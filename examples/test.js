var libotr = require('otr');
console.log("libotr version:",libotr.version());
//libotr.debugOn();

//create user alice, specify encryption keys and fingerprints files
var alice = new libotr.User({name:'alice',keys:'./db/alice.keys',fingerprints:'./db/alice.fp'});

//create a connection context used to identify communication with Bob
var BOB = alice.ConnContext("alice@telechat.org","telechat","BOB");

//setup an OTR communication channel with Bob
var otrchan_a = new libotr.OTRChannel(alice, BOB,{policy:libotr.POLICY("ALWAYS"),secret:'s3cr37'});

var bob = new libotr.User({name:'bob',keys:'./db/bob.keys',fingerprints:'./db/bob.fp'});
var ALICE = bob.ConnContext("bob@telechat.org","telechat","ALICE");
var otrchan_b = new libotr.OTRChannel(bob, ALICE,{policy:libotr.POLICY("ALWAYS"),secret:'s3cr37'});

console.log("Alice's OTR fingerprint:",alice.state.fingerprint("alice@telechat.org","telechat"));
console.log("Bob's OTR fingerprint:",bob.state.fingerprint("bob@telechat.org","telechat"));

console.log(ALICE.accountname,"<==>",BOB.accountname);

//dump the OTR channel objects
console.log(otrchan_a);
console.log(otrchan_b);

//simulate a connection between two parties
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
        if(TEST_PASSED) exit_test("");
    }else{
        //policy is set to ALWAYS so we should not get any unencrypted messages!
        console.log('not-encrypted!!!: Bob->Alice: ',msg);
    }
});

//output incoming messages to console
otrchan_b.on("message",function(msg){
    if(this.isEncrypted()) {
        console.log('encrypted: Alice->Bob: ', msg);
        if(TEST_PASSED) exit_test("");
    }else{
        //policy is set to ALWAYS so we should not get any unencrypted messages!
        console.log('not-encrypted!!!: Alice->Bob: ',msg);
    }
});

//connection is encrypted..
otrchan_a.on("gone_secure",function(){
    //if note previously authenticated (fingerpring verified do it now)
    if(this.context.trust!="smp"){
        console.log("doing SMP authentication to verify keys...");
        this.start_smp();    
    }
});

otrchan_b.on("smp_request",function(){
    this.respond_smp();    
});

//alice sends a message to bob
otrchan_a.send("Hello, World!");

var TEST_PASSED=false;

var loop = setInterval(function(){
    if(otrchan_a.isEncrypted() && otrchan_a.isAuthenticated()){
        console.log("Finger print verification successful");    
        TEST_PASSED=true;
        if(loop) clearInterval(loop);
        otrchan_b.send("Meet me at midnight...near the docks...");        
    }
},500);

var test_timeout = setTimeout(function(){
    if(loop) clearInterval(loop);
    exit_test("Timeout! - try again..");//usually something went wrong during SMP authentication
},10000);

function exit_test(msg){
    dumpConnContext(otrchan_a,"Alice's ConnContext:");
    dumpConnContext(otrchan_b,"Bob's ConnContext:");
    console.log(msg);
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
