%{
    #include <stdio.h>
    #include <string.h>
	  #include <stdlib.h>
	  #include <string.h>
    
	  #include "jqpath.tab.h"

    #include "jqpath.h"
    #include "hash.h"
    #include "string_funcs.h"

    extern int yylex();
    extern int intval;
    extern int last_int_len;
    extern float floatval;
    extern char * strval;
    extern struct jqpath path;
    int yyerror(char * er);
    void add_index_to_path(int idx,int len);
%}

%token  STRING;
%token  DOT;
%token  EQUALS;
%token  NOT_EQUALS;
%token  OPEN_ARR;
%token  CLOSE_ARR;
%token  FLOAT;
%token  INT;
%token  LABEL;

%%

/* bison needs the top most rule to come first */
path: 
    complete_path expression {/* printf("parse-PATH EXPRESSION\n"); */}
  | complete_path            {}
;

complete_path: 
  DOT LABEL                             { /* add_string_to_path(strval,strlen(strval));*/  }
| DOT LABEL array_def                   {                                         }   
| DOT LABEL array_def complete_path     { /* printf("parse-ARR\n"); */ }
| DOT LABEL complete_path               { /* printf("complex complete path\n"); */}
;

array_def: 
  OPEN_ARR CLOSE_ARR      {path.hash = merge_hash(path.hash,hash("[]",2));}
| OPEN_ARR INT CLOSE_ARR  {add_index_to_path(intval,last_int_len);}

expression: 
  EQUALS INT        { path.value.type = JQ_INT_VAL; path.value.value.int_val = intval; path.rendered = true; }
| EQUALS FLOAT      { path.value.type = JQ_FLOAT_VAL; path.value.value.float_val = floatval; path.rendered = true;}
| EQUALS STRING     { path.value.type = JQ_STRING_VAL; path.value.value.string_val = copy_string(strval); path.rendered = true;}
| NOT_EQUALS INT    { path.value.type = JQ_INT_VAL; path.value.value.int_val = intval; path.rendered = true;}
| NOT_EQUALS FLOAT  { path.value.type = JQ_FLOAT_VAL; path.value.value.float_val = floatval; path.rendered = true;}
| NOT_EQUALS STRING { path.value.type = JQ_STRING_VAL; path.value.value.string_val = copy_string(strval); path.rendered = true;}
;

%% 

int yyerror(char * er) {
    printf("Error: %s\n", er);
    return -1;
}

void add_index_to_path(int idx, int idx_len) {
    char * val = malloc(idx_len + 3);
    sprintf(val,"[%i]",idx);
    path.hash = merge_hash(path.hash,hash(val,strlen(val)));
    free(val);
}

static inline struct jqpath * replicate_path() {
    struct jqpath * p = malloc(sizeof(struct jqpath));
    if(p == NULL) {
        printf("Memeory allocation failure\n");
        abort();
    }

    p->hash = path.hash;
    p->op = path.op;
    p->depth = path.depth;
    p->value.type = path.value.type;
    p->rendered = false;

    // There is no operator '.name' rather than '.name = value'
    if(p->op == JQ_NOT_SET) {
      return p;
    }

    switch(p->value.type) {
      case JQ_STRING_VAL:
        p->value.value.string_val = path.value.value.string_val;
        path.value.value.string_val = NULL;
        p->rendered = true;
        break;

      case JQ_FLOAT_VAL: 
        p->value.value.float_val = path.value.value.float_val;
        p->rendered = true;
        break;

      case JQ_INT_VAL:
        p->value.value.int_val = path.value.value.int_val;
        p->rendered = true;
        break;

      default:
        printf("Error: Corrupted value\n");
        abort();
    }

    return p;
}

/* Declarations */
void set_input_string(const char* in);
void end_lexical_scan(void);

/* This function parses a string using yyparse. 
   Note that this is not reentrant */
struct jqpath * jqpath_parse_string(const char* in) {
  set_input_string(in);
  int rv = yyparse();
  end_lexical_scan();
  if(rv != 0) {
      return NULL;
  }

  return replicate_path();
}

void jqpath_close_path(struct jqpath * path) {
  if(path->value.type == JQ_STRING_VAL) {
      kill_string(path->value.value.string_val);
  }

  free(path);
}
