#include <benchmark/benchmark.h>

#include <memory>
#include <stdexcept>
#include <string>

#include "benchmark_fixtures.h"
#include "jsmn.hpp"
#include "jqpath.h"

namespace {

class jqpath_ptr {
  public:
    explicit jqpath_ptr(struct jqpath *path) : m_path(path) {}
    ~jqpath_ptr() { jqpath_close_path(m_path); }

    struct jqpath *get() const { return m_path; }

  private:
    struct jqpath *m_path;
};

static void BM_ParseSmallDocument(benchmark::State &state) {
    const std::string json = benchmark_fixtures::small_json();

    for (auto _ : state) {
        jsmn_parser parser(json.c_str(), 2);
        benchmark::DoNotOptimize(parser.parse());
    }
}

static void BM_ParseLargeDocument(benchmark::State &state) {
    const std::string json = benchmark_fixtures::large_json();

    for (auto _ : state) {
        jsmn_parser parser(json.c_str(), 2);
        benchmark::DoNotOptimize(parser.parse());
    }
}

static void BM_RebuildPathsMediumDocument(benchmark::State &state) {
    const std::string json = benchmark_fixtures::medium_json();
    jsmn_parser parser(json.c_str(), 2);
    if (parser.parse() < 0) {
        throw std::runtime_error("failed to parse medium benchmark fixture");
    }

    for (auto _ : state) {
        parser.render();
        benchmark::ClobberMemory();
    }
}

static void BM_GetPathJQ(benchmark::State &state) {
    const std::string json = benchmark_fixtures::small_json();
    jsmn_parser parser(json.c_str(), 2);
    if (parser.parse() < 0) {
        throw std::runtime_error("failed to parse small benchmark fixture");
    }

    jqpath_ptr path(jqpath_parse_string(".name[1]"));
    if (path.get() == nullptr) {
        throw std::runtime_error("failed to parse jq benchmark path");
    }

    for (auto _ : state) {
        benchmark::DoNotOptimize(parser.get_path(path.get()));
    }
}

static void BM_GetPathJSONPath(benchmark::State &state) {
    const std::string json = benchmark_fixtures::small_json();
    jsmn_parser parser(json.c_str(), 2);
    if (parser.parse() < 0) {
        throw std::runtime_error("failed to parse small benchmark fixture");
    }

    jqpath_ptr path(jsonpath_parse_string("$.name[1]"));
    if (path.get() == nullptr) {
        throw std::runtime_error("failed to parse jsonpath benchmark path");
    }

    for (auto _ : state) {
        benchmark::DoNotOptimize(parser.get_path(path.get()));
    }
}

static void BM_InsertIntoEmptyObject(benchmark::State &state) {
    const std::string json = benchmark_fixtures::empty_object_json();

    for (auto _ : state) {
        jsmn_parser parser(json.c_str(), 2);
        if (parser.parse() < 0) {
            throw std::runtime_error("failed to parse empty object fixture");
        }

        const bool inserted =
            parser.inster_kv_into_obj("\"name\"", "\"Barry\"", JSMN_STRING, 0);
        if (!inserted) {
            throw std::runtime_error("object insert benchmark failed");
        }
        benchmark::ClobberMemory();
    }
}

static void BM_InsertIntoEmptyArray(benchmark::State &state) {
    const std::string json = benchmark_fixtures::empty_array_json();

    for (auto _ : state) {
        jsmn_parser parser(json.c_str(), 2);
        if (parser.parse() < 0) {
            throw std::runtime_error("failed to parse empty array fixture");
        }

        const bool inserted =
            parser.insert_value_in_list("\"Barry\"", JSMN_STRING, 0);
        if (!inserted) {
            throw std::runtime_error("array insert benchmark failed");
        }
        benchmark::ClobberMemory();
    }
}

static void BM_UpdatePrimitiveValue(benchmark::State &state) {
    const std::string json = benchmark_fixtures::update_target_json();

    for (auto _ : state) {
        jsmn_parser parser(json.c_str(), 2);
        if (parser.parse() < 0) {
            throw std::runtime_error("failed to parse update benchmark fixture");
        }

        const int key_token = parser.find_key_in_object(0, "age");
        const bool updated = parser.update_value_for_key(key_token, "42");
        if (!updated) {
            throw std::runtime_error("update benchmark failed");
        }
        benchmark::ClobberMemory();
    }
}

BENCHMARK(BM_ParseSmallDocument);
BENCHMARK(BM_ParseLargeDocument);
BENCHMARK(BM_RebuildPathsMediumDocument);
BENCHMARK(BM_GetPathJQ);
BENCHMARK(BM_GetPathJSONPath);
BENCHMARK(BM_InsertIntoEmptyObject);
BENCHMARK(BM_InsertIntoEmptyArray);
BENCHMARK(BM_UpdatePrimitiveValue);

} // namespace

BENCHMARK_MAIN();
