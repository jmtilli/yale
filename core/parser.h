#ifndef _PARSER_H_
#define _PARSER_H_

#include "yale.h"
#include "regex.h"
#include <sys/uio.h>

struct dict {
  struct bitset bitset[YALE_UINT_MAX_LEGAL + 1];
  struct bitset has;
};

struct REGenEntry {
  struct bitset key;
  struct dfa_node *dfas;
  yale_uint_t dfacnt;
};

struct REGen {
  struct REGenEntry entries[YALE_UINT_MAX_LEGAL];
};

struct LookupTblEntry {
  struct yale_hash_list_node node;
  yale_uint_t nonterminal;
  yale_uint_t terminal;
  yale_uint_t cond;
  //char *condition;
  yale_uint_t val;
  uint8_t conflict:1;
  struct bitset cbs;
};

struct firstset_value {
  uint8_t is_bytes:1;
  yale_uint_t token;
  yale_uint_t cbsz;
  yale_uint_t *cbs;
};

struct firstset_values {
  struct firstset_value *values;
  yale_uint_t valuessz;
};

struct firstset_entry2 {
  struct yale_hash_list_node node;
  struct ruleitem *rhs;
  yale_uint_t rhssz;
  struct firstset_values values;
};

struct stackconfigitem {
  yale_uint_t val;
  yale_uint_t cb;
};

struct stackconfig {
  struct yale_hash_list_node node;
  struct stackconfigitem *stack;
  yale_uint_t sz;
  yale_uint_t cbsz;
  size_t i;
};

struct nonterminal_cond {
  yale_uint_t cond;
  yale_uint_t statetblidx;
  yale_uint_t pick_those;
};

struct nonterminal_conds {
  struct nonterminal_cond conds[YALE_UINT_MAX_LEGAL];
  yale_uint_t condcnt;
  uint8_t is_shortcut:1;
  yale_uint_t shortcut_rule;
};

struct ParserGen {
  yale_uint_t tokencnt;
  yale_uint_t nonterminalcnt;
  char *parsername;
  yale_uint_t start_state;
  yale_uint_t epsilon;
  char *state_include_str;
  char *init_include_str;
  char *bytes_size_type;
  //size_t Ficnt;
  uint8_t nofastpath;
  uint8_t shortcutting;
  yale_uint_t pick_thoses_cnt;
  yale_uint_t max_stack_size;
  yale_uint_t max_cb_stack_size;
  yale_uint_t max_bt;
  size_t stackconfigcnt;
  char *userareaptr;
  int tokens_finalized;
  yale_uint_t rulecnt;
  yale_uint_t cbcnt;
  yale_uint_t condcnt;
  size_t Tcnt;
  struct numbers_sets numbershash;
  struct yale_hash_table Fi2_hash;
  struct yale_hash_table stackconfigs_hash;
  struct firstset_values Fo2[YALE_UINT_MAX_LEGAL + 1];
  struct REGen re_gen;
  //yale_uint_t pick_thoses_id_by_nonterminal_cond[YALE_UINT_MAX_LEGAL][YALE_UINT_MAX_LEGAL];
  struct pick_those_struct pick_thoses[YALE_UINT_MAX_LEGAL];
  char *conds[YALE_UINT_MAX_LEGAL];
  struct nonterminal_conds nonterminal_conds[YALE_UINT_MAX_LEGAL];
  struct iovec re_by_idx[YALE_UINT_MAX_LEGAL];
  int priorities[YALE_UINT_MAX_LEGAL];
  int caseis[YALE_UINT_MAX_LEGAL];
  struct rule rules[YALE_UINT_MAX_LEGAL]; // 382 kB
  struct cb cbs[YALE_UINT_MAX_LEGAL];
  struct nfa_node ns[YALE_UINT_MAX_LEGAL]; // 2 MB
  struct dfa_node ds[YALE_UINT_MAX_LEGAL];
  struct yale_hash_table Thash;
  //struct LookupTblEntry T[YALE_UINT_MAX_LEGAL][YALE_UINT_MAX_LEGAL];
    // val==YALE_UINT_MAX_LEGAL: invalid
    // cb==YALE_UINT_MAX_LEGAL: no callback
  yale_uint_t pick_those[YALE_UINT_MAX_LEGAL][YALE_UINT_MAX_LEGAL]; // 64 kB
  struct LookupTblEntry Tentries[32768];
  //struct firstset_entry *Fi[8192]; // 64 kB
  //struct stackconfig stackconfigs[32768]; // 1.25 MB
  struct stackconfig *stackconfigs[32768]; // 0.25 MB
  struct transitionbufs bufs; // 16 MB, this could be made to use dynamic alloc
  char userarea[64*1024*1024];
};

void parsergen_init(struct ParserGen *gen, char *parsername);

void parsergen_free(struct ParserGen *gen);

void gen_parser(struct ParserGen *gen);

void parsergen_state_include(struct ParserGen *gen, char *stateinclude);

void parsergen_init_include(struct ParserGen *gen, char *initinclude);

void parsergen_nofastpath(struct ParserGen *gen);

void parsergen_shortcutting(struct ParserGen *gen);

void parsergen_set_bytessizetype(struct ParserGen *gen, char *type);

void parsergen_set_start_state(struct ParserGen *gen, yale_uint_t start_state);

yale_uint_t parsergen_add_token(struct ParserGen *gen, char *re, size_t resz, int prio, int casei);

void parsergen_finalize_tokens(struct ParserGen *gen);

yale_uint_t parsergen_add_nonterminal(struct ParserGen *gen);

void parsergen_set_rules(struct ParserGen *gen, const struct rule *rules, yale_uint_t rulecnt, const struct namespaceitem *ns);

void parsergen_set_conds(struct ParserGen *gen, char **conds, yale_uint_t condcnt);

void parsergen_set_cb(struct ParserGen *gen, const struct cb *cbs, yale_uint_t cbcnt);

ssize_t max_stack_sz(struct ParserGen *gen, size_t *maxcbszptr);

void parsergen_dump_headers(struct ParserGen *gen, FILE *f);

void parsergen_dump_parser(struct ParserGen *gen, FILE *f);

void firstset2_update(struct ParserGen *gen, struct firstset_values *val2, const struct firstset_values *val1, int noepsilon, int *changed);

void firstset_values_deep_free(struct firstset_values *orig);

#endif
