#include <glog/logging.h>
#include <gtest/gtest.h>

#include "jsmn.hpp"
#include "jqpath.h"


TEST(JQPathJSMN, testJSMNJQPathStringValues) {
    std::string json = "{\"name\":[\"value 1\",\"value 2\",\"value 3\"]}";
    std::string path_str = ".name[1] = \"value 2\"";


    jsmn_parser p(json,2);

    struct jqpath * path = jqpath_parse_string(path_str.c_str());

    int ret = p.parse();

    ASSERT_TRUE(ret > 0);
    ASSERT_TRUE(path != NULL);

    p.render();

    auto * np = p.get_path(path);
    ASSERT_TRUE(np != nullptr);

    ASSERT_TRUE(*np == "\"value 2\"");
}

TEST(JQPathJSMN, testJSMNJQPathOpenArrayStringValues) {
    std::string json = "{\"name\":[\"value 1\",\"value 2\",\"value 3\"]}";
    std::string path_str = ".name[1] = \"value 2\"";


    jsmn_parser p(json,2);

    struct jqpath * path = jqpath_parse_string(path_str.c_str());

    int ret = p.parse();

    ASSERT_TRUE(ret > 0);
    ASSERT_TRUE(path != NULL);

    p.render();

    auto * np = p.get_path(path);
    ASSERT_TRUE(np != nullptr);

    ASSERT_TRUE(*np == "\"value 2\"");
}

TEST(JQPathJSMN, testJSMNJQPathOpenArrayStringValuesComplex) {
    std::string json = "{\"name\":[\"value 1\",\"value 2\",\"value 3\"]}";
    std::string path_str = ".name[2] = \"value 3\"";


    jsmn_parser p(json,2);

    struct jqpath * path = jqpath_parse_string(path_str.c_str());

    int ret = p.parse();

    ASSERT_TRUE(ret > 0);
    ASSERT_TRUE(path != NULL);

    p.render();

    auto * np = p.get_path(path);
    ASSERT_TRUE(np != nullptr);

    ASSERT_TRUE(*np == "\"value 3\"");
}

TEST(JQPathJSMN, testJSMNJQPathOpenArrayStringValuesComplexWithoutEquals) {
    std::string json = "{\"name\":[\"value 1\",\"value 2\",\"value 3\"]}";
    std::string path_str = ".name[2]";


    jsmn_parser p(json,2);

    struct jqpath * path = jqpath_parse_string(path_str.c_str());

    int ret = p.parse();

    ASSERT_TRUE(ret > 0);
    ASSERT_TRUE(path != NULL);

    p.render();

    auto * np = p.get_path(path);
    ASSERT_TRUE(np != nullptr);

    ASSERT_TRUE(*np == "\"value 3\"");
}

TEST(JQPathJSMN, testJSMNJQPathOpenArrayStringValuesSimpleWithoutEquals) {
    std::string json = "{\"name\":\"Barry\"}";
    std::string path_str = ".name";


    jsmn_parser p(json,2);

    struct jqpath * path = jqpath_parse_string(path_str.c_str());

    int ret = p.parse();

    ASSERT_TRUE(ret > 0);
    ASSERT_TRUE(path != NULL);

    p.render();

    auto * np = p.get_path(path);
    ASSERT_TRUE(np != nullptr);

    ASSERT_TRUE(*np == "\"Barry\"");
}
