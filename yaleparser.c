#include "yale.h"
#include "yyutils.h"
#include <stdio.h>
#include "parser.h"

struct ParserGen gen;

int main(int argc, char **argv)
{
  FILE *f;
  struct yale yale = {};
  size_t i, iter;
  char cnamebuf[1024] = {0};
  char hnamebuf[1024] = {0};
  char hdefbuf[1024] = {0};
  size_t len;
  int c;
  int h;
  size_t iters = 1;

  if (argc != 3)
  {
    fprintf(stderr, "Usage: %s file.txt [c|h|b|p]\n", argv[0]);
    exit(1);
  }
  if (strcmp(argv[2], "c"))
  {
    c = 1;
  }
  else if (strcmp(argv[2], "h"))
  {
    h = 1;
  }
  else if (strcmp(argv[2], "b"))
  {
    c = 1;
    h = 1;
  }
  else if (strcmp(argv[2], "p"))
  {
    iters = 1000;
    c = 1;
    h = 1;
  }
  else
  {
    fprintf(stderr, "Usage: %s file.txt [c|h|b|p]\n", argv[0]);
    exit(1);
  }

  f = fopen(argv[1], "r");
  if (f == NULL)
  {
    fprintf(stderr, "Can't open input file\n");
    exit(1);
  }
  yaleyydoparse(f, &yale);
  fclose(f);
  if (check_actions(&yale) != 0)
  {
    printf("Fail action\n");
    exit(1);
  }

  snprintf(cnamebuf, sizeof(cnamebuf), "%s%s", yale.parsername, "cparser.c");
  snprintf(hnamebuf, sizeof(hnamebuf), "%s%s", yale.parsername, "cparser.h");
  snprintf(hdefbuf, sizeof(hnamebuf), "_%sCPARSER_H_", yale.parsername);
  len = strlen(hdefbuf);
  for (i = 0; i < len; i++)
  {
    hdefbuf[i] = toupper((unsigned char)hdefbuf[i]);
  }

  for (iter = 0; iter < iters; iter++)
  {
    parsergen_init(&gen, yale.parsername);
    for (i = 0; i < yale.tokencnt; i++)
    {
      yale.ns[yale.tokens[i].nsitem].val =
        parsergen_add_token(&gen, yale.tokens[i].re, strlen(yale.tokens[i].re), yale.tokens[i].priority); // FIXME '\0'
    }
    parsergen_finalize_tokens(&gen);
    for (i = 0; i < yale.nscnt; i++)
    {
      struct namespaceitem *nsit = &yale.ns[i];
      if (nsit->is_token)
      {
        if (nsit->is_lhs)
        {
          fprintf(stderr, "Error\n");
          exit(1);
        }
        continue;
      }
      if (!nsit->is_lhs)
      {
        fprintf(stderr, "Error\n");
        exit(1);
      }
      nsit->val = parsergen_add_nonterminal(&gen);
    }
    parsergen_state_include(&gen, yale.si.data);
    if (!yale.startns_present)
    {
      abort();
    }
    parsergen_set_start_state(&gen, yale.ns[yale.startns].val);
    parsergen_set_cb(&gen, yale.cbs, yale.cbcnt);
    parsergen_set_rules(&gen, yale.rules, yale.rulecnt, yale.ns);
    gen_parser(&gen);
    if (h)
    {
      f = fopen(hnamebuf, "w");
      fprintf(f, "#ifndef %s\n", hdefbuf);
      fprintf(f, "#define %s\n", hdefbuf);
      parsergen_dump_headers(&gen, f);
      fprintf(f, "#endif\n");
      fclose(f);
    }
    if (c)
    {
      f = fopen(cnamebuf, "w");
      if (yale.cs.data)
      {
        fprintf(f, "%s", yale.cs.data);
      }
      fprintf(f, "#include \"%s\"\n", hnamebuf);
      parsergen_dump_parser(&gen, f);
      fclose(f);
    }
  }
  yale_free(&yale);
  return 0;
}