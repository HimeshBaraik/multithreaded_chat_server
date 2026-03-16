#include "threadpool.h"
#include <iostream>

using namespace std;

int power(int base, int exponent) {
    int result = 1;
    for (int i = 0; i < exponent; ++i) {
        result *= base;
    }
    return result;
}

int main() {
    cout << "threadpool multiple power tasks example\n";

    Threadpool pool(4);
    vector<future<int>> results;

    for (int exp = 2; exp <= 9; ++exp) {
        results.push_back(pool.ExecuteTask(power, 2, exp));
    }

    for (int i = 0; i < (int)results.size(); ++i) {
        cout << "power(2, " << (i + 2) << ") = " << results[i].get() << "\n";
    }

    return 0;
}
