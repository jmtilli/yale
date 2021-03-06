%code requires {
#ifndef YY_TYPEDEF_YY_SCANNER_T
#define YY_TYPEDEF_YY_SCANNER_T
typedef void *yyscan_t;
#endif
#include "yale.h"
#include <sys/types.h>
}

%define api.prefix {yaleyy}

%{

#include "yale.h"
#include "yyutils.h"
#include "yale.tab.h"
#include "yale.lex.h"
#include <arpa/inet.h>

void yaleyyerror(YYLTYPE *yylloc, yyscan_t scanner, struct yale *yale, const char *str)
{
        fprintf(stderr, "error: %s at line %d col %d\n",str, yylloc->first_line, yylloc->first_column);
}

int yaleyywrap(yyscan_t scanner)
{
        return 1;
}

%}

%pure-parser
%lex-param {yyscan_t scanner}
%parse-param {yyscan_t scanner}
%parse-param {struct yale *yale}
%locations

%union {
  int i;
  char *s;
  struct escaped_string str;
  struct {
    int i;
    char *s;
  } both;
  struct {
    uint8_t has_i:1;
    uint8_t has_prio:1;
    int prio;
  } tokenoptstmp;
  struct {
    uint8_t i:1;
    int prio;
  } tokenopts;
}

%destructor { free ($$.str); } STRING_LITERAL
%destructor { free ($$); } FREEFORM_TOKEN
%destructor { free ($$); } PERCENTC_LITERAL

%token PERCENTC_LITERAL
%token STATEINCLUDE
%token PARSERINCLUDE
%token INITINCLUDE
%token HDRINCLUDE
%token BYTESSIZETYPE

%token TOKEN
%token ACTION
%token PRIO
%token DIRECTIVE
%token NOFASTPATH
%token SHORTCUTTING
%token MAIN

%token BYTES

%token PARSERNAME
%token EQUALS
%token SEMICOLON
%token COMMA
%token STRING_LITERAL
%token INT_LITERAL
%token FREEFORM_TOKEN
%token LT
%token GT
%token PIPE
%token MINUS
%token CB
%token COND
%token I


%token ERROR_TOK

%type<i> INT_LITERAL
%type<tokenopts> token_opts
%type<tokenoptstmp> token_optlist
%type<tokenoptstmp> token_opt
%type<i> maybe_minus
%type<i> token_ltgtexp
%type<i> cond_ltgtexp
%type<i> maybe_token_ltgt
%type<i> maybe_cond_ltgt
%type<i> bytes_ltgtexp
%type<i> maybe_bytes_ltgt
%type<str> STRING_LITERAL
%type<s> FREEFORM_TOKEN
%type<s> PERCENTC_LITERAL

%%

yalerules:
| yalerules yalerule
;

yalerule:
  TOKEN token_opts FREEFORM_TOKEN EQUALS STRING_LITERAL SEMICOLON
{
  struct token *tk;
  yale_uint_t i;
  if (yale->tokencnt >= sizeof(yale->tokens)/sizeof(*yale->tokens))
  {
    printf("1\n");
    YYABORT;
  }
  for (i = 0; i < yale->nscnt; i++)
  {
    if (strcmp(yale->ns[i].name, $3) == 0)
    {
      yale->ns[i].is_token = 1;
      if (yale->ns[i].is_lhs)
      {
        printf("1.1\n");
        YYABORT;
      } 
      free($3);
      break;
    }
  }
  if (i == yale->nscnt)
  {
    if (i >= YALE_UINT_MAX_LEGAL - 1)
    {
      printf("1.2\n");
      YYABORT;
    }
    yale->ns[i].name = $3;
    yale->ns[i].is_token = 1;
    yale->nscnt++;
  }
  tk = &yale->tokens[yale->tokencnt++];
  tk->priority = $2.prio;
  tk->i = $2.i;
  tk->nsitem = i;
  tk->re = $5;
}
| FREEFORM_TOKEN maybe_cond_ltgt EQUALS
{
  struct rule *rule;
  yale_uint_t i;
  if (yale->rulecnt >= sizeof(yale->rules)/sizeof(*yale->rules))
  {
    printf("3\n");
    YYABORT;
  }
  rule = &yale->rules[yale->rulecnt++];

  for (i = 0; i < yale->nscnt; i++)
  {
    if (strcmp(yale->ns[i].name, $1) == 0)
    {
      yale->ns[i].is_lhs = 1;
      if (yale->ns[i].is_token)
      {
        printf("3.1 is_token %s %d\n", $1, i);
        YYABORT;
      } 
      free($1);
      break;
    }
  }
  if (i == yale->nscnt)
  {
    if (i >= YALE_UINT_MAX_LEGAL - 1)
    {
      printf("3.2\n");
      YYABORT;
    }
    yale->ns[i].name = $1;
    yale->ns[i].is_lhs = 1;
    yale->nscnt++;
  }
  rule->cond = $2;
  rule->lhs = i;
}
elements SEMICOLON
| STATEINCLUDE PERCENTC_LITERAL SEMICOLON
{
  csaddstr(&yale->si, $2);
  free($2);
}
| INITINCLUDE PERCENTC_LITERAL SEMICOLON
{
  csaddstr(&yale->ii, $2);
  free($2);
}
| PARSERINCLUDE FREEFORM_TOKEN FREEFORM_TOKEN SEMICOLON
{
  csaddstr(&yale->hs, "\n#include \"");
  csaddstr(&yale->hs, $2);
  csaddstr(&yale->hs, "cparser.h\"\n");
  csaddstr(&yale->si, "\nstruct ");
  csaddstr(&yale->si, $2);
  csaddstr(&yale->si, "_parserctx ");
  csaddstr(&yale->si, $3);
  csaddstr(&yale->si, ";\n");
  csaddstr(&yale->ii, "\n");
  csaddstr(&yale->ii, $2);
  csaddstr(&yale->ii, "_parserctx_init(&pctx->");
  csaddstr(&yale->ii, $3);
  csaddstr(&yale->ii, ");\n");
  free($2);
  free($3);
}
| DIRECTIVE directive_continued
| HDRINCLUDE PERCENTC_LITERAL SEMICOLON
{
  csaddstr(&yale->hs, $2);
  free($2);
};
| PERCENTC_LITERAL
{
  csaddstr(&yale->cs, $1);
  free($1);
};
;

