//#include "lab1/lab1.cpp"
#include "lab2/lab2.cpp"

int main() {
//    quicksort q;
//    q.time_difference(5);
    bfs b;
    std::cout << "N = 250 \n";
    b.generate(300);
    std::cout << "N = 500 \n";
    b.generate(500);
    return 0;
}