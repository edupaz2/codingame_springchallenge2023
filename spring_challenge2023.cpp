#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <queue>
#include <limits>
#include <list>
#include <map>

using namespace std;

class Cell {
    public:
        int idx, cellType, resources, myAnts, oppAnts;
        array<int, 6> neighbors{-1, -1, -1, -1, -1, -1};

        Cell() = default;

        void set(int idx, int cellType, int resources, array<int, 6> neighbors, int myAnts = 0, int oppAnts = 0) {
            this->idx = idx;
            this->cellType = cellType;
            this->resources = resources;
            this->neighbors = neighbors;
            this->myAnts = myAnts;
            this->oppAnts = oppAnts;
        }

        void setDuringGame(int resources, int myAnts, int oppAnts) {
            this->resources = resources;
            this->myAnts = myAnts;
            this->oppAnts = oppAnts;
        }
};

/**
 * Auto-generated code below aims at helping you parse
 * the standard input according to the problem statement.
 **/

int main()
{
    int number_of_cells; // amount of hexagonal cells in this map
    cin >> number_of_cells; cin.ignore();
    int total_crystals, total_eggs;
    vector<Cell> cells(number_of_cells);
    vector<int> cellNeighboursCost(number_of_cells);
    vector<int> cellCost(number_of_cells, -1);
    vector<int> myResources;
    vector<int> myEggs;
    vector<int> myCrystals;
    int number_of_crystals_cells;
    for (int i = 0; i < number_of_cells; i++) {
        int type; // 0 for empty, 1 for eggs, 2 for crystal
        int initial_resources; // the initial amount of eggs/crystals on this cell
        int neigh_0; // the index of the neighbouring cell for each direction
        int neigh_1;
        int neigh_2;
        int neigh_3;
        int neigh_4;
        int neigh_5;
        cin >> type >> initial_resources >> neigh_0 >> neigh_1 >> neigh_2 >> neigh_3 >> neigh_4 >> neigh_5; cin.ignore();

        // cerr << i << ", " << type << ", " << initial_resources << ", " << neigh_0 << ", " << neigh_1 << ", " << neigh_2 << ", " << neigh_3 << ", " << neigh_4 << ", " << neigh_5 << endl;
        cells[i].set(i, type, initial_resources, {neigh_0, neigh_1, neigh_2, neigh_3, neigh_4, neigh_5}, 0, 0);
        if(type == 1) {
            total_eggs += initial_resources;
            myEggs.push_back(i);
        } else if(type == 2) {
            total_crystals += initial_resources;
            myCrystals.push_back(i);
        }
        if(type != 0) {
            myResources.push_back(i);
        }

        cellCost[i] = initial_resources;
    }

    for (auto c: cells) {
        for(auto n: c.neighbors) {
            if(n != -1) {
                cellNeighboursCost[c.idx] += cells[n].resources;
            }
        }
    }

    // const int crystalsToWin = total_crystals / 2 + 1;

    // Data structures for path finding
    list<int> toVisitCells;
    vector<int> distanceToBase(number_of_cells, -1);
    vector<int> previousNeighbour(number_of_cells, -1);
    vector<int> closestBase(number_of_cells, -1);

    int number_of_bases;
    cin >> number_of_bases; cin.ignore();
    vector<int> myBases, oppBases;
    for (int i = 0; i < number_of_bases; i++) {
        int my_base_index;
        cin >> my_base_index; cin.ignore();
        myBases.push_back(my_base_index);
        // cerr << "to visit add: " << my_base_index << endl;
        toVisitCells.push_back(my_base_index);
        distanceToBase[my_base_index] = 0;
        previousNeighbour[my_base_index] = my_base_index;
        closestBase[my_base_index] = my_base_index;
    }
    for (int i = 0; i < number_of_bases; i++) {
        int opp_base_index;
        cin >> opp_base_index; cin.ignore();
        oppBases.push_back(opp_base_index);
    }

    // Create a depth map from the bases
    // TODO: create the path from the cell to the base for the command BEACON
    while(!toVisitCells.empty()) {
        auto current = toVisitCells.front();
        // cerr << endl << "- current: " << current << ", dist: " << distanceToBase[current] << ", cost: " << cellCost[current] << endl;
        toVisitCells.pop_front();
        for(const auto& c: cells[current].neighbors) {
            if (c == -1 || find(myBases.cbegin(), myBases.cend(), c) != myBases.cend()) continue;
            // Select this new path if the cost is better
            int newCost = cells[c].resources + (cellNeighboursCost[c] / (distanceToBase[current]+2));
            // cerr << "-> idx: " << c << ", cost: " << cellCost[c] << ", new cost: " << newCost << endl;
            if(distanceToBase[c] == -1 || 
               newCost > cellCost[c] ||
               (newCost == cellCost[c] && distanceToBase[c] > distanceToBase[current]+1)) {
                cellCost[c] = newCost;
                distanceToBase[c] = distanceToBase[current]+1;
                previousNeighbour[c] = current;
                closestBase[c] = closestBase[current];
                // cerr << "+ add idx: " << c << ", ND: " <<  distanceToBase[c] << ", NC: " << cellCost[c] << endl;
                toVisitCells.remove(c);
                toVisitCells.insert(find_if(toVisitCells.begin(), toVisitCells.end(), [c, &cellCost](auto e){ return cellCost[c] > cellCost[e]; }), c);
            }
        }
    }

    std::sort(myEggs.begin(), myEggs.end(), [&cells, &distanceToBase](auto e1, auto e2){
        if(distanceToBase[e1] == distanceToBase[e2]) {
            return cells[e1].resources > cells[e2].resources;
        }
        return distanceToBase[e1] < distanceToBase[e2];
    });

    std::sort(myCrystals.begin(), myCrystals.end(), [&cells, &distanceToBase](auto c1, auto c2){
        if(distanceToBase[c1] == distanceToBase[c2]) {
            return cells[c1].resources > cells[c2].resources;
        }
        return distanceToBase[c1] < distanceToBase[c2];
    });

    std::sort(myResources.begin(), myResources.end(), [&cells, &distanceToBase, &cellCost](auto r1, auto r2){
        // Heuristic: the more resouces and closest, the better.
        //return cellCost[r1] / distanceToBase[r1] > cellCost[r2] / distanceToBase[r2];
        // return cellCost[r1] > cellCost[r2];
        if(cells[r1].resources == cells[r2].resources) {
            return cellCost[r1] > cellCost[r2];
        }
        return cells[r1].resources > cells[r2].resources;
    });

    // cerr << "Number of resources: " << myResources.size() << endl;
    // for(const auto& r: myResources) {
    //     cerr << "cell " << r << ", r:" << cells[r].resources << ", c:" << cellCost[r] << ", d:" << distanceToBase[r] << " to base: " << closestBase[r] << endl;
    // }
    // cerr << "Resources: " << endl;
    // for(int i = 0; i < myResources.size(); ++i) {
    //     cerr << "cell " << myResources[i] << ", resources: " << cells[myResources[i]].resources << endl;
    //     cerr << " d: " << distanceToBase[myResources[i]] << ", base: " << closestBase[myResources[i]] << ", c: " << cellCost[myResources[i]] << ", H: " << cellCost[myResources[i]] / distanceToBase[myResources[i]] << endl;
    // }

    // game loop
    int number_to_loops;
    while (1) {
        int available_crystals{0}, available_eggs{0}, available_ants{0}, opp_available_ants{0};
        for (int i = 0; i < number_of_cells; i++) {
            int resources; // the current amount of eggs/crystals on this cell
            int my_ants; // the amount of your ants on this cell
            int opp_ants; // the amount of opponent ants on this cell
            cin >> resources >> my_ants >> opp_ants; cin.ignore();
            cells[i].setDuringGame(resources, my_ants, opp_ants);
            available_ants += my_ants;
            opp_available_ants += opp_ants;
            if(cells[i].cellType == 1) {
                available_eggs += resources;
            } else if(cells[i].cellType == 2) {
                available_crystals += resources;
            }
        }

        // If we have eggs nearby, full resources
        int resources_harvested{0};
        for(const auto& e: myEggs) {
            if(distanceToBase[e] <= 4 && cells[e].resources > 0) {
                // cerr << "Adding " << e << ", d:" << distanceToBase[e] << endl;
                cout << "LINE " << e << " " << closestBase[e] << " 1;";
                resources_harvested += max(cells[e].resources, cells[e].myAnts);
            }
        }
        // for(const auto& c: myCrystals) {
        //     if(distanceToBase[c] == 1 && cells[c].resources > 0) {
        //         cout << "LINE " << c << " " << closestBase[c] << " 1;";
        //         resources_harvested += max(cells[e].resources, cells[e].myAnts);
        //     }
        // }

        // Go for the resources
        int remaining_ants = available_ants - resources_harvested;
        for(const auto& r: myResources) {
            // cerr << "Ants: " << remaining_ants << ", check " << r << ", res:" << cells[r].resources << ", d:" << distanceToBase[r] << endl;
            if(cells[r].resources > 0
                && remaining_ants > 0
                && distanceToBase[r] <= remaining_ants) {
                int idx = r;
                // cerr << "Adding " << r << ", d:" << distanceToBase[r] << endl;
                while(idx != closestBase[r]) {
                    cout << "LINE " << idx << " " << previousNeighbour[idx] << " 1;";
                    idx = previousNeighbour[idx];
                }
                remaining_ants -= distanceToBase[r];
            } else {
                continue;
            }
        }
        cout << endl;

        // Write an action using cout. DON'T FORGET THE "<< endl"
        // To debug: cerr << "Debug messages..." << endl;


        // WAIT | LINE <sourceIdx> <targetIdx> <strength> | BEACON <cellIdx> <strength> | MESSAGE <text>
        //cout << "WAIT" << endl;
        // cout << "LINE 13" << endl;
        number_to_loops++;
    }
}