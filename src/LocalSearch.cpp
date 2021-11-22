//
// Created by igor on 19/11/2021.
//

#include "LocalSearch.h"

// Exchange
bool isViableSwap(EvRoute& Ev0, EvRoute Ev1, int c0, int c1, const Instance& Inst, float& cost, NS_LocalSearch::LocalSearch& localSearch){
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
    NS_LocalSearch::LocalSearch bestLs;
    NS_LocalSearch::LocalSearch currentLs;
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
                                                        evRoute0.getMaxDemand() // a carga atual da rota0 + a carga do menor cliente da rota1  MENOS a menor carga da rota0 deve ser menor que a capacidade do veiculo.
                    && evRoute1.getInitialCapacity() >=
                       evRoute0.getMinDemand() + evRoute1.getDemand(Inst) - evRoute1.getMaxDemand()) { // e vice versa
                    // Para cada cliente c0 na rota0
                    for (int c0 = 1; c0 < evRoute0.size() - 1; c0++) {
                        // se nao for estacao de recarga e tambem nao tiver uma demanda que ultrapassa a demanda maxima que a troca suporta,
                        if (!evRoute0.isRechargingS(c0, Inst)
                            && evRoute0.getInitialCapacity() >= evRoute1.getMinDemand()
                                                                + evRoute0.getDemand(Inst)
                                                                - evRoute0.getDemandOf(c0, Inst)) { // a carga atual da rota0 + a carga do menor cliente da rota1  MENOS a carga do cliente c0 deve ser menor que a capacidade do veiculo.
                            // Para cada cliente c1 na rota1
                            for (int c1 = 1; c1 < evRoute1.size() - 1; c1++) {
                                // se nao for estacao de recarga..
                                float currentCost = 1e8;
                                currentLs = {};
                                if (isViableSwap(evRoute0, evRoute1, c0, c1, Inst, currentCost, currentLs)) {
                                    if (currentCost > bestCost) {
                                        //// ATUALIZA O OBJETO BEST_LS COM AS NOVAS INFORMACOES.
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
        }
    }
    return false;
}
