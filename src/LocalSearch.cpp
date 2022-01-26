//
// Created by igor on 19/11/2021.
//

#include "LocalSearch.h"
#include "Auxiliary.h"
#include "mersenne-twister.h"
#include <memory>
#include "greedyAlgorithm.h"

using namespace NS_LocalSearch;
using namespace NS_Auxiliary;
using namespace GreedyAlgNS;


/*
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
                        // e vice versa
                            // Para cada cliente c0 na rota evRoute0
                            for (int c0 = 1; c0 < evRoute0.size() - 1; c0++) {
                                if (!Inst.isRechargingStation(evRoute0[c0])) {
                                    // se nao for estacao de recarga e tambem nao tiver uma demanda que ultrapassa a demanda maxima que a troca suporta,
                                    for (int c1 = 1; c1 < evRoute1.size() - 1; c1++) {
                                        // se nao for estacao de recarga..
                                        if (!Inst.isRechargingStation(evRoute1[c1])) {
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

*/


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
        //NS_Auxiliary::shiftVectorDir(evRoute.rechargingStationRoute, insertion.pos+1, 1, evRoute.routeSize);

        evRoute.route[insertion.pos+1] = insertion.clientId;
        //evRoute.rechargingStationRoute[insertion.pos+1] = instance.isRechargingStation(insertion.clientId);

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
                    //evRoute.rechargingStationRoute[j] = evRoute.rechargingStationRoute[j+1];
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

                evRoute.print(instance);

                std::cout << "*******************************************\n\n";
                localSearchBest.print();

                throw "ERRO";
            }


            if(instance.isRechargingStation(evRoute.route[i]))
            {
                evRoute.vetRemainingBattery[i] = instance.getEvBattery();

            }

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

                    shifitInterRotasMvDuasRotas({satId0, satId1}, {routeId0, routeId1}, evRoute0, evRoute1, localSearchBest, instance);
                    shifitInterRotasMvDuasRotas({satId1, satId0}, {routeId1, routeId0}, evRoute1, evRoute0, localSearchBest, instance);

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
        shiftVectorDir(evRoute0.vetRemainingBattery, pos+1, 1,evRoute0.size());
        //shiftVectorDir(evRoute0.rechargingStationRoute, pos, 1,evRoute0.size());

        evRoute0.routeSize += 1;

        evRoute0.route[pos+1] = clienteId;
        //evRoute0.rechargingStationRoute[pos+1] = false;

        float remainingBattery = evRoute0.vetRemainingBattery[pos];

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

            }

        }

        solution.mvShiftInterRotas = true;
        return true;
    }
    else
        return false;

}

