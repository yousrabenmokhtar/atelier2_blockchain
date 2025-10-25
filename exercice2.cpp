#include <iostream>
#include <vector>
#include <string>
#include <bitset>
#include <cstdint>

using namespace std;

// -------------------- Automate cellulaire --------------------
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

// -------------------- Conversion texte -> bits --------------------
vector<int> text_to_bits(const string& input) {
    vector<int> bits;
    for (char c : input) {
        bitset<8> b(c); // chaque caractère en ASCII → 8 bits
        for (int i = 7; i >= 0; --i) {
            bits.push_back(b[i]);
        }
    }
    return bits;
}

// -------------------- Fonction de hachage AC --------------------
string ac_hash(const string& input, uint32_t rule, size_t steps) {
    // 1. Convertir texte en bits
    vector<int> state = text_to_bits(input);

    // 2. Appliquer l'automate cellulaire pendant `steps` générations
    for (size_t i = 0; i < steps; ++i) {
        state = evolve(state, rule);
    }

    // 3. Produire un hash fixe de 256 bits
    vector<int> hash_bits(256, 0);
    for (size_t i = 0; i < 256; ++i) {
        hash_bits[i] = state[i % state.size()]; // "mélange circulaire"
    }

    // 4. Convertir en string hexadécimale
    string hash_str = "";
    for (size_t i = 0; i < 256; i += 4) {
        int val = hash_bits[i]*8 + hash_bits[i+1]*4 + hash_bits[i+2]*2 + hash_bits[i+3];
        hash_str += "0123456789ABCDEF"[val];
    }

    return hash_str;
}

// -------------------- 2.4.un test que deux entrées différentes --------------------
int main() {
    string input1 = "Bonjour";
    string input2 = "Bonsoir";

    string hash1 = ac_hash(input1, 30, 100);  // Rule 30, 100 générations
    string hash2 = ac_hash(input2, 30, 100);

    cout << "Hash 1 : " << hash1 << endl;
    cout << "Hash 2 : " << hash2 << endl;

    if (hash1 != hash2) {
        cout << "Test OK : deux entrées différentes donnent deux sorties différentes." << endl;
    } else {
        cout << "Erreur : collision !" << endl;
    }

    return 0;
}

/*------------------Étape 2.2 — Conversion du texte d’entrée en bits:-------------------------------------------

Chaque caractère du texte est transformé en son code ASCII sur 8 bits.
par exemple si on prend 'A': son code ASCII EST 65,ce qui donne en binaire 01000001
donc on ajoute ces bits au vecteur final: 
"AB" → [0,1,0,0,0,0,0,1,  0,1,0,0,0,0,1,0] On obtient donc un vecteur binaire initial pour notre automate.

---------------------Étape 2.3 — Production du hash final fixe de 256 bits---------------------------------------

On fait évoluer l’automate sur steps générations avec la règle rule (par ex. Rule 30).
On prend le dernier état obtenu et on le ramène à 256 bits fixes :
Si la longueur < 256 → on répète les bits et Si > 256 → on tronque.
puis On regroupe les bits 4 par 4 → on convertit chaque groupe en hexadécimal (0 à f).
Cela donne une chaîne de 64 caractères hexadécimaux = 256 bits.

*/