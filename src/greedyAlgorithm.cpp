#include "greedyAlgorithm.h"
#include "Auxiliary.h"
#include <set>
#include <cfloat>
#include "mersenne-twister.h"
#include "LocalSearch.h"
#include "ViabilizadorRotaEv.h"

struct comparaCliente { // TODO: verifica na instancia qual cliente tem tempo final mais proximo
    bool operator() (const int& lhs, const int& rhs) const {
        return false;
    }
};

float GreedyAlgNS::palpiteTempoFinalPrimeiroNivel(const Instance &inst){
    int depotId = inst.getDepotIndex();
    float bestDistance = -1.0f;
    //> percorre todos os satelites procurando a maior distancia ate o deposito
    for(int i = inst.getFirstSatIndex(); i <= inst.getEndSatIndex(); i++){
        float distance = inst.getDistance(depotId, i);
        if(distance > bestDistance){
            bestDistance = distance;
        }
    }
    if( bestDistance == -1.0f){
        PRINT_DEBUG("", "ERRO @ GreedyAlgNS::palpiteTempoFinalPrimeiroNivel -> maior distancia = -1\n\n");
        throw "ERRO";
    }
    return bestDistance;
}


using namespace GreedyAlgNS;
using namespace std;
using namespace NS_Auxiliary;
using namespace NS_LocalSearch;
using namespace NameViabRotaEv;


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
    std::set<int, comparaCliente> clientsByTime;
    // TODO: fill clientsbytime;

    while(!visitAllClientes(visitedClients, Inst))
    {
        std::list<Insertion> restrictedList;

        for(auto& client : clientsByTime)
        // for(int clientId = FistIdClient; clientId < LastIdClient + 1; ++clientId)
        {

            if(visitedClients[client] == 0)
            {
                for(int satId = Inst.getFirstSatIndex(); satId <= Inst.getEndSatIndex(); satId++)
                {
                    Satelite *sat = sol.getSatelite(satId - Inst.getFirstSatIndex());

                    bool routeEmpty = false;
                    for(int routeId = 0; routeId < sol.getSatelite(satId)->getNRoutes(); routeId++)
                    {
                        EvRoute &route = sat->getRoute(routeId);
                        if((route.routeSize == 2)&&(!routeEmpty) || (route.routeSize > 2))
                        {
                            if(route.routeSize == 2)
                                routeEmpty = true;
                            if(route.getCurrentTime() < Inst.getClient(client).fimJanelaTempo){
                                Insertion insertion(routeId);
                                insertion.satId = satId;
                                if(canInsertSemBateria(route, client, Inst, insertion))
                                    restrictedList.push_back(insertion);
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
        satelite->demanda += topItem->demand;
        EvRoute &evRoute = satelite->getRoute(topItem->routeId);
        insert(evRoute, *topItem, Inst);
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

    for(Satelite &satelite:sol.satelites)
    {
        demandaNaoAtendidaSat.push_back(satelite.demanda);
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
            Satelite &satelite = sol.satelites[i];

            // Verifica se a demanda não atendida eh positiva
            if(demandaNaoAtendidaSat[i] > 0.0)
            {

                // Percorre todas as rotas
                for(int rotaId = 0; rotaId < sol.primeiroNivel.size(); ++rotaId)
                {
                    Route &route = sol.primeiroNivel[rotaId];

                    // Verifica se veiculo esta 100% da capacidade
                    if(route.totalDemand < Inst.getTruckCap(rotaId))
                    {
                        // Calcula a capacidade restante do veiculo
                        float capacidade = Inst.getTruckCap(rotaId) - route.totalDemand;
                        float demandaAtendida = capacidade;

                        if(demandaNaoAtendidaSat[i] < capacidade)
                            demandaAtendida = demandaNaoAtendidaSat[i];

                        Candidato candidato(rotaId, i, demandaAtendida, DOUBLE_MAX);

                        // Percorre todas as posicoes da rota
                        for(int p=0; (p+1) < route.routeSize; ++p)
                        {
                            double incrementoDist = 0.0;

                            // Realiza a insercao do satellite entre as posicoes p e p+1 da rota
                            const RouteNo &clienteP =  route.rota[p];
                            const RouteNo &clientePP = route.rota[p+1];

                            // Calcula o incremento da distancia (Sempre positivo, desigualdade triangular)
                            incrementoDist -= Inst.getDistance(clienteP.satellite, clientePP.satellite);
                            incrementoDist = incrementoDist+ Inst.getDistance(clienteP.satellite, i) + Inst.getDistance(i, clientePP.satellite);

                            if(incrementoDist < candidato.incrementoDistancia)
                            {

                                // Calcula o tempo de chegada e verifica a janela de tempo
                                const double tempoChegCand = clienteP.tempoChegada + Inst.getDistance(clienteP.satellite, i);

                                bool satViavel = true;

                                if(verificaViabilidadeSatelite(tempoChegCand, satelite, Inst, false))
                                {
                                    double tempoChegTemp = tempoChegCand + Inst.getDistance(i, clientePP.satellite);

                                    // Verificar viabilidade dos outros satelites
                                    for(int t=p+1; (t+1) < route.routeSize; ++t)
                                    {
                                        tempoChegTemp += Inst.getDistance(route.rota[t].satellite, route.rota[t+1].satellite);
                                        Satelite &sateliteTemp = sol.satelites[route.rota[t+1].satellite];

                                        if(!verificaViabilidadeSatelite(tempoChegTemp, sateliteTemp, Inst, false))
                                        {
                                            satViavel = false;
                                            break;
                                        }

                                    }

                                }
                                else
                                    satViavel = false;

                                if(satViavel)
                                {
                                    candidato.incrementoDistancia = incrementoDist;
                                    candidato.pos = p;
                                    candidato.tempoSaida = tempoChegCand;
                                }
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

            throw "ERRO FUNCAO NAO FINALIZADA!!\n";

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

            route.rota[candidato.pos+1].satellite = candidato.satelliteId;
            route.rota[candidato.pos+1].tempoChegada = candidato.tempoSaida;
            route.routeSize += 1;
            double tempoSaida = candidato.tempoSaida;

            for(int i=candidato.pos+1; (i+1) < route.routeSize; ++i)
            {
                const int satTemp = route.rota[i].satellite;
                if(!verificaViabilidadeSatelite(tempoSaida, sol.satelites[satTemp], Inst, true))
                {
                    string satStr;
                    sol.satelites[satTemp].print(satStr, Inst);

                    cout<<"ERRO! FUNCAO 'verificaViabilidadeSatelite' DEVERIA RETORNAR TRUE. TEMPO SAIDA SATELITE: "<<tempoSaida<<"\n\tSATELITE:\n\n"<<satStr<<"\n\n";
                    throw "ERRO!";
                }

                tempoSaida += Inst.getDistance(satTemp, route.rota[i+1].satellite);
            }

            // Atualiza demanda, vetor de demanda e distancia
            route.totalDemand += candidato.demand;
            route.satelliteDemand[candidato.satelliteId] = candidato.demand;
            route.totalDistence += candidato.incrementoDistancia;

            demandaNaoAtendidaSat[candidato.satelliteId] -= candidato.demand;
        }
        else
        {
            sol.viavel = false;
            break;
        }
    }

}

// Com o tempo de chegada ao satelite, eh verificado se as rotas dos EV's podem sair apos o tempo de chegada do veic a combustao
bool GreedyAlgNS::verificaViabilidadeSatelite(const double tempoChegada, Satelite &satelite, const Instance &instance, const bool modificaSatelite)
{

    bool viavel = false;

    // Verifica se os tempos de saida das rotas dos EV's eh maior que o tempo de chegada do veic a combustao
    for(auto &tempoSaidaEv:satelite.vetTempoSaidaEvRoute)
    {

        const double tempoEv = tempoSaidaEv.evRoute->route[0].tempoSaida;
        if(tempoEv >= tempoChegada)
        {
            viavel = true;
            break;
        }
        else
        {
            // Verifica se eh possivel realizar um shift na rota

            const int indice = tempoSaidaEv.evRoute->route[0].posMenorFolga;
            const int cliente = tempoSaidaEv.evRoute->route[indice].cliente;
            const double twFim = instance.vectCliente[cliente].fimJanelaTempo;

            double diferenca = twFim - tempoSaidaEv.evRoute->route[indice].tempoCheg;
            if(diferenca < 0.0)
                diferenca = 0.0;

            if(!((tempoEv+diferenca) >= tempoChegada))
            {
                viavel = false;
                break;
            }

        }
    }

    if(!modificaSatelite)
        return viavel;
    else
    {

        for(auto &tempoSaidaEv:satelite.vetTempoSaidaEvRoute)
        {

            const double tempoEv = tempoSaidaEv.evRoute->route[0].tempoSaida;
            if(tempoEv >= tempoChegada)
            {
                return true;
            }
            else
            {
                // Verifica se eh possivel realizar um shift na rota

                const int indice = tempoSaidaEv.evRoute->route[0].posMenorFolga;
                const int cliente = tempoSaidaEv.evRoute->route[indice].cliente;
                const double twFim = instance.vectCliente[cliente].fimJanelaTempo;

                double diferenca = twFim - tempoSaidaEv.evRoute->route[indice].tempoCheg;
                if(diferenca < 0.0)
                    diferenca = 0.0;

                if(!((tempoEv+diferenca) >= tempoChegada))
                {
                    // Nao deve chegar aqui
                    return false;
                }
                else
                {
                    if(!tempoSaidaEv.evRoute->alteraTempoSaida(tempoChegada, instance))
                    {
                        PRINT_DEBUG("", "ERRO AO ALTERAR O TEMPO DE SAIDA DA ROTA EV DE ID: "<<tempoSaidaEv.evRoute->idRota<<" DO SATELITE: "<<satelite.sateliteId<<"\n\n");
                        throw "ERRO";
                    }

                }

            }
        }


        PRINT_DEBUG("", "ERRO SATELITE: "<<satelite.sateliteId<<", DEVERIA TER PELO MENOS UMA ROTA(ORDENADAS DE FORMA CRESENTE COM TEMPO DE SAIDA) COM TEMPO DE SAIDA MAIOR OU IGUAL A "<<tempoChegada<<"\n\n");
        throw "ERRO";

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
    //if(secondEchelonGreedy(sol, Inst, alpha))


    firstEchelonGreedy(sol, Inst, beta);


}


/*
bool GreedyAlgNS::canInsert(EvRoute &evRoute, int node, const Instance &Inst, Insertion &insertion)
{
    float demand = Inst.getDemand(node);
    float bestInsertionCost = FLOAT_MAX;
    bool viavel = false;
    EvRoute evRouteAux(Inst);
    InsercaoEstacao insercaoEstacao;


    if((evRoute.getDemand() + demand) > Inst.getEvCap()){
        return false;
    }

    std::copy(evRoute.route.begin(), evRoute.route.begin()+evRoute.routeSize, evRouteAux.route.begin());
    shiftVectorDir(evRouteAux.route, 1, 1, evRoute.routeSize);
    evRouteAux.routeSize = evRoute.routeSize+1;

    float distanciaRota  = 0.0;
    if(evRoute.routeSize > 2)
    {
        for(int i=0; (i+1) < evRoute.routeSize; ++i)
            distanciaRota += Inst.getDistance(evRoute[i], evRoute[i+1]);
    }

    //cout<<"\tnode: "<<node<<"\n";
    //cout<<"ROTA: "<<printVector(evRoute.route, evRoute.routeSize)<<"\n";
    for(int pos = 0; pos < evRoute.routeSize-1; pos++)
    {

        //int prevNode = evRoute.route.at(pos);
        //int nextNode = evRoute.route.at(pos + 1);
        evRouteAux.route[pos+1] = node;

        //cout<<"NOVA ROTA: "<<printVector(evRouteAux.route, evRouteAux.routeSize)<<"\n";


        float distanceAux = Inst.getDistance(evRoute[pos], node) + Inst.getDistance(node, evRoute[pos+1]) - Inst.getDistance(evRoute[pos], evRoute[pos+1]);

        float batteryConsumptionAux = 0.0;

        // Ate nextNode(pos+1)
        float remaingBattery = evRoute.vetRemainingBattery[pos] - Inst.getDistance(evRoute[pos], node) - Inst.getDistance(node, evRoute[pos+1]);


        bool batteryOk = true;


        if(remaingBattery < -TOLERANCIA_BATERIA)
               batteryOk = false;

        if(Inst.isRechargingStation(evRoute[pos+1]))
            remaingBattery = Inst.getEvBattery();

        //cout<<"pos: "<<pos<<"; "<<evRoute[pos]<<": "<<evRoute.vetRemainingBattery[pos]<<"\n";
        //cout<<"pos+1: "<<pos+1<<"; "<<evRoute[pos+1]<<": "<<remaingBattery<<";\n";

        if(batteryOk)
        {

            for(int i = pos + 1; (i + 1) < evRoute.routeSize; ++i)
            {
                remaingBattery -= Inst.getDistance(evRoute[i], evRoute[i+1]);

                if(remaingBattery < -TOLERANCIA_BATERIA)
                {
                    batteryOk = false;
                    break;
                }


                if(Inst.isRechargingStation(evRoute[i]))
                    remaingBattery = Inst.getEvBattery();


                //cout<<"\tpos: "<<pos<<"; "<<evRoute[pos]<<": "<<remaingBattery<<";\n";

                //cout<<evRoute[i+1]<<": "<<remaingBattery<<"\n";
                //cout<<"pos: "<<i+1<<"; "<<evRoute[i+1]<<": "<<remaingBattery<<";\n";
            }
        }

        if(batteryOk)
        {

            float insertionCost = distanceAux;
            if(insertionCost < bestInsertionCost)
            {
                bestInsertionCost = insertionCost;
                insertion = Insertion(pos, node, insertionCost, demand, batteryConsumptionAux, insertion.routeId, insertion.satId, -1, -1, {});
                viavel = true;
                //cout<<"\t\tATUALIZOU BEST_INSERTION\n";

            }
        }
        else
        {
            //cout<<"VIABILIZA ROTA: "<<printVector(evRouteAux.route, evRouteAux.routeSize)<<"\n";
            if(viabilizaRotaEv(evRouteAux.route, evRouteAux.routeSize, Inst, true, insercaoEstacao))
            {
                float insertionCost = insercaoEstacao.distanciaRota - distanciaRota;
                if(insertionCost < bestInsertionCost)
                {
                    bestInsertionCost = insertionCost;
                    insertion = Insertion(pos, node, insertionCost, demand, batteryConsumptionAux, insertion.routeId, insertion.satId, -1, -1, insercaoEstacao);
                    viavel = true;
                    //cout<<"\t\tVIABILIZA ROTA ATUALIZOU BEST_INSERTION\n";
                }
            }
        }

        evRouteAux[pos+1] = evRouteAux[pos+2];

    }

    //cout<<"\n\n";
    return viavel;

}



bool GreedyAlgNS::insert(EvRoute &evRoute, Insertion &insertion, const Instance &Inst)
{

    const int pos = insertion.pos;
    const int node = insertion.clientId;
    if(pos < 0 )
        return false;

    if(node < 0)
        return false;


    // checar se consegue com a capacidade atual
    // checar se consegue com a bateria atual (a bateria do veiculo antes da proxima recarga.
    //  inserir
    //  atualizar capacidade
    //  atualizar bateria
    //  atualizar custo

    // atualiza estruturas auxiliares:
    evRoute.totalDemand += insertion.demand;
    evRoute.distance += insertion.cost;
    evRoute.maxDemand = insertion.demand > evRoute.maxDemand ? insertion.demand : evRoute.maxDemand;
    evRoute.minDemand = insertion.demand < evRoute.maxDemand ? insertion.demand : evRoute.maxDemand;

    // ignoring battery distance for now;
    //this->rechargingStations;
    int k = pos;


    //cout<<"ROTA ANTIGA: "<<printVector(evRoute.route, evRoute.routeSize)<<"\n";

    shiftVectorDir(evRoute.route, pos+1, 1, evRoute.routeSize);
    evRoute.route[pos+1] = node;
    evRoute.routeSize += 1;

    //cout<<"ROTA CLIENTE: "<<printVector(evRoute.route, evRoute.routeSize)<<"\n";

    if(insertion.insercaoEstacao.pos >= 0)
    {


        shiftVectorDir(evRoute.route, insertion.insercaoEstacao.pos+1, 1, evRoute.routeSize);
        evRoute.route[insertion.insercaoEstacao.pos+1] = insertion.insercaoEstacao.estacao;
        evRoute.routeSize += 1;
    }




    // Atualizando estruturas auxiliares (demanda maxima e minima da rota)
    if(insertion.demand < evRoute.getMinDemand())
        evRoute.minDemand = insertion.demand;

    if(insertion.demand > evRoute.getMaxDemand())
        evRoute.maxDemand = insertion.demand;


    //cout<<"INSERTE\n";

    evRoute.distance = testaRota(evRoute.route, evRoute.routeSize, Inst, &evRoute.vetRemainingBattery);

    if(evRoute.distance <= 0.0)
    {
        string rotaStr;
        evRoute.print(rotaStr, Inst);

        PRINT_DEBUG("", "ERRO NA FUNCAO GreedyAlgNS::insert, BATERIA DA ROTA EH INVIAVEL: "<<rotaStr<<"\n\n");
        throw "ERRO";
    }


    //evRoute.print(Inst);
//    evRoute.vetRemainingBattery

    return true;
}

*/
bool GreedyAlgNS::insereEstacao(int rotaId, int satId) { return false; };

bool GreedyAlgNS::canInsertSemBateria(EvRoute &evRoute, int node,
                                      const Instance &Instance,
                                      Insertion &insertion) {
  return false;
};
