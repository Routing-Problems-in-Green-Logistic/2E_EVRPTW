//
// Created by igor on 19/11/2021.
//

#include "LocalSearch.h"
#include <memory>
using namespace NS_LocalSearch;

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

        localSearchBest.print();

        string str;
        solution.print(str);
        cout<<str<<"\n\n";

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





                }
            }
        }
    }
}

void NS_LocalSearch::shifitInterRotasMvDuasRotas(const int satId0, const int satId1, const EvRoute &evRoute0, const EvRoute &evRoute1, LocalSearch &localSearchBest, const Instance &instance, const float distSol)
{
    if(evRoute1.routeSize <=2)
        return;

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

                            }


                        }

                    }


                }

            }
        }
    }

}