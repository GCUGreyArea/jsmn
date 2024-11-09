%{
    #include <stdio.h>
    #include <string.h>
    #include "jqpath.h"
    #include "hash.h"
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

/*
    bison needs the top most rule to come first
*/
path: 
    complete_path expression {/* printf("parse-PATH EXPRESSION\n"); */}
  | complete_path            {}
;

complete_path: 
  DOT LABEL                             { /* printf("parse-DOT LABEL path\n"); */ }
| DOT LABEL array_def complete_path     { /* printf("parse-ARR\n"); */ }
| DOT LABEL complete_path               { /* printf("complex complete path\n"); */}
;

array_def: 
  OPEN_ARR CLOSE_ARR      { /*printf("empty array: lex_depth=%d\n",lex_depth); */}
| OPEN_ARR INT CLOSE_ARR  {add_index_to_path(intval,last_int_len);}

expression: 
  EQUALS INT        { }
| EQUALS FLOAT      { }
| EQUALS STRING     { }
| NOT_EQUALS INT    { }
| NOT_EQUALS FLOAT  { }
| NOT_EQUALS STRING { }
;


%% 

int yyerror(char * er) {
    printf("Error: %s\n", er);
    return -1;
}

void add_index_to_path(int idx, int idx_len) {
    char * val = malloc(idx_len + 1);
    sprintf(val,"%i",idx);
    path.hash = merge_hash(path.hash,hash(val,strlen(val)));
    free(val);
}

/* Declarations */
void set_input_string(const char* in);
void end_lexical_scan(void);

/* This function parses a string using yyparse. 
   Note that this is not reentrant */
int jqpath_parse_string(const char* in) {
  set_input_string(in);
  int rv = yyparse();
  end_lexical_scan();
  return rv;
}

struct jqpath * jqpath_get_path() {
  return &path;
}