void NS_LocalSearch::shifitInterRotasMvDuasRotas(const pair<int, int> satIdPair, const pair<int, int> routeIdPair, const EvRoute &evRoute0, const EvRoute &evRoute1,
                                                 LocalSearch &localSearchBest, const Instance &instance)
{
    if(evRoute1.routeSize <=2)
        return;

    const int satId0 = satIdPair.first;
    const int satId1 = satIdPair.second;

    const int routeId0 = routeIdPair.first;
    const int routeId1 = routeIdPair.second;

    const float distanciaRotas = evRoute0.distance + evRoute1.distance;

    // Percorre as possicoes de rota0
    for(int posRoute0 = 0; posRoute0 < (evRoute0.routeSize-1); ++posRoute0)
    {

        // Escolhe um cliente da rota1 para ir para a rota0
        for(int posClieRoute1 = 1; posClieRoute1 < (evRoute1.routeSize-1); ++posClieRoute1)
        {
            const int clienteId = evRoute1.route[posClieRoute1];
            float distanciaRoute0 = evRoute0.distance;
            float distanciaRoute1 = evRoute1.distance;

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

                    // distSol??
                    const float incremento = novaDistanciaTotal - distanciaRotas;

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


    //cout<<"\nMOVIMENTO CROSS\n\n";

    string str;
    //solution.print(str, instance);
    //cout<<str<<"\n*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#\n*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#\n\n";


    int satId0 = rand_u32() % solution.getNSatelites();
    Satelite *sat0 = solution.getSatelite(satId0);

    int routeId0 = rand_u32() % sat0->getNRoutes();
    EvRoute *evRouteSol0 = &sat0->getRoute(routeId0);

    if(evRouteSol0->routeSize <= 2)
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
                evRouteSol0 = &sat0->getRoute(routeId0);

                if(evRouteSol0->routeSize > 2)
                    break;

                routeId0 = (routeId0 + 1) % sat0->getNRoutes();
            }

            if(evRouteSol0->routeSize > 2)
                break;

            satId0 = (satId0 + 1) % solution.getNSatelites();

        }while(satId0 != sat0Inic);

        if(evRouteSol0->routeSize <= 2)
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

            EvRoute *evRouteSol1 = &sat1->getRoute(routeId1);

            //string str;
            //solution.print(str);
            //cout<<str<<"\n*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#\n*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#\n\n";

            crossAux({satId0, satId1}, {routeId0, routeId1}, evRouteSol0, evRouteSol1, localSearchBest, instance);


        }

    }


    if(localSearchBest.incrementoDistancia < 0.0)
    {

        // Atualiza solucao


        const int routeId0 = localSearchBest.inser0.routeId;
        const int routeId1 = localSearchBest.inser1.routeId;

        const int satId0 = localSearchBest.inser0.satId;
        const int satId1 = localSearchBest.inser1.satId;

        Satelite *sat0 = solution.getSatelite(satId0);
        EvRoute *evRouteSol0 = &sat0->getRoute(routeId0);


        Satelite *sat1 = solution.getSatelite(satId1);
        EvRoute *evRouteSol1 = &sat1->getRoute(routeId1);



        std::vector<PosicaoEstacao> vectorEstacoesRoute0;
        achaEstacoes(evRouteSol0, vectorEstacoesRoute0, instance);

        std::vector<PosicaoEstacao> vectorEstacoesRoute1;
        achaEstacoes(evRouteSol1, vectorEstacoesRoute1, instance);

        std::vector<PosRota0Rota1Estacao> vectorEstacoesEmComun;
        achaEstacoesEmComun(vectorEstacoesRoute0, vectorEstacoesRoute1, vectorEstacoesEmComun);


/*        PRINT_DEBUG("", "SOLUCAO ANTES DO MOVIMENTO:");
        string temp;
        solution.print(temp);
        cout<<temp<<"\n\n";

        cout<<"Rota0: ";
        evRouteSol0->print();

        cout<<"\n\n";
        cout<<"Rota1: ";
        evRouteSol1->print();*/



        // Copia o vetor evRoute0
        vector<int> route0Aux(evRouteSol0->route);
        const int tamEvRoute0 = evRouteSol0->routeSize;

        float distAcumRota0 = calculaDistanciaAcumulada(evRouteSol0->route, localSearchBest.inser0.pos, instance);
        float distRota0 = calculaNovaDistanciaRoute0Cross(evRouteSol0, evRouteSol1->route, evRouteSol1->routeSize, vectorEstacoesEmComun, localSearchBest.inser0.pos,
                                                     localSearchBest.inser1.pos, distAcumRota0, instance, true, false);

        float distAcumRota1 = calculaDistanciaAcumulada(evRouteSol1->route, localSearchBest.inser1.pos, instance);
        float distRota1 = calculaNovaDistanciaRoute0Cross(evRouteSol1, route0Aux, tamEvRoute0, vectorEstacoesEmComun, localSearchBest.inser1.pos,
                                                          localSearchBest.inser0.pos, distAcumRota1, instance, true, true);

        if((distRota0 < 0.0) || (distRota1 < 0.0))
        {
            string erro0;
            if(distRota0 < 0.0)
                evRouteSol0->print(erro0, instance);
            string erro1;
            if(distRota1 < 0.0)
                evRouteSol1->print(erro1, instance);

            PRINT_DEBUG("", "ERRO MOV CROSS \n\nROTA ERRADA: ")

            if(distRota0 < 0.0)
                cout<<erro0<<"\n\n";

            if(distRota1 < 0.0)
                cout<<erro1<<"\n\n";

            cout<<"ROTA SO FOI COPIADA ATE O ERRO DE BATERIA\n\n";

            throw "ERRO";


        }

/*        cout<<"************************************************************************\n************************************************************************\n\n";

        PRINT_DEBUG("", "ROTAS APOS O MOVIMENTO:");
        evRouteSol0->print();
        cout<<"\n\n";
        evRouteSol1->print();

        cout<<"************************************************************************\n************************************************************************\n";
        cout<<"************************************************************************\n************************************************************************\n\n";

        string str;
        solution.print(str);
        cout<<str<<"\n";


        cout<<"************************************************************************\n************************************************************************\n";
        cout<<"************************************************************************\n************************************************************************\n\n\n\n";*/


        solution.mvCross = true;
        return true;


    }
    else
        return false;

}

