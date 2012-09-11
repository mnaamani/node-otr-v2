var otr=require("otr");
console.log("libotr version:",otr.version());

var users = {
    alice: new otr.UserState(),
    bob: new otr.UserState()
};

users.alice.conf ={
    keys: "./db/alice.keys",
    fingerprints: "./db/alice.fp",
    account: "alice@telechat.org",
    proto: "telechat"   
};

users.bob.conf={
    keys: "./db/bob.keys",
    fingerprints: "./db/bob.fp",
    account: "bob@telechat.org",
    proto: "telechat"    
};

function genKey(user,cb){
    var conf = user.conf;
    console.log("Generating OTR key for:", conf.proto+":"+conf.account);
    user.generateKey(conf.keys,conf.account,conf.proto,function(err){        
	    if(!err){
	     console.log("Key Generated. for:",conf.proto+":"+conf.account);	     
	    }else{
	     console.log(err);
	    }
        if(cb) cb(user);
    });
}
genKey(users.alice, function(){
	genKey(users.bob);
});

