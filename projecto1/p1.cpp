#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <algorithm>
#include <chrono>

using namespace std;

int affinity[256][256];

void initAffinity() {
    for (int i = 0; i < 256; i++) {
        affinity['T'][i] = 1;
        affinity[i]['T'] = 1;
    }
    
    affinity['P']['P'] = 1; affinity['P']['N'] = 3; affinity['P']['A'] = 1; affinity['P']['B'] = 3;
    affinity['N']['P'] = 5; affinity['N']['N'] = 1; affinity['N']['A'] = 0; affinity['N']['B'] = 1;
    affinity['A']['P'] = 0; affinity['A']['N'] = 1; affinity['A']['A'] = 0; affinity['A']['B'] = 4;
    affinity['B']['P'] = 1; affinity['B']['N'] = 3; affinity['B']['A'] = 2; affinity['B']['B'] = 3;
}

int n;
vector<int> potential;
string classes;

const int MAXN = 2005;
long long dp[MAXN][MAXN];
int choice[MAXN][MAXN];

long long energy(int left, int k, int right) {
    int p_left = (left == 0) ? 1 : potential[left];
    int p_k = potential[k];
    int p_right = (right == n + 1) ? 1 : potential[right];
    
    unsigned char c_left = (left == 0) ? 'T' : classes[left - 1];
    unsigned char c_k = classes[k - 1];
    unsigned char c_right = (right == n + 1) ? 'T' : classes[right - 1];
    
    long long e1 = (long long)p_left * affinity[c_left][c_k] * p_k;
    long long e2 = (long long)p_k * affinity[c_k][c_right] * p_right;
    
    return e1 + e2;
}

void getSequence(int left, int right, vector<int>& result) {
    if (right - left <= 1) return;
    int k = choice[left][right];
    getSequence(left, k, result);
    getSequence(k, right, result);
    result.push_back(k);
}

void solve() {
    //bttom-up dp
    for (int len = 2; len <= n + 1; len++) {
        for (int left = 0; left + len <= n + 1; left++) {
            int right = left + len;
            
            long long maxEnergy = -1;
            int bestK = -1;
            
            // Para cada k possível
            for (int k = left + 1; k < right; k++) {
                long long energyK = energy(left, k, right);
                long long totalEnergy = dp[left][k] + dp[k][right] + energyK;
                
                // Se encontrou energia maior, atualiza
                if (totalEnergy > maxEnergy) {
                    maxEnergy = totalEnergy;
                    bestK = k;
                }
                // Se energia igual, precisa comparar sequências
                else if (totalEnergy == maxEnergy && bestK != -1) {
                    // Precisamos comparar as sequências que k e bestK produzem
                    // Para isso, construímos as sequências usando os choice já calculados
                    
                    // Sequência atual (bestK)
                    vector<int> seqBest;
                    if (left + 1 < bestK) {
                        vector<int> temp;
                        getSequence(left, bestK, temp);
                        seqBest.insert(seqBest.end(), temp.begin(), temp.end());
                    }
                    if (bestK + 1 < right) {
                        vector<int> temp;
                        getSequence(bestK, right, temp);
                        seqBest.insert(seqBest.end(), temp.begin(), temp.end());
                    }
                    seqBest.push_back(bestK);
                    
                    // Sequência candidata (k)
                    vector<int> seqK;
                    if (left + 1 < k) {
                        vector<int> temp;
                        getSequence(left, k, temp);
                        seqK.insert(seqK.end(), temp.begin(), temp.end());
                    }
                    if (k + 1 < right) {
                        vector<int> temp;
                        getSequence(k, right, temp);
                        seqK.insert(seqK.end(), temp.begin(), temp.end());
                    }
                    seqK.push_back(k);
                    
                    // Se seqK é lexicograficamente menor, atualiza
                    if (seqK < seqBest) {
                        bestK = k;
                        seqBest = seqK;
                    }
                }
            }
            
            dp[left][right] = maxEnergy;
            choice[left][right] = bestK;
        }
    }
}

int main() {
    std::ios::sync_with_stdio(0);
    std::cin.tie(0);
    
    initAffinity();
    
    cin >> n;
    
    potential.resize(n + 2);
    potential[0] = 1;
    for (int i = 1; i <= n; i++) {
        cin >> potential[i];
    }
    potential[n + 1] = 1;
    
    cin >> classes;
    
    auto start = chrono::high_resolution_clock::now();
    memset(dp, 0, sizeof(dp));
    memset(choice, -1, sizeof(choice));
    
    solve();
    
    vector<int> sequence;
    getSequence(0, n + 1, sequence);
    
    cout << dp[0][n + 1] << '\n';
    for (size_t i = 0; i < sequence.size(); i++) {
        if (i > 0) cout << ' ';
        cout << sequence[i];
    }
    cout << '\n';

    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> elapsed = end - start;
    cout << "Tempo: " << elapsed.count() << "s\n";
    
    return 0;
}