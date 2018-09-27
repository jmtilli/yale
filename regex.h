#ifndef _REGEX_H_
#define _REGEX_H_

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/uio.h>
#include <ctype.h>
#include "bitset.h"

struct re;

struct wildcard {
};

struct emptystr {
};

struct literals {
  struct bitset bitmask;
};

struct concat {
  struct re *re1;
  struct re *re2;
};

struct altern {
  struct re *re1;
  struct re *re2;
};

struct alternmulti {
  struct re **res;
  uint8_t *pick_those;
  size_t resz;
};

struct star {
  struct re *re;
};

struct re_parse_result {
  size_t branchsz;
};

enum re_type {
  WILDCARD,
  EMPTYSTR,
  LITERALS,
  CONCAT,
  ALTERN,
  ALTERNMULTI,
  STAR
};

struct re {
  enum re_type type;
  union {
    struct wildcard wc;
    struct emptystr e;
    struct literals lit;
    struct concat cat;
    struct altern alt;
    struct alternmulti altmulti;
    struct star star;
  } u;
};

struct re *dup_re(struct re *re);

struct dfa_node {
  uint8_t d[256];
  uint8_t default_tr;
  uint8_t acceptid;
  uint8_t tainted:1;
  uint8_t accepting:1;
  uint8_t final:1;
  struct bitset acceptidset;
  uint64_t algo_tmp;
  size_t transitions_id;
};

struct nfa_node {
  struct bitset d[256];
  struct bitset defaults;
  struct bitset epsilon;
  uint8_t accepting:1;
  uint8_t taintid;
};

void nfa_init(struct nfa_node *n, int accepting, int taintid);

void nfa_connect(struct nfa_node *n, char ch, uint8_t node2);

void nfa_connect_epsilon(struct nfa_node *n, uint8_t node2);

void nfa_connect_default(struct nfa_node *n, uint8_t node2);

void epsilonclosure(struct nfa_node *ns, struct bitset nodes,
                    struct bitset *closurep, int *tainted,
                    struct bitset *acceptidsetp);

void dfa_init(struct dfa_node *n, int accepting, int tainted, struct bitset *acceptidset);

void dfa_init_empty(struct dfa_node *n);

void dfa_connect(struct dfa_node *n, char ch, uint8_t node2);

void dfa_connect_default(struct dfa_node *n, uint8_t node2);

void
check_recurse_acceptid_is(struct dfa_node *ds, uint8_t state, uint8_t acceptid);

void
check_recurse_acceptid_is_not(struct dfa_node *ds, uint8_t state, uint8_t acceptid);

void check_cb_first(struct dfa_node *ds, uint8_t acceptid, uint8_t state);

void check_cb(struct dfa_node *ds, uint8_t state, uint8_t acceptid);

struct bitset_hash_item {
  struct bitset key;
  uint8_t dfanodeid;
};

struct bitset_hash {
  struct bitset_hash_item tbl[255];
  uint8_t tblsz;
};

// FIXME this algorithm requires thorough review
ssize_t state_backtrack(struct dfa_node *ds, uint8_t state, size_t bound);

void __attribute__((noinline)) set_accepting(struct dfa_node *ds, uint8_t state, int *priorities);

ssize_t maximal_backtrack(struct dfa_node *ds, uint8_t state, size_t bound);

void dfaviz(struct dfa_node *ds, uint8_t cnt);

void nfaviz(struct nfa_node *ns, uint8_t cnt);

uint8_t nfa2dfa(struct nfa_node *ns, struct dfa_node *ds, uint8_t begin);

struct re *parse_re(const char *re, size_t resz, size_t *remainderstart);

struct re *
parse_bracketexpr(const char *re, size_t resz, size_t *remainderstart);

struct re *parse_atom(const char *re, size_t resz, size_t *remainderstart);

struct re *parse_piece(const char *re, size_t resz, size_t *remainderstart);

// branch: piece branch
struct re *parse_branch(const char *re, size_t resz, size_t *remainderstart);

// RE: branch | RE
struct re *parse_re(const char *re, size_t resz, size_t *remainderstart);

struct re *parse_res(struct iovec *regexps, uint8_t *pick_those, size_t resz);

void gennfa(struct re *regexp,
            struct nfa_node *ns, uint8_t *ncnt,
            uint8_t begin, uint8_t end,
            uint8_t taintid);

void gennfa_main(struct re *regexp,
                 struct nfa_node *ns, uint8_t *ncnt,
                 uint8_t taintid);

void gennfa_alternmulti(struct re *regexp,
                        struct nfa_node *ns, uint8_t *ncnt);

struct pick_those_struct {
  uint8_t *pick_those;
  size_t len;
  struct dfa_node *ds;
  size_t dscnt;
};

struct transitionbuf {
  uint8_t transitions[256];
};

#define MAX_TRANS 65536 // 256 automatons, 256 states per automaton

struct transitionbufs {
  struct transitionbuf all[MAX_TRANS];
  size_t cnt;
};

size_t
get_transid(const uint8_t *transitions, struct transitionbufs *bufs);

void
perf_trans(uint8_t *transitions, struct transitionbufs *bufs);

void
pick(struct nfa_node *nsglobal, struct dfa_node *dsglobal,
     struct iovec *res, struct pick_those_struct *pick_those, int *priorities);

void
collect(struct pick_those_struct *pick_thoses, size_t cnt,
        struct transitionbufs *bufs);

void dump_headers(FILE *f, const char *parsername, size_t max_bt);

void
dump_collected(FILE *f, const char *parsername, struct transitionbufs *bufs);

void
dump_one(FILE *f, const char *parsername, struct pick_those_struct *pick_those);

void
dump_chead(FILE *f, const char *parsername);

#endif