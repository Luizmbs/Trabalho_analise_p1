#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <algorithm>
#include <chrono>

using namespace std;

int main() {
    std::ios::sync_with_stdio(0);
    std::cin.tie(0);

    auto start = chrono::high_resolution_clock::now();

    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> elapsed = end - start;
    cout << "Tempo: " << elapsed.count() << "s\n";
    
    return 0;
}