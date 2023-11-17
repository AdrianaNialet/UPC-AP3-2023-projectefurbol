# FANTASY FOOTBALL

Implementar cerca exhaustiva, greedy i metaheurística per a trobar la solució més òptima.

## Introducció

Des de principis dels 90, diverses entitats relacionades amb el món de l’esport han estat organitzant competicions de Fantasy Football, cadascuna amb les seves petites diferències. Grosso modo, el joc té com a referència un competició futbolística real. Els participants del joc han de confeccionar un equip amb jugadors reals, que aniran rebent més o menys punts al llarg de la competició d’acord amb el seu rendiment en aquesta. Al final de la temporada, guanyarà el participant que hagi obtingut el major nombre de punts.

En aquest projecte se us demana implementar un programa de suport per a jugar a Fantasy Football. A tal fi disposem de la llista de tots els jugadors de la Lliga de Futbol Professional d’Espanya. Per cada jugador coneixem el seu nom, la posició, el preu, l’equip al qual pertany i la puntuació que va obtenir l’any anterior. L’objectiu del vostre programa és formar l’equip que maximitzi el nombre de punts que va aconseguir l’any anterior. No obstant, no podem escollir tots els jugadors que vulguem, ja que tenim un límit total de T euros i també un límit de J euros per cada jugador que escollim. Finalment, la tàctica a desplegar ens ve donada per tres enters N1, N2 i N3, que indiquen que hem d’escollir N1 defenses, N2 migcampistes i N3 davanters, on 1 ≤ N1, N2, N3 ≤ 5 i N1 + N2 + N3 = 10. Com és d’esperar, a més a més, sempre hem d’alinear un porter.

Les dades d’entrada per al vostre programa s’organitzen en dos fitxers, un amb la base de dades de jugadors, i un altre arxiu de consulta amb les restriccions de pressupost i la tàctica desitjada:

## ALGORISME GREEDY

*Molt ràpid però no acostuma a ser la resposta més òptima.*

```cpp
//1. Ordenar els jugadors segons els punts i posició (Aquí ja eliminem aquells jugadors q no ens podem permetre)

//2. Escollir el jugador per ordre de més punts i sempre i quan no superem preu total

#include <iostream>
#include <vector>
#include <fstream>
#include <algorithm>
#include <cassert>
#include <ctime>
#include <iomanip>

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
        vector<vector<Player>> pos_jug(3);

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

vector<Player> greedy(const Tactic& tactic) {
    vector<Player> res;
    uint PreuReal = 0;

    for (uint pos_index = 0; pos_index < 3; ++pos_index) {
        int c = 0; // Contador de jugadors afegits en aqiuesta posició
        int i = 0; // Index que recorre jugadors d'aquesta posició

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
    if (argc != 2) {
        cout << "Syntax: " << argv[0] << " data_base.txt" << endl;
        exit(1);
    }

    // Crear vector jugadors
    vector<Player> players = readPlayers(argv[1]);

    // Llegir i definir tàctica
    uint n1, n2, n3, T, J;
    cin >> n1 >> n2 >> n3 >> T >> J;
    Tactic tactic(n1, n2, n3, T, J, players);

    // Fer greedy
    vector<Player> res = greedy(tactic);

    // Imprimir el resultat
    for (const auto& player : res) {
        cout << player.name << "(id " << player.id << ") price = " << player.price << " points = " << player.points << endl;
    }
}```
