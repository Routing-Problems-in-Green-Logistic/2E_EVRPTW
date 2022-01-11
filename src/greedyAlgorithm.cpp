#include "greedyAlgorithm.h"
#include "Auxiliary.h"
#include <set>
#include <cfloat>
#include "mersenne-twister.h"
#include "LocalSearch.h"

using namespace GreedyAlgNS;
using namespace std;
using namespace NS_Auxiliary;
using namespace NS_LocalSearch;

bool GreedyAlgNS::secondEchelonGreedy(Solution& sol, const Instance& Inst, const float alpha)
{

    std::vector<int> visitedClients(1+Inst.getNSats()+Inst.getNClients());
    visitedClients[0]       = -1;

    for(int i=1; i < Inst.getNSats()+1; ++i)
        visitedClients[i] = -1;

    for(int i=Inst.getNSats()+1; i < visitedClients.size(); ++i)
        visitedClients[i] = 0;

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

            if(visitedClients[clientId] == 0)
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


        int randIndex = rand_u32()%(int(alpha*restrictedList.size() + 1));

        auto topItem = std::next(restrictedList.begin(), randIndex);

        visitedClients[topItem->clientId] = 1;


        Satelite *satelite = sol.getSatelite(topItem->satId-Inst.getFirstSatIndex());
        satelite->demand += topItem->demand;
        EvRoute &evRoute = satelite->getRoute(topItem->routeId);


        if(!evRoute.insert(*topItem, Inst))
        {

            std::cerr<<"\n\n\n\n************************************\nERRO, NAO FOI POSSIVEL INSERIR CLIENTE NA SULUCAO\n";

            throw "ERRO";
        }

    }


    return true;
}

bool GreedyAlgNS::visitAllClientes(std::vector<int> &visitedClients, const Instance &Inst)
{

    auto itClient = visitedClients.begin() + Inst.getFirstClientIndex();

    for(; itClient != visitedClients.end(); ++itClient)
        if(*itClient == 0) return false;

    return true;

}

void GreedyAlgNS::firstEchelonGreedy(Solution &sol, const Instance &Inst, const float beta)
{

    // Cria o vetor com a demanda de cada satellite

    std::vector<float> demandaNaoAtendidaSat;
    demandaNaoAtendidaSat.reserve(sol.getNSatelites()+1);
    int satId = 1;
    demandaNaoAtendidaSat.push_back(0.0);

    for(Satelite *satelite:sol.satelites)
    {
        demandaNaoAtendidaSat.push_back(satelite->demand);
    }

    const int NumSatMaisDep = sol.getNSatelites()+1;

    // Enquanto existir um satellite com demanda > 0, continua
    while(existeDemandaNaoAtendida(demandaNaoAtendidaSat))
    {
        // Cria a lista de candidatos
        std::list<Candidato> listaCandidatos;

        // Percorre os satellites
        for(int i=1; i < NumSatMaisDep; ++i)
        {
            Satelite *satelite = sol.satelites[i-1];

            // Verifica se a demanda não atendida eh positiva
            if(demandaNaoAtendidaSat[i] > 0.0)
            {

                // Percorre todas as rotas
                for(int rotaId = 0; rotaId < sol.primeiroNivel.size(); ++rotaId)
                {
                    Route &route = sol.primeiroNivel[rotaId];

                    // Verifica se veiculo esta 100% da capacidade
                    if(route.totalDemand < Inst.getTruckCap())
                    {
                        // Calcula a capacidade restante do veiculo
                        float capacidade = Inst.getTruckCap() - route.totalDemand;
                        float demandaAtendida = capacidade;

                        if(demandaNaoAtendidaSat[i] < capacidade)
                            demandaAtendida = demandaNaoAtendidaSat[i];

                        Candidato candidato(rotaId, i, demandaAtendida, FLT_MAX);

                        // Percorre todas as posicoes da rota
                        for(int p=0; (p+1) < route.routeSize; ++p)
                        {
                            float incrementoDist = 0.0;

                            // Realiza a insercao do satellite entre as posicoes p e p+1 da rota
                            const int clienteP = route.rota[p];
                            const int clientePP = route.rota[p+1];

                            // Calcula o incremento da distancia (Sempre positivo, desigualdade triangular)
                            incrementoDist -= Inst.getDistance(clienteP, clientePP);
                            incrementoDist = incrementoDist+ Inst.getDistance(clienteP, i) + Inst.getDistance(i, clientePP);

                            if(incrementoDist < candidato.incrementoDistancia)
                            {
                                candidato.incrementoDistancia = incrementoDist;
                                candidato.pos = p;
                            }
                        }

                        if(candidato.pos >= 0)
                            listaCandidatos.push_back(candidato);

                    }
                }
            }
        }


        if(!listaCandidatos.empty())
        {

            listaCandidatos.sort();

            // Escolhe o candidado da lista restrita
            int tam = int(beta * listaCandidatos.size()) + 1;
            int escolhido = rand_u32() % tam;
            auto it = listaCandidatos.begin();

            std::advance(it, escolhido);
            Candidato &candidato = *it;

            // Insere candidato na solucao
            Route &route = sol.primeiroNivel[candidato.rotaId];
            shiftVectorDir(route.rota, candidato.pos + 1, 1, route.routeSize);
            route.rota[candidato.pos + 1] = candidato.satelliteId;
            route.routeSize += 1;

            // Atualiza demanda, vetor de demanda e distancia
            route.totalDemand += candidato.demand;
            route.satelliteDemand[candidato.satelliteId] = candidato.demand;
            route.totalDistence += candidato.incrementoDistancia;

            demandaNaoAtendidaSat[candidato.satelliteId] -= candidato.demand;
        }
        else
        {
            // Adiciona mais um veiculo a solucao
            sol.primeiroNivel.push_back(Inst);
            sol.nTrucks += 1;
        }
    }

}

bool GreedyAlgNS::existeDemandaNaoAtendida(std::vector<float> &demandaNaoAtendida)
{
    for(float dem:demandaNaoAtendida)
    {
        if(dem > 0.0)
            return true;
    }

    return false;
}

void GreedyAlgNS::greedy(Solution &sol, const Instance &Inst, const float alpha, const float beta)
{
    if(secondEchelonGreedy(sol, Inst, alpha))
    {

        firstEchelonGreedy(sol, Inst, beta);


        std::string str = "";

        /*if(sol.viavel && sol.checkSolution(str, Inst))
        {
            float val = sol.getDistanciaTotal();
            //PRINT_DEBUG("", "Antes chamada");

            if(mvShifitIntraRota(sol, Inst))
            {
                //PRINT_DEBUG("", "Depois chamada");

                str = "";

                if(sol.checkSolution(str, Inst))
                    cout<<"Heuristica: "<<val<<"; Shifit: "<<sol.getDistanciaTotal()<<"\n";
                else
                {
                    std::cerr<<"INVIAVEL APOS SHIFIT INTRA SATELLITE\n\n"<<str<<"\n\n";
                    std::cout << "*******************************************\n";

                    throw "ERRO";
                }
            }

        }
        else if(sol.viavel)
        {
            std::cerr << str << "\n\n";
            std::cout << "*******************************************\n";

            throw "ERRO";
        }

         */

        if(sol.viavel)
        {
            if(!sol.checkSolution(str, Inst))
            {
                std::cerr<<str << "\n\n";
                std::cout << "*******************************************\n";

                throw "ERRO";
            }

        }
    }


}
