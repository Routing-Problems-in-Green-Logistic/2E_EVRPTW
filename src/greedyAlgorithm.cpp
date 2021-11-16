#include "greedyAlgorithm.h"
#include <set>
using namespace GreedyAlgNS;

bool GreedyAlgNS::secondEchelonGreedy(Solution& sol, const Instance& Inst, const float alpha)
{

    std::vector<int> visitedClients(1+Inst.getNSats()+Inst.getNClients());
    visitedClients[0]       = -1;
    const auto ItSat        = visitedClients.begin() + Inst.getFirstSatIndex();
    const auto ItSatEnd     = visitedClients.begin() + Inst.getEndSatIndex();
    const auto ItClient     = visitedClients.begin() + Inst.getFirstClientIndex();

    const int FistIdClient  = 1 + Inst.getNSats();
    const int LastIdClient  = Inst.getNSats() + Inst.getNClients();
    const auto ItEnd        = visitedClients.begin() + Inst.getNSats() + Inst.getNClients();

    std::fill(ItSat, ItSatEnd, -1);
    std::fill(ItClient, ItEnd, 0);


    while(!visitAllClientes(visitedClients, Inst))
    {
        std::list<Insertion> restrictedList;

        for(int clientId = FistIdClient; clientId < LastIdClient + 1; ++clientId){
            for(int satId = 0; satId < sol.getNSatelites(); satId++){
                Satelite* sat = sol.getSatelite(satId);
                for(int routeId = 0; routeId < sol.getSatelite(satId)->getNRoutes(); routeId++) {
                    EvRoute &route = sat->getRoute(routeId);
                    Insertion insertion(routeId);
                    bool canInsert = route.canInsert(clientId, Inst, insertion);
                    if (canInsert)
                    {
                        restrictedList.push_back(insertion); //insertionTable.at(satId).at(routeId).at(client - Inst.getFirstClientIndex()) = Ins;
                    }
                    /*
                    else {
                        //insertionTable.at(satId).at(routeId).at(client - Inst.getFirstClientIndex()) = nullptr;
                    }
                    //restrictedList.push_back(insertionTable.at(satId).at(routeId).at(client - Inst.getFirstClientIndex()));
                     */
                }
            }
        }

        int randIndex = rand()%(int(alpha*restrictedList.size() + 1));
        auto topItem = std::next(restrictedList.begin(), randIndex);

        visitedClients[topItem->clientId] = 1;

        Satelite *satelite = sol.getSatelite(topItem->satId);
        EvRoute &evRoute = satelite->getRoute(topItem->routeId);

        if(!evRoute.insert(*topItem, Inst))
        {

            std::string erro = "ERRO AO INSERIR O CANDIDATO NA SOLUCAO.\nArquivo: " + std::string(__FILENAME__) + "\nLinha: " +
                    std::to_string(__LINE__);

            throw erro.c_str();
        }

    }

    return false;
}

bool GreedyAlgNS::visitAllClientes(std::vector<int> &visitedClients, const Instance &Inst)
{

    auto itClient = visitedClients.begin() + Inst.getFirstClientIndex();

    for(; itClient != visitedClients.end(); ++itClient)
        if(*itClient == 0) return false;

    return true;

}