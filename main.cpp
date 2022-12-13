#include <iostream>
#include <fstream>
#include <random>
#include <algorithm>
#include <vector>

#define N_min 256
#define N_max 16777216
#define K 10

int *fill_in_forward(size_t size) {
    auto arr = new int[size];
    for (int i = 0; i < size - 1; i++) {
        arr[i] = i + 1;
    }
    arr[size - 1] = 0;
    return arr;
}

int *fill_in_backward(size_t size) {
    auto arr = new int[size];
    for (int i = 1; i < size; i++) {
        arr[i] = i - 1;
    }
    arr[0] = static_cast<int> (size - 1);
    return arr;
}

int *fill_in_random(int size) {
    auto arr = new int[size];
    std::vector<int> ind(size - 1);
    for (int i = 0; i < size - 1; i++)
        ind[i] = i + 1;
    int cur = 0, prev = 0;
    std::shuffle(ind.begin(), ind.end(), std::mt19937(std::random_device()()));
    for (int i = 0; i < size - 1 && !ind.empty(); i++) {
        cur = ind[ind.size() - 1];
        ind.pop_back();
        arr[prev] = cur;
        prev = cur;
    }
    arr[cur] = 0;
    return arr;
}

void cache_warm(const int *arr, size_t size) {
    for (int k = 0, i = 0; i < size; i++)
        k = arr[k];
}

void bypass(const int *arr, size_t size, std::ofstream &fout) {
    union ticks {
        unsigned long long t64;
        struct s32 {
            long th, tl;
        } t32;
    } start{}, end{};

    cache_warm(arr, size);

    asm volatile ("rdtsc\n":"=a"(start.t32.th), "=d"(start.t32.tl)::"memory");

    for (int k = 0, i = 0; i < size * K; i++)
        k = arr[k];

    asm volatile ("rdtsc\n":"=a"(end.t32.th), "=d"(end.t32.tl)::"memory");
    fout << "Size in bytes: " << size*4 << "  ";
    fout << "Time taken: " << (end.t64 - start.t64) / (size * K) << " ticks\n";
    fout.flush();

}

int main() {
    int *test_arr;
    std::ofstream forw_fout("forw_res.txt");
    std::ofstream backw_fout("backw_res.txt");
    std::ofstream rand_fout("rand_res.txt");
    if (!forw_fout.is_open() || !backw_fout.is_open() || !rand_fout.is_open())
        return 1;

    for (int s = N_min; s <= N_max; s *= 2) {   //262144  131072
        test_arr = fill_in_forward(s);
        forw_fout << "Forward    ";
        bypass(test_arr, s, forw_fout);
        delete[] test_arr;

        test_arr = fill_in_backward(s);
        backw_fout << "Backward   ";
        bypass(test_arr, s, backw_fout);
        delete[] test_arr;

        test_arr = fill_in_random(s);
        rand_fout << "Random   ";
        bypass(test_arr, s, rand_fout);
        cache_warm(test_arr, s);
        delete[] test_arr;
    }
}
