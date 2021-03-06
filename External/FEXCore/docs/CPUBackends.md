# FEXCore CPU Backends
---
FEXCore supports multiple CPU emulation backends. All of which ingest the IR that we have been generating.

## IR Interpreter
The first one is the easiest. This just walks the IR list and interprets the IR as it goes through it. It isn't meant to be fast and is for debugging purposes.
This is used to easily inspect what is going on with the code generation and making sure logic is sound. Will most likely last in to perpetuity since it isn't exactly difficult to maintain and it is useful to have around

## IR JIT
**Not yet implemented**
This is meant to be our first JIT of call and will serve multiple purposes. It'll be the JIT that is used for our runtime compilation of code.
This means it needs to be fast during compilation and have decent runtime performance.
Good chance that we will need to implement multiple of these depending on host architecture with some code reuse between them.
This JIT will also be what we use for gathering sampling data for passing off to our LLVM JIT for tiered recompilation and offline compilation later.
Should use xbyak for our x86-64 host and Vixl for our AArch64 host. For other targets in the future we will see what is available

## LLVM JIT
This is the last JIT that should theoretically generate the most optimal code for us.
This *should* be used for a tiered recompiler system using sampling data from the IR JIT.
Currently it just supports being a regular JIT core. There are still some hard problems that need to be solved with this JIT since LLVM isn't quite ideal for generating code for a JIT.

# Future ideas
---
* Support a custom ABI on the LLVM JIT to generate more optimal code that is shared between the IR JIT and LLVM JIT
  * This can let us do fun things like reserve host registers for guest register state. Trivial in the IR JIT, not so much for LLVM.
  * Needs a local build of LLVM that we statically link in.
* Create an inline ASM or JIT'd dispatcher loop. Will allow our JITs to be more optimal by reserving more registers for guest state.
* WebAssmembly or other browser language?
  * Might allow decent runtime performance of things emulated in a browser. Could be interesting.
