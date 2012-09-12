# Off-the-Record Messaging [protocol v2]
# NodeJs Module

This module exposes an OO Evented API which wraps the native libotr installed.
Supports only versions v3.2.x of library. (v 4.0 is not backwards compatible)

Only tested on Ubuntu. (If you can help to make it build on windows please do).

Still a work in progress..

## Building the module

Required packages

	sudo apt-get install nodejs nodejs-dev libotr2 libotr2-dev

Build and install the node module (locally):

	make
	make install

Run a quick test

	node examples/test.js


## Using
The module exports the following functions:

methods:
* version()

constructors:
* User
* UserState
* ConnContext
* MessageAppOps
* OTRChannel : EventEmitter

## version()
The version() function will return the version of the native libotr.so loaded by nodejs.

    var libotr_version = require("otr").version();

    
## User( config )
The User object is a wrapper for UserState (see below). It holds a user's configuration [name, keys, fingerprints] 

    var libotr = require("otr");
    var alice = new libotr.User({
        name:'Alice',			      //an identifier for the User object
        keys:'../home/alice/alice.keys',      //path to OTR keys file (required)
        fingerprints:'../home/alice/alice.fp' //path to fingerprints file (required)
    });

If files exists the keys and fingerprints will be loaded into the userstate automatically.
A warning will be displayed otherwise.

If you need to generate a new OTR key for a given accountname and protocol: 

    alice.generateKey("alice@jabber.org", "xmpp", function(err){
        //callback function
        //if error occured err will be the text representation of gcrypt error number.
    });

To directly access the wrapped UserState object:

    var userstate = alice.state;
    userstate.accounts().forEach(function(account){
        console.log(account.fingerprint);
    });

## UserState()
The UserState holds keys and fingerprints in memory. It exposes methods to read/write these keys
and fingerprints to the file system, as well as methods to generate keys. There are two sets of methods:

Async Methods:
* generateKey()
* readKeys()
* readFingerprints()
* writeFingerprints()
  
Sync Methods
* fingerprint()
* accounts()
* readKeysSync()
* readFingerprintsSync()
* writeFingerprintsSync()

example usage: https://github.com/mnaamani/node-telehash/blob/master/experiment/otr-keymanager.js

## ConnContext
A ConnContext with a recipient 'BOB' for a given UserState (userstate) can be created as follows:

    var ctx = new libotr.ConnContext(userstate, "alice@jabber.org","xmpp","BOB" );

where the second and third arguments specifiy which OTR key to use. The last argument is
our selected name for the recipient Bob;

Or from a User object (alice):

    var ctx = alice.ConnContext("alice@jabber.org","xmpp","BOB");

The following properties of the ConnContext object are exposed:

* protocol
* username
* accountname
* msgstate
* fingerprint
* protocol_version
* smstate
* trust
  
            
## MessageAppOps
  messageSending()
  messageReceiving()
  fragmentAndSend()
  disconnect()
  initSMP()
  respondSMP()
  
## OTRChannel

    var otrchannel = new libotr.OTRChannel(alice, BOB, {
        policy:59,          //optional - policy - default = 59
        MTU:1450,           //optional - max fragment size in bytes - default = 1450
        secret:"SECRET",    //secret for SMP authentication.
                            //questions and answers also for SMP Authentication.
        secrets:{'question-1':'secret-1','question-2':'secret-2'}
    });

Methods:

* connect
* send
* recv
* close
* start_smp
* start_smp_question
* respond_smp
* isEncrypted
* isAuthenticated

Events:

* smp_request
* smp_complete
* smp_failed
* smp_aborted
* display_otr_message
* is_logged_in
* gone_secure
* gone_insecure
* remote_disconnected
* policy
* update_context_list
* max_message_size
* inject_message
* create_privkey
* notify
* log_message
* new_fingerprint
* write_fingerprints
* still_secure

## Otr-talk
A very experimental p2p OTR messaging application..

https://github.com/mnaamani/node-telehash/blob/master/experiment/otr-talk.js

## License
GPLv2

## Links
http://www.cypherpunks.ca/otr/

## TODO

