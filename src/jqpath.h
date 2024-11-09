#ifndef JQ_DEFS
#define JQ_DEFS

#ifdef __cplusplus
extern "C" {
#endif

#include "jqpath.tab.h"


enum jq_operator {
    JQ_NOT_SET = 0,        // No value has been set yet
    JQ_EQUALS,             // Equality operator represented by = 
    JQ_NOT_EQUALS,         // Non equality operator != 
    JQ_GREATER_THAN,       // Less than operator <
    JQ_LESS_THAN,          // Greater than operator >
    JQ_MATCH,              // Rrepresents a regex that must be defined as R"regex text " 
    JQ_CLOSE_TO            // Use Levenshtein distance
};

struct jqpath {
    unsigned int hash;      // The hash of the path
    unsigned int depth;     // The depth of the path
    enum jq_operator op;    // The operator to use if there is an eequality argument
    char * string;          // The string representing the equality argument
};

int jqpath_parse_string(const char * str);
struct jqpath* jqpath_get_path();

#ifdef __cplusplus
}
#endif

#endif//JQ_DEFS