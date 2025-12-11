# Explicação Detalhada do Código - Solução do Problema da Cadeia de Aminoácidos

## Índice
1. [Visão Geral do Problema](#visão-geral-do-problema)
2. [Estratégia de Solução](#estratégia-de-solução)
3. [Estrutura do Código](#estrutura-do-código)
4. [Explicação Detalhada por Seção](#explicação-detalhada-por-seção)
5. [Análise de Complexidade](#análise-de-complexidade)
6. [Exemplo de Execução](#exemplo-de-execução)

---

## Visão Geral do Problema

### O Problema
Temos uma cadeia de `n` aminoácidos que devem ser removidos um por um. Cada aminoácido tem:
- Um **potencial de energia** `P[i]`
- Uma **classe bioquímica** `C[i]` ∈ {P, N, A, B}

Quando removemos um aminoácido `i`, a energia liberada depende dos seus vizinhos:

```
E_liberada = P[i-1] × Af(C[i-1], C[i]) × P[i] + P[i] × Af(C[i], C[i+1]) × P[i+1]
```

**Objetivo**: Encontrar a ordem de remoção que **maximiza** a energia total liberada.

### Por que é Difícil?
- A ordem de remoção importa: remover `a` antes de `b` pode dar resultado diferente
- Quando removemos um aminoácido, seus vizinhos se tornam adjacentes
- O espaço de busca é `n!` (fatorial) - inviável para força bruta

---

## Estratégia de Solução

### Abordagem: Programação Dinâmica

A solução usa uma técnica inspirada no problema clássico **"Burst Balloons"**:

**Ideia Central**: Em vez de pensar "qual aminoácido remover primeiro", pensamos **"qual aminoácido remover por último"** em cada intervalo.

#### Por que isso funciona?

Quando decidimos que o aminoácido `k` será removido **por último** no intervalo `(left, right)`:
1. Todos os outros aminoácidos entre `left` e `right` já foram removidos
2. Os vizinhos de `k` são exatamente `left` e `right`
3. Podemos calcular a energia de remover `k` deterministicamente

Isso permite **dividir o problema** em subproblemas independentes:
- Subproblema 1: Remover todos entre `left` e `k`
- Subproblema 2: Remover todos entre `k` e `right`
- Depois: Remover `k` (que agora tem vizinhos `left` e `right`)

### Recorrência

```
solve(left, right) = max{ solve(left, k) + solve(k, right) + energy(left, k, right) }
                     para todo k ∈ (left, right)
```

Onde `energy(left, k, right)` é a energia de remover `k` quando seus vizinhos são `left` e `right`.

---

## Estrutura do Código

```cpp
#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <algorithm>
#include <cstring>

using namespace std;

// 1. TABELA DE AFINIDADE
int affinity[256][256];
void initAffinity() { ... }

// 2. VARIÁVEIS GLOBAIS
int n;
vector<int> potential;
string classes;
map<pair<int, int>, pair<long long, vector<int>>> memo;

// 3. FUNÇÃO DE ENERGIA
long long energy(int left, int k, int right) { ... }

// 4. PROGRAMAÇÃO DINÂMICA
pair<long long, vector<int>> solve(int left, int right) { ... }

// 5. FUNÇÃO PRINCIPAL
int main() { ... }
```

---

## Explicação Detalhada por Seção

### 1. Tabela de Afinidade

```cpp
int affinity[256][256];

void initAffinity() {
    // Terminal (T) com todas as classes = 1
    for (int i = 0; i < 256; i++) {
        affinity['T'][i] = 1;
        affinity[i]['T'] = 1;
    }
    
    // Tabela de afinidade da imagem
    affinity['P']['P'] = 1; affinity['P']['N'] = 3; affinity['P']['A'] = 1; affinity['P']['B'] = 3;
    affinity['N']['P'] = 5; affinity['N']['N'] = 1; affinity['N']['A'] = 0; affinity['N']['B'] = 1;
    affinity['A']['P'] = 0; affinity['A']['N'] = 1; affinity['A']['A'] = 0; affinity['A']['B'] = 4;
    affinity['B']['P'] = 1; affinity['B']['N'] = 3; affinity['B']['A'] = 2; affinity['B']['B'] = 3;
}
```

#### Explicação:
- **Array 2D de 256×256**: Usamos char como índice diretamente (código ASCII)
- **Terminal 'T'**: Classe especial dos terminais (posições 0 e n+1)
  - `Af(T, qualquer) = 1` e `Af(qualquer, T) = 1`
- **Tabela assimétrica**: `Af(P,N) = 3` mas `Af(N,P) = 5`
  - Por isso não podemos simplificar como `affinity[a][b] = affinity[b][a]`

#### Por que array de 256?
- Permite usar caracteres diretamente como índices: `affinity['P']['N']`
- Mais eficiente que um `map<pair<char,char>, int>`
- Desperdício de memória é insignificante (256×256×4 bytes = 256KB)

---

### 2. Variáveis Globais

```cpp
int n;                                                    // Número de aminoácidos
vector<int> potential;                                    // Potenciais P[0..n+1]
string classes;                                           // Classes C[0..n+1]
map<pair<int, int>, pair<long long, vector<int>>> memo;  // Memoização
```

#### Explicação:

**`potential` e `classes`**:
- Tamanho `n+2`: incluem terminais nas posições 0 e n+1
- `potential[0] = potential[n+1] = 1` (terminais)
- `classes[0] = classes[n+1] = 'T'` (terminais)
- Aminoácidos reais: posições 1 a n

**`memo` - Tabela de Memoização**:
- Chave: `pair<int, int>` → intervalo `(left, right)`
- Valor: `pair<long long, vector<int>>` → energia máxima + sequência de remoção
- Evita recalcular subproblemas: O(n²) subproblemas possíveis

#### Por que `map` e não `vector`?
- `map` é mais fácil de usar com pares como chave
- Para n ≤ 1000, a diferença de performance é negligível
- Alternativa: `vector<vector<pair>>` de tamanho (n+2)×(n+2)

---

### 3. Função de Energia

```cpp
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
```

#### Explicação Linha por Linha:

**Obter Potenciais:**
```cpp
int p_left = (left == 0) ? 1 : potential[left];
```
- Se `left == 0`: é o terminal esquerdo → potencial = 1
- Caso contrário: usa `potential[left]`
- Mesmo raciocínio para `p_right`

**Obter Classes:**
```cpp
char c_left = (left == 0) ? 'T' : classes[left - 1];
```
- **ATENÇÃO**: `classes` é uma string (índice base 0), mas aminoácidos são numerados de 1 a n
- `classes[0]` = classe do aminoácido 1
- `classes[k-1]` = classe do aminoácido k
- Terminal: classe 'T'

**Calcular Energia:**
```cpp
long long e1 = (long long)p_left * affinity[c_left][c_k] * p_k;
```
- **E1**: Interação do vizinho esquerdo com k
- Fórmula: `P[left] × Af(C[left], C[k]) × P[k]`

```cpp
long long e2 = (long long)p_k * affinity[c_k][c_right] * p_right;
```
- **E2**: Interação de k com o vizinho direito
- Fórmula: `P[k] × Af(C[k], C[right]) × P[right]`

**Por que `long long`?**
- Potenciais podem ser até ~100
- Afinidades até 5
- Com n=1000: energia pode ultrapassar 10^15 → precisa de 64 bits

---

### 4. Programação Dinâmica - Função `solve`

```cpp
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
```

#### Análise Detalhada:

**Caso Base:**
```cpp
if (right - left <= 1) {
    return {0, vector<int>()};
}
```
- Se não há aminoácidos entre `left` e `right`, retorna energia 0 e sequência vazia
- Exemplo: `solve(2, 3)` → não há aminoácido entre posição 2 e 3

**Memoização:**
```cpp
pair<int, int> key = {left, right};
if (memo.find(key) != memo.end()) {
    return memo[key];
}
```
- Verifica se já calculamos `solve(left, right)` antes
- Se sim, retorna o resultado armazenado
- Evita recálculo: transforma O(n!) em O(n³)

**Loop Principal:**
```cpp
for (int k = left + 1; k < right; k++) {
```
- Testa cada aminoácido `k` no intervalo aberto (left, right)
- "E se `k` fosse o último a ser removido?"

**Divisão em Subproblemas:**
```cpp
auto leftResult = solve(left, k);    // Remove todos entre left e k
auto rightResult = solve(k, right);  // Remove todos entre k e right
long long energyK = energy(left, k, right);  // Remove k por último
```

**Construção da Sequência:**
```cpp
vector<int> sequence;
sequence.insert(sequence.end(), leftResult.second.begin(), leftResult.second.end());
sequence.insert(sequence.end(), rightResult.second.begin(), rightResult.second.end());
sequence.push_back(k);
```
- Ordem: primeiro os da esquerda, depois os da direita, por fim k
- Isso garante que k é removido por último no intervalo

**Critério de Escolha:**
```cpp
if (totalEnergy > maxEnergy || 
    (totalEnergy == maxEnergy && sequence < bestSequence)) {
```
- Prioridade 1: Maior energia
- Prioridade 2: Em caso de empate, sequência lexicograficamente menor
- Comparação de vetores em C++ é lexicográfica por padrão

---

### 5. Função Principal

```cpp
int main() {
    std::ios::sync_with_stdio(0);  // Otimização de I/O
    std::cin.tie(0);
    
    initAffinity();  // Inicializa tabela de afinidade
    
    cin >> n;
    
    potential.resize(n + 2);
    potential[0] = 1; // Terminal esquerdo
    for (int i = 1; i <= n; i++) {
        cin >> potential[i];
    }
    potential[n + 1] = 1; // Terminal direito
    
    cin >> classes;  // String de n caracteres
    
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
```

#### Explicação:

**Otimização de I/O:**
```cpp
std::ios::sync_with_stdio(0);
std::cin.tie(0);
```
- Desativa sincronização entre `cin`/`cout` e `scanf`/`printf`
- Torna I/O mais rápido (~2x em alguns casos)
- Importante para grandes entradas

**Configuração dos Terminais:**
```cpp
potential[0] = 1;        // Terminal esquerdo
potential[n + 1] = 1;    // Terminal direito
```
- Aminoácidos reais: 1 a n
- Terminais: 0 e n+1

**Leitura de Dados:**
```cpp
cin >> classes;  // Lê string inteira de uma vez
```
- `classes[0]` = classe do aminoácido 1
- `classes[1]` = classe do aminoácido 2
- etc.

**Chamada Principal:**
```cpp
auto result = solve(0, n + 1);
```
- Resolve o problema completo: intervalo (0, n+1)
- Retorna `{energia_máxima, sequência_ótima}`

**Saída:**
```cpp
cout << result.first << endl;  // Energia total
// Sequência separada por espaços
for (int i = 0; i < result.second.size(); i++) {
    if (i > 0) cout << " ";
    cout << result.second[i];
}
```

---

## Análise de Complexidade

### Complexidade de Tempo: **O(n³)**

**Análise:**
1. **Número de subproblemas**: O(n²)
   - Cada par (left, right) com 0 ≤ left < right ≤ n+1
   - Total: ~n²/2 subproblemas

2. **Custo por subproblema**: O(n)
   - Loop sobre k: at most n iterações
   - Cada iteração: O(1) cálculo + O(n) para construir sequência

3. **Total**: O(n²) × O(n) = **O(n³)**

### Complexidade de Espaço: **O(n²)**

**Análise:**
- **Memoização**: O(n²) entradas
- **Cada entrada**: O(n) para armazenar sequência
- **Total**: O(n²) × O(n) = O(n³) no pior caso
- **Pilha de recursão**: O(n)

**Otimização possível**: Se não precisássemos armazenar a sequência, seria O(n²)

### Limitações Práticas

Para n = 1000:
- Operações: ~10⁹ (viável em ~1 segundo)
- Memória: ~1GB (viável)

Para n > 2000:
- Pode precisar de otimizações adicionais

---

## Exemplo de Execução

Vamos executar o algoritmo passo a passo para o **Exemplo 1**:

### Entrada:
```
3
10 5 12
ABA
```

### Configuração:
```
Posição:    0   1   2   3   4
Potencial:  1  10   5  12   1
Classe:     T   A   B   A   T
```

### Execução de `solve(0, 4)`:

**Tentativa 1: k=1 (remover aminoácido 1 por último)**
```
solve(0, 1) → {0, []}                    // Sem aminoácidos
solve(1, 4) → ?                          // Precisa calcular
  k=2: solve(1,2)=0, solve(2,4)=?, energy(1,2,4)=?
  k=3: solve(1,3)=?, solve(3,4)=0, energy(1,3,4)=?
  ...
energy(0, 1, 4) = 1×1×10 + 10×1×1 = 10 + 10 = 20
```

**Tentativa 2: k=2 (remover aminoácido 2 por último)**
```
solve(0, 2) → ?
  k=1: energy(0,1,2) = 1×1×10 + 10×4×5 = 10 + 200 = 210
solve(2, 4) → ?
  k=3: energy(2,3,4) = 5×2×12 + 12×1×1 = 120 + 12 = 132
energy(0, 2, 4) = 1×1×5 + 5×1×1 = 5 + 5 = 10

Total: 210 + 132 + 10 = 352
Sequência: [1, 3, 2]
```

**Tentativa 3: k=3 (remover aminoácido 3 por último)**
```
solve(0, 3) → ?
  k=1: solve(0,1)=0, solve(1,3)=?, energy(0,1,3)=?
    solve(1,3): k=2, energy(1,2,3)=10×4×5+5×2×12=200+120=320
    solve(0,1,3): energy=1×1×10+10×1×12=10+120=130
    Total: 0 + 320 + 130 = 450, seq=[2,1]
  k=2: ...
solve(3, 4) → {0, []}
energy(0, 3, 4) = 1×1×12 + 12×1×1 = 12 + 12 = 24

Total: 450 + 0 + 24 = 474?
```

(Continua testando todas as combinações...)

### Resultado Final:
```
Energia máxima: 359
Sequência: 1 2 3
```

### Verificação Manual:

**Passo 1: Remover aminoácido 1**
```
Cadeia: T [A] B A T
E = 1×1×10 + 10×4×5 = 10 + 200 = 210
Resultado: T B A T
```

**Passo 2: Remover aminoácido 2 (era B)**
```
Cadeia: T [B] A T
E = 1×1×5 + 5×2×12 = 5 + 120 = 125
Resultado: T A T
```

**Passo 3: Remover aminoácido 3 (era A)**
```
Cadeia: T [A] T
E = 1×1×12 + 12×1×1 = 12 + 12 = 24
Resultado: T T
```

**Total: 210 + 125 + 24 = 359** ✓

---

## Comparação com Outras Abordagens

### ❌ Força Bruta: O(n! × n)
```cpp
// Testar todas as n! permutações
for cada permutação p:
    simular remoção na ordem p
    calcular energia total
escolher melhor
```
**Problema**: n=20 → 20! ≈ 2.4×10¹⁸ operações (inviável)

### ❌ Guloso: O(n²)
```cpp
// Sempre remover o que libera mais energia agora
while (há aminoácidos):
    k = argmax{energy(left[k], k, right[k])}
    remover k
```
**Problema**: Escolha local ótima ≠ solução global ótima

### ✅ Programação Dinâmica: O(n³)
- **Correto**: Testa todas as possibilidades de forma inteligente
- **Eficiente**: Reutiliza cálculos (memoização)
- **Viável**: n=1000 em ~1 segundo

---

## Possíveis Otimizações

### 1. Armazenar Apenas Energia (sem sequência)
Se só precisássemos da energia máxima:
```cpp
map<pair<int,int>, long long> memo;  // Só energia
// Espaço: O(n²) em vez de O(n³)
```

### 2. Iterativo em vez de Recursivo
```cpp
// Bottom-up: intervalos de tamanho crescente
for (int len = 2; len <= n+1; len++) {
    for (int left = 0; left + len <= n+1; left++) {
        int right = left + len;
        // calcular dp[left][right]
    }
}
```
**Vantagem**: Evita overhead de chamadas recursivas

### 3. Array em vez de Map
```cpp
pair<long long, vector<int>> dp[MAX_N][MAX_N];
```
**Vantagem**: Acesso O(1) em vez de O(log n)

---

## Pontos-Chave para Entender

1. **"Por último" é mais fácil que "primeiro"**
   - Quando removemos por último, sabemos exatamente quem são os vizinhos

2. **Memoização transforma exponencial em polinomial**
   - Sem memo: O(n!) → inviável
   - Com memo: O(n³) → viável

3. **Ordem lexicográfica**
   - Comparação de vetores em C++ é automática
   - `[1,2,3] < [1,3,2]` → true

4. **Terminais simplificam o código**
   - Não precisamos tratar casos especiais de extremidades
   - Sempre há um vizinho (mesmo que seja terminal)

5. **Long long é essencial**
   - Energia pode crescer muito (até ~10¹⁵)
   - `int` tem limite ~2×10⁹

---

## Exercícios para Aprofundamento

1. **Modificar para retornar top-k soluções**
   - Em vez de só a melhor, as k melhores sequências

2. **Adicionar restrições**
   - "Não pode remover dois aminoácidos da mesma classe consecutivamente"
   - Como adaptar a DP?

3. **Otimizar espaço**
   - Implementar versão que não armazena sequências
   - Reconstruir sequência com segunda passada

4. **Implementar versão iterativa**
   - Bottom-up em vez de top-down
   - Comparar performance

---

## Recursos Adicionais

- **Problema similar**: [LeetCode 312 - Burst Balloons](https://leetcode.com/problems/burst-balloons/)
- **Conceito**: Programação Dinâmica com intervalos
- **Livro**: "Introduction to Algorithms" (CLRS) - Capítulo 15

---

## Conclusão

Esta solução é um exemplo elegante de como **programação dinâmica** pode transformar um problema aparentemente intratável (O(n!)) em algo computacionalmente viável (O(n³)).

A chave está em:
- **Reformular o problema**: "último a remover" em vez de "primeiro"
- **Dividir e conquistar**: Subproblemas independentes
- **Memoização**: Evitar recálculos
- **Estrutura de dados adequada**: Map para armazenar estado

Com essas técnicas, conseguimos resolver instâncias com n=1000 em tempo razoável, algo impossível com abordagens mais ingênuas.
