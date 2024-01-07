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
#include <random>
#include <iterator>
#include <cmath>

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

    vector<vector<Player>> GetPositionedPlayers(const vector<Player>& players) {
        vector<vector<Player>> pos_jug(4);

        for (auto& player : players) {
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

vector<Player> grasp(const Tactic& tactic, int maxIterations);

vector<Player> constructGreedySolution(const Tactic& tactic, std::default_random_engine& rng);

vector<Player> localSearch(const Tactic& tactic, vector<Player>& currentSolution);

vector<Player> grasp(const Tactic& tactic, int maxIterations) {
    vector<Player> bestSolution;

    random_device rd;
    default_random_engine rng(rd());

    for (int iteration = 0; iteration < maxIterations; ++iteration) {
        vector<Player> currentSolution = constructGreedySolution(tactic, rng);
        currentSolution = localSearch(tactic, currentSolution);

        tuple<int, int> currentTotals = calculateTotalPointsAndPrice(currentSolution);
        int currentTotalPoints = get<0>(currentTotals);
        int currentTotalPrice = get<1>(currentTotals);

        tuple<int, int> bestTotals = calculateTotalPointsAndPrice(bestSolution);
        int bestTotalPoints = get<0>(bestTotals);

        // Introduce a temperature parameter for the probabilistic acceptance criterion
        double temperature = 1.0; // You can experiment with different values

        // Calculate the probability of accepting a worse solution
        double acceptanceProbability = exp((currentTotalPoints - bestTotalPoints) / temperature);

        // Accept the new solution with the calculated probability
        uniform_real_distribution<double> distribution(0.0, 1.0);
        if (distribution(rng) < acceptanceProbability || (currentTotalPoints > bestTotalPoints && currentTotalPrice <= tactic.maxPreuTotal)) {
            bestSolution = currentSolution;
        }
    }

    return bestSolution;
}

vector<Player> constructGreedySolution(const Tactic& tactic, default_random_engine& rng) {
    vector<Player> solution;

    for (size_t i = 0; i < tactic.pos_jug.size(); ++i) {
        // Shuffle the order of players for the current position
        vector<Player> randomizedPlayers = tactic.pos_jug[i];
        shuffle(randomizedPlayers.begin(), randomizedPlayers.end(), rng);

        // Add players that satisfy the budget constraint and fill the required number for the position
        int n = 0; // Number of players
        for (const Player& player : randomizedPlayers) {
            tuple<int, int> pointsAndPrice = calculateTotalPointsAndPrice(solution);
            int priceSolution = get<1>(pointsAndPrice);

            // Check if adding the player satisfies the budget constraints and if the player's position is not already filled with the required number
            if (count_if(solution.begin(), solution.end(), [&player](const Player& p) {
                    return p.position == player.position;
                }) < tactic.getPlayersCount(i) && priceSolution + player.price <= tactic.maxPreuTotal && player.price <= tactic.maxPreuJug) {
                
                cout << tactic.getPlayersCount(i) << endl; // Number of players required for the position
                solution.push_back(player);
                n++;
            }

            // Check if we have added enough players for this position
            if (n == tactic.getPlayersCount(i)) {
                break;
            }
        }
    }

    return solution;
}




vector<Player> localSearchRecursive(const Tactic& tactic, vector<Player>& currentSolution, size_t positionIndex, int currentTotalPoints, int currentTotalPrice, vector<Player>& bestSolution, int& bestTotalPoints, int& bestTotalPrice);

vector<Player> localSearch(const Tactic& tactic, vector<Player>& currentSolution) {
    vector<Player> bestSolution = currentSolution;

    tuple<int, int> bestTotals = calculateTotalPointsAndPrice(bestSolution);
    int bestTotalPoints = get<0>(bestTotals);
    int bestTotalPrice = get<1>(bestTotals);

    for (size_t i = 0; i < tactic.pos_jug.size(); ++i) {
            int currentTotalPoints = get<0>(calculateTotalPointsAndPrice(currentSolution));
            int currentTotalPrice = get<1>(calculateTotalPointsAndPrice(currentSolution));

            localSearchRecursive(tactic, currentSolution, i, currentTotalPoints, currentTotalPrice, bestSolution, bestTotalPoints, bestTotalPrice);
    }
    return bestSolution;
}

vector<Player> localSearchRecursive(const Tactic& tactic, vector<Player>& currentSolution, size_t positionIndex, int currentTotalPoints, int currentTotalPrice, vector<Player>& bestSolution, int& bestTotalPoints, int& bestTotalPrice) {
    if (currentSolution.size() == tactic.getPlayersCount(positionIndex)) {
        // Reached the required number of players for this position
        tuple<int, int> currentTotals = calculateTotalPointsAndPrice(currentSolution);
        int newTotalPoints = get<0>(currentTotals);
        int newTotalPrice = get<1>(currentTotals);

        cout << "New total points: " << newTotalPoints << ", New total price: " << newTotalPrice << endl;

        if (newTotalPoints > bestTotalPoints && newTotalPrice <= tactic.maxPreuTotal) {
            bestSolution = currentSolution;
            bestTotalPoints = newTotalPoints;
            bestTotalPrice = newTotalPrice;
            cout << "Updated best solution: " << endl;
            for (const auto& player : bestSolution) {
                cout << "Player " << player.name << " at position " << positionIndex << endl;
            }
        }
        return bestSolution;
    }

    // Try adding each eligible player to the current solution
    for (size_t j = 0; j < tactic.pos_jug[positionIndex].size(); ++j) {
        const Player& player = tactic.pos_jug[positionIndex][j];

        // Check eligibility
        if (find_if(currentSolution.begin(), currentSolution.end(), [&player](const Player& p) {
                return p.position == player.position;
            }) == currentSolution.end() && currentTotalPrice + player.price <= tactic.maxPreuTotal && player.price <= tactic.maxPreuJug) {
            
            // Add the player to the current solution
            currentSolution.push_back(player);

            // Recursively explore further possibilities
            localSearchRecursive(tactic, currentSolution, positionIndex, currentTotalPoints + player.points, currentTotalPrice + player.price, bestSolution, bestTotalPoints, bestTotalPrice);

            // Remove the added player to backtrack
            currentSolution.pop_back();
        }
    }

    return bestSolution;
}


int main(int argc, char** argv) {
    if (argc != 4) {
        cout << "Syntax: " << argv[0] << " data_base.txt formation.txt output.txt" << endl;
        exit(1);
    }

    auto start = chrono::high_resolution_clock::now();

    vector<Player> players = readPlayers(argv[1]);

    ifstream formationFile(argv[2]);
    if (!formationFile.is_open()) {
        cerr << "Error opening formation file." << endl;
        exit(1);
    }

    uint n1, n2, n3, T, J;
    formationFile >> n1 >> n2 >> n3 >> T >> J;
    formationFile.close();

    Tactic tactic(n1, n2, n3, T, J, players);

    vector<Player> bestSolution = grasp(tactic, 10000);

    auto stop = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::microseconds>(stop - start);

    double seconds = duration.count() / 1e6;

    int totalPoints = 0;
    int totalPrice = 0;

    for (const auto& player : bestSolution) {
        totalPoints += player.points;
        totalPrice += player.price;
    }

    ofstream out(argv[3]);
    out << "Time: " << seconds << " seconds" << endl;

    const vector<string> positionsOrder = { "por", "def", "mig", "dav" };
    for (const auto& pos : positionsOrder) {
        out << char(pos[0] - 32) << char(pos[1] - 32) << char(pos[2] - 32) << ": ";
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
