%{
    #include <stdio.h>
    extern int yylex();
    extern int lex_depth;
    extern int intval;
    extern float floatval;
    extern char * strval;
    int yyerror(char * er);
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
;

complete_path: 
  DOT LABEL                             { /* printf("parse-DOT LABEL path\n"); */ }
| DOT LABEL array_def complete_path     { /* printf("parse-ARR\n"); */ }
| DOT LABEL complete_path               { /* printf("complex complete path\n"); */}
;

array_def: 
  OPEN_ARR CLOSE_ARR {lex_depth++; /*printf("empty array: lex_depth=%d\n",lex_depth); */}
| OPEN_ARR INT CLOSE_ARR { /*printf("int array def: %d: lex_depth=%d\n", intval, lex_depth);*/}

expression: 
  EQUALS INT        { /* printf("INT EXP: %d\n", intval); */}
| EQUALS FLOAT      { /*printf("FLOAT EXP: %f\n", floatval);*/ }
| EQUALS STRING     { /*printf("STRING EXP: %s\n", strval);*/ }
| NOT_EQUALS INT    { /*printf("INT NOT EXP: %d\n", intval); */}
| NOT_EQUALS FLOAT  { /*printf("FLOAT NOT EXP: %f\n", floatval);*/ }
| NOT_EQUALS STRING { /*printf("STRING NOT EXP: %s\n", strval);*/ }
;


%% 

int yyerror(char * er) {
    printf("Error: %s\n", er);
    return -1;
}


/* Declarations */
void set_input_string(const char* in);
void end_lexical_scan(void);

/* This function parses a string using yyparse */
int jqpath_parse_string(const char* in) {
  set_input_string(in);
  lex_depth = 0;
  int rv = yyparse();
  end_lexical_scan();
  return rv;
}
