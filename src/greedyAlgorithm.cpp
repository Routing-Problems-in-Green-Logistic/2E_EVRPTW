#include "greedyAlgorithm.h"
#include "Auxiliary.h"

#include <set>
using namespace GreedyAlgNS;
using namespace std;

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

        for(int clientId = FistIdClient; clientId < LastIdClient + 1; ++clientId)
        {

            if(!visitedClients[clientId])
            {
                for(int satId = Inst.getFirstSatIndex(); satId <= Inst.getEndSatIndex(); satId++)
                {
                    Satelite *sat = sol.getSatelite(satId - Inst.getFirstSatIndex());

                    bool routeEmpty = false;
                    for(int routeId = 0; routeId < sol.getSatelite(satId - Inst.getFirstSatIndex())->getNRoutes(); routeId++)
                    {
                        EvRoute &route = sat->getRoute(routeId);

                        if((route.routeSize == 2)&&(!routeEmpty) || (route.routeSize > 2))
                        {

                            if(route.routeSize == 2)
                                routeEmpty = true;

                            Insertion insertion(routeId);
                            insertion.satId = satId;


                            bool canInsert = route.canInsert(clientId, Inst, insertion);


                            if(canInsert)
                            {


                                restrictedList.push_back(insertion); //insertionTable.at(satId).at(routeId).at(client - Inst.getFirstClientIndex()) = Ins;
                            }


                        }
                    }
                }
            }
        }



        if(restrictedList.empty())
        {
            sol.viavel = false;
            break;
        }


        int randIndex = rand()%(int(alpha*restrictedList.size() + 1));

        auto topItem = std::next(restrictedList.begin(), randIndex);

        visitedClients[topItem->clientId] = 1;


        Satelite *satelite = sol.getSatelite(topItem->satId-Inst.getFirstSatIndex());
        EvRoute &evRoute = satelite->getRoute(topItem->routeId);


        if(!evRoute.insert(*topItem, Inst))
        {

            std::cerr<<"\n\n\n\n************************************\nERRO, NAO FOI POSSIVEL INSERIR CLIENTE NA SULUCAO\n";

            throw "ERRO";
        }

    }

    std::string str = "";
    if(!sol.checkSolution(str, Inst))
    {
        std::cerr<<str<<"\n\n";
        std::cout<<"*******************************************\n";

        throw "ERRO";
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