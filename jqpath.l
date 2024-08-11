%{
    /** 
    ** This is a basic lexer for JQ Path strings 
    ** We now need a parser to make sure everything 
    ** is parsed in the correct order and to do error 
    ** checking on input strings.
    **/

    #include "jqpath.h"
    #include "y.tab.h"

    char * capture = NULL;
    /*
     * In JQ syntax, a dot represents the start of a
     * JSON object. The JSON {name = "value"} could be 
     * matched with the JQ Path .name = "value". 
     * {name : {value:[1,2,3]}} could be matched 
     * with .name.value[0] = 1 
     **/
    int depth = 0;
    int intval;
    float floatval;
    char* strval;
    int add_int_element(char * el);
    int add_float_element(char * el);
    int add_string_element(char * el);

    YY_BUFFER_STATE yy_scan_string(const char *str);
    void yy_delete_buffer(YY_BUFFER_STATE buffer);

%}

%%

\".*\"                      { capture = yytext; printf("quoted string: %s\n", capture); add_string_element(capture); return STRING;}
\.                          { capture = yytext; printf("dot\n"); depth++; printf("depth: %d\n",depth); return DOT; }
=                           { capture = yytext; printf("equals\n"); return EQUALS;}
\[                          { capture = yytext; printf("open array\n"); return OPEN_ARR;}
\]                          { capture = yytext; printf("close array\n"); depth++; return CLOSE_ARR;}
[0-9]+\.[0-9]+              { capture = yytext; printf("float: %s\n", capture); add_float_element(capture); return FLOAT;}
[0-9]+                      { capture = yytext; printf("int: %s\n", capture); add_int_element(capture); return INT;}
[a-zA-Z][a-zA-Z0-9]+        { capture = yytext; printf("label: %s\n", capture); return LABEL;}

%%

int add_int_element(char * el) {
    printf("El: %d - depth %d\n",atoi(el),depth);
    intval = atoi(el);
    return 0;
}

int add_float_element(char * el) {
    printf("El: %f - depth %d\n", atof(el),depth);
    floatval = atof(el);
    return 0;
}

int add_string_element(char * el) {
    printf("El :%s - depth %d\n",el,depth);
    strval = el;
    return 0;
}

void parse_string(const char *str) {
    if (str == NULL) {return;}

    YY_BUFFER_STATE buffer = yy_scan_string(str);
    yy_switch_to_buffer(buffer);
    int token;
    while ((token = yylex()) != 0) {
        printf("Token: %d\n", token);
    }
    yy_delete_buffer(buffer);
}

void set_input_string(const char* in) {
    yy_scan_string(in);
}

void end_lexical_scan(void) {
    yy_delete_buffer(YY_CURRENT_BUFFER);
}