float NS_LocalSearch::calculaDistanciaAcumulada(const vector<int> &rota, const int pos, const Instance &instance)
{
    float dist = 0.0;

    for(int i=0; i < pos; ++i)
    {
        dist += instance.getDistance(rota[i], rota[i+1]);
    }

    return dist;
}


int NS_LocalSearch::buscaEstacao(const std::vector<PosRota0Rota1Estacao> &vector, const int estacao)
{

    for(int i=0; i<vector.size(); ++i)
    {
        if(vector[i].rechargingStationId == estacao)
            return i;
    }

    return -1;
}

bool NS_LocalSearch::ajustaBateriaRestante(EvRoute *evRoute, const int pos, const Instance &instance)
{

    float bateriaRestante = evRoute->vetRemainingBattery[pos];

    float distancia = 0.0;

    for(int i=0; i < (pos+1); ++i)
        distancia += instance.getDistance((*evRoute)[i], (*evRoute)[i+1]);

    for(int i=pos; i < (evRoute->routeSize); ++i)
    {
        bateriaRestante -= instance.getDistance((*evRoute)[i], (*evRoute)[i+1]);
        distancia += instance.getDistance((*evRoute)[i], (*evRoute)[i+1]);

        if(bateriaRestante < -BATTERY_TOLENCE)
        {
            string str;
            evRoute->print(str, instance);

            PRINT_DEBUG("", "ERRO BATERIA RESTANTE DO EV: ("<<str<<")");
            throw "ERRO";

        }

        if(instance.isRechargingStation((*evRoute)[i+1]))
        {
            bateriaRestante = instance.getEvBattery();
            //evRoute->rechargingStationRoute[i+1] = true;
        }
        //else
        //    evRoute->rechargingStationRoute[i+1] = true;

        evRoute->vetRemainingBattery[i+1] = bateriaRestante;

    }

    evRoute->distance = distancia;
    return true;

}

