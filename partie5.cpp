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

// ===========================================================
// ============ FONCTION AC_HASH =============================
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

// ===========================================================
// ============ PARTIE 5 : EFFET AVALANCHE ===================
// ===========================================================

// Convertir une chaîne hexadécimale en bits
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

// Compter le nombre de bits différents entre deux hashes
int countDifferentBits(const string& hash1, const string& hash2) {
    vector<int> bits1 = hexToBits(hash1);
    vector<int> bits2 = hexToBits(hash2);

    int count = 0;
    int minSize = min(bits1.size(), bits2.size());

    for (int i = 0; i < minSize; ++i) {
        if (bits1[i] != bits2[i])
            count++;
    }

    return count;
}

// Inverser un bit dans une chaîne (à une position donnée en bits)
string flipBitInString(const string& input, int bitPosition) {
    string result = input;
    int bytePos = bitPosition / 8;
    int bitInByte = 7 - (bitPosition % 8);

    if (bytePos < (int)result.length()) {
        result[bytePos] ^= (1 << bitInByte);
    }

    return result;
}

// 5.1 - Analyse de l'effet avalanche
void analyzeAvalancheEffect(int numTests = 100) {
    cout << "  " << endl;
    cout << "       PARTIE 5 : ANALYSE DE L'EFFET AVALANCHE            " << endl;
    cout << " \n" << endl;

    cout << "Configuration :" << endl;
    cout << "  - Nombre de tests : " << numTests << endl;
    cout << "  - Taille du hash : 256 bits (64 caractères hexadécimaux)" << endl;
    cout << "  - Méthode : Modifier 1 bit aléatoire dans le message d'entrée\n" << endl;

    cout << " \n" << endl;

    vector<double> percentages;
    vector<int> bitCounts;

    srand(time(nullptr));

    // Exemples détaillés pour les 5 premiers tests
    cout << "EXEMPLES DÉTAILLÉS (5 premiers tests) :\n" << endl;

    for (int test = 0; test < numTests; ++test) {
        // Créer un message de test
        stringstream ss;
        ss << "Message test numero " << test << " pour effet avalanche";
        string message = ss.str();

        // Calculer le hash original
        string hash1 = ac_hash(message, 30, 20);

        // Modifier un seul bit aléatoire
        int bitToFlip = rand() % (message.length() * 8);
        string modifiedMessage = flipBitInString(message, bitToFlip);

        // Calculer le hash modifié
        string hash2 = ac_hash(modifiedMessage, 30, 20);

        // Compter les bits différents
        int differentBits = countDifferentBits(hash1, hash2);
        double percentage = (differentBits * 100.0) / 256.0;

        percentages.push_back(percentage);
        bitCounts.push_back(differentBits);

        // Afficher les 5 premiers tests en détail
        if (test < 5) {
            cout << "Test #" << (test + 1) << " :" << endl;
            cout << "  Message original  : \"" << message.substr(0, 40) << "...\"" << endl;
            cout << "  Bit modifie      : position " << bitToFlip << endl;
            cout << "  Hash original     : " << hash1.substr(0, 20) << "..." << endl;
            cout << "  Hash modifie      : " << hash2.substr(0, 20) << "..." << endl;
            cout << "  Bits differents   : " << differentBits << " / 256" << endl;
            cout << "  Pourcentage       : " << fixed << setprecision(2) << percentage << " %\n" << endl;
        }
    }

    // Calculer les statistiques
    double sum = 0;
    int minBits = bitCounts[0];
    int maxBits = bitCounts[0];

    for (int i = 0; i < numTests; ++i) {
        sum += percentages[i];
        if (bitCounts[i] < minBits) minBits = bitCounts[i];
        if (bitCounts[i] > maxBits) maxBits = bitCounts[i];
    }

    double average = sum / numTests;

    // Calculer l'écart-type
    double variance = 0;
    for (int i = 0; i < numTests; ++i) {
        variance += pow(percentages[i] - average, 2);
    }
    double stdDev = sqrt(variance / numTests);

    // 5.2 - Affichage des résultats numériques

    cout << "ReSULTATS STATISTIQUES (sur " << numTests << " tests) :" << endl;



    cout << "             RESULTATS EFFET AVALANCHE (5.1)              " << endl;
    cout << "Pourcentage MOYEN de bits differents  : " << fixed << setprecision(2)
         << setw(6) << average << " %       " << endl;
    cout << " Ecart-type                             : " << setw(6) << stdDev << " %       " << endl;
    cout << " Minimum de bits differents             : " << setw(3) << minBits
         << " bits      " << endl;
    cout << " Maximum de bits differents             : " << setw(3) << maxBits
         << " bits      " << endl;
    cout << " " << endl;
    cout << " Pourcentage ideal (effet avalanche)   :  50.00 %       " << endl;
    cout << " " << endl;

    // Interprétation

    cout << "INTERPRETATION :" << endl;


    if (average >= 45.0 && average <= 55.0) {
        cout << " EXCELLENT : effet avalanche est très bon !" << endl;
        cout << "  Le changement d'un seul bit modifie environ 50% du hash." << endl;
    } else if (average >= 40.0 && average <= 60.0) {
        cout << "✓ BON : effet avalanche est acceptable." << endl;
        cout << "  Le hash présente une bonne diffusion." << endl;
    } else {
        cout << "✗ FAIBLE : L'effet avalanche est insuffisant." << endl;
        cout << "  Le hash ne diffuse pas assez les changements." << endl;
    }

    cout << "\nUn bon effet avalanche implique qu'en moyenne ~50% des bits" << endl;
    cout << "du hash changent lorsqu'on modifie un seul bit de l'entree." << endl;

}

// ===========================================================
// ===================== MAIN ================================
// ===========================================================

int main() {

    cout << "         PARTIE 5 - TEST EFFET AVALANCHE AC_HASH         " << endl;


    // 5.1 & 5.2 : Analyser l'effet avalanche avec 100 tests
    analyzeAvalancheEffect(100);

    return 0;
}