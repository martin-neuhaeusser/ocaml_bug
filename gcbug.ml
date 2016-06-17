type dummy

external wrapper_create_dummy : int -> dummy = "wrapper_create_dummy"

(* Wrapper to allow construction of weak sets of dummies *)
module DummyWrapper = struct
  type t = dummy

  (* The compare, hash, and equal functions use the C-layer implementation
     as defined by the custom_operations block in dummy.c *)
  let compare (d1 : t) d2 = Pervasives.compare d1 d2
  let hash (d1 : t) = Hashtbl.hash d1
  let equal (d1 : t) d2 = (compare d1 d2 = 0)
end

module WeakDummySet = Weak.Make (DummyWrapper)

(* Create a "hash consed" dummy, i.e. lookup the new dummy value in
   a weak set and return an existing equal dummy, if it already exists *)
let create_dummy =
  let dummy_set = WeakDummySet.create 17 in
  fun num ->
    let new_dummy = wrapper_create_dummy num in
    try WeakDummySet.find dummy_set new_dummy with
    | Not_found ->
      WeakDummySet.add dummy_set new_dummy;
      new_dummy

(* Create a list of cnt random dummy instances *)
let rec create_dummies cnt accu =
  if cnt > 0 then
    let x = create_dummy (Random.int 100) in
    create_dummies (cnt - 1) (x::accu)
  else
    accu

(* Create a list of cnt random dummy instances by selecting only
   one (again, random) element per iteration from a huge list of
   dummies. This is intended to create many unreachable instances
   of dummies that eventually become subject to a garbage collection *)
let rec iterate cnt accu =
  if cnt > 0 then
    let dummies = create_dummies 1024 [] in
    let elem = List.nth dummies (Random.int 1025) in
    iterate (cnt - 1) (elem::accu)
  else
    accu

let () =
  let result = iterate 1000 [] in
  Printf.printf "%d dummies created.\n" (List.length result)
