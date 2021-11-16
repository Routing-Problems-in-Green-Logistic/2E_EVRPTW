#include "greedyAlgorithm.h"
#include <set>
using namespace GreedyAlgNS;

bool GreedyAlgNS::secondEchelonGreedy(Solution& Sol, const Instance& Inst, const float alpha)
{

    std::vector<int> visitedClients(1+Inst.getNSats()+Inst.getNClients());
    visitedClients[0]       = -1;
    const auto itSat        = visitedClients.begin() + 1;                                       // 1
    const auto itSatEnd     = visitedClients.begin() + Inst.getNSats();
    const auto itClient     = visitedClients.begin() + 1+Inst.getNSats();
    const int fistIdClient  = 1+Inst.getNSats();
    const int lastIdClient  = Inst.getNSats()+Inst.getNClients();
    const auto itEnd        = visitedClients.begin() + Inst.getNSats()+Inst.getNClients();

    std::fill(itSat, itSatEnd, -1);
    std::fill(itClient, itEnd, 0);

    std::list<Insertion> restrictedList;
    while(!visitAllClientes(visitedClients, Inst))
    {
        for(int clientId = fistIdClient; clientId < lastIdClient+1; ++clientId){
            for(int s = 0; s < Sol.getNSatelites(); s++){
                Satelite* Sat = Sol.getSatelite(s);
                for(int r = 0; r < Sol.getSatelite(s)->getNRoutes(); r++) {
                    EvRoute &route = Sat->getRoute(r);
                    Insertion insertion{};
                    bool canInsert = route.canInsert(clientId, Inst, insertion);
                    if (canInsert)
                    {
                        restrictedList.push_back(insertion); //insertionTable.at(s).at(r).at(client - Inst.getFirstClientIndex()) = Ins;
                    } else {
                        //insertionTable.at(s).at(r).at(client - Inst.getFirstClientIndex()) = nullptr;
                    }
                    //restrictedList.push_back(insertionTable.at(s).at(r).at(client - Inst.getFirstClientIndex()));
                }
            }
        }

        int randIndex = rand()%(int(alpha*restrictedList.size() + 1));
        auto topItem = std::next(restrictedList.begin(), randIndex);
    }
}

bool GreedyAlgNS::visitAllClientes(std::vector<int> &visitedClients, const Instance &Inst)
{

    auto itClient = visitedClients.begin() + 1+Inst.getNSats();

    for(; itClient != visitedClients.end(); ++itClient)
        if(*itClient == 0) return false;

    return true;

}