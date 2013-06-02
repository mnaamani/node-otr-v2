## OTR3 - Off-the-Record Messaging [native-bindings]

This module exposes a simple evented API which wraps the native libotr installed.
Supports only versions v3.2.x of library.

Only tested on Debian/Ubuntu. (If you can help to make it build on windows please do).

Still a work in progress..

### Install pre-requisits:

    $ apt-get install make awk g++ nodejs nodejs-dev libotr2 libotr2-dev
    $ npm install -g node-gyp
	
### Getting the Module through git:

    git clone https://github.com/mnaamani/node-otr-v2.git node_modules/otr3
    cd node_modules/otr3
    npm install

### Getting the module directly from npm registry:

    npm install otr3

If the install fails because compiling failed, check that you have all the pre-requisists installed.
After resolving the problem you must reinstall the module:

    npm install otr3 --force

[API Documentation](https://github.com/mnaamani/node-otr-v2/blob/master/doc/API.md)

### License
GPLv2

### Links
The Excellent OTR Messaging software:
http://www.cypherpunks.ca/otr/

Great guide for writing C++ node.js extensions:
http://kkaefer.github.com/node-cpp-modules/

Very useful set of tools when working with v8/Node:
http://code.google.com/p/v8-juice/wiki/V8Convert
