var libotr = require('otr');

var alice = new libotr.User({name:'alice',keys:'./db/alice.keys',fingerprints:'./db/alice.fp'});
var BOB = alice.ConnContext("alice@telechat.org","telechat","BOB");
var otrchan_a = new libotr.OTRChannel(alice, BOB);

var bob = new libotr.User({name:'bob',keys:'./db/bob.keys',fingerprints:'./db/bob.fp'});
var ALICE = bob.ConnContext("bob@telechat.org","telechat","ALICE");
var otrchan_b = new libotr.OTRChannel(bob, ALICE);

console.log(alice.state.fingerprint("alice@telechat.org","telechat"));
console.log(bob.state.fingerprint("bob@telechat.org","telechat"));

console.log(ALICE.accountname,BOB.accountname);

console.log(otrchan_a);
console.log(otrchan_b);

otrchan_a.on("inject_message",function(msg){
	otrchan_b.recv(msg);
});
otrchan_b.on("inject_message",function(msg){
	otrchan_a.recv(msg);
});

otrchan_a.connect();
otrchan_b.connect();

otrchan_a.on("message",console.log);
otrchan_b.on("message",otrchan_b.send);

otrchan_a.send("Hello, World!");

