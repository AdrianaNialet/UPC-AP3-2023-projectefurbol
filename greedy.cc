//1. Ordenar els jugadors segons els punts i posició (Aquí ja eliminem aquells jugadors q no ens podem permetre)

//2. Escollir el jugador per ordre de més punts i sempre i quan no superem preu total

#include <iostream>
#include <vector>
#include <fstream>
#include <algorithm>
#include <cassert>
#include <ctime>
#include <iomanip>
#include <chrono>
#include <map>

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
    vector<vector<Player>> pos_jug;

    Tactic(uint n1, uint n2, uint n3, uint T, uint J, const vector<Player>& players) :
        npor(1), ndef(n1), nmig(n2), ndav(n3), maxPreuTotal(T), maxPreuJug(J) {
        Initialize(players);
    }

    void Initialize(const vector<Player>& players) {
        pos_jug = GetPositionedPlayers(players);
    }

    //Fem vector de vector de por, def, mig i dav respectivament
    vector<vector<Player>> GetPositionedPlayers(const vector<Player>& players) {
        vector<vector<Player>> pos_jug(4);

        for (const auto& player : players) {
            if (player.price <= maxPreuJug) {
                if (player.position == "por") {
                    pos_jug[0].push_back(player);
                }
                else if (player.position == "def") {
                    pos_jug[1].push_back(player);
                }
                
                else if (player.position == "mig") {
                    pos_jug[2].push_back(player);
                }
                else {
                    pos_jug[3].push_back(player);
                }
            }
        }

        //Ordena jugadors descendetment segons els punts
        for (auto& pos_players : pos_jug) {
            sort(pos_players.begin(), pos_players.end(), [](const Player& a, const Player& b) {
                return a.points > b.points;
            });
        }

        return pos_jug;
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
/*
void displayTeamSelection(const vector<Player>& selplayers, ofstream &output) {
    //Arupa els jugadors seleccionats per posició
    unordered_map<string, vector<string>> groupedPlayers;
    int totalPoints = 0;
    int totalPrice = 0;

    for (const auto& player : selplayers) {
        groupedPlayers[player.position].push_back(player.name);
        totalPoints += player.points;
        totalPrice += player.price;
    }

    const vector<string> positionsOrder = {"POR", "DEF", "MIG", "DAV"};
    for (const auto& pos : positionsOrder) {
        auto it = groupedPlayers.find(pos);
        if (it != groupedPlayers.end()) {
            output << it->first << ": ";
            for (size_t i = 0; i < it->second.size(); ++i) {
                output << it->second[i];
                if (i < it->second.size() - 1) {
                    output << ";";
                }
            }
            output << endl;
        }
    }

    output << "Punts: " << totalPoints << endl;
    output << "Preu: " << totalPrice << endl;
}
*/

vector<Player> greedy(const Tactic& tactic) {
    vector<Player> res;
    uint PreuReal = 0;

    for (uint pos_index = 0; pos_index < 4; ++pos_index) {
        int c = 0; //Contador de jugadors afegits en aqiuesta posició
        int i = 0; //Index que recorre jugadors d'aquesta posició

        while (c < tactic.getPlayersCount(pos_index) && i < tactic.pos_jug[pos_index].size()) {
            if (PreuReal + tactic.pos_jug[pos_index][i].price <= tactic.maxPreuTotal) {
                res.push_back(tactic.pos_jug[pos_index][i]);
                PreuReal += tactic.pos_jug[pos_index][i].price;
                c += 1;
            }
            i += 1;
        }
    }

    return res;
}

int main(int argc, char** argv) {
    if (argc != 4) {
        cout << "Syntax: " << argv[0] << " data_base.txt formation.txt output.txt" << endl;
        exit(1);
    }

    //Inici cronometre
    auto start = chrono::high_resolution_clock::now();

    //Crea vector dels jugadors
    vector<Player> players = readPlayers(argv[1]);

    //Llegeix la tactica del fitxer
    ifstream formationFile(argv[2]);
    if (!formationFile.is_open()) {
        cerr << "Error opening formation file." << endl;
        exit(1);
    }

    uint n1, n2, n3, T, J;
    formationFile >> n1 >> n2 >> n3 >> T >> J;
    formationFile.close();

    Tactic tactic(n1, n2, n3, T, J, players);

    //Algoritme greedy
    vector<Player> res = greedy(tactic);

    //Atura el cronometre
    auto stop = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::microseconds>(stop - start);
    
    double seconds = duration.count() / 1e6;

    unordered_map<string, vector<string>> groupedPlayers;
    int totalPoints = 0;
    int totalPrice = 0;

    for (const auto& player : res) {
        groupedPlayers[player.position].push_back(player.name);
        totalPoints += player.points;
        totalPrice += player.price;
    }

    //Mostra el resultat
    ofstream out(argv[3]);
    out << "Time: " << seconds << " seconds" << endl;
    
    //Agrupa els jugadors seleccionats per posició
    const vector<string> positionsOrder = {"por", "def", "mig", "dav"};
    for (const auto& pos : positionsOrder) {
        out << char(pos[0]-32) << char(pos[1]-32) << char(pos[2]-32) << ": ";
        for (const auto& player : res) {
            if (player.position == pos) {
                out << player.name << ";";
            }
        }
        out << endl;
    }

    out << "Punts: " << totalPoints << endl;
    out << "Preu: " << totalPrice << endl;
    out.close();
    return 0;
}

