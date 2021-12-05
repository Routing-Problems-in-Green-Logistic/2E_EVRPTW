//
// Created by igor on 19/11/2021.
//

#include "LocalSearch.h"
using namespace NS_LocalSearch;

// Exchange
bool isViableSwap(EvRoute& Ev0, EvRoute Ev1, int c0, int c1, const Instance& Inst, float& cost, LocalSearch2& localSearch){
    float remainingCap0 = Ev0.getCurrentCapacity();
    float remainingCap1 = Ev1.getCurrentCapacity();
    float demand0 = Ev0.getDemandOf(c0, Inst);
    float demand1 = Ev1.getDemandOf(c1, Inst);
    // check capacity
    if(remainingCap0 - demand1 < 0 || remainingCap1 - demand0 < 0){
        return false;
    }
    float distEv0toC1 =
            + Inst.getDistance(Ev0.getNodeAt(c0 - 1), Ev1.getNodeAt(c1)) // anterior ate o novo
            + Inst.getDistance(Ev0.getNodeAt(c0 + 1), Ev1.getNodeAt(c1)) // novo ate o proximo
            - Inst.getDistance(Ev0.getNodeAt(c0 - 1), Ev0.getNodeAt(c0)) // (-) anterior ate antigo
            - Inst.getDistance(Ev0.getNodeAt(c0 + 1), Ev0.getNodeAt(c0)); // novo ate o proximo
    float distEv1toC0 =
            + Inst.getDistance(Ev1.getNodeAt(c0 - 1), Ev0.getNodeAt(c0)) // anterior ate o novo
            + Inst.getDistance(Ev1.getNodeAt(c0 + 1), Ev0.getNodeAt(c0)) // novo ate o proximo
            - Inst.getDistance(Ev1.getNodeAt(c0 - 1), Ev1.getNodeAt(c1)) // (-) anterior ate antigo
            - Inst.getDistance(Ev1.getNodeAt(c0 + 1), Ev1.getNodeAt(c1)); // novo ate o proximo
    // Check battery;
    if(distEv0toC1 > Ev0.getRemainingBatteryBefore(c0) || distEv1toC0 > Ev1.getRemainingBatteryBefore(c1)){
        return false;
    }
    cost = distEv0toC1 + distEv1toC0;
    //Insertion ins0(c0, Ev0.getNodeAt(c0), dist
    return true;
}