token_opts:
{
  $$.prio = 0;
  $$.i = 0;
}
| LT token_optlist GT
{
  $$.prio = $2.prio;
  $$.i = $2.has_i;
}
;

token_optlist:
token_opt
{
  $$.has_i = 0;
  $$.has_prio = 0;
  $$.prio = 0;
  if ($1.has_i)
  {
    $$.has_i = 1;
  }
  if ($1.has_prio)
  {
    $$.has_prio = 1;
    $$.prio = $1.prio;
  }
}
| token_optlist COMMA token_opt
{
  $$ = $1;
  if ($3.has_i)
  {
    $$.has_i = 1;
  }
  if ($3.has_prio)
  {
    $$.has_prio = 1;
    $$.prio = $3.prio;
  }
}
;

token_opt:
  PRIO EQUALS maybe_minus INT_LITERAL
{
  $$.prio = $3 * $4;
  $$.has_prio = 1;
  $$.has_i = 0;
}
| I
{
  $$.has_prio = 0;
  $$.has_i = 1;
}
;

maybe_minus:
{
  $$ = +1;
}
| MINUS
{
  $$ = -1;
}
;


directive_continued:
MAIN EQUALS FREEFORM_TOKEN SEMICOLON
{
  yale_uint_t i;
  for (i = 0; i < yale->nscnt; i++)
  {
    if (strcmp(yale->ns[i].name, $3) == 0)
    {
      yale->ns[i].is_lhs = 1;
      if (yale->ns[i].is_token)
      {
        printf("M.1\n");
        YYABORT;
      } 
      free($3);
      break;
    }
  }
  if (i == yale->nscnt)
  {
    if (i >= YALE_UINT_MAX_LEGAL - 1)
    {
      printf("M.2\n");
      YYABORT;
    }
    yale->ns[i].is_lhs = 1;
    yale->ns[i].name = $3;
    yale->nscnt++;
  }
  yale->startns = i;
  yale->startns_present = 1;
}
| PARSERNAME EQUALS FREEFORM_TOKEN SEMICOLON
{
  yale->parsername = $3;
}
| BYTESSIZETYPE EQUALS FREEFORM_TOKEN SEMICOLON
{
  yale->bytessizetype = $3;
}
| NOFASTPATH SEMICOLON
{
  yale->nofastpath = 1;
}
| SHORTCUTTING SEMICOLON
{
  yale->shortcutting = 1;
}
;

elements:
alternation
;

alternation:
| concatenation
| alternation PIPE
{
  struct rule *rule;
  if (yale->rulecnt >= sizeof(yale->rules)/sizeof(*yale->rules))
  {
    printf("6\n");
    YYABORT;
  }
  rule = &yale->rules[yale->rulecnt];
  rule->lhs = yale->rules[yale->rulecnt-1].lhs;
  rule->cond = yale->rules[yale->rulecnt-1].cond;
  yale->rulecnt++;
}
concatenation
;

