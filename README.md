This is a minimal example that (hopefully) reproduces a presumed bug in the OCaml 4.03.0 garbage collector.

It seems that starting with OCaml 4.02.0, OCaml's Weak module sometimes makes the garbage collector release custom blocks while they are actually still alive (or creates copies).
The example code works as follows:
It uses custom blocks with C stubs that allocate a structure outside the OCaml heap for each custom block that is created by the OCaml program. Once the custom block is finalized by the GC, this fact is recorded in its corresponding structure. The example program never creates two custom blocks holding pointers to the same structure.
However, if compiled with the release version of, e.g. OCaml 4.03.0, the example program continues to operate on custom blocks that have been finalized before. More precisely, some custom blocks that are presented to the C stubs turn out to refer to structures of a custom block that was already finalized.

The results differ between the compiler versions. The example works correctly at least for
- 3.12.1
- 4.00.0
- 4.00.1
- 4.01.0
- 4.03.0+beta1

Starting from 4.02.0, the above bug (?) seems to be triggered (with 4.03.0+beta1 being a notable exception):
- 4.02.0
- 4.02.1
- 4.02.2
- 4.02.3
- 4.03.0+beta2
- 4.03.0

To reproduce, compile with
```
ocamlbuild -use-ocamlfind gcbug.native
```
