# Off-the-Record Messaging [protocol v2]
# NodeJs Module

This module exposes a simple evented API which wraps the native libotr installed.
Supports only versions v3.2.x of library. (v4 is not backwards compatible)

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


## API
The module exports the following functions:

methods:
* version()
* POLICY()

constructors:
* User()
* UserState()
* ConnContext()
* MessageAppOps()
* OTRChannel() : EventEmitter

## version()
The version() function will return the version of the native libotr.so loaded by nodejs.

    var libotr_version = require("otr").version();

## POLICY(name)
The policy is used as a parameter in OTRChannel.

	var libotr = require("otr");
	var policy = libotr.POLICY("DEFAULT");

	//You can select from below policies
    'NEVER'
    'ALLOW_V1',
    'ALLOW_V2'
    'REQUIRE_ENCRYPTION'
    'SEND_WHITESPACE_TAG'
    'WHITESPACE_START_AKE'
    'ERROR_START_AKE'
	'VERSION_MASK'
	'OPPORTUNISTIC'
	'MANUAL'
	'ALWAYS'
	'DEFAULT'
    
## User()
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
        //otherwise err will be 'null'
    });

To directly access the wrapped UserState object:

    var userstate = alice.state;
    userstate.accounts().forEach(function(account){
        console.log(account.fingerprint);
    });

## UserState()
The UserState holds keys and fingerprints in memory. It exposes methods to read/write these keys
and fingerprints to the file system, as well as methods to generate them.
	
	var libotr = require("otr");
	var userstate = new libotr.UserState();


### userstate.generateKey(path_to_keys_file, accountname, protocol, [callback])
generateKey() will create a new OTR key for provided accountname/protocol (overwriting existing key).
The newly generated key will be stored stored in the userstate. When the process is complete the 
userstate/keys are written out to file.

	userstate.generateKey('/home/alice/myotr.keys', 'alice@jabber.org','xmpp',function(err){
		//call back with err if any
		if(err){
			//oops something went wrong.
			console.log(err);
		}
	});

### userstate.readKeys(path_to_keys_file, [callback])
Asynchronously reads the stored keys into the userstate.

### userstate.readFingerprints(path_to_fingerprints_file, [callback])
Asynchronously reads the stored fingerprints into the userstate.

### userstate.writeFingerprints(path_to_fingerprints_file, [callback])
Asynchronously writes out the fingerprints in userstate to file.

### userstate.fingerprint(accountname,protocol)
Returns the fingerprint of the key associated with accountname and protocol of the form:

	'65D366AF CF9B065F 41708CB0 1DC26F61 D3DF5935'

### userstate.accounts()
Returns an array of account objects:

	[ { accountname: 'alice@jabber.org',
	    protocol: 'xmpp',
	    fingerprint: '65D366AF CF9B065F 41708CB0 1DC26F61 D3DF5935' } ]

Synchronous versions of above methods:

### userstate.readKeysSync(path_to_keys_file)
### userstate.readFingerprintsSync(path_to_fingerprints_file)
### userstate.writeFingerprintsSync(path_to_fingerprints_file)

example code: https://github.com/mnaamani/node-telehash/blob/master/experiment/otr-keymanager.js

## ConnContext
A ConnContext with a recipient 'BOB' for a given UserState (userstate) can be created as follows:

    var ctx = new libotr.ConnContext(userstate, "alice@jabber.org","xmpp","BOB" );

where the second and third arguments specifiy which OTR key to use. The last argument is
our selected name for the recipient Bob;

Or from a User object (alice):

    var ctx = alice.ConnContext("alice@jabber.org","xmpp","BOB");

The following properties of the ConnContext object are exposed:

* protocol: string: eg. "xmpp"
* username: string: name we have given to the recipient, "BOB"
* accountname: string: account name of the otr key, eg. "alice@jabber.org"
* fingerprint: string: active fingerprint - of recipient's key
* protocol_version: number: otr protocol version no. in use, eg. 2
* msgstate: number: 0 = plaintext, 1 = encrypted
* smstate: number: current state of the SMP (Socialist Millionaire's Protocol)
* trust: string: 'smp' if recipient's fingerprint has been verified by SMP.
  
## OTRChannel
OTRChannel creates a simple interface for exchanging messages with a recipient. As arguments
it takes a UserState,ConnContext,and a dictionary of parameters for the channel:

    var otrchannel = new libotr.OTRChannel(alice, BOB, {
        policy:libotr.POLICY("ALWAYS"), //optional policy - default = POLICY("DEFAULT")
        MTU:1450,          //optional - max fragment size in bytes - default = 1450
        secret:"SECRET",   //secret for SMP authentication.                           
        secrets:{'question-1':'secret-1',
                 'question-2':'secret-2'} //questions and answers also for SMP Authentication.
    });

### Methods:

### otrchan.connect()
connect() will initiate the otr protocol with remote side.
This can be used if we wish to initiate the protocol without sending an actual message.

### otrchan.send(message)
send() will fragment and send message.toString() to remote side.

### otrchan.recv(message)
call recv() when receiving message from remote side.

### otrchan.close()
close() will shutdown the otr channel.

### otrchan.start_smp([secret])
starts SMP authentication. If otional [secret] is not passed it is taken from the parameters.

### otrchan.start_smp_question(question,[secret])
start SMP authentication with a question and optional [secret]. If secret is not passed 
it is taken from the parameters of the otrchannel.

### otrchan.respond_smp([secret])
responds to SMP authentication request with optional [secret]. If secret is not passed 
it is taken from the parameters of the otrchannel.

### otrchan.isEncrypted()
returns 'true' only if current session is encrypted.

### otrchan.isAuthenticated()
return 'true' only if fingerprint of remote side has been authenticated/verified by SMP.

### Events

* message(msg)

* inject_message(msg_fragment)

* gone_secure()
* gone_insecure()
* still_secure()

* create_privkey()
* new_fingerprint(fingerprint)

* smp_request(question)
* smp_complete()
* smp_failed()
* smp_aborted()

* remote_disconnected()
* update_context_list()
* shutdown()

* display_otr_message(msg) //human readable notification message
* notify(title,primary,secondary) //notification (fired after display_otr_message for same notification message)
* log_message(msg) //debug log messages from libotr


## MessageAppOps
### messageSending()
### messageReceiving()
### fragmentAndSend()
### disconnect()
### initSMP()
### respondSMP()


## Otr-talk
A very experimental p2p OTR messaging application..

https://github.com/mnaamani/node-telehash/blob/master/experiment/otr-talk.js

## License
GPLv2

## Links
http://www.cypherpunks.ca/otr/

## TODO


