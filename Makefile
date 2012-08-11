NODE_PREFIX = /usr/local
CXX = g++
CFLAGS = 
NODE_CFLAGS := -g -O2 -fPIC -DPIC -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 -D_GNU_SOURCE -DEV_MULTIPLICITY=0 -I$(NODE_PREFIX)/include/node
CCLD = $(CXX)
LDFLAGS = 
NODE_LDFLAGS = 
LIBS = -lotr

module:
	node-waf configure build
	mkdir -p node_modules
	rm -rf node_modules/otr
	mkdir -p node_modules/otr/lib
	cp package.json node_modules/otr
	cp otr.js node_modules/otr/lib
	cp build/Release/otrnat.node node_modules/otr/lib

install:
	cp -r node_modules/otr ~/.node_libraries/
clean:
	node-waf distclean
	rm -fR node_modules/
