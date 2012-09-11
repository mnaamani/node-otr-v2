# Off-the-Record messaging module for Nodejs (protocol v2)

This module exposes an OO Evented API which wraps the native libotr v2 installed.

Only tested on Ubuntu. (If you can help to make it build on windows please do).

Still work in progress..

## Building the module

Required packages

	sudo apt-get install nodejs nodejs-dev libotr2 libotr2-dev

Build and install the node module (locally):

	make
	make install

Run a quick test

	node examples/test.js

If you don't see any errors all is well.

## Using the module

Look at the examples.. I will update this section soon.

User
UserState 
ConnContext
MessageAppOps
OTRChannel

## Otr-talk

A very experimental p2p OTR messaging application..

https://github.com/mnaamani/node-telehash/blob/master/experiment/otr-talk.js

## License
	
GPLv2

## Links

http://www.cypherpunks.ca/otr/
	
