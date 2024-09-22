#ifndef JQ_DEFS
#define JQ_DEFS

#ifdef __cplusplus
extern "C" {
#endif

#include "jqpath.tab.h"

int jqpath_parse_string(const char * str);

#ifdef __cplusplus
}
#endif

#endif//JQ_DEFS