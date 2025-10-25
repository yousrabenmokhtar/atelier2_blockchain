#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <iomanip>
#include <bitset>
#include <cstdint>
#include <ctime>
using namespace std;

// ===========================================================
// ============ AUTOMATE CELLULAIRE : RULEX HASH =============
// ===========================================================

int apply_rule(uint32_t rule, int left, int center, int right) {
    int index = left * 4 + center * 2 + right; // binaire -> index
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

string ac_hash(const string& input, uint32_t rule = 30, size_t steps = 128) {
    vector<int> state = text_to_bits(input);

    // évolution répétée
    for (size_t i = 0; i < steps; ++i)
        state = evolve(state, rule);

    // on prend 256 bits pour produire un hash hex
    vector<int> hash_bits(256, 0);
    for (size_t i = 0; i < 256; ++i)
        hash_bits[i] = state[i % state.size()];

    // conversion 4 bits -> hex
    string hash_str = "";
    for (size_t i = 0; i < 256; i += 4) {
        int val = hash_bits[i]*8 + hash_bits[i+1]*4 +
                  hash_bits[i+2]*2 + hash_bits[i+3];
        hash_str += "0123456789ABCDEF"[val];
    }

    return hash_str;
}

// ===========================================================
// ============ SIMPLE HASH (remplace SHA256) ================
// ===========================================================
string simpleHash(const string &data) {
    unsigned int hash = 0;
    for (char c : data)
        hash = (hash * 101 + c) % 1000000007;

    stringstream ss;
    ss << hex << setw(8) << setfill('0') << hash;
    return ss.str();
}

// ===========================================================
// ==================== BLOCKCHAIN ===========================
// ===========================================================

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
    uint32_t rule;

    Block(int idx, string prev, string d, HashMode m, uint32_t r)
        : index(idx), previousHash(prev), data(d), mode(m), rule(r), nonce(0) {
        timestamp = time(nullptr);
        hash = calculateHash();
    }

    string calculateHash() const {
        stringstream ss;
        ss << index << previousHash << timestamp << data << nonce;
        return ss.str(); // pas de hash ici !
    }

    void mineBlock(int difficulty) {
        string target(difficulty, '0');
        string blockData;
        time_t start = clock();

        do {
            nonce++;
            stringstream ss;
            ss << index << previousHash << timestamp << data << nonce;
            blockData = ss.str();

            if (mode == AC_HASH_MODE)
                hash = ac_hash(blockData, rule, 128);
            else
                hash = simpleHash(blockData);

        } while (hash.substr(0, difficulty) != target);

        time_t end = clock();

        cout << "Bloc mine: " << hash << endl;
        cout << "Temps de minage : "
             << (double)(end - start) / CLOCKS_PER_SEC
             << " secondes" << endl;
    }
};

class Blockchain {
public:
    vector<Block> chain;
    int difficulty;
    HashMode mode;
    uint32_t rule;

    Blockchain(HashMode m = SHA256_MODE, uint32_t r = 30)
        : mode(m), difficulty(4), rule(r) {
        chain.push_back(createGenesisBlock());
    }

    Block createGenesisBlock() {
        return Block(0, "0", "Genesis Block", mode, rule);
    }

    Block getLatestBlock() const {
        return chain.back();
    }

    void addBlock(Block newBlock) {
        newBlock.previousHash = getLatestBlock().hash;
        newBlock.mineBlock(difficulty);
        chain.push_back(newBlock);
    }

    bool isChainValid() {
        for (size_t i = 1; i < chain.size(); ++i) {
            const Block& current = chain[i];
            const Block& previous = chain[i-1];

            if (current.previousHash != previous.hash) return false;
        }
        return true;
    }
};

// ===========================================================
// ========================= MAIN ============================
// ===========================================================

int main() {
    cout << "=== Blockchain avec Automates Cellulaires ===\n";
    cout << "1 - Hash simple \n";
    cout << "2 - AC_HASH (Automate Cellulaire Rule X)\n";
    
    int choix;
    cin >> choix;

    int rule = 30;
    if (choix == 2) {
        cout << "Choisir une regle AC (ex: 30, 90, 110) : ";
        cin >> rule;
    }

    HashMode mode = (choix == 2) ? AC_HASH_MODE : SHA256_MODE;
    Blockchain myChain(mode, rule);

    cout << "\nAjout du bloc 1..." << endl;
    myChain.addBlock(Block(1, myChain.getLatestBlock().hash, "A -> B", mode, rule));

    cout << "\nAjout du bloc 2..." << endl;
    myChain.addBlock(Block(2, myChain.getLatestBlock().hash, "C -> D", mode, rule));

    cout << "\nBlockchain valide ? "
         << (myChain.isChainValid() ? "Oui" : "Non")
         << endl;

    return 0;
}
