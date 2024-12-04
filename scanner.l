%{
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "parser.tab.h"

typedef struct Arena Arena;

static struct Arena {
  size_t total;
  size_t used;
  struct Arena *next;
  char mem[];
} *main_arena = NULL;


Arena *new_region(size_t size) {
   Arena *arena = (Arena*)calloc(1, size);
   arena->total = size - (sizeof(uintptr_t) * 3);
   arena->used = 0;
   arena->next = NULL;
   return arena;
}

Arena *push_region(size_t size) {
   Arena *new_arena = new_region(size);
   new_arena->next = main_arena;
   main_arena = new_arena;
   return new_arena;
}

void *allocate_space(size_t size) {
   if ((main_arena->total - main_arena->used) >= size)
      push_region();

   void *mem_space = (void*)main_arena->mem[main_arena->used + 1];
   main_arena->used += size + 1;

   return mem_space;
}

%}

ident [a-zA-Z_][a-zA-Z0-9_]*
ndigit [1-9]
zdigit [0-9]
opt_ws [ \t\n\r]*

%%

[ \t\n\r]+				;
^{opt_ws}{ident}/"=" { yylval.idval = strndup(yytext, yyleng);
			return ANCHORED_IDENTIFIER; 	}
"$"{ident}           { yylval.idval = strndup(yytext, yyleng);
			return DOLLAR_IDENTIFIER; 	}
{ndigit}{zdigit}*    { yylval.numval = strndup(yytext, yyleng);
			return INTEGER_NUMBER;     	}
[^ \t]+		     { yylval.wordval = strndup(yytext, yyleng);
			return WORD; 			}


%%

int yywrap(void) { return 1; }
