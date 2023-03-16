#include <iostream>
//#include "tbb/parallel_invoke.h"
//#include "tbb/task_scheduler_observer.h"
//#include "tbb/flow_graph.h"
#include <random>
#include <chrono>
#include <cassert>

//#pragma once
class quicksort {
    int BLOCK = 1000;
    std::random_device rd;
    std::mt19937 gen{rd()};

    int n = 1000 * 1000 * 100;

//    tbb::task_arena taskArena{5, 1}; // 1 процесс  для работы приложения, 4 для работы функций

    void print(std::vector<float> &a) {
        for (auto elem: a) {
            std::cout << elem << " ";
        }
        std::cout << "\n";
    }

    int partition(std::vector<int> &a, int l, int r) {
        std::uniform_int_distribution<> distr(l, r - 1);
        auto thresholdVal = a[distr(gen)];
        auto lBound = l;
        auto rBound = r - 1;
        while (lBound <= rBound) {
            while (a[lBound] < thresholdVal) {
                ++lBound;
            }
            while (a[rBound] > thresholdVal) {
                --rBound;
            }

            if (lBound <= rBound) {
                if (a[lBound] > a[rBound]) {
                    std::swap(a[lBound], a[rBound]);
                }
                ++lBound;
                --rBound;
            }
        }
        return lBound;
    }

    void sequenceQuicksort(std::vector<int> &a, int l, int r) {
        if (r - l <= 1) return;
        int index = partition(a, l, r);
        sequenceQuicksort(a, l, index);
        sequenceQuicksort(a, index, r);
    }

//
//    void parallelQuicksort(std::vector<int> &a, int l, int r) {
//        if (std::abs(l - r) <= BLOCK) {
//            sequenceQuicksort(a, l, r);
//            return;
//        }
//        int index = partition(a, l, r);
//        taskArena.execute([&] {
//            tbb::parallel_invoke([&] { parallelQuicksort(a, l, index); },
//                                 [&] { parallelQuicksort(a, index, r); });
//        });
//
//
//    }

    void generate(std::vector<int> &a) {
        std::uniform_int_distribution<> distr(INT16_MIN, INT16_MAX);
        for (int i = 0; i < n; ++i) {
            a.push_back(distr(gen));
        }
    }

public:
   // float time_difference(int k) {
//        float result = 0;
//        std::vector<float> sequence_time(k, 0);
//        std::vector<float> parallel_time(k, 0);
//        std::vector<float> diffecence(k, 0);
//        for (int i = 0; i < k; ++i) {
//            std::vector<int> a;
//            std::cout << "generate round " + std::to_string(i + 1) + "\n";
//            generate(a);
//            auto b = a;
//            auto c = a;
//
//            auto begin = std::chrono::steady_clock::now();
//            sequenceQuicksort(a, 0, a.size());
//            auto end = std::chrono::steady_clock::now();
//            auto a_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
//            sequence_time[i] = (float) a_time;
//
//            begin = std::chrono::steady_clock::now();
//            parallelQuicksort(b, 0, b.size());
//            end = std::chrono::steady_clock::now();
//            auto b_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
//            parallel_time[i] = (float) b_time;
//
//            diffecence[i] = (float) a_time / (float) b_time;
//            result += diffecence[i];
//
//            std::sort(c.begin(), c.end());
////            assert(a = c);
////            assert(b = c);
//        }
//        std::cout << "sequence: ";
//        print(sequence_time);
//        std::cout << "\n";
//
//        std::cout << "parallel: ";
//        print(parallel_time);
//        std::cout << "\n";
//
//        std::cout << "difference: ";
//        print(diffecence);
//        std::cout << "\n";
//
//        std::cout << "result: " + std::to_string(result / (float) k) << "\n";
//        std::cout << "\n";
//
//        return result / (float) k;
    //}
};