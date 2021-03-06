gltfpp
======
[![Build Status](https://travis-ci.org/mmha/gltfpp.svg?branch=master)](https://travis-ci.org/mmha/gltfpp)

gltfpp is a glTF 2.0 loader written in C++14 targeting desktop platforms and WebAssembly with a focus on ease of use, type safety and extensibility.

**WORK IN PROGRESS!** This library is not finished yet.

Requirements
------------
- gcc >= 6 or clang >= 3.6
- CMake >= 3.2
- Boost >= 1.61

[Boost.Outcome](https://github.com/ned14/boost.outcome), [GSL](https://github.com/Microsoft/gsl), [nlohmann/json](https://github.com/nlohmann/json), [better-enums](https://github.com/aantron/better-enums) and [Catch](https://github.com/philsquared/Catch) are included via git submodules.

Building
--------
gltfpp is header only and therefore does not require a build step. Make sure you have all dependencies set up correctly if you only copy the directory.
### Clang Modules
When using clang, modules can by enabled by passing `-DGLTFPP_ENABLE_MODULES=1` to CMake. This should speed up the compilation, however this is experimental and has only been tested with clang 5.0 snapshots so far.

### Targeting the Web
Install emscripten, then invoke CMake via emconfigure:
```
emconfigure cmake .. [-GNinja] [-DGLTFPP_WEBASSEMBLY=1] # Default is wasm instead of asm.js
```
Compiling to WebAssembly requires binaryen. Note that this library does not export anything, so you can't use it in the browser yet.
By passing the flag `-DGLTFPP_AS_HTML=1`, emscripten generates a html page with a terminal emulator for each executable (read: the unit test runner).

Running the tests
-----------------
*[Run them in your browser (requires WebAssembly support)](https://mmha.github.io/gltfpp/)*
```
mkdir build && cd build
cmake .. -DGLTFPP_BUILD_TESTS=1 [-GNinja]
make gltfpp_test # Or ninja
ctest .
```
### Fuzzing
To run the fuzzer, build the `fuzz` target and execute it via `test/fuzz`. There is no meaningful dictionary for efficient fuzzing yet.

Finished tasks
------
- Parsing infrastructure (trivial properties can be parsed simply by declaring their fields)
- Finished properties: `Asset`, `Buffer` (only data uris), `BufferView`
- Build system support for Emscripten

TODO
----
- deserialization of external buffer data (GLB)
- Networking (HTTP on desktop, XHR on WebAssembly), asynchronous loading
- Proper documentation
- out of tree extensions?
- a small viewer based on the loader
- Export functions to the web browser
