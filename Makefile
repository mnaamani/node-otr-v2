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
uninstall:
	rm -fr ~/.node_libraries/otr
clean:
	node-waf distclean
	rm -fr node_modules/
