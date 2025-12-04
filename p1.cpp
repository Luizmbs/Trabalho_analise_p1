#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <algorithm>
#include <cstring>

using namespace std;

// Tabela de afinidade entre classes bioquímicas
int affinity[256][256];

void initAffinity() {
    // Terminal (T) com todas as classes = 1
    for (int i = 0; i < 256; i++) {
        affinity['T'][i] = 1;
        affinity[i]['T'] = 1;
    }
    
    // Tabela de afinidade da imagem
    // Af[linha][coluna]
    affinity['P']['P'] = 1; affinity['P']['N'] = 3; affinity['P']['A'] = 1; affinity['P']['B'] = 3;
    affinity['N']['P'] = 5; affinity['N']['N'] = 1; affinity['N']['A'] = 0; affinity['N']['B'] = 1;
    affinity['A']['P'] = 0; affinity['A']['N'] = 1; affinity['A']['A'] = 0; affinity['A']['B'] = 4;
    affinity['B']['P'] = 1; affinity['B']['N'] = 3; affinity['B']['A'] = 2; affinity['B']['B'] = 3;
}

int n;
vector<int> potential;
string classes;
map<pair<int, int>, pair<long long, vector<int>>> memo;

// Calcula a energia liberada ao remover o aminoácido na posição k
// left e right são os índices dos aminoácidos ainda presentes à esquerda e direita
long long energy(int left, int k, int right) {
    int p_left = (left == 0) ? 1 : potential[left];
    int p_k = potential[k];
    int p_right = (right == n + 1) ? 1 : potential[right];
    
    char c_left = (left == 0) ? 'T' : classes[left - 1];
    char c_k = classes[k - 1];
    char c_right = (right == n + 1) ? 'T' : classes[right - 1];
    
    long long e1 = (long long)p_left * affinity[c_left][c_k] * p_k;
    long long e2 = (long long)p_k * affinity[c_k][c_right] * p_right;
    
    return e1 + e2;
}

// Programação dinâmica: resolver para o intervalo (left, right)
// Retorna a energia máxima e a sequência de remoção
pair<long long, vector<int>> solve(int left, int right) {
    // Caso base: nenhum aminoácido no intervalo
    if (right - left <= 1) {
        return {0, vector<int>()};
    }
    
    // Verifica se já calculamos este subproblema
    pair<int, int> key = {left, right};
    if (memo.find(key) != memo.end()) {
        return memo[key];
    }
    
    long long maxEnergy = -1;
    vector<int> bestSequence;
    
    // Tenta remover cada aminoácido k no intervalo (left, right) por último
    for (int k = left + 1; k < right; k++) {
        // Energia de remover k quando left e right são seus vizinhos
        long long energyK = energy(left, k, right);
        
        // Resolve recursivamente os subintervalos
        auto leftResult = solve(left, k);
        auto rightResult = solve(k, right);
        
        long long totalEnergy = leftResult.first + rightResult.first + energyK;
        
        // Constrói a sequência: primeiro remove left e right, depois k
        vector<int> sequence;
        sequence.insert(sequence.end(), leftResult.second.begin(), leftResult.second.end());
        sequence.insert(sequence.end(), rightResult.second.begin(), rightResult.second.end());
        sequence.push_back(k);
        
        // Atualiza o melhor resultado (lexicograficamente menor em caso de empate)
        if (totalEnergy > maxEnergy || 
            (totalEnergy == maxEnergy && sequence < bestSequence)) {
            maxEnergy = totalEnergy;
            bestSequence = sequence;
        }
    }
    
    memo[key] = {maxEnergy, bestSequence};
    return memo[key];
}

int main() {
    std::ios::sync_with_stdio(0);
    std::cin.tie(0);
    
    initAffinity();
    
    cin >> n;
    
    potential.resize(n + 2);
    potential[0] = 1; // Terminal esquerdo
    for (int i = 1; i <= n; i++) {
        cin >> potential[i];
    }
    potential[n + 1] = 1; // Terminal direito
    
    cin >> classes;
    
    // Resolve o problema
    auto result = solve(0, n + 1);
    
    cout << result.first << endl;
    for (int i = 0; i < result.second.size(); i++) {
        if (i > 0) cout << " ";
        cout << result.second[i];
    }
    cout << endl;
    
    return 0;
}
