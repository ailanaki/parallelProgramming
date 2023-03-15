#include <vector>
#include <iostream>
#include <queue>
#include <cassert>
#include <execution>
#include <tbb/parallel_invoke.h>
#include "tbb/parallel_for.h"
#include "tbb/task_scheduler_observer.h"
#include "tbb/parallel_scan.h"
#include "tbb/parallel_pipeline.h"


using graph = std::vector<std::vector<int>>;
using list = std::vector<int>;
using atomic = std::vector<std::atomic<bool>>;

class bfs {
    int n = 500;
    int SCAN_BLOCK = 4000;
    tbb::task_arena taskArena{5, 1}; // 1 процесс  для работы приложения, 4 для работы функций

    static void print(graph &a) {
        for (auto list_: a) {
            print(list_);
        }
    }

    static void sequentinalBFS(std::vector<std::vector<int>> &a, std::vector<int> &distance) {
        list res = list(a.size(), 0);
        std::vector<bool> visited = std::vector<bool>(a.size(), false);
        std::queue<int> q;
        int start = 0;
        visited[start] = true;
        q.push(start);
        while (!q.empty()) {
            int cur = q.front();
            q.pop();
            for (int i: a[cur]) {
                if (!visited[i]) {
                    visited[i] = true;
                    res[i] = res[cur] + 1;
                    q.push(i);
                }
            }
        }
        std::swap(res, distance);
    }

    void scanUp(list &a, list &sums, int left, int right, int index) {
        if (right - left < SCAN_BLOCK) {
            int sum = 0;

            for (int i = left; i <= right; i++) {

                sum += a[i];
            }
            sums[index] = sum;
            return;
        }
        int m = (left + right) / 2;
        taskArena.execute([&] {
            tbb::parallel_invoke([&] {
                                     scanUp(a, sums, left, m, index * 2);
                                 },
                                 [&] {
                                     scanUp(a, sums,
                                            m + 1, right, index * 2 + 1);
                                 });
        });
        sums[index] = sums[index * 2] + sums[index * 2 + 1];
    }


    void scanDown(list &a, list &result, list &sums, int left, int right, int index, int sumLeft) {
        if (right - left < SCAN_BLOCK) {
            result[left + 1] = sumLeft + a[left];
            for (int i = left + 1; i <= right; i++) {
                result[i + 1] = result[i] + a[i];
            }
            return;
        }

        int m = (left + right) / 2;
        taskArena.execute([&] {
            tbb::parallel_invoke([&] {
                                     scanDown(a, result, sums, left, m,
                                              index * 2, sumLeft);
                                 },
                                 [&] {
                                     scanDown(a, result, sums,
                                              m + 1, right,
                                              index * 2 + 1, sumLeft + sums[index * 2]);
                                 });
        });
    }


    void scan(list &a, list &result) {
        auto sums = list(a.size() * 2 + 1, 0);
        scanUp(a, sums, 0, (int) a.size() - 1, 1);
        scanDown(a, result, sums, 0, (int)a.size() - 1, 1, 0);
    }

    template<size_t limit = 2000, class Func>
    void maybeParallelFor(int left, int right, Func func) {
        if (right - left < limit) {
            for (int i = left; i <= right; i++) {
                func(i);
            }
            return;
        }

        auto m = (left + right) / 2;
        taskArena.execute([&] {
            tbb::parallel_invoke([&] {
                                     maybeParallelFor(left, m, func);
                                 },
                                 [&] {
                                     maybeParallelFor(m + 1, right, func);
                                 });
        });
    }