bool NS_LocalSearch::intraSatelliteSwap(Solution &Sol, int satId, const Instance& Inst, float &improvement) {
    Satelite* sat = Sol.getSatelite(satId);
    LocalSearch2 bestLs;
    LocalSearch2 currentLs;
    float bestCost = 1e8;
    bool improving = true;
    while(improving) {
        improving = false;
        for (int i = 0; i < sat->getNRoutes(); i++) {
            for (int j = 0; j < sat->getNRoutes(); j++) {
                EvRoute &evRoute0 = sat->getRoute(i);
                EvRoute &evRoute1 = sat->getRoute(j);
                if (i != j
                    && evRoute0.getInitialCapacity() >= evRoute1.getMinDemand() + evRoute0.getDemand(Inst) -
                                                        evRoute0.getMaxDemand() // a carga atual da rota0 + a carga do menor satelliteId da rota1  MENOS a menor carga da rota0 deve ser menor que a capacidade do veiculo.
                    && evRoute1.getInitialCapacity() >=
                       evRoute0.getMinDemand() + evRoute1.getDemand(Inst) - evRoute1.getMaxDemand()) { // e vice versa
                    // Para cada satelliteId c0 na rota0
                    for (int c0 = 1; c0 < evRoute0.size() - 1; c0++) {
                        // se nao for estacao de recarga e tambem nao tiver uma demanda que ultrapassa a demanda maxima que a troca suporta,
                        if (!evRoute0.isRechargingS(c0, Inst)
                            && evRoute0.getInitialCapacity() >= evRoute1.getMinDemand()
                                                                + evRoute0.getDemand(Inst)
                                                                - evRoute0.getDemandOf(c0, Inst)) { // a carga atual da rota0 + a carga do menor satelliteId da rota1  MENOS a carga do satelliteId c0 deve ser menor que a capacidade do veiculo.
                            // Para cada satelliteId c1 na rota1
                            for (int c1 = 1; c1 < evRoute1.size() - 1; c1++) {
                                // se nao for estacao de recarga..
                                float currentCost = 1e8;
                                currentLs = {};
                                if (isViableSwap(evRoute0, evRoute1, c0, c1, Inst, currentCost, currentLs)) {
                                    if (currentCost > bestCost) {
                                        //// ATUALIZA O OBJETO bestLs COM AS NOVAS INFORMACOES.
                                        bestLs = {false, satId, -1, true, MOV_SWAP, i, j, c0, c1};
                                        improving = true;
                                        bestCost = currentCost;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        if(improving){
            /// aplica movimento de swap na solucao;
            swapMov(Sol, bestLs);
        }
    }
    return false;
}

bool NS_LocalSearch::interSatelliteSwap(Solution &Sol, const Instance &Inst, float &improvement) {
    LocalSearch2 bestLs;
    LocalSearch2 currentLs;
    float bestCost = 1e8;
    bool improving = true;
    while (improving) {
        improving = false;
        for (int s0 = 0; s0 < Sol.getNSatelites(); s0++) {
            Satelite *sat0 = Sol.getSatelite(s0);
            for (int s1 = 0; s1 < Sol.getNSatelites(); s1++) {
                Satelite *sat1 = Sol.getSatelite(s1);
                for (int i = 0; i < sat0->getNRoutes(); i++) {
                    for (int j = 0; j < sat1->getNRoutes(); j++) {
                        EvRoute &evRoute0 = sat0->getRoute(i);
                        EvRoute &evRoute1 = sat1->getRoute(j);
                        if (i != j
                            && evRoute0.getInitialCapacity() >= evRoute1.getMinDemand() + evRoute0.getDemand(Inst) -
                                                                evRoute0.getMaxDemand() // a carga atual da rota0 + a carga do menor cliente da rota1  MENOS a menor carga da rota0 deve ser menor que a capacidade do veiculo.
                            && evRoute1.getInitialCapacity() >=
                               evRoute0.getMinDemand() + evRoute1.getDemand(Inst) -
                               evRoute1.getMaxDemand()) { // e vice versa
                            // Para cada cliente c0 na rota0
                            for (int c0 = 1; c0 < evRoute0.size() - 1; c0++) {
                                // se nao for estacao de recarga e tambem nao tiver uma demanda que ultrapassa a demanda maxima que a troca suporta,
                                if (!evRoute0.isRechargingS(c0, Inst)
                                    && evRoute0.getInitialCapacity() >= evRoute1.getMinDemand()
                                                                        + evRoute0.getDemand(Inst)
                                                                        - evRoute0.getDemandOf(c0,
                                                                                               Inst)) { // a carga atual da rota0 + a carga do menor cliente da rota1  MENOS a carga do cliente c0 deve ser menor que a capacidade do veiculo.
                                    // Para cada cliente c1 na rota1
                                    for (int c1 = 1; c1 < evRoute1.size() - 1; c1++) {
                                        // se nao for estacao de recarga..
                                        float currentCost = 1e8;
                                        currentLs = {};
                                        if (isViableSwap(evRoute0, evRoute1, c0, c1, Inst, currentCost, currentLs)) {
                                            if (currentCost > bestCost) {
                                                //// ATUALIZA O OBJETO BEST_LS COM AS NOVAS INFORMACOES.
                                                bestLs = {true, s0, s1, true, MOV_SWAP, i, j, c0, c1};
                                                improving = true;
                                                bestCost = currentCost;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        if(improving){
            /// aplica movimento de swap na solucao;
            swapMov(Sol, bestLs);
        }
    }
    return false;
}
void NS_LocalSearch::swapMov(Solution& Sol, const LocalSearch2& mov){
    int indexSat0, indexSat1;
    int indexRoute0, indexRoute1;
    int c0, c1;
    indexSat0 = mov.idSat0; // TODO(Samuel): rename LocalSearch2::idSat to indexSat
    if(mov.satellites2){
        indexSat1 = mov.idSat1;
    } else {
        indexSat1 = indexSat0;
    }
    indexRoute0 = mov.idRoute0; //TODO(samuel): also rename to indexRoute0
    if(mov.interRoutes) {
        indexRoute1 = mov.idRoute1;
    }
    else{
        indexRoute1 = indexRoute0;
    }
    c0 = mov.pos0;
    c1 = mov.pos1;
    EvRoute& evRoute0 = Sol.getSatelite(indexSat0)->getRoute(indexRoute0);
    EvRoute& evRoute1 = Sol.getSatelite(indexSat1)->getRoute(indexRoute1);
}

bool NS_LocalSearch::intraSatelliteShifit(Solution &solution, const Instance &instance)
{

    LocalSearch localSearchBest;


    // Shifit intra rota
    for(int satId = 0; satId < instance.getNSats(); ++satId)
    {
        Satelite *satelite = solution.satelites[satId];

        // Percorre todos os satellites
        for(int routeId = 0; routeId < satelite->getNRoutes(); ++routeId)
        {
            EvRoute &evRoute = satelite->vetEvRoute[routeId];

            if(evRoute.routeSize > 2)
            {
                // Percorre todos os clientes da rota
                for(int i=1; i < (evRoute.routeSize-1); ++i)
                {
                    float incrementoDistancia = -instance.getDistance(evRoute.route[i-1], evRoute.route[i]) -instance.getDistance(evRoute.route[i], evRoute.route[i+1]) +
                                                + instance.getDistance(evRoute.route[i-1], evRoute.route[i+1]);

                    const int cliente = evRoute.route[i];

                    if(!instance.isRechargingStation(cliente))
                    {

                        for(int pos = 0; i < (evRoute.routeSize - 1); ++pos)
                        {
                            if((i - 1) != pos)
                            {
                                const float incDistAux = incrementoDistancia - instance.getDistance(evRoute.route[pos],evRoute.route[pos +1])+instance.getDistance(evRoute.route[pos], cliente) +
                                                         +instance.getDistance(cliente, evRoute.route[pos + 1]);

                                // Verifica se o incremento eh negativo(melhora) e menor que o melhor movimento
                                if(incDistAux < 0.0 && incDistAux < localSearchBest.incrementoDistancia)
                                {
                                    // Verifica viabilidade

                                    float remainingBatt = evRoute.vetRemainingBattery[pos] - instance.getDistance(evRoute.route[pos], cliente) - instance.getDistance(cliente, evRoute.route[pos+1]);
                                    int indice = pos+1;

                                    bool viavel = true;

                                    // Verifica a batteria
                                    while(remainingBatt >= 0.0 && (indice+1) < evRoute.routeSize)
                                    {
                                        // Verifica se (indice+1) e indice sao diferentes de cliente
                                        if(evRoute.route[indice+1] != cliente && evRoute.route[indice] != cliente)
                                           remainingBatt -= instance.getDistance(evRoute.route[indice], evRoute.route[indice+1]);

                                        // verifica se (indice) eh igual a cliente
                                        else if(evRoute.route[indice] == cliente)
                                            remainingBatt -= instance.getDistance(evRoute.route[indice-1], evRoute.route[indice+1]);

                                        // Verifica se (indice+1) eh igual a cliente
                                        else if(evRoute.route[indice+1] == cliente)
                                            remainingBatt -= instance.getDistance(evRoute.route[indice], evRoute.route[indice+2]);

                                        // Verifica se (indice+1) eh uma estacao de recarga
                                        if(instance.isRechargingStation(evRoute.route[indice+1]))
                                        {
                                            // Verifica se a batteria restante eh 'negativa'
                                            if(remainingBatt < -BATTERY_TOLENCE)
                                            {
                                                viavel = false;
                                                break;
                                            }
                                            else
                                            {
                                                viavel = true;
                                                remainingBatt = instance.getEvBattery();
                                                break;
                                            }

                                        }
                                    }

                                    // Verifica a viabilidade
                                    if(remainingBatt >= -BATTERY_TOLENCE && viavel)
                                    {
                                        // Atualiza localSearchBest

                                        localSearchBest.idSat0 = satId;
                                        localSearchBest.idSat1 = -1;
                                        localSearchBest.incrementoDistancia = incDistAux;
                                        localSearchBest.interRoutes = false;
                                        localSearchBest.mov = MOV_SHIFIT;

                                        localSearchBest.inser0.satId = satId;
                                        localSearchBest.inser0.routeId = routeId;
                                        localSearchBest.inser0.clientId = cliente;
                                        localSearchBest.inser0.pos = pos;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }


    // Verifica se o incremento eh negativo
    if(localSearchBest.incrementoDistancia < 0.0)
    {
        Satelite *satelite = solution.satelites[localSearchBest.idSat0];
        Insertion &insertion = localSearchBest.inser0;
        EvRoute &evRoute = satelite->vetEvRoute[insertion.routeId];

        //   V   .
        // 0 1 2 3 4 5 0
        //

        // Shift em pos+1

        NS_Auxiliary::shiftVectorDir(evRoute.route, insertion.pos+1, 1, evRoute.routeSize);
        evRoute.route[insertion.pos+1] = insertion.clientId;

        for(int i=0; i < evRoute.routeSize+1; ++i)
        {
            if(evRoute.route[i] == insertion.clientId && i != insertion.pos+1)
            {
                for(int j=i; j < evRoute.routeSize; ++j)
                    evRoute.route[j] = evRoute.route[j+1];
            }
        }

    }

}