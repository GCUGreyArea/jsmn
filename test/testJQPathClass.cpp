#include <gtest/gtest.h>
#include <glog/logging.h>

#include "jsmn.hpp"
#include "string_funcs.h"
#include "JQ.hpp"

TEST(JQPathClass,testBasic) {
    struct jqpath * path = NULL;
    const char* str = ".name.value = \"new\"";
    path = jqpath_parse_string(str);

    JQ p(*path);

    ASSERT_TRUE(p == *path);
}