    void parallelBFS(graph &a, list &distances) {
        distances = list(a.size(), 0);
        auto visited = atomic(a.size());
        visited[0] = true;
        auto f = list(1, 1);
        auto deg = list(1);
        distances[0] = 0;
        deg[0] = (int) a[0].size();
        auto dist = 1;

        while (!f.empty()) {
            auto startBlock = list(deg.size() + 1);
            scan(deg, startBlock);
            int nextFrontierSize = startBlock.back();
            auto nextFrontier = list(nextFrontierSize);
            deg = list(nextFrontierSize);
            maybeParallelFor(0, f.size() - 1, [&](int currentIndex) {
                int current = f[currentIndex] - 1;
                if (current != -1) {
                    auto neighbours = &a[current];
                    auto ind = startBlock[currentIndex];
                    maybeParallelFor(0, (int) neighbours->size() - 1, [&](int i) {
                        int next = neighbours->operator[](i);
                        bool exp = false;
                        if (visited[next].compare_exchange_strong(exp, true)) {
                            distances[next] = dist;
                            nextFrontier[ind + i] = next + 1;
                            deg[ind + i] = a[next].size();
                        }

                    });
                }
            });
            std::swap(f, nextFrontier);
            dist++;
        }
    }

    void generate_cubic(std::vector<std::vector<int>> &a) const {
        a = std::vector<std::vector<int>>(n * n * n);

        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                for (int k = 0; k < n; ++k) {
                    auto add_list = list();
                    add(add_list, n, i, j - 1, k);
                    add(add_list, n, i, j, k - 1);
                    add(add_list, n, i + 1, j, k);
                    add(add_list, n, i, j + 1, k);
                    add(add_list, n, i, j, k + 1);
                    add(add_list, n, i - 1, j, k);
                    a[(i * n + j) * n + k] = add_list;
                }
            }
        }
    }

    static void add(list &a, int size, int x, int y, int z) {

        if (x >= 0 && y >= 0 && z >= 0 && x < size && y < size && z < size) {
            auto point = (x * size + y) * size + z;
            a.push_back(point);
        }
    }

    static void print(std::vector<int> &a) {
        for (auto elem: a) {
            std::cout << elem << " ";
        }
        std::cout << "\n";
    }

    static void generate_normal(std::vector<std::vector<int>> &a) {
        a.push_back({1, 2});
        a.push_back({3});
        a.push_back({4});
        a.push_back({4});
        a.emplace_back();
    }

    bool checkBFSResult(list &distances, int size) {
        for (int i = 0; i < size; i++) {
            for (int j = 0; j < size; j++) {
                for (int k = 0; k < size; k++) {
                    int position = (i * size + j) * size + k;
                    if (distances[position] != i + j + k) {
                        return false;
                    }
                }
            }
        }
        return true;
    }

    void print(std::vector<float> &a) {
        for (auto elem: a) {
            std::cout << elem << " ";
        }
        std::cout << "\n";
    }

public:

    void generate() {
        int k = 1;
        float result = 0;
        std::vector<float> sequence_time(k, 0);
        std::vector<float> parallel_time(k, 0);
        std::vector<float> diffecence(k, 0);
        for (int i = 0; i < k; ++i) {
            std::cout << "generate round " + std::to_string(i + 1) + "\n";
            std::vector<std::vector<int>> a;
            std::vector<int> distance1, distance2;

            auto begin = std::chrono::high_resolution_clock::now();
            generate_cubic(a);
            auto end = std::chrono::high_resolution_clock::now();
            auto gen_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
            std::cout << "GENTIME: " << gen_time << "\n";


            begin = std::chrono::high_resolution_clock::now();
            sequentinalBFS(a, distance1);
            end = std::chrono::high_resolution_clock::now();
            auto a_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
            sequence_time[i] = (float) a_time;

            begin = std::chrono::high_resolution_clock::now();
            parallelBFS(a, distance2);
            end = std::chrono::high_resolution_clock::now();
            auto b_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
            parallel_time[i] = (float) b_time;
            std::cout << "CHECK: " << (checkBFSResult(distance2, n)) << " " << (distance1 == distance2) << "\n";


            diffecence[i] = (float) a_time / (float) b_time;
            result += diffecence[i];
        }

        std::cout << "sequence: ";
        print(sequence_time);
        std::cout << "\n";

        std::cout << "parallel: ";
        print(parallel_time);
        std::cout << "\n";

        std::cout << "difference: ";
        print(diffecence);
        std::cout << "\n";

        std::cout << "result: " + std::to_string(result / (float) k) << "\n";
        std::cout << "\n";
    }

};