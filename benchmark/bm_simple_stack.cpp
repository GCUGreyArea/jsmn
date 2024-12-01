#include <benchmark/benchmark.h>
#include "simple_stack.hpp"
#include <stack>

static void BM_RunSimpleStack(benchmark::State &state) {
    simple_stack<unsigned int> stack(100,2);
    for (auto _ : state) {
        for(unsigned int i=0;i<(1024^10);i++) {
            stack.push(i);
        }

        stack.reset();
    }
}


static void BM_RunSimpleStackClear(benchmark::State &state) {
    simple_stack<unsigned int> stack(100,2);
    for (auto _ : state) {
        for(unsigned int i=0;i<(1024^10);i++) {
            stack.push(i);
        }

        stack.clear();
    }
}


static void BM_RunSimpleStackClearBig(benchmark::State &state) {
    simple_stack<unsigned int > stack(1024^10,2);
    for (auto _ : state) {
        for(int i=0;i<(1024^10);i++) {
            stack.push(i);
        }

        stack.clear();
    }
}

static void BM_RunSimpleStackResetBig(benchmark::State &state) {
    simple_stack<unsigned int > stack(1024^10,2);
    for (auto _ : state) {
        for(int i=0;i<(1024^10);i++) {
            stack.push(i);
        }

        stack.reset();
    }
}


static void BM_RunStdStackBig(benchmark::State &state) {
    for (auto _ : state) {
        std::stack<unsigned int > stack;
        for(int i=0;i<(1024^10);i++) {
            stack.push(i);
        }
    }
}


// Register the function as a benchmark
BENCHMARK(BM_RunSimpleStack);
BENCHMARK(BM_RunSimpleStackClear);
BENCHMARK(BM_RunSimpleStackClearBig);
BENCHMARK(BM_RunSimpleStackResetBig);
BENCHMARK(BM_RunStdStackBig);
