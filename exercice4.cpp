#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <iomanip>
#include <bitset>
#include <cstdint>
#include <ctime>
#include <chrono>
using namespace std;
using namespace std::chrono;

// ======================================================================
// 1. Fonctions utilitaires pour AC_HASH (Automate cellulaire)
// ======================================================================
int apply_rule(uint32_t rule, int left, int center, int right) {
    int index = left * 4 + center * 2 + right;
    return (rule >> index) & 1;
}

vector<int> evolve(const vector<int>& state, uint32_t rule) {
    int n = state.size();
    vector<int> next_state(n, 0);
    for (int i = 0; i < n; i++) {
        int left   = (i == 0) ? 0 : state[i-1];
        int center = state[i];
        int right  = (i == n-1) ? 0 : state[i+1];
        next_state[i] = apply_rule(rule, left, center, right);
    }
    return next_state;
}

vector<int> text_to_bits(const string& input) {
    vector<int> bits;
    for (char c : input) {
        bitset<8> b(c);
        for (int i = 7; i >= 0; --i)
            bits.push_back(b[i]);
    }
    return bits;
}

string ac_hash(const string& input, uint32_t rule = 30, size_t steps = 5) {
    vector<int> state = text_to_bits(input);
    if (state.size() > 512) state.resize(512);

    for (size_t i = 0; i < steps; ++i)
        state = evolve(state, rule);

    vector<int> hash_bits(64, 0);
    for (size_t i = 0; i < 64; ++i)
        hash_bits[i] = state[i % state.size()];

    string hash_str = "";
    for (size_t i = 0; i < 64; i += 4) {
        int val = hash_bits[i]*8 + hash_bits[i+1]*4 + hash_bits[i+2]*2 + hash_bits[i+3];
        hash_str += "0123456789ABCDEF"[val];
    }
    return hash_str;
}

// ======================================================================
// 2. Simple hash pour le mode SHA256 simulé
// ======================================================================
string simpleHash(const string &data) {
    unsigned int hash = 0;
    for (char c : data)
        hash = (hash * 101 + c) % 1000000007; 
    stringstream ss;
    ss << hex << setw(8) << setfill('0') << hash;
    return ss.str();
}

// ======================================================================
// 3. Définition du bloc et blockchain
// ======================================================================
enum HashMode { SHA256_MODE, AC_HASH_MODE };

class Block {
public:
    int index;
    string previousHash;
    string data;
    long timestamp;
    int nonce;
    string hash;
    HashMode mode;

    Block(int idx, string prev, string d, HashMode m)
        : index(idx), previousHash(prev), data(d), mode(m), nonce(0) {
        timestamp = time(nullptr);
        hash = calculateHash();
    }

    string calculateHash() const {
        stringstream ss;
        ss << index << previousHash << timestamp << data << nonce;
        string blockData = ss.str();

        if (mode == SHA256_MODE)
            return simpleHash(blockData);
        else
            return ac_hash(blockData, 30, 1);
    }
};

class Blockchain {
public:
    vector<Block> chain;
    int difficulty;
    HashMode mode;

    Blockchain(HashMode m = SHA256_MODE) : mode(m), difficulty(3) {
        chain.push_back(createGenesisBlock());
    }

    Block createGenesisBlock() {
        return Block(0, "0", "Genesis Block", mode);
    }

    Block getLatestBlock() const {
        return chain.back();
    }
};

// ======================================================================
// 4. Benchmark avec sécurité contre les boucles infinies
// ======================================================================
struct BenchmarkResult {
    double totalTime;
    int totalIterations;
    double avgTimePerBlock;
    double avgIterationsPerBlock;
};

