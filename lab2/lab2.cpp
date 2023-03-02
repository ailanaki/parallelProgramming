#include <vector>
#include <random>
#include <iostream>
#include <queue>
#include "tbb/parallel_for.h"
#include "tbb/task_scheduler_observer.h"

using graph = std::vector<std::vector<int>>;
using list = std::vector<int>;
using atomic = std::vector<std::atomic<bool>>;

class bfs {
    int n = 10;
    tbb::task_arena taskArena{5, 1}; // 1 процесс  для работы приложения, 4 для работы функций

    void print(graph &a) {
        for (auto list_: a) {
            print(list_);
        }
    }

    void sequentinalBFS(std::vector<std::vector<int>> &a, std::vector<int> &distance) {
        auto queue = std::queue<int>();
        queue.push(0);
        distance = std::vector<int>(a.size(), -1);
        distance[0] = 0;

        while (!queue.empty()) {
            auto node = queue.front();
            queue.pop();
            for (auto edge: a[node]) {
                if (distance[edge] == -1) {
                    distance[edge] = distance[node] + 1;
                    queue.push(edge);
                }
            }
        }
    }

    void parallelBFS(graph &a, list &distances) {
        auto f = list(1, 0);
        auto dist = 0;
        distances = list(a.size(), -1);
        auto visited = atomic(a.size());
        visited[0] = true;
        while (!f.empty()) {
            auto size = f.size();
            auto degs = list(size);


            taskArena.execute([&] {
                tbb::parallel_for(0, (int) size, [&](size_t ind) {
                    auto node = f[ind];
                    distances[node] = dist;
                    degs[ind] = (int) a[node].size();
                });
            });

            dist += 1;

            for (int i = 1; i < size; ++i) {
                degs[i] += degs[i - 1];
            }

            auto distinctDegs = list(size);
            auto nodes = list(degs.back(), -1);

            taskArena.execute([&] {
                tbb::parallel_for(0, (int) size, [&](size_t ind) {
                    auto rInd = (ind == 0) ? 0 : degs[ind - 1];
                    auto currNodeInd = rInd;
                    for (auto node: a[f[ind]]) {
                        bool expect = false;
                        if (visited[node].compare_exchange_weak(expect, true)) {
                            nodes[currNodeInd++] = node;
                        }
                    }
                    distinctDegs[ind] = currNodeInd - rInd;
                });
            });
            print(nodes);

            for (int i = 1; i < size; ++i) {
                distinctDegs[i] += distinctDegs[i - 1];
            }
            if (distinctDegs.back() == nodes.size()) {
                f = nodes;
                continue;
            }
            f = list(distinctDegs.back());
            taskArena.execute([&] {
                tbb::parallel_for(0, (int) f.size(), [&](size_t ind) {
                    if (ind == 0) {
                        for (int i = 0; i < distinctDegs[ind]; ++i) {
                            f[i] = nodes[i];
                        }
                    }else {
                        auto rInd = degs[ind - 1], fRelativeInd = distinctDegs[ind - 1];
                        auto copySize = distinctDegs[ind] - fRelativeInd;
                        for (int i = 0; i < copySize; ++i) {
                            f[fRelativeInd + i] = nodes[rInd + i];
                        }
                    }
                });
            });
        }
    }

    void generate_cubic(std::vector<std::vector<int>> &a) {
        a = std::vector<std::vector<int>>(n * n * n);
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                for (int k = 0; k < n; ++k) {
                    add(a, n, i, j, k, 1, 0, 0);
                    add(a, n, i, j, k, 0, 1, 0);
                    add(a, n, i, j, k, 0, 0, 1);
                }
            }
        }
    }

    static void add(graph & a, int size, int x, int y, int z, int dx, int dy, int dz) {
        if (x + dx < 0 || y + dy < 0 || z + dz < 0 || x + dx >= size || y + dy >= size || z + dz >= size) {
            return;
        }
        auto i = x + y * size + z * size * size;
        auto j = x + dx + (y + dy) * size + (z + dz) * size * size;
        a[i].push_back(j);
        a[j].push_back(i);
    }

    void print(std::vector<int> &a) {
        for (auto elem: a) {
            std::cout << elem << " ";
        }
        std::cout << "\n";
    }

    void generate_normal(std::vector<std::vector<int>> &a) {
        a.push_back({1, 2});
        a.push_back({3});
        a.push_back({2});
        a.push_back({4});
        a.emplace_back();
    }

public:
    void generate() {
        std::vector<std::vector<int>> a;
        std::vector<int> distance1, distance2;
        generate_cubic(a);
        sequentinalBFS(a, distance1);
        parallelBFS(a, distance2);
    }
};