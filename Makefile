module:
	node-gyp configure
	node-gyp build
	rm -rf node_modules/otr
	mkdir -p node_modules
	mkdir -p node_modules/otr/lib
	cp package.json node_modules/otr
	cp otr.js node_modules/otr/lib
	cp build/Release/lib.target/otrnat.node node_modules/otr/lib

install:
	mkdir -p ~/.node_libraries/otr
	cp -r node_modules/otr ~/.node_libraries/

uninstall:
	rm -fr ~/.node_libraries/otr
clean:
	node-gyp clean
	rm -fr node_modules/
