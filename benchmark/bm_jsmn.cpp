#include <benchmark/benchmark.h>

#include <fstream>
#include <iostream>
#include <string>

#include <jsmn.h>

static void BM_RunJSMNCParser(benchmark::State &state) {

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

    jsmntok_t tokens[1024];
    jsmn_parser p;

    for (auto _ : state) {
        jsmn_init(&p);
        jsmn_parse(&p,buffer.c_str(),buffer.length(),tokens,1024);
    }

}


BENCHMARK(BM_RunJSMNCParser);