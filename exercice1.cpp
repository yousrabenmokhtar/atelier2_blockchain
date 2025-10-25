#include <iostream>
#include <vector>
using namespace std;

// 1.1. Initialiser l'état à partir d'un vecteur de bits
vector<int> init_state(const vector<int>& initial_bits) {
    return initial_bits;
}

// 1.2. Appliquer la règle de transition (exemple Rule 30)
int rule30(int left, int center, int right) {
    int index = left * 4 + center * 2 + right;
    int rule[8] = {0, 1, 1, 1, 1, 0, 0, 0};
    return rule[index];
}

// Fonction evolve pour calculer la génération suivante
vector<int> evolve(const vector<int>& state) {
    int n = state.size();
    vector<int> next_state(n, 0);

    for (int i = 0; i < n; i++) {
        int left   = (i == 0) ? 0 : state[i - 1];
        int center = state[i];
        int right  = (i == n - 1) ? 0 : state[i + 1];

        next_state[i] = rule30(left, center, right);
    }

    return next_state;
}

// 1.3. Vérification avec un petit état initial
int main() {
    vector<int> state = init_state({0,0,0,0,0,1,0,0,0,0,0});
    state = evolve(state);

    // Afficher le nouvel état
    for(int cell : state)
        cout << cell << " ";
    cout << endl;

    return 0;
}
