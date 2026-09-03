#include "database.h"

#include <chrono>
#include <filesystem>
#include <iostream>
#include <random>
#include <string>

int main(int argc, char** argv) {
    using clock = std::chrono::steady_clock;
    const int n = argc > 1 ? std::stoi(argv[1]) : 20000;
    const std::filesystem::path root = "benchmark_data";
    std::filesystem::remove_all(root);

    minidb::Database db(root);
    std::string error;
    db.create_table("users", error);

    auto t0 = clock::now();
    for (int i = 1; i <= n; ++i) {
        if (!db.insert("users", i, "user_" + std::to_string(i), error)) {
            std::cerr << error << '\n';
            return 1;
        }
    }
    auto t1 = clock::now();

    std::mt19937 rng(42);
    std::uniform_int_distribution<int> dist(1, n);
    constexpr int kLookups = 5000;
    auto t2 = clock::now();
    for (int i = 0; i < kLookups; ++i) {
        error.clear();
        auto row = db.select_by_id("users", dist(rng), error);
        if (!row) return 2;
    }
    auto t3 = clock::now();

    auto t4 = clock::now();
    auto rows = db.select_all("users", error);
    auto t5 = clock::now();

    const auto insert_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
    const auto lookup_us = std::chrono::duration_cast<std::chrono::microseconds>(t3 - t2).count();
    const auto scan_us = std::chrono::duration_cast<std::chrono::microseconds>(t5 - t4).count();

    std::cout << "rows=" << n << '\n'
              << "insert_total_ms=" << insert_ms << '\n'
              << "indexed_lookup_avg_us=" << (static_cast<double>(lookup_us) / kLookups) << '\n'
              << "full_scan_us=" << scan_us << '\n'
              << "scan_rows=" << rows.size() << '\n';

    std::filesystem::remove_all(root);
    return 0;
}
