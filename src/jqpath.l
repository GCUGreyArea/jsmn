%{
    /** 
    ** This is a basic lexer for JQ Path strings 
    **/

    #include <stdio.h>
    #include "jqpath.h"
    #include "jqpath.tab.h"

    char * capture = NULL;

    /*
     * In JQ syntax, a dot represents the start of a
     * JSON object. The JSON {name = "value"} could be 
     * matched with the JQ Path .name = "value". 
     * {name : {value:[1,2,3]}} could be matched 
     * with .name.value[0] = 1 
     **/
    int lex_depth = 0;
    int intval;
    int last_int_len;
    float floatval;
    char* strval;

    struct jqpath path = {0};

    int add_int_element(char * el);
    int add_float_element(char * el);
    int add_string_element(char * el);
    void add_label_to_path(char * Lebel);

    YY_BUFFER_STATE yy_scan_string(const char *str);
    void yy_delete_buffer(YY_BUFFER_STATE buffer);

%}

%%

\".*\"                      { capture = yytext; add_string_element(capture); return STRING;}
\.                          { capture = yytext; path.depth++; return DOT;} 
\!\=                        { capture = yytext; path.op = JQ_NOT_EQUALS; return NOT_EQUALS;}
\=                          { capture = yytext; path.op = JQ_EQUALS; return EQUALS;}
\[                          { capture = yytext; return OPEN_ARR;}
\]                          { capture = yytext; path.depth++; return CLOSE_ARR;}
[0-9]+\.[0-9]+              { capture = yytext; add_float_element(capture); return FLOAT;}
[0-9]+                      { capture = yytext; add_int_element(capture); return INT;}
[a-zA-Z][a-zA-Z0-9]+        { capture = yytext; add_label_to_path(yytext); return LABEL;}
[ \t]                       { } /* skip whitespace which is not part of a string */
[\r\n]                      { } /* count any characters that are not counted by other rules */
.                           { } /* count any characters that are not counted by other rules */


%%

/* We turn all elements that are deamed numeric into their numeric identity 
   so that we perform normalisation. In other words 0001 becomes 1. */
int add_int_element(char * el) {
    intval = atoi(el);  
    last_int_len = strlen(el);
    return 0;
}

int add_float_element(char * el) {
    floatval = atof(el);
    return 0;
}

int add_string_element(char * el) {
    strval = el;
    return 0;
}

void add_label_to_path(char * label) {
    path.hash = merge_hash(path.hash,hash(label,strlen(label)));
}

void parse_string(const char *str) {
    
    if (str == NULL) {return;}

    path.op = 0;
    path.depth = 0;
    path.hash  = 0;

    YY_BUFFER_STATE buffer = yy_scan_string(str);
    yy_switch_to_buffer(buffer);
    int token;
    while ((token = yylex()) != 0) {
        // printf("Token: %d\n", token);
    }
    yy_delete_buffer(buffer);
}

void set_input_string(const char* in) {
    path.op = 0;
    path.depth = 0;
    path.hash  = 0;
    path.value.value.int_val = 0; 

    yy_scan_string(in);
}

void end_lexical_scan(void) {
    yy_delete_buffer(YY_CURRENT_BUFFER);
}

int yywrap() {
    return(1);
}
