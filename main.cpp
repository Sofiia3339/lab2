#include <iostream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <random>

template <typename Function>
double MeasureExecutionTime(Function&& f) {
    auto start = std::chrono::high_resolution_clock::now();
    f();
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double, std::milli>(end - start).count();
}

int main() {
    const size_t dataSize = 1000000;
    std::vector<int> vec(dataSize);
    std::mt19937 gen(123);
    std::uniform_int_distribution<int> distrib(1, 1000000);
    for (int& val : vec) {
        val = distrib(gen) * 2 + 1;
    }

    auto predicate = [](int x) { return x % 2 == 0; };



    return 0;
}
