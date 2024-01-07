//1. Ordenar els jugadors segons els punts i posició (Aquí ja eliminem aquells jugadors que no ens podem permetre)

//2. Escollir el jugador per ordre de més punts i sempre i quan no superem preu total

#include <iostream>
#include <vector>
#include <fstream>
#include <algorithm>
#include <cassert>
#include <ctime>
#include <iomanip>
#include <string>

using namespace std;

class Player {
public:
    int id;
    string name;
    string position;
    int price;
    string club;
    int points;

    Player(int ident, const string& n, const string& pos, int pr, const string& c, int p) :
        id(ident), name(n), position(pos), price(pr), club(c), points(p) {}
};

class Tactic {
public:
    uint npor;
    uint ndef;
    uint nmig;
    uint ndav;
    uint maxPreuTotal;
    uint maxPreuJug;
    vector<Player> pos_jug;

    Tactic(uint n1, uint n2, uint n3, uint T, uint J, const vector<Player>& players) :
        npor(1), ndef(n1), nmig(n2), ndav(n3), maxPreuTotal(T), maxPreuJug(J) {
        Initialize(players);
    }

    void Initialize(const vector<Player>& players) {
        pos_jug = GetPositionedPlayers(players);
    }

    //Fem vector de vector de por, def, mig i dav respectivament
    vector<Player> GetPositionedPlayers(const vector<Player>& players) {

        // Ordena els jugadors descendentment segons els punts i preu
        for (const auto& player : players) {
            if (player.price <= maxPreuJug && ((player.points > 0) || (player.points == 0 && player.price == 0))) {
                pos_jug.push_back(player);
            }
        }

        sort(pos_jug.begin(), pos_jug.end(), [](const Player& a, const Player& b) {
            if (a.points == b.points) return a.price < b.price;
            return a.points > b.points;
        });

        return pos_jug;
    }

    uint getPlayersPosition(string pos_index) const {
        if (pos_index == "por") return 0;
        else if (pos_index == "def") return 1;
        else if (pos_index == "mig") return 2;
        else if (pos_index == "dav") return 3;
    }

    //Funció per saber el nx que li pertoca a cada posició. pos=0 n1; pos=1  n2; pos=2 n3.
    uint getPlayersCount(uint pos_index) const {
        if (pos_index == 0) return npor;
        else if (pos_index == 1) return ndef;
        else if (pos_index == 2) return nmig;
        else if (pos_index == 3) return ndav;
        else return 0;
    }
};

//Crear vector dels jugadors
vector<Player> readPlayers(const string& filename) {
    vector<Player> players;
    ifstream in(filename);

    while (!in.eof()) {
        string name, position, team;
        int price, points;
        getline(in, name, ';');
        if (name == "") break;
        getline(in, position, ';');
        in >> price;
        char aux; in >> aux;
        getline(in, team, ';');
        in >> points;
        string aux2;
        getline(in, aux2);

        players.push_back(Player(players.size(), name, position, price, team, points));
    }

    in.close();
    return players;
}

Tactic readTactic(const string& filename, const vector<Player>& players) {
    ifstream in(filename);
        uint n1, n2, n3, T, J;
        in >> n1 >> n2 >> n3 >> T >> J;
        Tactic tactic(n1, n2, n3, T, J, players);

    in.close();
    return tactic;

}


vector<Player> greedy(const Tactic& tactic) {
    vector<Player> res;
    uint PreuReal = 0;
    vector<uint> players_count(4, 0);  // Count of players for each position (0 for por, 1 for def, 2 for mig, 3 for dav)

    int i = 0; // Index that iterates over players
    cout << tactic.pos_jug.size() << endl;
    while (i < tactic.pos_jug.size()) {
        uint positionIndex = tactic.getPlayersPosition(tactic.pos_jug[i].position);

        if (players_count[positionIndex] < tactic.getPlayersCount(positionIndex) && PreuReal + tactic.pos_jug[i].price <= tactic.maxPreuTotal) {
            res.push_back(tactic.pos_jug[i]);
            PreuReal += tactic.pos_jug[i].price;
            players_count[positionIndex] += 1;

            if (res.size() == 11) {
                return res;
            }
        }
        i += 1;
    }
    return res;
}


int main(int argc, char** argv) {
    if (argc != 3) {
        cout << "Sintaxi: " << argv[0] << " data_base.txt" << endl;
        exit(1);
    }

    // Crear vector jugadors
    vector<Player> players = readPlayers(argv[1]);

    // Llegir i definir tàctica
    Tactic tactic = readTactic(argv[2], players);

    // Fer greedy
    vector<Player> res = greedy(tactic);

     for (const auto& player : tactic.pos_jug) {
        cout << player.name << "(id " << player.id << ") preu = " << player.price << " punts = " << player.points << player.position << endl;
    }

    // Imprimir el resultat
    int preu = 0;
    int punts = 0;
    for (const auto& player : res) {
        cout << player.name << "(id " << player.id << ") preu = " << player.price << " punts = " << player.points << player.position << endl;
        preu += player.price;
        punts += player.points;
    }
    cout << preu << endl;
    cout << punts << endl;
}
