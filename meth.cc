#include <iostream>
#include <vector>
#include <fstream>
#include <algorithm>
#include <ctime>
#include <iomanip>
#include <chrono>
#include <unordered_map>
#include <unordered_set>
#include <tuple>

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

    uint getPlayersCount(uint pos_index) const {
        if (pos_index == 0) return npor;
        else if (pos_index == 1) return ndef;
        else if (pos_index == 2) return nmig;
        else if (pos_index == 3) return ndav;
        else return 0;
    }
};

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

tuple<int, int> calculateTotalPointsAndPrice(const vector<Player>& team) {
    int totalPoints = 0;
    int totalPrice = 0;

    for (const Player& player : team) {
        totalPoints += player.points;
        totalPrice += player.price;
    }

    return make_tuple(totalPoints, totalPrice);
}


vector<Player> localSearch(const Tactic& tactic, const vector<Player>& currentSolution) {
    cout << "Entering localSearch function" << endl;
    vector<Player> bestSolution = currentSolution;
    
    tuple<int, int> bestTotals = calculateTotalPointsAndPrice(bestSolution);
    int bestTotalPoints = get<0>(bestTotals);
    int bestTotalPrice = get<1>(bestTotals);

    // Iterate through each position
    for (size_t i = 0; i < tactic.pos_jug.size(); ++i) {
        // Check if the position has reached the required number of players
        if (currentSolution.size() < tactic.getPlayersCount(i)) {
            // Iterate through each player in the current position
            for (size_t j = 0; j < tactic.pos_jug[i].size(); ++j) {
                cout << "Considering player " << tactic.pos_jug[i][j].name << " for position " << i << endl;

                vector<Player> newSolution = currentSolution;
                newSolution.push_back(tactic.pos_jug[i][j]);

                tuple<int, int> newTotals = calculateTotalPointsAndPrice(newSolution);
                int newTotalPoints = get<0>(newTotals);
                int newTotalPrice = get<1>(newTotals);

                // Print debug statements
                cout << "New total points: " << newTotalPoints << ", New total price: " << newTotalPrice << endl;

                // Check if the new solution is better and within budget constraints
                if (newTotalPoints > bestTotalPoints && newTotalPrice <= tactic.maxPreuTotal) {
                    bestSolution = newSolution;
                    bestTotalPoints = newTotalPoints;
                    bestTotalPrice = newTotalPrice; // Update bestTotalPrice

                    // Print debug statement
                    cout << "Updated best solution: " << endl;
                    for (const auto& player : bestSolution) {
                        cout << "Player " << player.name << " at position " << i << endl;
                    }
                }
            }
        }
    }

    cout << "Exiting localSearch function" << endl;

    return bestSolution;
}

/*
vector<Player> localSearch(const Tactic& tactic, const vector<Player>& currentSolution) {
    cout << "Entering localSearch function" << endl;
    vector<Player> bestSolution = currentSolution;

    tuple<int, int> bestTotals = calculateTotalPointsAndPrice(bestSolution);
    int bestTotalPoints = get<0>(bestTotals);
    int bestTotalPrice = get<1>(bestTotals);

    // Iterate through each player in the current solution
    for (size_t i = 0; i < tactic.pos_jug.size(); ++i) {
        // Check if the position has reached the required number of players
        if (currentSolution.size() < tactic.getPlayersCount(i)) {
            // Select candidates that satisfy budget constraints
            vector<Player> candidates;
            for (const auto& candidate : tactic.pos_jug[i]) {
                int newTotalPrice = bestTotalPrice - currentSolution[i].price + candidate.price;
                if (newTotalPrice <= tactic.maxPreuTotal) {
                    candidates.push_back(candidate);
                }
            }

            // Sort candidates by points in descending order
            sort(candidates.begin(), candidates.end(), [](const Player& a, const Player& b) {
                return a.points > b.points;
            });

            // Consider the best candidate
            if (!candidates.empty()) {
                vector<Player> newSolution = currentSolution;
                newSolution[i] = candidates[0];

                tuple<int, int> newTotals = calculateTotalPointsAndPrice(newSolution);
                int newTotalPoints = get<0>(newTotals);
                int newTotalPrice = get<1>(newTotals);

                // Check if the new solution is better and within budget constraints
                if (newTotalPoints > bestTotalPoints && newTotalPrice <= tactic.maxPreuTotal) {
                    bestSolution = newSolution;
                    bestTotalPoints = newTotalPoints;
                    bestTotalPrice = newTotalPrice; // Update bestTotalPrice
                }
            }
        }
    }

    cout << "Exiting localSearch function" << endl;

    return bestSolution;
}

*/




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

    // Initialize initialSolution with players from each position
    vector<Player> initialSolution;

    // Add one goalkeeper
    initialSolution.push_back(tactic.pos_jug[0][0]);

    // Remaining players for other positions
    for (size_t i = 1; i < tactic.pos_jug.size(); ++i) {
        for (size_t j = 0; j < tactic.getPlayersCount(i); ++j) {
            // Check if adding the player satisfies the budget constraints
            if (initialSolution[0].price + tactic.pos_jug[i][j].price <= tactic.maxPreuTotal) {
                initialSolution.push_back(tactic.pos_jug[i][j]);
                if (initialSolution.size() == tactic.getPlayersCount(i)) {
                    break;  // Move to the next position after adding required number of players
                }
            }
        }
    }


    //Local Search
    vector<Player> bestSolution = localSearch(tactic, initialSolution);
    
    //Atura el cronometre
    auto stop = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::microseconds>(stop - start);
    
    double seconds = duration.count() / 1e6;

    unordered_map<string, vector<string>> groupedPlayers;
    int totalPoints = 0;
    int totalPrice = 0;

    for (const auto& player : bestSolution) {
        groupedPlayers[player.position].push_back(player.name);
        totalPoints += player.points;
        totalPrice += player.price;
    }

    //Mostra el resultat
    ofstream out(argv[3]);
    out << "Time: " << seconds << " seconds" << endl;
    
    const vector<string> positionsOrder = {"por", "def", "mig", "dav"};
    for (const auto& pos : positionsOrder) {
        out << char(pos[0]-32) << char(pos[1]-32) << char(pos[2]-32) << ": "; //converteix les minúscules en majúscules
        for (const auto& player : bestSolution) {
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