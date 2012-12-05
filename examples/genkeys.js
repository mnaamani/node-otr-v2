var otr=require("otr3");
console.log("libotr version:",otr.version());
var users = {
    alice: new otr.UserState(),
    bob: new otr.UserState()
};

users.alice.conf ={
    keys: __dirname+"/keys/alice.keys",
    fingerprints: __dirname+"/keys/alice.fp",
    account: "alice@telechat.org",
    proto: "telechat"   
};

users.bob.conf={
    keys: __dirname+"/keys/bob.keys",
    fingerprints: __dirname+"/keys/bob.fp",
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

