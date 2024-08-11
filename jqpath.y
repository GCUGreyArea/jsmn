%{
    // #include "jqpath.h"
    #include <stdio.h>
    extern int yylex();
    extern int depth;
    extern int intval;
    extern float floatval;
    extern char * strval;
    int yyerror(char * er);
%}

%token  STRING;
%token  DOT;
%token  EQUALS;
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
    complete_path expression {printf("parse-PATH EXPRESSION\n"); }
;

complete_path: 
  DOT LABEL                             {printf("parse-DOT LABEL path`\n"); }
| DOT LABEL array_def complete_path     {printf("parse-ARR\n"); }
;

array_def: 
  OPEN_ARR CLOSE_ARR {depth++;printf("empty array: depth=%d\n",depth);}
| OPEN_ARR INT CLOSE_ARR {printf("int array def: %d: depth=%d\n", intval, depth);}

expression: 
  EQUALS INT    {printf("INT EXP: %d\n", intval); }
| EQUALS FLOAT  {printf("FLOAT EXP: %f\n", floatval); }
| EQUALS STRING {printf("STRING EXP: %s\n", strval); }
;


%% 

int yyerror(char * er) {
    printf("Error: %s\n", er);
}


/* Declarations */
void set_input_string(const char* in);
void end_lexical_scan(void);

/* This function parses a string */
int parse_yacc_string(const char* in) {
  set_input_string(in);
  int rv = yyparse();
  end_lexical_scan();
  return rv;
}

int main(int argc, char ** argv) {
  if(argc > 1)
    return parse_yacc_string(argv[1]);

  return -1;   
}