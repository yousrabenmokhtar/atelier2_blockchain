#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <iomanip>
#include <bitset>
#include <cstdint>
#include <ctime>
#include <cmath>
using namespace std;

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

string ac_hash(const string& input, uint32_t rule = 30, size_t steps = 20) {
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

vector<int> hexToBits(const string& hexStr) {
    vector<int> bits;
    for (char c : hexStr) {
        int val;
        if (c >= '0' && c <= '9') val = c - '0';
        else if (c >= 'A' && c <= 'F') val = c - 'A' + 10;
        else if (c >= 'a' && c <= 'f') val = c - 'a' + 10;
        else continue;
        
        for (int i = 3; i >= 0; --i)
            bits.push_back((val >> i) & 1);
    }
    return bits;
}

void analyzeBitDistribution(int numHashes = 400) {
    cout << "PARTIE 6 - Distribution des bits\n\n";

    int bitsPerHash = 256;
    int totalBits = numHashes * bitsPerHash;

    srand(time(nullptr));

    int countOnes = 0;
    int countZeros = 0;
    vector<int> allBits;

    for (int i = 0; i < numHashes; ++i) {
        stringstream ss;
        ss << "Message test " << i << " " << rand();
        string message = ss.str();
        
        string hash = ac_hash(message, 30, 20);
        vector<int> bits = hexToBits(hash);
        
        for (int bit : bits) {
            allBits.push_back(bit);
            if (bit == 1) countOnes++;
            else countZeros++;
        }
    }

    int totalBitsCollected = allBits.size();
    double percentageOnes = (countOnes * 100.0) / totalBitsCollected;
    double percentageZeros = (countZeros * 100.0) / totalBitsCollected;
    double deviation = abs(percentageOnes - 50.0);

    cout << "Total bits : " << totalBitsCollected << "\n";
    cout << "Bits a 1 : " << countOnes << " (" << fixed << setprecision(2) << percentageOnes << "%)\n";
    cout << "Bits a 0 : " << countZeros << " (" << fixed << setprecision(2) << percentageZeros << "%)\n";
    cout << "Deviation : " << deviation << "%\n";

    if (deviation < 1.0) cout << "EXCELLENT\n";
    else if (deviation < 2.0) cout << "BON\n";
    else if (deviation < 5.0) cout << "ACCEPTABLE\n";
    else cout << "FAIBLE\n";

    int barWidth = 50;
    int onesBar = (int)((percentageOnes / 100.0) * barWidth);
    int zerosBar = (int)((percentageZeros / 100.0) * barWidth);

    cout << "Bits a 1 : [";
    for (int i = 0; i < onesBar; ++i) cout << "#";
    for (int i = onesBar; i < barWidth; ++i) cout << " ";
    cout << "] " << percentageOnes << "%\n";

    cout << "Bits a 0 : [";
    for (int i = 0; i < zerosBar; ++i) cout << "#";
    for (int i = zerosBar; i < barWidth; ++i) cout << " ";
    cout << "] " << percentageZeros << "%\n";
}

int main() {
    analyzeBitDistribution(400);
    return 0;
}