BenchmarkResult benchmarkBlockchain(HashMode mode, int numBlocks, int difficulty) {
    BenchmarkResult result;
    result.totalIterations = 0;
    
    Blockchain chain(mode);
    chain.difficulty = difficulty;
    
    auto start = high_resolution_clock::now();

    int maxTries = 200000; // Limite de sécurité
    
    for (int i = 1; i <= numBlocks; i++) {
        Block newBlock(i, chain.getLatestBlock().hash, 
                       "Transaction " + to_string(i), mode);
        newBlock.previousHash = chain.getLatestBlock().hash;
        
        string target(difficulty, '0');
        bool mined = false;

        do {
            newBlock.nonce++;
            result.totalIterations++;
            newBlock.hash = newBlock.calculateHash();

            // Critère différent pour AC_HASH (dernier char == '0')
            if (mode == AC_HASH_MODE)
                mined = (newBlock.hash.back() == '0');
            else
                mined = (newBlock.hash.substr(0, difficulty) == target);

        } while (!mined && newBlock.nonce < maxTries);
        
        chain.chain.push_back(newBlock);

        if (mined)
            cout << "  Block " << i << " mined (nonce: " << newBlock.nonce << ")" << endl;
        else
            cout << "  Block " << i << " could not be mined (max tries reached)" << endl;
    }
    
    auto end = high_resolution_clock::now();
    duration<double> elapsed = end - start;
    
    result.totalTime = elapsed.count();
    result.avgTimePerBlock = result.totalTime / numBlocks;
    result.avgIterationsPerBlock = (double)result.totalIterations / numBlocks;
    
    return result;
}

// ======================================================================
// 5. Affichage comparatif
// ======================================================================
void displayComparisonTable(const BenchmarkResult& sha256Result, 
                           const BenchmarkResult& acHashResult,
                           int numBlocks, int difficulty) {
    cout << "\n+--------------------------------------------+" << endl;
    cout << "|       COMPARISON TABLE                     |" << endl;
    cout << "+--------------------------------------------+" << endl;
    cout << "| Blocks: " << setw(2) << numBlocks << " | Difficulty: " << difficulty << "            |" << endl;
    cout << "+-------------------------------+---------+-------------+" << endl;
    cout << "| Metric                        | SHA256  | AC_HASH     |" << endl;
    cout << "+-------------------------------+---------+-------------+" << endl;
    
    cout << fixed << setprecision(4);
    
    cout << "| Total time (s)                 | " 
         << setw(7) << sha256Result.totalTime << " | " 
         << setw(11) << acHashResult.totalTime << " |" << endl;
    
    cout << "| Avg time per block (s)         | " 
         << setw(7) << sha256Result.avgTimePerBlock << " | " 
         << setw(11) << acHashResult.avgTimePerBlock << " |" << endl;
    
    cout << "+-------------------------------+---------+-------------+" << endl;
    
    cout << "| Total iterations               | " 
         << setw(7) << sha256Result.totalIterations << " | " 
         << setw(11) << acHashResult.totalIterations << " |" << endl;
    
    cout << "| Avg iterations per block       | " 
         << setw(7) << (int)sha256Result.avgIterationsPerBlock << " | " 
         << setw(11) << (int)acHashResult.avgIterationsPerBlock << " |" << endl;
    
    cout << "+-------------------------------+---------+-------------+" << endl;
    
    cout << "\nANALYSIS:" << endl;
    cout << "AC_HASH is " << setprecision(2) 
         << (acHashResult.totalTime / sha256Result.totalTime) 
         << "x slower than SHA256" << endl;
    cout << "Time difference: " << setprecision(4)
         << (acHashResult.totalTime - sha256Result.totalTime) 
         << " seconds" << endl;
    cout << "AC_HASH requires " << setprecision(2)
         << (acHashResult.avgIterationsPerBlock / sha256Result.avgIterationsPerBlock)
         << "x more iterations on average" << endl;
}

// ======================================================================
// 6. main()
// ======================================================================
int main() {
    int numBlocks = 10;
    int difficulty = 1;
    
    cout << "Configuration:" << endl;
    cout << "  - Number of blocks: " << numBlocks << endl;
    cout << "  - Difficulty: " << difficulty << " (prefix of zeros required)" << endl;
    cout << "\n----------------------------------------------\n" << endl;
    
    cout << "TEST 1: Mining with SHA256" << endl;
    cout << "----------------------------------------------" << endl;
    BenchmarkResult sha256Result = benchmarkBlockchain(SHA256_MODE, numBlocks, difficulty);
    cout << "Done in " << fixed << setprecision(4) << sha256Result.totalTime << " seconds\n" << endl;
    
    cout << "TEST 2: Mining with AC_HASH" << endl;
    cout << "----------------------------------------------" << endl;
    BenchmarkResult acHashResult = benchmarkBlockchain(AC_HASH_MODE, numBlocks, difficulty);
    cout << "Done in " << fixed << setprecision(4) << acHashResult.totalTime << " seconds\n" << endl;
    
    displayComparisonTable(sha256Result, acHashResult, numBlocks, difficulty);

    return 0;
}
