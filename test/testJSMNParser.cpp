#include <glog/logging.h>
#include <gtest/gtest.h>

#include <jsmn.hpp>

#include <fstream>
#include <iostream>
#include <string>


TEST(JSMNParser, testEmptyStrings) {
    jsmn_parser p("{}");

    unsigned res = p.parse();

    ASSERT_EQ(res, 1);
    auto *t = p.get_tokens();

    ASSERT_EQ(t[0].type, JSMN_OBJECT);
    p.init("[]");
    res = p.parse();

    ASSERT_EQ(res, 1);
    t = p.get_tokens();
    ASSERT_EQ(t[0].type, JSMN_ARRAY);

    p.init("[{},{}]");
    res = p.parse();
    ASSERT_EQ(res, 3);
    t = p.get_tokens();
    ASSERT_EQ(t[0].type, JSMN_ARRAY);
    ASSERT_EQ(t[1].type, JSMN_OBJECT);
    ASSERT_EQ(t[2].type, JSMN_OBJECT);
}

TEST(JSMNParser, testComplexObjects) {
    jsmn_parser p("{\"a\":0}");

    unsigned res = p.parse();

    ASSERT_EQ(res, 3);
    auto *t = p.get_tokens();
    ASSERT_EQ(t[0].type, JSMN_OBJECT);
    ASSERT_EQ(t[1].type, JSMN_STRING);
    ASSERT_EQ(t[2].type, JSMN_PRIMITIVE);

    p.init("{\"a\":{},\"b\":{}}");
    res = p.parse();
    ASSERT_EQ(res, 5);
    t = p.get_tokens();
    ASSERT_EQ(t[0].type, JSMN_OBJECT);
    ASSERT_EQ(t[1].type, JSMN_STRING);
    ASSERT_EQ(t[2].type, JSMN_OBJECT);
    ASSERT_EQ(t[3].type, JSMN_STRING);
    ASSERT_EQ(t[4].type, JSMN_OBJECT);

    p.init("{\n \"Day\": 26,\n \"Month\": 9,\n \"Year\": 12\n }");
    res = p.parse();
    ASSERT_EQ(res, 7);
    t = p.get_tokens();
    ASSERT_EQ(t[0].type, JSMN_OBJECT);
    ASSERT_EQ(t[1].type, JSMN_STRING);
    ASSERT_EQ(t[2].type, JSMN_PRIMITIVE);
    ASSERT_EQ(t[3].type, JSMN_STRING);
    ASSERT_EQ(t[4].type, JSMN_PRIMITIVE);
    ASSERT_EQ(t[5].type, JSMN_STRING);
    ASSERT_EQ(t[6].type, JSMN_PRIMITIVE);
}

TEST(JSMNParser, testSimpleFileRead) {
    using namespace std;

    string line;
    ifstream t("test/resources/test.json");
    if (!t.is_open()) {
        ASSERT_EQ(false,true);
    }
    
    t.seekg(0, std::ios::end);
    size_t size = t.tellg();
    std::string buffer(size, ' ');
    t.seekg(0);
    t.read(&buffer[0], size);

    jsmn_parser p(buffer.c_str());

    unsigned int ret = p.parse();
    ASSERT_EQ(ret,70);
}


TEST(JSMNParser, testLargeFileRead) {
    using namespace std;

    string line;
    ifstream t("test/resources/nginx_json_logs.json");
    if (!t.is_open()) {
        ASSERT_EQ(false,true);
    }
    
    t.seekg(0, std::ios::end);
    size_t size = t.tellg();
    std::string buffer(size, ' ');
    t.seekg(0);
    t.read(&buffer[0], size);

    jsmn_parser p(buffer.c_str());

    unsigned int ret = p.parse();
    ASSERT_EQ(ret,874855);
}
