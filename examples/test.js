var libotr = require('otr');
console.log("libotr version:",libotr.version());
//libotr.debugOn();

//create user alice, specify encryption keys and fingerprints files
var alice = new libotr.User({name:'alice',keys:'./db/alice.keys',fingerprints:'./db/alice.fp'});

//create a connection context used to identify communication with Bob
var BOB = alice.ConnContext("alice@telechat.org","telechat","BOB");

//setup an OTR communication channel with Bob
var otrchan_a = new libotr.OTRChannel(alice, BOB,{secret:'s3cr37'});

var bob = new libotr.User({name:'bob',keys:'./db/bob.keys',fingerprints:'./db/bob.fp'});
var ALICE = bob.ConnContext("bob@telechat.org","telechat","ALICE");
var otrchan_b = new libotr.OTRChannel(bob, ALICE,{secret:'s3cr37'});

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
    }else{
        console.log('not-encrypted: Bob->Alice: ',msg);
    }
    if(TEST_PASSED){ console.log("TEST PASSED"); process.exit();} 
});

//output incoming messages to console
otrchan_b.on("message",function(msg){
    if(this.isEncrypted()) {
        console.log('encrypted: Alice->Bob: ', msg);
    }else{
        console.log('not-encrypted: Alice->Bob: ',msg);
    }
    if(TEST_PASSED){ console.log("TEST PASSED"); process.exit();} 
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
otrchan_a.send("Hello, World!"); //this would not be encrypted.

var TEST_PASSED=false;

var loop = setInterval(function(){
    if(otrchan_a.isEncrypted() && otrchan_a.isAuthenticated()){
        console.log("Finger print verification successful");    
        TEST_PASSED=true;
        clearInterval(loop);
        otrchan_b.send("Meet me at midnight...near the docks...");        
    }
},500);

var test_timeout = setTimeout(function(){
    if(TEST_PASSED){ console.log("TEST PASSED"); } else { console.log("TEST FAILED"); }
    if(loop) clearInterval(loop);
},10000);
