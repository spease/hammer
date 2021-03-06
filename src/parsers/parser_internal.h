#ifndef HAMMER_PARSE_INTERNAL__H
#define HAMMER_PARSE_INTERNAL__H
#include "../hammer.h"
#include "../internal.h"

#define a_new_(arena, typ, count) ((typ*)h_arena_malloc((arena), sizeof(typ)*(count)))
#define a_new(typ, count) a_new_(state->arena, typ, count)
// we can create a_new0 if necessary. It would allocate some memory and immediately zero it out.

static inline HParseResult* make_result(HParseState *state, HParsedToken *tok) {
  HParseResult *ret = a_new(HParseResult, 1);
  ret->ast = tok;
  ret->arena = state->arena;
  return ret;
}

// return token size in bits...
static inline size_t token_length(HParseResult *pr) {
  if (pr) {
    return pr->bit_length;
  } else {
    return 0;
  }
}


#endif // HAMMER_PARSE_INTERNAL__H