concatenation:
element
maybe_concatenationlist
;

maybe_concatenationlist:
| maybe_concatenationlist element
;

element:
ACTION maybe_token_ltgt
{
  struct rule *rule;
  struct ruleitem *it;
  rule = &yale->rules[yale->rulecnt - 1];
  if (rule->itemcnt == YALE_UINT_MAX_LEGAL)
  {
    printf("7\n");
    YYABORT;
  }
  it = &rule->rhs[rule->itemcnt++];
  it->is_action = 1;
  it->is_bytes = 0;
  it->value = YALE_UINT_MAX_LEGAL;
  it->cb = $2;
}
| FREEFORM_TOKEN maybe_token_ltgt
{
  struct rule *rule;
  struct ruleitem *it;
  struct ruleitem *it2;
  yale_uint_t i;
  rule = &yale->rules[yale->rulecnt - 1];
  if (rule->itemcnt == YALE_UINT_MAX_LEGAL || rule->noactcnt == YALE_UINT_MAX_LEGAL)
  {
    printf("7\n");
    YYABORT;
  }
  for (i = 0; i < yale->nscnt; i++) // FIXME check all cnt uses
  {
    if (strcmp(yale->ns[i].name, $1) == 0)
    {
      break;
    }
  }
  it = &rule->rhs[rule->itemcnt++];
  if (i != yale->nscnt)
  {
    it->value = i;
    it->cb = $2;
    if ($2 != YALE_UINT_MAX_LEGAL && yale->ns[i].is_lhs)
    {
      printf("7.1\n");
      YYABORT;
    }
  }
  else
  {
    if (i >= YALE_UINT_MAX_LEGAL - 1)
    {
      printf("7.2\n");
      YYABORT;
    }
    yale->ns[i].name = strdup($1);
    it->is_action = 0;
    it->is_bytes = 0;
    it->value = i;
    it->cb = $2;
    yale->nscnt++;
  }
  it2 = &rule->rhsnoact[rule->noactcnt++];
  *it2 = *it;
  free($1);
}
| BYTES maybe_bytes_ltgt
{
  struct rule *rule;
  struct ruleitem *it;
  struct ruleitem *it2;

  rule = &yale->rules[yale->rulecnt - 1];
  if (rule->itemcnt == YALE_UINT_MAX_LEGAL)
  {
    printf("7\n");
    YYABORT;
  }
  it = &rule->rhs[rule->itemcnt++];
  it->is_action = 0;
  it->is_bytes = 1;
  it->value = YALE_UINT_MAX_LEGAL-1;
  it->cb = $2;

  it2 = &rule->rhsnoact[rule->noactcnt++];
  *it2 = *it;
}
;

maybe_cond_ltgt:
{
  $$ = YALE_UINT_MAX_LEGAL;
}
| LT cond_ltgtexp GT
{
  $$ = $2;
}
;

cond_ltgtexp:
COND EQUALS PERCENTC_LITERAL
{
  int i;
  i = yale->condcnt++;
  yale->conds[i] = $3;
  $$ = i;
}
;

maybe_token_ltgt:
{
  $$ = YALE_UINT_MAX_LEGAL;
}
| LT token_ltgtexp GT
{
  $$ = $2;
}
;

token_ltgtexp:
CB EQUALS FREEFORM_TOKEN
{
  yale_uint_t i;
  for (i = 0; i < yale->cbcnt; i++)
  {
    if (strcmp(yale->cbs[i].name, $3) == 0)
    {
      free($3);
      break;
    }
  }
  if (i == yale->cbcnt)
  {
    if (i == YALE_UINT_MAX_LEGAL)
    {
      printf("9\n");
      YYABORT;
    }
    yale->cbs[i].name = $3;
    yale->cbcnt++;
  }
  $$ = i;
}
;

maybe_bytes_ltgt:
{
  $$ = YALE_UINT_MAX_LEGAL;
}
| LT bytes_ltgtexp GT
{
  $$ = $2;
}
;

bytes_ltgtexp:
  CB EQUALS FREEFORM_TOKEN
{
  yale_uint_t i;
  for (i = 0; i < yale->cbcnt; i++)
  {
    if (strcmp(yale->cbs[i].name, $3) == 0)
    {
      free($3);
      break;
    }
  }
  if (i == yale->cbcnt)
  {
    if (i == YALE_UINT_MAX_LEGAL)
    {
      printf("9\n");
      YYABORT;
    }
    yale->cbs[i].name = $3;
    yale->cbcnt++;
  }
  $$ = i;
}
;
