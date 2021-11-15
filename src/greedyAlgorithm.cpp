#include "greedyAlgorithm.h"
#include <set>
bool secondEchelonGreedy(Solution& Sol, const Instance& Inst, float alpha){
    /*
    std::vector<std::vector<std::vector<Insertion*>>> insertionTable(Inst.getNSats(),
                                                                     std::vector<std::vector<Insertion*>>(1,
                                                                             std::vector<Insertion*>(Inst.getNClients(),
                                                                                                     nullptr))); // Os indices da lista sao os IDs dos clientes, os indices de seus vectors os satelites e os indices destes os clientes. // TODO: maybe use matrix instead of vec<vec<>>;
    //expects empty Solution

    for(int i = Inst.getFirstClientIndex(); i < Inst.getFirstClientIndex()+Inst.getNClients(); i++){
        unvisitedClients.insert(i);
    }
     */
    std::set<int> unvisitedClients;
    std::set<Insertion> restrictedList;
    while(!unvisitedClients.empty()){
        for(int client : unvisitedClients){
            for(int s = 0; s < Sol.getNSatelites(); s++){
                Satelite* Sat = Sol.getSatelite(s);
                for(int r = 0; r < Sol.getSatelite(s)->getNRoutes(); r++) {
                    EvRoute &route = Sat->getRoute(r);
                    Insertion Ins{};
                    bool canInsert = route.canInsert(client, Inst, Ins);
                    if (canInsert) {
                        restrictedList.insert(Ins);
                        //insertionTable.at(s).at(r).at(client - Inst.getFirstClientIndex()) = Ins;
                    } else {
                        //insertionTable.at(s).at(r).at(client - Inst.getFirstClientIndex()) = nullptr;
                    }
                    //restrictedList.push_back(insertionTable.at(s).at(r).at(client - Inst.getFirstClientIndex()));
                }
            }
        }
        int randIndex = rand()%((int)(0.3*(int)restrictedList.size() + 1));
        auto topItem = std::next(restrictedList.begin(), randIndex);
    }
}
