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
    ifstream t("test/resources/test-data/large-file.json");
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
    ASSERT_EQ(ret,1252988);
}

TEST(JSMNParser, testLargeFileReadSerialise) {
    using namespace std;

    string line;
    ifstream t("test/resources/test-data/large-file.json");
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
    ASSERT_EQ(ret,1252988);

    // Serialise 
    p.serialise("test/resources/outfile.bin");

    // Desierialise into new instance
    jsmn_parser p2;
    p2.deserialise("test/resources/outfile.bin");

    unsigned int last = p2.last_token();

    ASSERT_EQ(ret,last);

    // Test that the strings are the same 
    ASSERT_STREQ(p.get_json().c_str(),p2.get_json().c_str());

    // Test that all the tokens are the same
    jsmntok_t * t1 = p.get_tokens();
    jsmntok_t * t2 = p2.get_tokens();
    for(unsigned int i=0;i<last+1;i++) {
        ASSERT_EQ(t1->end,t2->end);
        ASSERT_EQ(t1->parent,t2->parent);
        ASSERT_EQ(t1->size,t2->size);
        ASSERT_EQ(t1->start,t2->start);
        ASSERT_EQ(t1->type,t2->type);
    }
}
