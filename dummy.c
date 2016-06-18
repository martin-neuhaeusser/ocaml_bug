#include <stdio.h>
#include <assert.h>
#include <caml/memory.h>
#include <caml/custom.h>

const unsigned c_data_id = 123;
const unsigned c_data_invalid_id = 321;

/* A struct to hold data that is managed within the C-bindings,
   outside of the OCaml heap */
typedef struct {
  unsigned unique_id;       /* just for debugging purposes */
  int dummy_data;
} c_data;

/* The pointer to these structures. We store those pointers within
   a custom block */
typedef c_data* c_data_ptr;

void check_c_data(c_data_ptr p) {
  if (p->unique_id != c_data_id) {
    if (p->unique_id == c_data_invalid_id)
      printf("Received value with custom block that has already been finalized before: invalid unique_id = %d\n", p->unique_id);
    else
      printf("Trying to access custom block with invalid unique_id = %d\n", p->unique_id);
    assert (p->unique_id == c_data_id);
  }
  return;
}

void finalize_c_data(value v) {
  /* Get pointer out of custom block and check that its unique id is correct
     and that the custom block has not been finalized before (that would have
     changed its unique_id, see below */
  c_data_ptr p = *((c_data_ptr*)Data_custom_val(v));
  check_c_data(p);

  /* For debugging purposes, we do _not_ free p here (which intentionally leads to a memory leak),
     but keep it allocated and only modify the unique_id to indicate that a finalizer
     has been called for that block.
     In this way, we reliably detect illegal uses of already finalized custom blocks without running into a segfault. */
  p->unique_id = c_data_invalid_id;
  return;
}

int compare_c_data(value v1, value v2) {
  c_data_ptr p1 = *((c_data_ptr*)Data_custom_val(v1));
  c_data_ptr p2 = *((c_data_ptr*)Data_custom_val(v2));
  check_c_data(p1);
  check_c_data(p2);

  if (p1->dummy_data == p2->dummy_data)
    return 0;
  else if (p1->dummy_data > p2->dummy_data)
    return 1;
  else
    return -1;
}

intnat hash_c_data(value v) {
  c_data_ptr p = *((c_data_ptr*)Data_custom_val(v));
  check_c_data(p);
  return ((intnat)(p->dummy_data));
}

static struct custom_operations c_data_ops = {
  .identifier = "martin-neuhaeusser-c_data_ops",
  .finalize = finalize_c_data,
  .compare = compare_c_data,
  .hash = hash_c_data,
  .serialize = custom_serialize_default,
  .deserialize = custom_deserialize_default
};

/* Allocate memory to store a c_data structure outside the OCaml heap
   and store the pointer to the data into a fresh custom block. */
CAMLprim value wrapper_create_dummy(value some_value) {
  CAMLparam1(some_value);
  CAMLlocal1(v_new);
  c_data_ptr x;
  int param = Int_val(some_value);

  /* allocate and initialize a structure outside the OCaml heap */
  x = malloc(sizeof(c_data));
  assert (x != NULL);
  x->unique_id = c_data_id;
  x->dummy_data = param;

  /* allocate a custom block to hold the pointer to the C-data */
  v_new = caml_alloc_custom(&c_data_ops, sizeof(c_data_ptr), 0, 1);
  *((c_data_ptr*)Data_custom_val(v_new)) = x;

  CAMLreturn(v_new);
}
