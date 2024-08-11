#ifndef JQ_DEFS
#define JQ_DEFS

typedef enum {
    JQ_STRING,
    JQ_INT,
    JQ_FLOAT,
    JQ_ARRAY,
    JQ_OBJ
} jq_type;

typedef struct jq_def {
    unsigned int depth;
    jq_type type;
    char * string;
    union {
        int num_int;
        float num_float;
        const char * string;
    } jqpath;
    unsigned hash;
} jq_def;

#endif//JQ_DEFS