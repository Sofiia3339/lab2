#include <iostream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <random>
#include <thread>
#include <numeric>
#include <functional>
#include <iomanip>
#include <execution>

template <typename Function>
double MeasureExecutionTime(Function&& f) {
    auto start = std::chrono::high_resolution_clock::now();
    f();
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double, std::milli>(end - start).count();
}

bool ParallelNoneOf(const std::vector<int>& vec, int K, std::function<bool(int)> predicate) {
    size_t dataSize = vec.size();
    if (dataSize == 0) return true;
    size_t chunkSize = (dataSize + K - 1) / K;
    std::vector<std::thread> threads;
    std::vector<bool> results(K, true);

    for (int i = 0; i < K; ++i) {
        size_t start_idx = i * chunkSize;
        size_t end_idx = std::min(start_idx + chunkSize, dataSize);

        if (start_idx >= end_idx) continue;

        threads.emplace_back([&vec, &results, predicate, start_idx, end_idx, i]() {
            results[i] = std::none_of(vec.begin() + start_idx, vec.begin() + end_idx, predicate);
        });
    }
    for (auto& t : threads) {
        t.join();
    }

    return std::all_of(results.begin(), results.end(), [](bool res) { return res; });
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

    std::cout << " Standard Library Algorithms\n";
    double time_no_policy = MeasureExecutionTime([&]() {
        volatile bool result = std::none_of(vec.begin(), vec.end(), predicate);
    });
    std::cout << "Time without policy: " << time_no_policy << " ms\n";

    double time_seq = MeasureExecutionTime([&]() {
            volatile bool result = std::none_of(std::execution::seq, vec.begin(), vec.end(), predicate);
        });
    std::cout << "Time with sequential policy: " << time_seq << " ms\n";

    double time_par = MeasureExecutionTime([&]() {
        volatile bool result = std::none_of(std::execution::par, vec.begin(), vec.end(), predicate);
    });
    std::cout << "Time with parallel policy: " << time_par << " ms\n";

    double time_par_unseq = MeasureExecutionTime([&]() {
        volatile bool result = std::none_of(std::execution::par_unseq, vec.begin(), vec.end(), predicate);
    });
    std::cout << "Time with parallel-unsequenced policy: " << time_par_unseq << " ms\n";

    return 0;
}
