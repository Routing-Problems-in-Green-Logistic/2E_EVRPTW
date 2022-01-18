//
// Created by igor on 19/11/2021.
//

#include "LocalSearch.h"
#include "Auxiliary.h"
#include "mersenne-twister.h"
#include <memory>

using namespace NS_LocalSearch;
using namespace NS_Auxiliary;

bool isViableSwap(EvRoute& Ev0, EvRoute& Ev1, int c0, int c1, const Instance& Inst, LocalSearch2& localSearch){
    try {
        float remainingCap0 = Ev0.getCurrentCapacity();
        float remainingCap1 = Ev1.getCurrentCapacity();
        float demand0 = Ev0.getDemandOf(c0, Inst);
        float demand1 = Ev1.getDemandOf(c1, Inst);
        // check capacity
        if (remainingCap0 - demand1 < 0 || remainingCap1 - demand0 < 0) {
            return false;
        }
        float distEv0toC1 =
                +Inst.getDistance(Ev0.getNodeAt(c0 - 1), Ev1.getNodeAt(c1)) // anterior ate o novo
                + Inst.getDistance(Ev0.getNodeAt(c0 + 1), Ev1.getNodeAt(c1)) // novo ate o proximo
                - Inst.getDistance(Ev0.getNodeAt(c0 - 1), Ev0.getNodeAt(c0)) // (-) anterior ate antigo
                - Inst.getDistance(Ev0.getNodeAt(c0 + 1), Ev0.getNodeAt(c0)); // novo ate o proximo
        float distEv1toC0 =
                +Inst.getDistance(Ev1.getNodeAt(c1 - 1), Ev0.getNodeAt(c0)) // anterior ate o novo
                + Inst.getDistance(Ev1.getNodeAt(c1 + 1), Ev0.getNodeAt(c0)) // novo ate o proximo
                - Inst.getDistance(Ev1.getNodeAt(c1 - 1), Ev1.getNodeAt(c1)) // (-) anterior ate antigo
                - Inst.getDistance(Ev1.getNodeAt(c1 + 1), Ev1.getNodeAt(c1)); // novo ate o proximo
        // Check battery;
        if (distEv0toC1 > Ev0.getRemainingBatteryBefore(c0) || distEv1toC0 > Ev1.getRemainingBatteryBefore(c1)) {
            return false;
        }
        localSearch.incrementoDistancia = distEv0toC1 + distEv1toC0;
        return true;
    }
    catch(std::out_of_range &e){
        cerr << "out of range @ LocalSearch::isViableSwap";
        exit(14);
    }
    catch(const char* e) {
        std::cerr << "outro erro @ LocalSearch::isViableSwap";
    }

}
bool NS_LocalSearch::interSatelliteSwap(Solution &Sol, const Instance &Inst, float &improvement) {
    LocalSearch2 bestLs;
    LocalSearch2 currentLs;
    for (int s0 = 0; s0 < Sol.getNSatelites(); s0++) {
        Satelite *sat0 = Sol.getSatelite(s0);
        // para cada satelite sat1
        for (int s1 = 0; s1 < Sol.getNSatelites(); s1++) {
            Satelite *sat1 = Sol.getSatelite(s1);
            // if s1 != s0 ??
            // para cada rota evRoute0
            for (int i = 0; i < sat0->getNRoutes(); i++) {
                EvRoute &evRoute = sat1->getRoute(i);
                evRoute.setAuxStructures(Inst);
            }
        }
    }
    bool improving = true;
    while (improving) {
        improving = false;
        // para cada satelite sat0
        for (int s0 = 0; s0 < Sol.getNSatelites(); s0++) {
            Satelite *sat0 = Sol.getSatelite(s0);
            // para cada satelite sat1
            for (int s1 = 0; s1 < Sol.getNSatelites(); s1++) {
                Satelite *sat1 = Sol.getSatelite(s1);
                // if s1 != s0 ??
                // para cada rota evRoute0
                for (int i = 0; i < sat0->getNRoutes(); i++) {
                    EvRoute &evRoute0 = sat0->getRoute(i);
                    // para cada rota evRoute1
                    for (int j = 0; j < sat1->getNRoutes(); j++) {
                        EvRoute &evRoute1 = sat1->getRoute(j);
                        if (i != j){
                            /* && evRoute0.size() > 2 && evRoute1.size() > 2
                            && evRoute0.getInitialCapacity() >= evRoute1.getMinDemand() + evRoute0.getDemand(Inst) -
                                                                evRoute0.getMaxDemand() // a carga atual da rota0 + a carga do menor cliente da rota1  MENOS a menor carga da rota0 deve ser menor que a capacidade do veiculo.
                            && evRoute1.getInitialCapacity() >=
                               evRoute0.getMinDemand() + evRoute1.getDemand(Inst) -
                               evRoute1.getMaxDemand())
                               */
                        // e vice versa
                            // Para cada cliente c0 na rota evRoute0
                            for (int c0 = 1; c0 < evRoute0.size() - 1; c0++) {
                                if (!evRoute0.isRechargingS(c0, Inst)) {
                                    // se nao for estacao de recarga e tambem nao tiver uma demanda que ultrapassa a demanda maxima que a troca suporta,
                                    /*if (!evRoute0.isRechargingS(c0, Inst)
                                        && evRoute0.getInitialCapacity() >= evRoute1.getMinDemand()
                                                                            + evRoute0.getDemand(Inst)
                                                                            - evRoute0.getDemandOf(c0,
                                                                                                   Inst))  // a carga atual da rota0 + a carga do menor cliente da rota1  MENOS a carga do cliente c0 deve ser menor que a capacidade do veiculo.
                                        // Para cada cliente c1 na rota1
                                        */
                                    for (int c1 = 1; c1 < evRoute1.size() - 1; c1++) {
                                        // se nao for estacao de recarga..
                                        if (!evRoute1.isRechargingS(c1, Inst)) {
                                            currentLs = {true, s0, s1, true, MOV_SWAP, i, j, c0, c1, FLOAT_MAX};
                                            if (isViableSwap(evRoute0, evRoute1, c0, c1, Inst, currentLs)) {
                                                if (currentLs.incrementoDistancia < bestLs.incrementoDistancia) {
                                                    //// ATUALIZA O OBJETO BEST_LS COM AS NOVAS INFORMACOES.
                                                    // note que a best_ls eh sempre atualizada, mesmo que nao tenha melhoramento na solucao. (Claro que nao conta como melhoramento propriamente  dito.
                                                    bestLs = currentLs;
                                                    if(bestLs.incrementoDistancia < BATTERY_TOLENCE) {
                                                        improving = true;
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
            }
        }
        if(improving){
            /// aplica movimento de swap na solucao;
            swapMov(Sol, bestLs, Inst);
        }
    }
    return false;
}
void NS_LocalSearch::swapMov(Solution& Sol, const LocalSearch2& mov, const Instance& Inst){
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

    int client0 = evRoute0.getNodeAt(c0);
    int client1 = evRoute1.getNodeAt(c1);
    // troca os elementos
    evRoute0.replace(c0, client1, mov.incrementoDistancia, Inst);
    evRoute1.replace(c1, client0, mov.incrementoDistancia, Inst);
}



void NS_LocalSearch::getMov(const int movId, string &mov)
{
    switch(movId)
    {
        case MOV_SHIFIT:
            mov = "MOV_SHIFIT";

        case MOV_SWAP:
            mov = "MOV_SWAP";

        case MOV_2_OPT:
            mov = "MOV_2_OPT";

        case MOV_CROSS:
            mov = "MOV_CROSS";

        default:
            mov = "MOV";

    }
}



void NS_LocalSearch::LocalSearch::print() const
{
    string movStr;
    getMov(mov, movStr);

    cout<<"LOCAL SEARCH:\n";
    cout<<"\nMOV: "<<movStr;
    cout<<"INTER ROUTES: "<<(interRoutes?"TRUE":"FALSE");
    cout<<"\nidSat0: "<<idSat0;

    cout<<"\n\ninser0:";
    cout<<"\n\tpos: "<<inser0.pos;
    cout<<"\n\tclientId: "<<inser0.clientId;

    cout<<"\nINCR. DIST.: "<<incrementoDistancia;

}


void NS_LocalSearch::LocalSearch::print(string &str)
{


}

/*
 * Posiçao erada
 */

bool NS_LocalSearch::mvShifitIntraRota(Solution &solution, const Instance &instance)
{

    LocalSearch localSearchBest;


    // Shifit intra rota
    for(int satId = 0; satId < instance.getNSats(); ++satId)
    {
        //PRINT_DEBUG("", "ANTES DE PEGAR SATELLITE");


        Satelite *satelite = solution.satelites[satId];

        //PRINT_DEBUG("", "DEPOIS DE PEGAR SATELLITE");

        //cout<<"\n\n";

        // Percorre todas as rotas do satellite
        for(int routeId = 0; routeId < satelite->getNRoutes(); ++routeId)
        {

            //PRINT_DEBUG("", "ANTES DE PEGAR ROTA EV");

            EvRoute &evRoute = satelite->vetEvRoute[routeId];

            //evRoute.print();

            //PRINT_DEBUG("", "DEPOIS DE PEGAR ROTA EV");

            //cout<<"\n\n";

            if(evRoute.routeSize > 3)
            {
                // Percorre todos os clientes da rota
                // Cliente na possicao i fara o shift
                for(int i=1; i < (evRoute.routeSize-1); ++i)
                {


                    //PRINT_DEBUG("", "ANTES CALCULO INCREMENTO DISTANCIA");

                    float incrementoDistancia = -instance.getDistance(evRoute.route[i-1], evRoute.route[i]) -instance.getDistance(evRoute.route[i], evRoute.route[i+1]) +
                                                + instance.getDistance(evRoute.route[i-1], evRoute.route[i+1]);

                    //PRINT_DEBUG("", "DEPOIS CALCULO INCREMENTO DISTANCIA");
                    //cout<<"\n\n";

                    const int clienteShift = evRoute.route[i];

                    if(!instance.isRechargingStation(clienteShift))
                    {
                        // pos é a nova possicao para o cliente
                        for(int pos = 0; pos < (evRoute.routeSize - 1); ++pos)
                        {
                            if(i != (pos+1) && i != pos)
                            {
                                //PRINT_DEBUG("", "ANTES CALCULO INC DIST AUX");
                                int r = evRoute.route[pos+1];
                                //PRINT_DEBUG("\t", "pos+1: route["<<pos+1<<"]: "<<r);
                                const float incDistAux = incrementoDistancia - instance.getDistance(evRoute.route[pos],evRoute.route[pos +1]) + instance.getDistance(evRoute.route[pos], clienteShift) +
                                                         +instance.getDistance(clienteShift, evRoute.route[pos + 1]);

                                //PRINT_DEBUG("", "DEPOIS CALCULO INC DIST AUX");
                                //cout<<"\n\n";

                                // Verifica se o incremento eh negativo(melhora) e menor que o melhor movimento
                                if(incDistAux < 0.0 && incDistAux < localSearchBest.incrementoDistancia)
                                {
                                    // Verifica viabilidade

                                    float remainingBatt = evRoute.vetRemainingBattery[pos] - instance.getDistance(evRoute.route[pos], clienteShift) - instance.getDistance(clienteShift, evRoute.route[pos + 1]);
                                    int indice = pos+1;

                                    bool viavel = true;

                                    // Verifica a batteria
                                    while(remainingBatt >= 0.0 && (indice+1) < evRoute.routeSize)
                                    {
                                        // Verifica se (indice+1) e indice sao diferentes de clienteShift
                                        if(evRoute.route[indice+1] != clienteShift && evRoute.route[indice] != clienteShift)
                                           remainingBatt -= instance.getDistance(evRoute.route[indice], evRoute.route[indice+1]);

                                        // verifica se (indice) eh igual a clienteShift
                                        else if(evRoute.route[indice] == clienteShift)
                                            remainingBatt -= instance.getDistance(evRoute.route[indice-1], evRoute.route[indice+1]);

                                        // Verifica se (indice+1) eh igual a clienteShift
                                        else if(evRoute.route[indice+1] == clienteShift)
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

                                        indice += 1;
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
                                        localSearchBest.inser0.clientId = clienteShift;
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
    if(localSearchBest.incrementoDistancia < -1e-2 && localSearchBest.mov >= 0)
    {

        //localSearchBest.print();

//        string str;
//        solution.print(str);
//        cout<<str<<"\n\n";

        Satelite *satelite = solution.satelites[localSearchBest.idSat0];
        Insertion &insertion = localSearchBest.inser0;
        EvRoute &evRoute = satelite->vetEvRoute[insertion.routeId];

        //   V   .
        // 0 1 2 3 4 5 0
        //

        // Shift em pos+1

        NS_Auxiliary::shiftVectorDir(evRoute.route, insertion.pos+1, 1, evRoute.routeSize);
        NS_Auxiliary::shiftVectorDir(evRoute.rechargingStationRoute, insertion.pos+1, 1, evRoute.routeSize);

        evRoute.route[insertion.pos+1] = insertion.clientId;
        evRoute.rechargingStationRoute[insertion.pos+1] = instance.isRechargingStation(insertion.clientId);

        int clientePos = -1;

        for(int i=0; i < evRoute.routeSize+1; ++i)
        {
            // Verifica se o cliente na possicao i eh igual a clienteId
            if(evRoute.route[i] == insertion.clientId && i != insertion.pos+1)
            {
                clientePos = i;

                // Exclui a posicao i dos vetores route e rechargingStationRoute
                for(int j=i; j < evRoute.routeSize; ++j)
                {
                    evRoute.route[j] = evRoute.route[j + 1];
                    evRoute.rechargingStationRoute[j] = evRoute.rechargingStationRoute[j+1];
                }

                break;
            }
        }

        const int posMenor = (clientePos < insertion.pos+1)? clientePos:insertion.pos+1;
        const int posMaior = (clientePos >= insertion.pos+1)? clientePos:insertion.pos+1;

        // Corrige o vetor rechargingStationRoute entre posMenor e posMaior ate encontrar um estacao de recarga ou chegar ao final da rota
        for(int i=posMenor; i < evRoute.routeSize; ++i)
        {
            evRoute.vetRemainingBattery[i] = evRoute.vetRemainingBattery[i-1] - instance.getDistance(evRoute.route[i-1], evRoute.route[i]);

            // Verifica se a batteria eh menor que a tolerancia
            if(evRoute.vetRemainingBattery[i] < -BATTERY_TOLENCE)
            {
                std::cerr <<"ERRO\nARQUIVO: "<<__FILE__<<"\nLINHA: "<<__LINE__<<"\nvetRemaingBattery["<<i<<"] = "<<evRoute.vetRemainingBattery[i] <<
                            " < -"<<BATTERY_TOLENCE<<" ID: "<<evRoute.route[i]<<"\n";

                evRoute.print();

                std::cout << "*******************************************\n\n";
                localSearchBest.print();

                throw "ERRO";
            }


            if(instance.isRechargingStation(evRoute.route[i]))
            {
                evRoute.vetRemainingBattery[i] = instance.getEvBattery();
                evRoute.rechargingStationRoute[i] = true;

                /*if(i > posMaior)
                    break;
                */
            }
            else
                evRoute.rechargingStationRoute[i] = false;
        }

        //PRINT_DEBUG("", "DISTANCIA ROTA ANTES: "<<evRoute.distance);

        // Atualiza distancia
        evRoute.distance += localSearchBest.incrementoDistancia;


        /*
        PRINT_DEBUG("", "INCREMENTO DISTANCIA: "<<localSearchBest.incrementoDistancia);
        PRINT_DEBUG("", "DISTANCIA: "<<evRoute.distance);
        PRINT_DEBUG("", "POS: "<<localSearchBest.inser0.pos);
        PRINT_DEBUG("", "CLIENTE: "<<localSearchBest.inser0.clientId);
        */
        //PRINT_DEBUG("", "MOVIMENTO SHIFIT INTRA ROTA");

        solution.mvShiftIntraRota = true;

        return true;

    }
    else
        return false;

}

bool NS_LocalSearch::mvShiftInterRotas(Solution &solution, const Instance &instance)
{

    LocalSearch localSearchBest;

    // Percorre os satellites
    for(int satId0 = 0; satId0 < instance.getNSats(); ++satId0)
    {

        Satelite *satelite0 = solution.satelites[satId0];

        // Percorre todas as rotas do satellite0
        for(int routeId0 = 0; routeId0 < satelite0->getNRoutes(); ++routeId0)
        {

            EvRoute &evRoute0 = satelite0->vetEvRoute[routeId0];

            for(int satId1 = satId0; satId1 < instance.getNSats(); ++satId1)
            {
                Satelite *satelite1 = solution.satelites[satId1];

                for(int routeId1 = 0; routeId1 < satelite1->getNRoutes(); ++routeId1)
                {
                    // Verifica se routeId0 == routeId1
                    if(satId0==satId1 && routeId0==routeId1)
                        continue;

                    EvRoute &evRoute1 = satelite1->vetEvRoute[routeId1];

                    shifitInterRotasMvDuasRotas({satId0,satId1}, {routeId0,routeId1}, evRoute0, evRoute1, localSearchBest, instance, solution.getDistanciaTotal());
                    shifitInterRotasMvDuasRotas({satId1,satId0}, {routeId1,routeId0}, evRoute1, evRoute0, localSearchBest, instance, solution.getDistanciaTotal());

                }
            }
        }
    }

    // Verifica se movimento eh de melhora
    if(localSearchBest.incrementoDistancia < 0.0)
    {
        // Atualiza solucao

        const int pos = localSearchBest.inser0.pos;
        const int clienteId = localSearchBest.inser0.clientId;

        const int satId0 = localSearchBest.inser0.satId;
        const int routeId0 = localSearchBest.inser0.routeId;
        EvRoute &evRoute0 = solution.getSatelite(satId0)->getRoute(routeId0);

        const int satId1 = localSearchBest.inser1.satId;
        const int routeId1 = localSearchBest.inser1.routeId;
        EvRoute &evRoute1 = solution.getSatelite(satId1)->getRoute(routeId1);

        const int posClienteRoute1 = localSearchBest.inser1.pos;

        evRoute0.distance += -instance.getDistance(evRoute0.route[pos], evRoute0.route[pos+1]) + instance.getDistance(evRoute0.route[pos], clienteId) +
                instance.getDistance(clienteId, evRoute0.route[pos+1]);

        shiftVectorDir(evRoute0.route, pos+1, 1, evRoute0.size());
        shiftVectorDir(evRoute0.vetRemainingBattery, pos, 1,evRoute0.size());
        shiftVectorDir(evRoute0.rechargingStationRoute, pos, 1,evRoute0.size());

        evRoute0.routeSize += 1;

        evRoute0.route[pos+1] = clienteId;
        evRoute0.rechargingStationRoute[pos+1] = false;

        float remainingBattery = evRoute0.rechargingStationRoute[pos];

        for(int i=pos; (i+1) < evRoute0.size(); ++i)
        {
            remainingBattery -= instance.getDistance(evRoute0.route[i], evRoute0.route[i+1]);

            if(remainingBattery < -BATTERY_TOLENCE)
            {
                PRINT_DEBUG("", "ERRO BATERIA!!");
                throw "erro";
            }

            evRoute0.vetRemainingBattery[i+1] = remainingBattery;

            if(instance.isRechargingStation(evRoute0.route[i+1]))
            {
                evRoute0.vetRemainingBattery[i + 1] = instance.getEvBattery();
                break;
            }

        }

        solution.mvShiftInterRotas = true;
        return true;
    }
    else
        return false;

}

void NS_LocalSearch::shifitInterRotasMvDuasRotas(const pair<int, int> satIdPair, const pair<int, int> routeIdPair, const EvRoute &evRoute0, const EvRoute &evRoute1, LocalSearch &localSearchBest, const Instance &instance, const float distSol)
{
    if(evRoute1.routeSize <=2)
        return;

    const int satId0 = satIdPair.first;
    const int satId1 = satIdPair.second;

    const int routeId0 = routeIdPair.first;
    const int routeId1 = routeIdPair.second;

    // Percorre as possicoes de rota0
    for(int posRoute0 = 0; posRoute0 < (evRoute0.routeSize-1); ++posRoute0)
    {

        // Escolhe um cliente da rota1 para ir para a rota0
        for(int posClieRoute1 = 1; posClieRoute1 < (evRoute1.routeSize-1); ++posClieRoute1)
        {
            const int clienteId = evRoute1.route[posClieRoute1];
            float distanciaRoute0 = evRoute0.distance;
            float distanciaRoute1 = evRoute1.distance;

            const float distanciaTotal = distanciaRoute0 + distanciaRoute1;

            if(!instance.isRechargingStation(clienteId))
            {

                // Verifica a capacidade do EV
                if((evRoute0.getDemand() + instance.getDemand(clienteId)) <= instance.getEvCap())
                {
                    // Calcula as novas distancias

                    distanciaRoute1 += -instance.getDistance(evRoute1.route[posClieRoute1-1], clienteId) - instance.getDistance(clienteId, evRoute1.route[posClieRoute1+1]) +
                                    instance.getDistance(evRoute1.route[posClieRoute1-1], evRoute1.route[posClieRoute1+1]);

                    distanciaRoute0 += -instance.getDistance(evRoute0.route[posRoute0], evRoute0.route[posRoute0+1]) + instance.getDistance(evRoute0.route[posRoute0], clienteId) +
                                    instance.getDistance(clienteId, evRoute0.route[posRoute0+1]);

                    const float novaDistanciaTotal = distanciaRoute0+distanciaRoute1;


                    const float incremento = distSol - distanciaTotal + novaDistanciaTotal;

                    // Verifica se o movimento melhora a solucao
                    if(incremento < 0.0)
                    {

                        PRINT_DEBUG("", "INCREMENTETO < 0");

                        if(incremento < localSearchBest.incrementoDistancia)
                        {
                            // Verifica a viabilidade da rota

                            float cargaBateria = evRoute0.vetRemainingBattery[posRoute0];

                            cargaBateria += -instance.getDistance(evRoute0.route[posRoute0], clienteId) - instance.getDistance(clienteId, evRoute0.route[posRoute0+1]);
                            int p = posRoute0+1;
                            bool viavel = true;

                            while((p+1) < evRoute0.routeSize)
                            {

                                if(cargaBateria >= -BATTERY_TOLENCE)
                                {
                                    if(instance.isRechargingStation(evRoute0.route[p]))
                                        cargaBateria = instance.getEvBattery();


                                    cargaBateria -= instance.getDistance(evRoute0.route[p], evRoute0.route[p+1]);

                                }
                                else
                                {
                                    viavel = false;
                                    break;
                                }

                                p += 1;
                            }

                            if(viavel)
                            {
                                // Atualiza melhor movimento

                                localSearchBest.inser0.pos = posRoute0;
                                localSearchBest.inser0.clientId = clienteId;
                                localSearchBest.inser0.satId = satId0;
                                localSearchBest.inser0.routeId = routeId0;

                                localSearchBest.incrementoDistancia = incremento;

                                localSearchBest.inser1.pos = posClieRoute1;
                                localSearchBest.inser1.clientId = clienteId;
                                localSearchBest.inser1.satId = satId1;
                                localSearchBest.inser1.routeId = routeId1;

                            }
                            else
                                PRINT_DEBUG("", "BATERIA INVIAVEL!");
                        }
                    }
                }
            }
        }
    }
}

/**
 *
 * Descricao ...
 *
 * @param solution      Solucao ; Pode ser alterado
 * @param instance      Instancia do problema ; Constante
 * @param evRoute0      Rota0 do veiculo eletrico, utilizada no movimento ; Pode ser alterado
 * @param evRoute1      Rota1 do veiculo eletrico, utilizada no movimento ; Pode ser alterado
 * @return              Retorna se conseguiu realizar o movimento
 */

bool NS_LocalSearch::mvCross(Solution &solution, const Instance &instance)
{
    PRINT_DEBUG("", "MV CROSS");

    int satId0 = rand_u32() % solution.getNSatelites();
    Satelite *sat0 = solution.getSatelite(satId0);

    int routeId0 = rand_u32() % sat0->getNRoutes();
    EvRoute &evRouteSol0 = sat0->getRoute(routeId0);

    if(evRouteSol0.routeSize <= 2)
    {

        const int sat0Inic = satId0;

        // Percorre todos os satellites
        do
        {

            const int route0Inic = routeId0;
            routeId0 = (routeId0 + 1) % sat0->getNRoutes();

            // Percorre todas as rotas
            while(routeId0 != route0Inic)
            {
                evRouteSol0 = sat0->getRoute(routeId0);

                if(evRouteSol0.routeSize > 2)
                    break;

                routeId0 = (routeId0 + 1) % sat0->getNRoutes();
            }

            if(evRouteSol0.routeSize > 2)
                break;

            satId0 = (satId0 + 1) % solution.getNSatelites();

        }while(satId0 != sat0Inic);

        if(evRouteSol0.routeSize <= 2)
            return false;

    }


    LocalSearch localSearchBest;

    for(int satId1 = 0; satId1 < instance.getNSats(); ++satId1)
    {

        Satelite *sat1 = solution.getSatelite(satId1);

        for(int routeId1 = 0; routeId1 < sat1->getNRoutes(); ++routeId1)
        {
            if((satId1 == satId0) && (routeId1 == routeId0))
                continue;

            EvRoute &evRouteSol1 = sat1->getRoute(routeId1);

            crossAux({satId0, satId1}, {routeId0, routeId1}, evRouteSol0, evRouteSol1, localSearchBest, instance);


        }

    }


    if(localSearchBest.incrementoDistancia < 0.0)
    {

        // Atualiza solucao

        // *************************************************************************************************************
        // *************************************************************************************************************



        const int routeId0 = localSearchBest.inser0.routeId;
        const int routeId1 = localSearchBest.inser1.routeId;

        const int satId0 = localSearchBest.inser0.satId;
        const int satId1 = localSearchBest.inser1.satId;

        Satelite *sat0 = solution.getSatelite(satId0);
        EvRoute &evRouteSol0 = sat0->getRoute(routeId0);


        Satelite *sat1 = solution.getSatelite(satId0);
        EvRoute &evRouteSol1 = sat0->getRoute(routeId0);


            // Armazena tuplas com (posicao de route; recharging station id)
            std::vector<PosRouteRechS_ID> vectorEstacoesRoute0;
            std::vector<PosRouteRechS_ID> vectorEstacoesRoute1;

            vectorEstacoesRoute0.reserve(instance.getN_RechargingS());
            vectorEstacoesRoute1.reserve(instance.getN_RechargingS());



            for(int i = 0; i < evRouteSol0.routeSize; ++i)
            {
                if(instance.isRechargingStation(evRouteSol0.route[i]))
                    vectorEstacoesRoute0.push_back({i, evRouteSol0.route[i]});
            }


            for(int i = 0; i < evRouteSol1.routeSize; ++i)
            {
                if(instance.isRechargingStation(evRouteSol1.route[i]))
                    vectorEstacoesRoute1.push_back({i, evRouteSol1.route[i]});
            }


            std::sort(vectorEstacoesRoute0.begin(), vectorEstacoesRoute0.end());
            std::sort(vectorEstacoesRoute1.begin(), vectorEstacoesRoute1.end());

            // Armazena estacoes de recarga que sao usadas pos ambas as rotas
            std::vector<PosRoute0PosRoute1RechS_ID> vectorTupleEstacoes;

            for(int i = 0; i < vectorEstacoesRoute0.size(); ++i)
            {
                for(int j = 0; j < vectorEstacoesRoute1.size(); ++j)
                {
                    if(vectorEstacoesRoute0[i].rechargingStationId == vectorEstacoesRoute1[j].rechargingStationId)
                        vectorTupleEstacoes.push_back({vectorEstacoesRoute0[i].pos, vectorEstacoesRoute1[i].pos, vectorEstacoesRoute0[i].rechargingStationId});
                }
            }


        // *************************************************************************************************************
        // *************************************************************************************************************

        // Copia o vetor evRoute0
        vector<int> route0Aux(evRouteSol0.route);

        // Escreve a nova sequencia de route0


        int ultimoCliente = -1;
        int posEvRoute1 = localSearchBest.inser1.pos;
        int posEvRoute0 = localSearchBest.inser0.pos;

        if(instance.isRechargingStation(evRouteSol1.route[posEvRoute1+1]))
        {
            int id = buscaEstacao(vectorTupleEstacoes, evRouteSol1.route[posEvRoute1+1]);
            if(id != -1)
            {
                const auto aux = vectorTupleEstacoes[id];

                if(localSearchBest.inser0.pos >= aux.posRoute0 && localSearchBest.inser1.pos < aux.posRoute1)
                    ultimoCliente = evRouteSol0.route[localSearchBest.inser0.pos];
            }
        }

        if(ultimoCliente == -1)
            evRouteSol0.route[posEvRoute0+1] = evRouteSol1.route[posEvRoute1+1];

        ultimoCliente = -1;

        posEvRoute0 = posEvRoute0 + 2;
        posEvRoute1 = posEvRoute1 + 2;

        while(posEvRoute1 < (evRouteSol1.routeSize-1))
        {
            if(instance.isRechargingStation(evRouteSol1.route[posEvRoute1]))
            {
                int id = buscaEstacao(vectorTupleEstacoes, evRouteSol1.route[posEvRoute1]);
                if(id != -1)
                {
                    const auto aux = vectorTupleEstacoes[id];

                    if(localSearchBest.inser0.pos >= aux.posRoute0)
                        ultimoCliente = evRouteSol1.route[posEvRoute1];

                }

            }

            if(ultimoCliente == -1)
            {
                evRouteSol0.route[posEvRoute0] = evRouteSol1.route[posEvRoute1];
                posEvRoute0 = posEvRoute0 + 1;
            }
            else
                ultimoCliente = -1;

            posEvRoute1 = posEvRoute1 + 1;
        }

        evRouteSol0.route[posEvRoute0] = satId0;
        evRouteSol1.routeSize = posEvRoute0+1;





        ultimoCliente = -1;
        posEvRoute1 = localSearchBest.inser1.pos;
        posEvRoute0 = localSearchBest.inser0.pos;


        if(instance.isRechargingStation(route0Aux[posEvRoute0+1]))
        {
            int id = buscaEstacao(vectorTupleEstacoes, route0Aux[posEvRoute0+1]);

            if(id != -1)
            {
                const auto aux = vectorTupleEstacoes[id];

                if(localSearchBest.inser1.pos >= aux.posRoute1 && localSearchBest.inser0.pos < aux.posRoute0)
                    ultimoCliente = route0Aux[posEvRoute0+1];

            }
        }

        if(ultimoCliente == -1)
        {
            evRouteSol1.route[posEvRoute1 + 1] = route0Aux[posEvRoute0 + 1];
            ultimoCliente = -1;
        }

        posEvRoute0 += 2;
        posEvRoute1 += 2;

        while(posEvRoute0 < (route0Aux.size()-1))
        {

            if(instance.isRechargingStation(route0Aux[posEvRoute0]))
            {
                int id = buscaEstacao(vectorTupleEstacoes, route0Aux[posEvRoute0]);
                if(id != -1)
                {
                    const auto aux = vectorTupleEstacoes[id];

                    if(localSearchBest.inser1.pos >= aux.posRoute1)
                        ultimoCliente = route0Aux[posEvRoute0];

                }

            }

            if(ultimoCliente == -1)
            {
                evRouteSol1.route[posEvRoute1] = route0Aux[posEvRoute0];
                posEvRoute1 = posEvRoute1 + 1;
            }
            else
                ultimoCliente = -1;

            posEvRoute0 = posEvRoute0 + 1;
        }

        evRouteSol1.route[posEvRoute1] = satId1;
        evRouteSol1.routeSize = posEvRoute1+1;

        // Copia realizada

        // Ajuste de bateria e distancia

        ajustaBateriaRestante(evRouteSol0, localSearchBest.inser0.pos, instance);
        ajustaBateriaRestante(evRouteSol1, localSearchBest.inser1.pos, instance);

        solution.mvCross = true;

        return true;


    }
    else
        return false;

}


int NS_LocalSearch::buscaEstacao(const std::vector<PosRoute0PosRoute1RechS_ID> &vector, const int estacao)
{
    PRINT_DEBUG("\t", "INICIO FUNCAO buscaEstacao");

    for(int i=0; i<vector.size(); ++i)
    {
        if(vector[i].rechargingStationId == estacao)
        {
            PRINT_DEBUG("\t", "FIM FUNCAO buscaEstacao");
            return i;
        }
    }

    PRINT_DEBUG("\t", "FIM FUNCAO buscaEstacao");

    return -1;
}

bool NS_LocalSearch::ajustaBateriaRestante(EvRoute &evRoute, const int pos, const Instance &instance)
{

    float bateriaRestante = evRoute.vetRemainingBattery[pos];

    float distancia = 0.0;

    for(int i=0; i < (pos+1); ++i)
        distancia += instance.getDistance(evRoute[i], evRoute[i+1]);

    for(int i=pos; i < (evRoute.routeSize); ++i)
    {
        bateriaRestante -= instance.getDistance(evRoute[i], evRoute[i+1]);
        distancia += instance.getDistance(evRoute[i], evRoute[i+1]);

        if(bateriaRestante < -BATTERY_TOLENCE)
        {
            string str;
            evRoute.print(str);

            PRINT_DEBUG("", "ERRO BATERIA RESTANTE DO EV: ("<<str<<")");
            throw "ERRO";

        }

        if(instance.isRechargingStation(evRoute[i+1]))
        {
            bateriaRestante = instance.getEvBattery();
            evRoute.rechargingStationRoute[i+1] = true;
        }
        else
            evRoute.rechargingStationRoute[i+1] = true;

        evRoute.vetRemainingBattery[i+1] = bateriaRestante;

    }

    evRoute.distance = distancia;
    return true;

}

void NS_LocalSearch::crossAux(const pair<int, int> satIdPair, const pair<int, int> routeIdPair, const EvRoute &evRoute0, const EvRoute &evRoute1, LocalSearch &localSearchBest, const Instance &instance)
{

    PRINT_DEBUG("", "CROSS AUX");

    if((evRoute0.routeSize <= 2) || (evRoute1.routeSize <= 2))
        return;

    // Armazena tuplas com (posicao de route; recharging station id)
    std::vector<PosRouteRechS_ID> vectorEstacoesRoute0;
    std::vector<PosRouteRechS_ID> vectorEstacoesRoute1;

    vectorEstacoesRoute0.reserve(instance.getN_RechargingS());
    vectorEstacoesRoute1.reserve(instance.getN_RechargingS());

    for(int i=0; i < evRoute0.routeSize; ++i)
    {
        if(instance.isRechargingStation(evRoute0.route[i]))
            vectorEstacoesRoute0.push_back({i, evRoute0.route[i]});
    }


    for(int i=0; i < evRoute1.routeSize; ++i)
    {
        if(instance.isRechargingStation(evRoute1.route[i]))
            vectorEstacoesRoute1.push_back({i, evRoute1.route[i]});
    }


    std::sort(vectorEstacoesRoute0.begin(), vectorEstacoesRoute0.end());
    std::sort(vectorEstacoesRoute1.begin(), vectorEstacoesRoute1.end());

    // Armazena estacoes de recarga que sao usadas pos ambas as rotas
    std::vector<PosRoute0PosRoute1RechS_ID> vectorTupleEstacoes;

    for(int i=0; i < vectorEstacoesRoute0.size(); ++i)
    {
        for(int j=0; j < vectorEstacoesRoute1.size(); ++j)
        {
            if(vectorEstacoesRoute0[i].rechargingStationId == vectorEstacoesRoute1[j].rechargingStationId)
                vectorTupleEstacoes.push_back({vectorEstacoesRoute0[i].pos, vectorEstacoesRoute1[i].pos, vectorEstacoesRoute0[i].rechargingStationId});
        }
    }

    float demandaAcumRoute0 = 0.0;
    float demandaAcumRoute1 = 0.0;

    float distanciaAcumRoute0 = 0.0;
    float distanciaAcumRoute1 = 0.0;


    // Percorre todas as possicoes da rota0
    for(int posEvRoute0 = 0; (posEvRoute0+1) < evRoute0.routeSize; ++posEvRoute0)
    {
        if(posEvRoute0 >= 1)
        {
            demandaAcumRoute0 += instance.getDemand(evRoute0.route[posEvRoute0]);
            distanciaAcumRoute0 += instance.getDistance(evRoute0.route[posEvRoute0-1], evRoute0.route[posEvRoute0]);
        }

        distanciaAcumRoute1 = 0.0;
        demandaAcumRoute1 = 0.0;

        // Percorre todas as possicoes da rota1
        for(int posEvRoute1 = 0; (posEvRoute1+1) < evRoute1.routeSize; ++posEvRoute1)
        {
            PRINT_DEBUG("", "posEvRoute0: "<<posEvRoute0<<"; posEvRoute1: "<<posEvRoute1);

            if(posEvRoute1 >= 1)
            {
                demandaAcumRoute1 += instance.getDemand(evRoute1.route[posEvRoute1]);
                distanciaAcumRoute1 += instance.getDistance(evRoute1.route[posEvRoute1-1], evRoute1.route[posEvRoute1]);
            }

            // Calcula as novas demandas
            const float novaDemandaRoute0 = demandaAcumRoute0 + (evRoute1.getDemand()-demandaAcumRoute1);
            const float novaDemandaRoute1 = demandaAcumRoute1 + (evRoute0.getDemand()-demandaAcumRoute0);

            // Verifica se as novas demandas ultrapassam a capacidade do ev
            if(novaDemandaRoute0 < instance.getEvCap() && novaDemandaRoute1 < instance.getEvBattery())
            {

                // Calcula as novas distancias

                float novaDistanciaRoute0 = distanciaAcumRoute0 + instance.getDistance(evRoute0.route[posEvRoute0], evRoute1.route[posEvRoute1+1]);

                // Verifica se evRoute1[posEvRoute1+1] eh uma estacao de recarga que esta sendo usada pelas duas rotas e se esta apos posEvRoute1+1 antes de posEvRoute0

                int ultimoCliente = -1;

                if(instance.isRechargingStation(evRoute1.route[posEvRoute1+1]))
                {
                    for(int i=0; i < vectorTupleEstacoes.size(); ++i)
                    {
                        if(vectorTupleEstacoes[i].rechargingStationId == evRoute1.route[posEvRoute1+1])
                        {
                            if(vectorTupleEstacoes[i].posRoute0 <= posEvRoute0)
                            {
                                ultimoCliente = evRoute0.route[posEvRoute0];
                                novaDistanciaRoute0 = distanciaAcumRoute0;
                            }
                        }
                    }
                }

                // Atualiza a distancia da nova rota0
                for(int i=posEvRoute1+1; (i+1) < (evRoute1.routeSize-1); ++i )
                {
                    if(ultimoCliente != -1)
                    {
                        novaDistanciaRoute0 += instance.getDistance(ultimoCliente, evRoute1.route[i+1]);
                        ultimoCliente = -1;
                    }
                    else
                    {
                        int id = -1;
                        if(instance.isRechargingStation(evRoute1.route[i+1]))
                        {
                           id = buscaEstacao(vectorTupleEstacoes, evRoute1.route[i+1]);

                           if(id >= 0)
                           {
                               PosRoute0PosRoute1RechS_ID &aux = vectorTupleEstacoes[id];
                               if(posEvRoute0 >= aux.posRoute0)
                               {
                                    ultimoCliente = evRoute1.route[i];
                               }
                               else
                                   id = -1;

                           }

                        }

                        if(id == -1)
                            novaDistanciaRoute0 += instance.getDistance(evRoute1.route[i], evRoute1.route[i + 1]);
                    }
                }


                if(ultimoCliente != -1)
                    novaDistanciaRoute0 += instance.getDistance(ultimoCliente, satIdPair.first);
                else
                    novaDistanciaRoute0 += instance.getDistance(evRoute1.route[evRoute1.routeSize-1], satIdPair.first);


                float novaDistanciaRoute1 = distanciaAcumRoute1;// + instance.getDistance(evRoute1.route[posEvRoute1], evRoute0.route[posEvRoute0+1]);
                ultimoCliente = -1;

                if(instance.isRechargingStation(evRoute0.route[posEvRoute0+1]))
                {
                    int id = buscaEstacao(vectorTupleEstacoes, evRoute0.route[posEvRoute0+1]);
                    if(id != -1)
                    {
                        // Verifica se estacao evRoute0[posEvRoute0+1] esta em uma posicao <= posEvRoute1 na rota1
                        const PosRoute0PosRoute1RechS_ID &aux = vectorTupleEstacoes[id];

                        if(aux.posRoute1 <= posEvRoute1)
                            ultimoCliente = evRoute1.route[posEvRoute1];
                    }
                    else
                        novaDistanciaRoute1 += instance.getDistance(evRoute1.route[posEvRoute1], evRoute0.route[posEvRoute0+1]);
                }
                else
                    novaDistanciaRoute1 += instance.getDistance(evRoute1.route[posEvRoute1], evRoute0.route[posEvRoute0+1]);


                // Calcula a nova distancia da rota1
                for(int i=posEvRoute0+1; (i+1) < (evRoute0.routeSize-1); ++i )
                {

                    if(ultimoCliente != -1)
                    {
                        novaDistanciaRoute1 += instance.getDistance(ultimoCliente, evRoute0.route[i+1]);
                        ultimoCliente = -1;
                    }
                    else
                    {
                        int id = -1;

                        // Verifica se evRoute0[i+1] eh estacao
                        if(instance.isRechargingStation(evRoute0.route[i+1]))
                        {
                            // Verifica se a estacao eh usada nas duas rotas
                            id = buscaEstacao(vectorTupleEstacoes, evRoute0.route[i+1]);

                            if(id != -1)
                            {
                                // Verificar se essa estacao eh usada antes de <= posRoute1
                                const auto &aux = vectorTupleEstacoes[id];
                                if(aux.posRoute1 <= posEvRoute1)
                                    ultimoCliente = evRoute0.route[i];

                            }
                        }

                        if(id == -1)
                            novaDistanciaRoute1 += instance.getDistance(evRoute0.route[i], evRoute0.route[i+1]);
                    }


                }

// ****************************************************************************************************************
// Roudou ate aqui

                if(ultimoCliente != -1)
                {
                    novaDistanciaRoute1 += instance.getDistance(ultimoCliente, satIdPair.second);
                }
                else
                    novaDistanciaRoute1 += instance.getDistance(evRoute0.route[evRoute0.routeSize-1], satIdPair.second);

                const float incremento = (novaDistanciaRoute0 + novaDistanciaRoute1) - (evRoute0.distance + evRoute1.distance);

                if(incremento < 0.0 && incremento < localSearchBest.incrementoDistancia)
                {
                    // Verifica as baterias dos Ev's

                    // No veeiculo0 ate posEvRoute0(inclusive) a bateria EH viavel, verificar se a bateria eh viavel apos posEvRoute1 no novo veiculo0

                    float evRoute0NovoBateriaRestante =  evRoute0.vetRemainingBattery[posEvRoute0]; // - instance.getDistance(evRoute0.route[posEvRoute0], evRoute1.route[posEvRoute1+1]);
                    ultimoCliente = -1;

                    if(instance.isRechargingStation(evRoute1.route[posEvRoute1+1]))
                    {
                        int id = buscaEstacao(vectorTupleEstacoes, evRoute1.route[posEvRoute1+1]);
                        if(id != -1)
                        {
                            auto aux = vectorTupleEstacoes[id];
                            if(aux.posRoute0 <= posEvRoute0)
                                ultimoCliente = evRoute0.route[posEvRoute0];
                        }
                    }

                    if(ultimoCliente == -1)
                        evRoute0NovoBateriaRestante -= instance.getDistance(evRoute0.route[posEvRoute0], evRoute1.route[posEvRoute1+1]);

// ******************************************************************************************************************************************************
                    // Inicio ERRO
                    PRINT_DEBUG("", "PRIMEIRA PARTE CALCULO BATERIA");

                    if(evRoute0NovoBateriaRestante >= -BATTERY_TOLENCE)
                    {
                        if(instance.isRechargingStation(evRoute1.route[posEvRoute1+1]) && (ultimoCliente==-1))
                            evRoute0NovoBateriaRestante = instance.getEvBattery();

                        for(int i=posEvRoute1+1; (i+1) < (evRoute1.routeSize); ++i)
                        {
                            PRINT_DEBUG("", "i= "<<i);

                            if(ultimoCliente != -1)
                            {
                                evRoute0NovoBateriaRestante -= instance.getDistance(ultimoCliente, evRoute1.route[i+1]);
                                ultimoCliente = -1;


                            }
                            else
                            {

                                int id = -1;
                                if(instance.isRechargingStation(evRoute1.route[i+1]))
                                {
                                    id = buscaEstacao(vectorTupleEstacoes, evRoute1.route[i+1]);
                                    if(id!=-1)
                                    {
                                        const auto aux = vectorTupleEstacoes[id];
                                        if(aux.posRoute0 <= posEvRoute0)
                                        {
                                            ultimoCliente = evRoute1.route[i];
                                        }
                                    }
                                }

                                PRINT_DEBUG("\t", " "<<evRoute1.route[i]<<"  "<<evRoute1.route[i+1]);

                                if(id == -1)
                                    evRoute0NovoBateriaRestante -= instance.getDistance(evRoute1.route[i], evRoute1.route[i + 1]);
                                PRINT_DEBUG("", "");
                            }

                            if(evRoute0NovoBateriaRestante < -BATTERY_TOLENCE)
                                break;


                            PRINT_DEBUG("", "");

                            if(instance.isRechargingStation(evRoute1.route[i+1]) && ultimoCliente == -1)
                            {
                                evRoute0NovoBateriaRestante = instance.getEvBattery();
                            }

                        }

// ******************************************************************************************************************************************************
                        PRINT_DEBUG("", "CALCULO DA BATERIA DA NOVA ROTA0");

                        if(evRoute0NovoBateriaRestante < -BATTERY_TOLENCE)
                            continue;

                        // Adiciona satellite0
                        if(ultimoCliente == -1)
                            evRoute0NovoBateriaRestante -= instance.getDistance(evRoute1.route[evRoute1.size()-1], satIdPair.first);
                        else
                        {
                            evRoute0NovoBateriaRestante -= instance.getDistance(ultimoCliente, satIdPair.first);
                            ultimoCliente = -1;
                        }

                        if(evRoute0NovoBateriaRestante < -BATTERY_TOLENCE)
                            continue;



                    }
                    else
                        continue;


                    // No veeiculo1 ate posEvRoute1(inclusive) a bateria EH viavel, verificar se a bateria eh viavel apos posEvRoute0 no novo veiculo1

                    float evRoute1NovoBateriaRestante =  evRoute1.vetRemainingBattery[posEvRoute1];
                    ultimoCliente = -1;

                    if(instance.isRechargingStation(evRoute0.route[posEvRoute0+1]))
                    {
                        const int id = buscaEstacao(vectorTupleEstacoes, evRoute0.route[posEvRoute0+1]);

                        if(id != -1)
                        {
                            auto aux = vectorTupleEstacoes[id];
                            if(aux.posRoute1 <= posEvRoute1)
                            {
                                ultimoCliente = evRoute1.route[posEvRoute1];
                            }
                        }
                    }

                    if(ultimoCliente == -1)
                        evRoute1NovoBateriaRestante -= instance.getDistance(evRoute1.route[posEvRoute1], evRoute0.route[posEvRoute0+1]);

                    if(evRoute1NovoBateriaRestante > -BATTERY_TOLENCE)
                    {
                        if(instance.isRechargingStation(evRoute0.route[posEvRoute0+1]) && ultimoCliente==-1)
                            evRoute1NovoBateriaRestante = instance.getEvBattery();

                        for(int i=posEvRoute0+1; (i+1) < (evRoute0.routeSize); ++i)
                        {
                            if(ultimoCliente != -1)
                            {
                                evRoute1NovoBateriaRestante -= instance.getDistance(ultimoCliente, evRoute0.route[i+1]);
                                ultimoCliente = -1;
                            }
                            else
                            {
                                int id = -1;
                                if(instance.isRechargingStation(evRoute0.route[i+1]))
                                {
                                    id = buscaEstacao(vectorTupleEstacoes, evRoute0.route[i+1]);
                                    if(id!= -1)
                                    {
                                        const auto aux = vectorTupleEstacoes[id];

                                        if(posEvRoute1 <= aux.posRoute1)
                                            ultimoCliente = evRoute0.route[i];
                                    }
                                }

                                if(ultimoCliente != -1)
                                    evRoute1NovoBateriaRestante -= instance.getDistance(evRoute0.route[i], evRoute0.route[i+1]);


                            }



                            if(evRoute1NovoBateriaRestante < -BATTERY_TOLENCE)
                                break;

                            if(instance.isRechargingStation(evRoute0.route[i+1]) && (ultimoCliente != -1))
                                evRoute1NovoBateriaRestante = instance.getEvBattery();

                        }

                        if(evRoute1NovoBateriaRestante < -BATTERY_TOLENCE)
                            continue;

                        // Adiciona satellite1

                        if(ultimoCliente == -1)
                            evRoute1NovoBateriaRestante -= instance.getDistance(evRoute0.route[evRoute1.size()-1], satIdPair.second);
                        else
                        {
                            evRoute1NovoBateriaRestante -= instance.getDistance(ultimoCliente, satIdPair.second);
                            ultimoCliente = -1;
                        }

                        if(evRoute1NovoBateriaRestante < -BATTERY_TOLENCE)
                            continue;


                    }
                    else
                        continue;


                    // Bateria dos dois veiculos sao viaveis
                    // Atualiza  localSearchBest

                    localSearchBest.incrementoDistancia = incremento;
                    localSearchBest.mov = MOV_CROSS;

                    localSearchBest.inser0.satId = satIdPair.first;
                    localSearchBest.inser1.satId = satIdPair.second;

                    localSearchBest.inser0.routeId = routeIdPair.first;
                    localSearchBest.inser1.routeId = routeIdPair.second;


                    localSearchBest.inser0.pos = posEvRoute0;
                    localSearchBest.inser1.pos = posEvRoute1;



                }

            }
        }
    }

}