void NS_LocalSearch::crossAux(const pair<int, int> satIdPair, const pair<int, int> routeIdPair, EvRoute *evRoute0, EvRoute *evRoute1, LocalSearch &localSearchBest, const Instance &instance)
{

    //PRINT_DEBUG("", "CROSS AUX");

    if((evRoute0->routeSize <= 2) || (evRoute1->routeSize <= 2))
        return;

    if(evRoute0->routeSize == 3 && evRoute1->routeSize == 3)
        return;

    // Armazena Estacoes de cada rota com (posicao de route; recharging station id)
    std::vector<PosicaoEstacao> vectorEstacoesRoute0;
    achaEstacoes(evRoute0, vectorEstacoesRoute0, instance);

    std::vector<PosicaoEstacao> vectorEstacoesRoute1;
    achaEstacoes(evRoute1, vectorEstacoesRoute1, instance);


    // Armazena estacoes de recarga que sao usadas pos ambas as rotas
    std::vector<PosRota0Rota1Estacao> vectorEstacoesEmComum;
    achaEstacoesEmComun(vectorEstacoesRoute0, vectorEstacoesRoute1, vectorEstacoesEmComum);



    float demandaAcumRoute0 = 0.0;
    float demandaAcumRoute1 = 0.0;

    float distanciaAcumRoute0 = 0.0;
    float distanciaAcumRoute1 = 0.0;


    // Percorre todas as possicoes da rota0
    for(int posEvRoute0 = 0; (posEvRoute0+1) < evRoute0->routeSize; ++posEvRoute0)
    {
        if(posEvRoute0 >= 1)
        {
            demandaAcumRoute0 += instance.getDemand(evRoute0->route[posEvRoute0]);
            distanciaAcumRoute0 += instance.getDistance(evRoute0->route[posEvRoute0-1], evRoute0->route[posEvRoute0]);
        }

        distanciaAcumRoute1 = 0.0;
        demandaAcumRoute1 = 0.0;

        // Percorre todas as possicoes da rota1
        for(int posEvRoute1 = 0; (posEvRoute1+1) < evRoute1->routeSize; ++posEvRoute1)
        {

            if(posEvRoute1 >= 1)
            {
                demandaAcumRoute1 += instance.getDemand(evRoute1->route[posEvRoute1]);
                distanciaAcumRoute1 += instance.getDistance(evRoute1->route[posEvRoute1-1], evRoute1->route[posEvRoute1]);
            }

            // Calcula as novas demandas
            const float novaDemandaRoute0 = demandaAcumRoute0 + (evRoute1->getDemand()-demandaAcumRoute1);
            const float novaDemandaRoute1 = demandaAcumRoute1 + (evRoute0->getDemand()-demandaAcumRoute0);

            // Verifica se as novas demandas ultrapassam a capacidade do ev
            if(novaDemandaRoute0 < instance.getEvCap() && novaDemandaRoute1 < instance.getEvBattery())
            {

                // Calcula as novas distancias

                float novaDistanciaRoute0 = calculaNovaDistanciaRoute0Cross(evRoute0, evRoute1->route, evRoute1->routeSize, vectorEstacoesEmComum, posEvRoute0,
                                                                            posEvRoute1, distanciaAcumRoute0, instance, false, false);
                if(novaDistanciaRoute0 < 0.0)
                    continue;

                float novaDistanciaRoute1 = calculaNovaDistanciaRoute0Cross(evRoute1, evRoute0->route, evRoute0->routeSize, vectorEstacoesEmComum, posEvRoute1,
                                                                            posEvRoute0, distanciaAcumRoute1, instance, false, true);
                if(novaDistanciaRoute1 < 0.0)
                    continue;



                const float incremento = (novaDistanciaRoute0 + novaDistanciaRoute1) - (evRoute0->distance + evRoute1->distance);

                if(incremento < 0.0 && incremento < localSearchBest.incrementoDistancia)
                {

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

void NS_LocalSearch::achaEstacoes(const EvRoute  *const evRoute, std::vector<PosicaoEstacao> &vectorEstacoes, const Instance &instance)
{

    vectorEstacoes.reserve(instance.getN_RechargingS());

    for(int i=0; i < evRoute->routeSize; ++i)
    {
        if(instance.isRechargingStation(evRoute->route[i]))
            vectorEstacoes.push_back({i, evRoute->route[i]});
    }

    if(!vectorEstacoes.empty())
        std::sort(vectorEstacoes.begin(), vectorEstacoes.end());

}

void NS_LocalSearch::achaEstacoesEmComun(const std::vector<PosicaoEstacao> &vectorRota0Estacoes, const std::vector<PosicaoEstacao> &vectorRota1Estacoes, std::vector<PosRota0Rota1Estacao> &vectorEsracoesEmComun)
{
    if(vectorRota0Estacoes.empty() || vectorRota1Estacoes.empty())
        return;

   vectorEsracoesEmComun.reserve(vectorRota0Estacoes.size()+vectorRota1Estacoes.size()) ;

    for(int i=0; i < vectorRota0Estacoes.size(); ++i)
    {
        int pos = buscaEstacao(vectorRota1Estacoes, vectorRota0Estacoes[i].rechargingStationId);

        if(pos != -1)
            vectorEsracoesEmComun.push_back({vectorRota0Estacoes[i].pos, vectorRota1Estacoes[pos].pos, vectorRota0Estacoes[i].rechargingStationId});

    }

    if(!vectorEsracoesEmComun.empty())
        std::sort(vectorEsracoesEmComun.begin(), vectorEsracoesEmComun.end());

}

int NS_LocalSearch::buscaEstacao(const std::vector<PosicaoEstacao> &vector, const int estacao)
{
    for(int i=0; i < vector.size(); ++i)
    {
        if(vector[i].rechargingStationId == estacao)
            return i;
    }

    return -1;
}

/**
 * Calcula a distancia de evRoute0 apos o movimento cross na pos0 de evRoute0 e pos1 de evRoute1.
 * A funcao verifica o combustivel.
 * Se esvreveRoute0 eh TRUE,  a nova rota0 e o combustivel sao escritos em evRoute0
 *
 * @param evRoute0                              Rota 0 ; Pode ser modificado caso escreveRoute0 for TRUE
 * @param evRoute1                              Sequencia da Rota 1
 * @param vectorEstacoesEmComun                 Vetor que contem as estacoes de recarga que sao comuns as duas rotas
 * @param pos0                                  Possicao em rota0 para o movimento cross
 * @param pos1                                  Possicao em rota1 para o movimento cross
 * @param distanciaAcumRota0                    Distancia acumulada em rota0 ate <pos>(inclusive)
 * @param instance                              Instancia
 * @param escreveRoute0                         Indica se a nova rota0 e se o seu combustivel deve ser escrito na rota0
 * @param inverteRotaEmVectorEstacoesEmComun    Se TRUE inverte posRota0 com posRota1 no vectorEstacoesEmComun
 * @return                                      Retorna distancia da nova rota0 ou -1.0 se for inviavel
 */
//Calcula a distancia de evRoute0 apos o movimento cross na pos0 de evRoute0 e pos1 de evRoute1. A funcao tambem verifica o combustivel
float NS_LocalSearch::calculaNovaDistanciaRoute0Cross(EvRoute *evRoute0, const std::vector<int> &evRoute1, const int tamEvRoute1, std::vector<PosRota0Rota1Estacao> &vectorEstacoesEmComun, const int pos0, const int pos1,
                                                      const float distanciaAcumRota0, const Instance &instance, const bool escreveRoute0, const bool inverteRotaEmVectorEstacoesEmComun)
{

/* Exemplo:
 *
 *            V
 * rota0: 0 1 2 3 4 0
 *
 *            V
 * rota1: 0 5 6 1 8 0
 *
 * pos0: 2
 * pos1: 2
 *
 * Estacao de recarga: 1
 *
 * Novas rotas:
 *
 *      nova rota0: 0 1 2 8 0
 *      nova rota1: 0 5 6 3 4 0
 *
 *      Estacao de recarga 1 foi excluida na nova rota0
 *
 * Condicao:
 *
 *      Sendo <PosRota0Rota1Estacao estacao> armazena as posicoes da estacao 1 nas rotas
 *
 *      Se (pos0 >= estacao.pos0) && (pos1 < estacao.pos1)
 *      Verifica-se a estacao 1 esta antes de pos0 e se a estacao 1 esta depois de pos1
 *
 */


    float distanciaRota0 = distanciaAcumRota0;
    float bateriaRestante = evRoute0->vetRemainingBattery[pos0];

    int ultimoCliente = (*evRoute0)[pos0];
    int auxPos0 = pos0+1;
    int auxPos1 = pos1+1;

    for(; auxPos1 < (tamEvRoute1-1); )
    {
        const bool cliePos1RecS = instance.isRechargingStation(evRoute1[auxPos1]);

        if(cliePos1RecS)
        {
            // Verifica se a estacao evRoute1[auxPos1] tambem eh usada na rota0
            const int p = buscaEstacao(vectorEstacoesEmComun, evRoute1[auxPos1]);
            if(p != -1)
            {
                const auto estacao = vectorEstacoesEmComun[p];

                // Verifica se a nova rota0 irar conter duas estacoes evRoute1[auxPos1]
                if(((pos0 >= estacao.posRoute0) && (pos1 < estacao.posRoute1)&&!inverteRotaEmVectorEstacoesEmComun) ||
                   ((pos0 >= estacao.posRoute1) && (pos1 < estacao.posRoute0)&&inverteRotaEmVectorEstacoesEmComun))
                {
                    ++auxPos1;
                    continue;
                }

            }
        }
        const float distAux =  instance.getDistance(ultimoCliente, evRoute1[auxPos1]);

        distanciaRota0 += distAux;
        bateriaRestante -= distAux;

        if(bateriaRestante < -BATTERY_TOLENCE)
        {
            if(escreveRoute0)
            {
                evRoute0->vetRemainingBattery[auxPos0] = bateriaRestante;
                (*evRoute0)[auxPos0] = evRoute1[auxPos1];

            }
            return -1.0;
        }

        if(cliePos1RecS)
            bateriaRestante = instance.getEvBattery();

        if(escreveRoute0)
        {
            (*evRoute0)[auxPos0] = evRoute1[auxPos1];
            evRoute0->vetRemainingBattery[auxPos0] = bateriaRestante;
            //evRoute0->rechargingStationRoute[auxPos0] = cliePos1RecS;
        }

        ultimoCliente = evRoute1[auxPos1];

        auxPos1 += 1;
        auxPos0 += 1;

    }


    distanciaRota0 += instance.getDistance(ultimoCliente, (*evRoute0)[0]);
    bateriaRestante -=  instance.getDistance(ultimoCliente, (*evRoute0)[0]);

    if(bateriaRestante < -BATTERY_TOLENCE)
    {

        if(escreveRoute0)
            evRoute0->vetRemainingBattery[auxPos0] = bateriaRestante;

        return -1.0;

    }

    if(escreveRoute0)
    {
        evRoute0->vetRemainingBattery[auxPos0] = bateriaRestante;
        //evRoute0->rechargingStationRoute[auxPos0] = false;
        (*evRoute0)[auxPos0] = (*evRoute0)[0];
        evRoute0->routeSize = auxPos0+1;
        evRoute0->distance = distanciaRota0;
    }

    return distanciaRota0;
}