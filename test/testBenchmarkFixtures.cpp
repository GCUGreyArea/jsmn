#include <gtest/gtest.h>

#include "benchmark_fixtures.h"
#include "jsmn.hpp"
#include "jqpath.h"

TEST(BenchmarkFixtures, testSmallFixtureParsesAndSupportsLookup) {
    const std::string json = benchmark_fixtures::small_json();

    jsmn_parser parser(json.c_str(), 2);
    ASSERT_TRUE(parser.parse() > 0);

    struct jqpath *path = jqpath_parse_string(".name[1]");
    ASSERT_TRUE(path != NULL);

    auto *value = parser.get_path(path);
    ASSERT_TRUE(value != nullptr);
    ASSERT_TRUE(*value == "\"value 2\"");

    jqpath_close_path(path);
}

TEST(BenchmarkFixtures, testLargeFixtureParses) {
    const std::string json = benchmark_fixtures::large_json();

    jsmn_parser parser(json.c_str(), 2);
    ASSERT_EQ(parser.parse(), 1252990);
}
