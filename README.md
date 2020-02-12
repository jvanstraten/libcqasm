# This is a work in progress.
## The old version is [here](https://github.com/QE-Lab/libqasm/)

libcqasm (cQASM parser & printer)
=================================

Why? What's wrong with the old one (libqasm)?
---------------------------------------------

 - The grammar specification is exceedingly contrived, mostly due to whitespace
   and comments not being filtered out by the lexer.

 - Some things in the grammar are just fundamentally wrong. `version -0.e-0`
   should probably not be acceptable, yet the parser thinks it's perfectly fine
   (version numbers are not floats).

 - The C++ AST classes are a awkwardly designed. For instance, the `Operation`
   class has separate qubit reference members for single-qubit gates, two-qubit
   gates, and Toffoli gates (nevermind that Fredkin gates may be introduced at
   some point), instead of just having a vector of `Qubits`, sized by the number
   of qubit arguments. As another example, users of the library are forced to
   determine which gate is what by a string-based name, instead of through
   compile-time checked OOP methods or an enumeration.

 - `libqasm` just spams `new` calls around, without any sort of `delete` to
   clean back up or informing the user that it now owns the objects and should
   call delete. Basically, it leaks memory *by design*.

 - For some reason the C++ library tries to both throw exceptions and return
   error codes. This is confusing at best. For the record, throwing an
   exception makes the statements below unreachable, so the functions will in
   fact always return success.

 - There is practically no documentation for the API that I could find.

 - The C++ classes use the namespace `compiler`, which is overly generic and
   also wrong, as it's just a parser, not a compiler.

 - While we're on the subject of namespacing, the build system for the old
   library only outputs a shared object, generically named `liblexgram.so`.
   Shared objects can't really be namespaced other than the name you give them
   initially (or hacks like rpath, which cause more problems than they solve),
   so installing anything using this library in the normal Linux way will have
   it end up in your system root (`/usr/lib` most likely). A static library is
   more appropriate in my opinion, but the shared object should at least be
   named `libcqasm.so`.

 - The Python wrapper is certainly not documented through docstrings, as it is
   entirely generated from the C++ classes. Basically, to use libqasm in
   Python, you first have to understand the C++ API, kind of defeating the
   purpose.

 - The documentation for cQASM itself (`1805.09607.pdf`, for some reason) does
   not correspond exactly to the implemented grammar. For example, U gates are
   entirely undocumented, and parity measurements are supposedly implemented
   for any number of qubits, but this number is hardcoded to 2 in the
   implementation (using another `std::pair<Qubits, Qubits>`). The `qasm.bnf`
   file is different yet again.

 - There is no way to print an internal representation back to a valid cQASM
   file. If cQASM is to be used as some form of intermediate representation
   between several QCA tools, having everyone write their own printer is a bit
   nasty.

The above is all fine for something thrown together to solve a problem, which I
guess is what it was at the time. It's decidedly *not* a good foundation to
build a composable compiler framework around.

How?
----

The goal of the rewrite is the following:

 - Rewrite the grammar in such a way that it becomes a proper superset of the
   existing grammar and the documentation (that is, it must accept all
   previously accepted input), without accepting programs that were reasonably
   rejected by the previous version (that is, no language features are added).

 - Properly document the grammar specification-style instead of paper-style.

 - Redesign the C++ API to more logically reflect the AST. Use exceptions.
   Where necessary, use `std::shared_ptr` to prevent memory leaks.

 - Allow the user to manipulate the AST or build a new one from scratch, in
   such a way that any errors introduced along the way are detected
   immediately.

 - Allow the user to pretty-print the ASTs back to cQASM.

 - Properly document the above.

 - Write a Pythonic Python API on top of the above, using Python docstrings
   for documentation.

 - Write a layer on top of the new C++ library that's interface-compatible with
   the old one for backward-compatibility during the transition period.

 - Use a CMake buildsystem that works on Windows, Mac, and Linux (like the
   old one) and that allows itself to be included in other CMake files using
   `FetchContent`.

 - Use `GoogleTest` to run tests, and stuff it into continuous integration.

When?
-----

I (Jeroen van Straten) will work on this for the foreseeable future whenever I
don't have any higher-priority stuff to do for the ABS group (Zaid Al-Ars).
