all:
	emcc detector.c -O3 -s ALLOW_MEMORY_GROWTH=1 -s EXPORTED_FUNCTIONS=@exports.json -s EXPORTED_RUNTIME_METHODS=cwrap -o detector.js
	#emcc detector.c -O3 -s EXPORTED_FUNCTIONS=@exports.json -o detector.js
