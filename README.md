# OTR3 - Off-the-Record Messaging [protocol v2][native bindings]

This module exposes a simple evented API which wraps the native libotr installed.
Supports only versions v3.2.x of library.

Only tested on Ubuntu. (If you can help to make it build on windows please do).

Still a work in progress..

## Building the module

Required tools:

	$ apt-get install nodejs nodejs-dev libotr2 libotr2-dev

node-gyp:

	$ npm install -g node-gyp

install the module with npm:

    npm -g install

Run a quick test

	node examples/test.js


You can also install the module directly from npm registry:

    npm -g install otr3

## License
GPLv2

## Links
The Excellent OTR Messaging software:
http://www.cypherpunks.ca/otr/

Great guide for writing C++ node.js extensions:
http://kkaefer.github.com/node-cpp-modules/

Very useful set of tools when working with v8/Node:
http://code.google.com/p/v8-juice/wiki/V8Convert
