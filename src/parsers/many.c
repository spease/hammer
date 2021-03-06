#include "parser_internal.h"

// TODO: split this up.
typedef struct {
  const HParser *p, *sep;
  size_t count;
  bool min_p;
} HRepeat;

static HParseResult *parse_many(void* env, HParseState *state) {
  HRepeat *env_ = (HRepeat*) env;
  HCountedArray *seq = h_carray_new_sized(state->arena, (env_->count > 0 ? env_->count : 4));
  size_t count = 0;
  HInputStream bak;
  while (env_->min_p || env_->count > count) {
    bak = state->input_stream;
    if (count > 0) {
      HParseResult *sep = h_do_parse(env_->sep, state);
      if (!sep)
	goto err0;
    }
    HParseResult *elem = h_do_parse(env_->p, state);
    if (!elem)
      goto err0;
    if (elem->ast)
      h_carray_append(seq, (void*)elem->ast);
    count++;
  }
  if (count < env_->count)
    goto err;
 succ:
  ; // necessary for the label to be here...
  HParsedToken *res = a_new(HParsedToken, 1);
  res->token_type = TT_SEQUENCE;
  res->seq = seq;
  return make_result(state, res);
 err0:
  if (count >= env_->count) {
    state->input_stream = bak;
    goto succ;
  }
 err:
  state->input_stream = bak;
  return NULL;
}

static const HParserVtable many_vt = {
  .parse = parse_many,
};

const HParser* h_many(const HParser* p) {
  return h_many__m(&system_allocator, p);
}
const HParser* h_many__m(HAllocator* mm__, const HParser* p) {
  HParser *res = h_new(HParser, 1);
  HRepeat *env = h_new(HRepeat, 1);
  env->p = p;
  env->sep = h_epsilon_p__m(mm__);
  env->count = 0;
  env->min_p = true;
  res->vtable = &many_vt;
  res->env = env;
  return res;
}

const HParser* h_many1(const HParser* p) {
  return h_many1__m(&system_allocator, p);
}
const HParser* h_many1__m(HAllocator* mm__, const HParser* p) {
  HParser *res = h_new(HParser, 1);
  HRepeat *env = h_new(HRepeat, 1);
  env->p = p;
  env->sep = h_epsilon_p__m(mm__);
  env->count = 1;
  env->min_p = true;
  res->vtable = &many_vt;
  res->env = env;
  return res;
}

const HParser* h_repeat_n(const HParser* p, const size_t n) {
  return h_repeat_n__m(&system_allocator, p, n);
}
const HParser* h_repeat_n__m(HAllocator* mm__, const HParser* p, const size_t n) {
  HParser *res = h_new(HParser, 1);
  HRepeat *env = h_new(HRepeat, 1);
  env->p = p;
  env->sep = h_epsilon_p__m(mm__);
  env->count = n;
  env->min_p = false;
  res->vtable = &many_vt;
  res->env = env;
  return res;
}

const HParser* h_sepBy(const HParser* p, const HParser* sep) {
  return h_sepBy__m(&system_allocator, p, sep);
}
const HParser* h_sepBy__m(HAllocator* mm__, const HParser* p, const HParser* sep) {
  HParser *res = h_new(HParser, 1);
  HRepeat *env = h_new(HRepeat, 1);
  env->p = p;
  env->sep = sep;
  env->count = 0;
  env->min_p = true;
  res->vtable = &many_vt;
  res->env = env;
  return res;
}

const HParser* h_sepBy1(const HParser* p, const HParser* sep) {
  return h_sepBy1__m(&system_allocator, p, sep);
}
const HParser* h_sepBy1__m(HAllocator* mm__, const HParser* p, const HParser* sep) {
  HParser *res = h_new(HParser, 1);
  HRepeat *env = h_new(HRepeat, 1);
  env->p = p;
  env->sep = sep;
  env->count = 1;
  env->min_p = true;
  res->vtable = &many_vt;
  res->env = env;
  return res;
}

typedef struct {
  const HParser *length;
  const HParser *value;
} HLenVal;

static HParseResult* parse_length_value(void *env, HParseState *state) {
  HLenVal *lv = (HLenVal*)env;
  HParseResult *len = h_do_parse(lv->length, state);
  if (!len)
    return NULL;
  if (len->ast->token_type != TT_UINT)
    errx(1, "Length parser must return an unsigned integer");
  // TODO: allocate this using public functions
  HRepeat repeat = {
    .p = lv->value,
    .sep = h_epsilon_p(),
    .count = len->ast->uint,
    .min_p = false
  };
  return parse_many(&repeat, state);
}

static const HParserVtable length_value_vt = {
  .parse = parse_length_value,
};

const HParser* h_length_value(const HParser* length, const HParser* value) {
  return h_length_value__m(&system_allocator, length, value);
}
const HParser* h_length_value__m(HAllocator* mm__, const HParser* length, const HParser* value) {
  HParser *res = h_new(HParser, 1);
  res->vtable = &length_value_vt;
  HLenVal *env = h_new(HLenVal, 1);
  env->length = length;
  env->value = value;
  res->env = (void*)env;
  return res;
}
