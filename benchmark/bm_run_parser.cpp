#include <benchmark/benchmark.h>

#include <jsmn.hpp>

#include <fstream>
#include <iostream>
#include <string>

static void BM_RunParser(benchmark::State &state) {

    std::ifstream t("benchmark/resources/test.json");
    if (!t.is_open()) {
        throw std::runtime_error("failed to open file");
    }
    
    t.seekg(0, std::ios::end);
    size_t size = t.tellg();
    std::string buffer(size, ' ');
    t.seekg(0);
    t.read(&buffer[0], size);
    t.close();

    jsmn_parser p;
    for (auto _ : state) {
        p.init(buffer.c_str());
        p.parse();
    }

}


// Register the function as a benchmark
BENCHMARK(BM_RunParser);
