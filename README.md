This is a minimal example that (hopefully) reproduces a presumed bug in the OCaml 4.03.0 garbage collector.

It seems that in 4.03.0, OCaml's Weak module sometimes makes the garbage collector release custom blocks while they are actually still alive (or creates copies).
The example code works as follows:
It uses custom blocks with C stubs that allocate a structure outside the OCaml heap for each custom block that is created by the OCaml program. Once the custom block is finalized by the GC, this fact is recorded in its corresponding structure. The example program never creates two custom blocks holding pointers to the same structure.
However, if compiled with the release version of OCaml 4.03.0, the example program continues to operate on custom blocks that have been finalized before. More precisely, some custom blocks that are presented to the C stubs turn out to refer to structures of a custom block that was already finalized.


The program terminates correctly with the 4.03.0+beta1 pre-release version,
but crashes if compiled with the official 4.03.0 version of OCaml.

To reproduce, compile with
```
ocamlbuild -use-ocamlfind gcbug.native
```
