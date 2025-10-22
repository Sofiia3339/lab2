#include <iostream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <random>
#include <thread>
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

void AnalyzeCustomParallelPerformance(const std::vector<int>& vec) {
    unsigned int max_k = std::thread::hardware_concurrency() * 2;
    if (max_k == 0) max_k = 16;

    std::cout << "\n--- Custom Parallel Algorithm Analysis ---\n";
    std::cout << std::left << std::setw(25) << "Number of Threads (K)" << std::setw(20) << "Execution Time (ms)" << std::endl;
    std::cout << std::string(45, '-') << std::endl;

    double bestTime = std::numeric_limits<double>::max();
    int bestK = 0;

    for (unsigned int k = 1; k <= max_k; ++k) {
        double exec_time = MeasureExecutionTime([&]() {
            ParallelNoneOf(vec, k, [](int x) { return x % 2 == 0; });
        });

        std::cout << std::left << std::setw(25) << k << std::setw(20) << exec_time << std::endl;

        if (exec_time < bestTime) {
            bestTime = exec_time;
            bestK = k;
        }
    }
    std::cout << std::string(45, '-') << std::endl;
    std::cout << "Optimal K found: " << bestK
              << " (Execution time: " << bestTime << " ms)\n";
    std::cout << "Hardware concurrency: " << std::thread::hardware_concurrency() << std::endl;
    std::cout << "Ratio (Optimal K / Hardware concurrency): " << static_cast<double>(bestK) / std::thread::hardware_concurrency() << std::endl;
}

void RunFullAnalysis(size_t dataSize) {
    std::cout << "\n==================================================" << std::endl;
    std::cout << "   ANALYSIS FOR DATA SIZE: " << dataSize << " elements" << std::endl;
    std::cout << "==================================================" << std::endl;

    // Генерація випадкових даних
    std::vector<int> vec(dataSize);
    std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<int> distrib(1, 1'000'000);
    for (size_t i = 0; i < dataSize; ++i) {
        vec[i] = distrib(gen) * 2 + 1;
    }

    auto predicate = [](int x) { return x % 2 == 0; };

    // Тести стандартних алгоритмів
    std::cout << "\n--- Standard Library Algorithms ---\n";
    double time_no_policy = MeasureExecutionTime([&]() { std::none_of(vec.begin(), vec.end(), predicate); });
    std::cout << "Time without policy: " << time_no_policy << " ms\n";

    double time_seq = MeasureExecutionTime([&]() { std::none_of(std::execution::seq, vec.begin(), vec.end(), predicate); });
    std::cout << "Time with sequential policy: " << time_seq << " ms\n";

    double time_par = MeasureExecutionTime([&]() { std::none_of(std::execution::par, vec.begin(), vec.end(), predicate); });
    std::cout << "Time with parallel policy: " << time_par << " ms\n";

    double time_par_unseq = MeasureExecutionTime([&]() { std::none_of(std::execution::par_unseq, vec.begin(), vec.end(), predicate); });
    std::cout << "Time with parallel-unsequenced policy: " << time_par_unseq << " ms\n";

    // Тест власного алгоритму
    AnalyzeCustomParallelPerformance(vec);
}

int main() {
    std::vector<size_t> data_sizes = { 1'000'000, 10'000'000, 50'000'000 };

    for (size_t size : data_sizes) {
        RunFullAnalysis(size);
    }

    return 0;
}
