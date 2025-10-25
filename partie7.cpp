// test_ac_hash.cpp
// Compile: g++ -O2 -std=c++17 test_ac_hash.cpp -o test_ac_hash
// Usage: ./test_ac_hash
#include <bits/stdc++.h>
using namespace std;
using u32 = uint32_t;
using u8 = uint8_t;

// -------------------- Utilities --------------------
static inline string to_hex(const array<uint8_t,32>& b) {
    static const char* hex = "0123456789abcdef";
    string s; s.reserve(64);
    for (int i=0;i<32;i++){
        s.push_back(hex[b[i]>>4]);
        s.push_back(hex[b[i]&0xF]);
    }
    return s;
}

static inline int hamming_hex256(const string& h1, const string& h2) {
    // h1,h2 are 64-hex-chars -> 256 bits
    int diff = 0;
    for (size_t i=0;i<64;i+=2) {
        uint8_t a = (uint8_t)stoi(h1.substr(i,2), nullptr, 16);
        uint8_t b = (uint8_t)stoi(h2.substr(i,2), nullptr, 16);
        diff += __builtin_popcount((unsigned)(a^b));
    }
    return diff;
}

// -------------------- Cellular Automaton Hash --------------------
/*
 Design decisions (concise):
 - Input -> bytes (UTF-8), each byte -> 8 bits MSB-first.
 - CA: binary 1D, neighborhood r=1, periodic boundary (circular).
 - Rule provided as uint32_t 'rule' (use low 8 bits). Mapping standard:
    index = (left<<2)|(self<<1)|(right) -> rule bit at index.
 - Evolve 'steps' iterations (we do repeated steps to mix).
 - To produce 256-bit fixed output: we take the final CA state (vector<bool>)
   and fold it (xor/rotate) into 32 bytes output; if state shorter than needed
   we continue evolving and folding until 256 bits' worth of entropy have been
   folded (but folding is deterministic).
 - Output returned as 64-hex char string (256 bits).
*/
vector<uint8_t> bytes_from_string(const string& s){
    vector<uint8_t> out(s.begin(), s.end());
    return out;
}

vector<uint8_t> bits_from_bytes(const vector<uint8_t>& bytes) {
    vector<uint8_t> bits;
    bits.reserve(bytes.size()*8);
    for (uint8_t b : bytes){
        for (int i=7;i>=0;i--){
            bits.push_back((b>>i)&1);
        }
    }
    if (bits.empty()) bits.push_back(0); // avoid zero-length
    return bits;
}

vector<uint8_t> evolve_ca(const vector<uint8_t>& state, u32 rule, size_t steps){
    size_t n = state.size();
    vector<uint8_t> cur = state;
    vector<uint8_t> next(n);
    uint8_t rbyte = rule & 0xFF;
    for (size_t s=0;s<steps;s++){
        for (size_t i=0;i<n;i++){
            uint8_t left = cur[(i+n-1)%n];
            uint8_t me   = cur[i];
            uint8_t right= cur[(i+1)%n];
            uint8_t idx = (left<<2)|(me<<1)|(right);
            next[i] = (rbyte >> idx) & 1;
        }
        cur.swap(next);
    }
    return cur;
}

string ac_hash(const string& input, u32 rule, size_t steps){
    // convert to bits
    auto bytes = bytes_from_string(input);
    auto bits = bits_from_bytes(bytes); // vector<uint8_t> {0,1,...}
    // We'll perform CA and fold results into 32-byte output using XOR with rotation
    array<uint8_t,32> out{};
    out.fill(0);
    vector<uint8_t> state = bits;
    size_t folded_bits = 0;
    size_t round = 0;
    // We'll run CA multiple times until we've "seen" at least 1024*state_length bits or a cap.
    // But for determinism and speed we run (steps) once, then repeat small evolutions to gather entropy.
    // First run:
    state = evolve_ca(state, rule, steps);
    auto fold_state_into_out = [&](const vector<uint8_t>& st){
        // XOR each st bit into out byte (pos = (folded_bits + i) % 256)
        for (size_t i=0;i<st.size();++i){
            size_t pos = (folded_bits + i) % 256;
            size_t byte_idx = pos / 8;
            size_t bit_idx  = pos % 8;
            // We'll XOR with bit shifted by bit_idx (LSB = bit_idx 0)
            out[byte_idx] ^= (uint8_t)(st[i] << (bit_idx));
        }
        folded_bits += st.size();
    };
    // Fold initial state
    fold_state_into_out(state);
    // Continue a few rounds to mix more (deterministic)
    const size_t MAX_ROUNDS = 8; // small number — tunable
    for (round=1; round<MAX_ROUNDS; ++round){
        // Evolve few steps each round
        state = evolve_ca(state, rule, max<size_t>(1, steps/ (round+1)));
        fold_state_into_out(state);
    }
    // Final mixing pass: XOR with rule+steps metadata to avoid trivial collisions
    for (int i=0;i<32;i++){
        out[i] ^= (uint8_t)((rule>>((i%4)*8)) ^ (uint8_t)(steps & 0xFF) ^ (uint8_t)i);
    }
    return to_hex(out);
}

