#include <gtest/gtest.h>
#include <glog/logging.h>

#include <jsmn.hpp>

extern int lex_depth;

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

TEST(JQParser,testJQDepth) {
    struct jqpath* path = jqpath_get_path();

    // We also want to insure that each path results in a unique hash value
    std::set<unsigned int> ints;

    const char* str = ".name";
    int ret = jqpath_parse_string(str);
    ASSERT_EQ(ret,0);
    ASSERT_EQ(path->depth,1);
    auto single = ints.emplace(path->hash);
    ASSERT_TRUE(single.second);

    str = ".name.value";
    ret = jqpath_parse_string(str);
    ASSERT_EQ(ret,0);
    ASSERT_EQ(path->depth,2);
    single = ints.emplace(path->hash);
    ASSERT_TRUE(single.second);


    str = ".name[1].value";
    ret = jqpath_parse_string(str);
    ASSERT_EQ(ret,0);
    ASSERT_EQ(path->depth,3);
    single = ints.emplace(path->hash);
    ASSERT_TRUE(single.second);

    str = ".name[1].value.noun";
    ret = jqpath_parse_string(str);
    ASSERT_EQ(ret,0);
    ASSERT_EQ(path->depth,4);
    single = ints.emplace(path->hash);
    ASSERT_TRUE(single.second);

    str = ".name[1].value.three[].four";
    ret = jqpath_parse_string(str);
    ASSERT_EQ(ret,0);
    ASSERT_EQ(path->depth,6);
    single = ints.emplace(path->hash);
    ASSERT_TRUE(single.second);
}