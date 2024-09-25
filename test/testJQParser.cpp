#include <gtest/gtest.h>
#include <glog/logging.h>

#include <jsmn.hpp>

TEST(JQParser,testJQParser) {
    const char* str = ".name.value = \"new\"";
    int ret = jqpath_parse_string(str);
    ASSERT_EQ(ret,0);

    str = ".name.value != \"test\"";
    ret = jqpath_parse_string(str);
    ASSERT_EQ(ret,0);

    str = ".name[1].value != \"test\"";
    ret = jqpath_parse_string(str);
    ASSERT_EQ(ret,0);

    str = ".name[101].value = 12";
    ret = jqpath_parse_string(str);
    ASSERT_EQ(ret,0);

    str = ".name[1].value = 100.12";
    ret = jqpath_parse_string(str);
    ASSERT_EQ(ret,0);

}