// -------------------- Tests for part 7 --------------------
int main(){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    vector<u32> rules = {30u, 90u, 110u};
    const string sample = "The quick brown fox jumps over the lazy dog";
    const int N_RUNS = 40; // average over several runs
    const int N_SENS = 128; // number of single-bit flip samples to estimate Hamming

    struct Result {
        u32 rule;
        double avg_ms;
        bool deterministic;
        double avg_hamming; // bits
        string sample_hash;
    };
    vector<Result> results;

    cout << "=== AC_HASH rule comparison (Rule 30,90,110) ===\n";
    cout << "Sample input: \"" << sample << "\"\n\n";

    for (u32 rule : rules) {
        // 1) Determinism & timing
        vector<string> hashes;
        hashes.reserve(N_RUNS);
        vector<double> times;
        times.reserve(N_RUNS);
        for (int i=0;i<N_RUNS;i++){
            auto t0 = chrono::high_resolution_clock::now();
            string h = ac_hash(sample, rule, 64);
            auto t1 = chrono::high_resolution_clock::now();
            double ms = chrono::duration<double, milli>(t1-t0).count();
            hashes.push_back(h);
            times.push_back(ms);
        }
        // average time
        double avg = accumulate(times.begin(), times.end(), 0.0) / times.size();
        // determinism: check all hashes equal
        bool det = true;
        for (size_t i=1;i<hashes.size();++i) if (hashes[i]!=hashes[0]) { det=false; break; }
        string sample_hash = hashes.front();

        // 2) Sensitivity (avalanche-like) : flip single bit in input and test Hamming
        // We'll flip one bit at random positions across several samples to get average
        std::mt19937_64 rng(123456 + rule);
        uniform_int_distribution<size_t> dist_pos(0, sample.size()*8 ? sample.size()*8 - 1 : 0);
        double total_ham = 0.0;
        for (int s=0;s<N_SENS;s++){
            // create a copy of sample but with one bit flipped
            string modified = sample;
            size_t bitpos = dist_pos(rng);
            size_t bytepos = bitpos / 8;
            size_t bpos = 7 - (bitpos % 8); // MSB-first encoding in our scheme
            if (bytepos >= modified.size()){
                // unlikely, but ensure safe
                bytepos = modified.size() - 1;
            }
            modified[bytepos] = modified[bytepos] ^ (char(1<<bpos));
            string h1 = sample_hash;
            string h2 = ac_hash(modified, rule, 64);
            int hd = hamming_hex256(h1,h2);
            total_ham += hd;
        }
        double avg_ham = total_ham / N_SENS; // bits out of 256
        results.push_back({rule, avg, det, avg_ham, sample_hash});
    }

    // Print a table
    cout << left;
    cout << setw(8) << "Rule" << setw(16) << "Avg time (ms)" << setw(14) << "Deterministic" << setw(20) << "Avg Hamming (bits)" << "Sample hash\n";
    cout << string(110,'-') << "\n";
    for (auto &r : results) {
        cout << setw(8) << r.rule
             << setw(16) << fixed << setprecision(3) << r.avg_ms
             << setw(14) << (r.deterministic ? "yes" : "NO")
             << setw(20) << fixed << setprecision(3) << r.avg_hamming
             << r.sample_hash << "\n";
    }
    cout << "\nNotes:\n";
    cout << "- Hamming is measured in absolute bits (out of 256). 256 bits -> 100% difference.\n";
    cout << "- Deterministic must be 'yes' for cryptographic reproducibility.\n\n";

    // Simple recommendation heuristic:
    // choose rule with highest avg_hamming (closer to 128 bits) while keeping reasonable speed and determinism.
    double best_score = -1e18;
    u32 best_rule = 0;
    for (auto &r : results) {
        // score = closeness to 128 (ideal avalanche) minus small penalty for time
        double avalanche_score = -fabs(r.avg_hamming - 128.0); // bigger when closer to 128
        double speed_penalty = r.avg_ms * 0.1; // tuneable weight
        double score = avalanche_score - speed_penalty;
        if (score > best_score) { best_score = score; best_rule = r.rule; }
    }

    cout << "Recommendation: rule " << best_rule << " seems most suitable according to this simple test (balance of avalanche closeness and speed).\n\n";

    cout << "Detailed outputs (sample hashes):\n";
    for (auto &r : results){
        cout << "Rule " << r.rule << " -> hash: " << r.sample_hash << " | avg_time=" << fixed << setprecision(3) << r.avg_ms << "ms | avg_hamming=" << r.avg_hamming << "\n";
    }

    cout << "\nDone.\n";
    return 0;
}
