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
// ============ PARTIE 2 : FONCTION AC_HASH ==================
// ===========================================================

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

string ac_hash(const string& input, uint32_t rule = 30, size_t steps = 100) {
    vector<int> state = text_to_bits(input);
    for (size_t i = 0; i < steps; ++i)
        state = evolve(state, rule);

    vector<int> hash_bits(256, 0);
    for (size_t i = 0; i < 256; ++i)
        hash_bits[i] = state[i % state.size()];

    string hash_str = "";
    for (size_t i = 0; i < 256; i += 4) {
        int val = hash_bits[i]*8 + hash_bits[i+1]*4 + hash_bits[i+2]*2 + hash_bits[i+3];
        hash_str += "0123456789ABCDEF"[val];
    }

    return hash_str;
}

// ===========================================================
// ============  SHA256  ======
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
// ============ PARTIE 3 : INTÉGRATION BLOCKCHAIN ============
// ===========================================================

enum HashMode { SHA256_MODE, AC_HASH_MODE };

// ---- Classe Block ----
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
            return simpleHash(blockData); // version simplifiée
        else
            return ac_hash(blockData, 30, 100);
    }

    void mineBlock(int difficulty) {
        string target(difficulty, '0');
        do {
            nonce++;
            hash = calculateHash();
        } while (hash.substr(0, difficulty) != target);

        cout << "Bloc miné : " << hash << endl;
    }
};

// ---- Classe Blockchain ----
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

    void addBlock(Block newBlock) {
        newBlock.previousHash = getLatestBlock().hash;
        newBlock.mineBlock(difficulty);
        chain.push_back(newBlock);
    }

    bool isChainValid() {
        for (size_t i = 1; i < chain.size(); ++i) {
            const Block& current = chain[i];
            const Block& previous = chain[i-1];

            if (current.hash != current.calculateHash()) return false;
            if (current.previousHash != previous.hash) return false;
        }
        return true;
    }
};

// ===========================================================
// ===================== MAIN ================================
// ===========================================================

int main() {
    cout << "Sélectionnez le mode de hachage :\n";
    cout << "1 - SHA256 \n";
    cout << "2 - AC_HASH (automate cellulaire)\n";
    int choix;
    cin >> choix;

    HashMode mode = (choix == 2) ? AC_HASH_MODE : SHA256_MODE;

    Blockchain myChain(mode);

    cout << "Ajout du bloc 1" << endl;
    myChain.addBlock(Block(1, myChain.getLatestBlock().hash, "Transaction A -> B", mode));

    cout << "Ajout du bloc 2" << endl;
    myChain.addBlock(Block(2, myChain.getLatestBlock().hash, "Transaction C -> D", mode));

    cout << "\nBlockchain valide ? " << (myChain.isChainValid() ? "Oui " : "Non ") << endl;

    return 0;
}
