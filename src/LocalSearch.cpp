/*
 * ./run ../instancias/2e-vrp-tw/Customer_5/C103_C5x.txt 1657747507

 */

#include "LocalSearch.h"
#include "Auxiliary.h"
#include "mersenne-twister.h"
#include <memory>
#include "greedyAlgorithm.h"
#include "ViabilizadorRotaEv.h"

using namespace NS_LocalSearch;
using namespace NS_Auxiliary;
using namespace GreedyAlgNS;
using namespace NameViabRotaEv;

#define PRINT_MV_SHIFIT_INTRA FALSE

void NS_LocalSearch::getMov(const int movId, string &mov)
{
    switch(movId)
    {
        case MOV_SHIFIT_INTRA:
            mov = "MOV_SHIFIT_INTRA";
            break;

        case MOV_SWAP_INTRA:
            mov = "MOV_SWAP_INTRA";
            break;

        case MOV_2_OPT:
            mov = "MOV_2_OPT";
            break;

        case MOV_CROSS:
            mov = "MOV_CROSS";
            break;

        default:
            mov = "MOV";
            break;
    }
}



void NS_LocalSearch::LocalSearch::print() const
{
    string movStr;
    getMov(mov, movStr);

    cout<<"LOCAL SEARCH:\n";
    cout<<"\nMOV: "<<movStr;
    cout<<"\nidSat0: "<<noLocal0.idSat;
    cout<<"\nidEvRota: "<<noLocal0.idEvRota;

    cout<<"\n\ninser0:";
    cout<<"\n\tpos: "<<noLocal0.pos;
    cout<<"\n\tclientId: "<<noLocal0.i;

    cout<<"\nINCR. DIST.: "<<incrementoDistancia;

}





/**
 * A funcao realiza ...
 *
 * @param solucao
 * @param instancia
 * @param evRouteAux
 * @param selecao
 * @return
 */

bool NS_LocalSearch::mvEvShifitIntraRota(Solucao &solucao, Instance &instancia, EvRoute &evRouteAux, const int selecao)
{

    #if PRINT_MV_SHIFIT_INTRA
        cout<<"mvEvShifitIntraRota\n";
    #endif
    std::list<LocalSearch> listLocalSearch;
    InsercaoEstacao insercaoEstacao;

    for(int satId = 1; satId <= instancia.getNSats(); ++satId)
    {


        #if PRINT_MV_SHIFIT_INTRA
            cout<<"satId: "<<satId<<"\n";
        #endif

        Satelite *satelite = &solucao.satelites[satId];

        // Percorre todas as rotas do satellite
        for(int routeId = 0; routeId < satelite->getNRoutes(); ++routeId)
        {

            EvRoute &evRoute = satelite->vetEvRoute[routeId];
            string str;

            #if PRINT_MV_SHIFIT_INTRA
                cout<<"routeId: "<<routeId<<"\n\n";
                evRoute.print(str, instance, true);
                cout<<"ROTA: "<<str<<"\n";
            #endif


            if(evRoute.routeSize <= 2)
                continue;

            evRouteAux.copiaParametros(evRoute);

            // Percorre todos os clientes da rota
            // Cliente na possicao i fara o shift
            for(int i = 1; i < (evRoute.routeSize-1); ++i)
            {

                #if PRINT_MV_SHIFIT_INTRA
                    cout<<"i: "<<i<<"\n";
                #endif

                const int clienteShift = evRoute.route[i].cliente;

                if(!instancia.isRechargingStation(clienteShift))
                {

                    // Calcula o incremento de retirar os arcos (i-1, i), (i, i+1) e adicionar (i-1, i+1)
                    const double incDistTemp =
                            -instancia.getDistance(evRoute.route[i - 1].cliente, evRoute.route[i].cliente) +
                            -instancia.getDistance(evRoute.route[i].cliente, evRoute.route[i + 1].cliente) +
                            instancia.getDistance(evRoute.route[i - 1].cliente, evRoute.route[i + 1].cliente);

                    // pos + 1 é a nova possicao para o cliente
                    for(int pos = 0; pos < (evRoute.routeSize-1); ++pos)
                    {

                        insercaoEstacao = InsercaoEstacao();


                        #if PRINT_MV_SHIFIT_INTRA
                            cout<<"pos: "<<pos<<"\n";
                        #endif

                        //if(i != pos)
                        {

                            if((i==(pos+1)) || (pos==(evRoute.routeSize-2)))
                                continue;

                            // Shift a partir de pos+1

                            // Caso especial, shifit eh igual a swap
/*                            if(abs(i-pos) == 1)
                            {

                                continue;
                            }*/

                            // RETIRAR

                            #if PRINT_MV_SHIFIT_INTRA
                                //INICIO
                                int r = setRotaMvEvShifitIntraRota(evRoute, evRouteAux, i, pos, instance);

                                if(r != -1)
                                {
                                    str = "";
                                    evRouteAux.print(str, instance, true);
                                    cout<<"\ti: "<<i<<"; pos: "<<pos<<" Nova rota: "<<str<<"\n";
                                }

                                // FIM
                            #endif

                            // Verifica se a nova rota eh menor que a rota original

                            double incDist = -instancia.getDistance(evRoute[pos].cliente, evRoute[pos + 1].cliente) +
                                             + instancia.getDistance(evRoute[pos].cliente, clienteShift) +
                                             + instancia.getDistance(clienteShift, evRoute[pos + 1].cliente);

                            if((i-pos)==2)
                                incDist += instancia.getDistance(clienteShift, evRoute[pos + 1].cliente);

                            else if(i == pos)
                                incDist += instancia.getDistance(evRoute[pos].cliente, evRoute[pos + 2].cliente) +
                                           + instancia.getDistance(evRoute[pos + 1].cliente, evRoute[pos].cliente);

                            incDist += incDistTemp;

                            //if((i+1) == pos)
                              //  incDist += instance.getDistance(evRoute[i].cliente, evRoute[pos].cliente);

                            // Verifica se a nova rota atualiza a rota original
                            if(incDist <= -INCREM_DIST)
                            {

                                #if PRINT_MV_SHIFIT_INTRA
                                    cout<<"\t\tIncremento distancia: "<<incDist<<"\n";
                                #endif

                                if(selecao == SELECAO_BEST && !listLocalSearch.empty())
                                {
                                    const LocalSearch &localSearch = listLocalSearch.front();

                                    if(incDist >= localSearch.incrementoDistancia)
                                        continue;
                                }

                                // Escreve a nova rota
                                const int min = setRotaMvEvShifitIntraRota(evRoute, evRouteAux, i, pos, instancia);

                                if(min < 0)
                                {
                                    PRINT_DEBUG("", "RESULTADO DA FUNC. setRotaMvEvShifitIntraRota EH -1. ");
                                    cout<<"i: "<<i<<"; pos: "<<pos<<"\n\nevRoute: \n\n";
                                    evRoute.print(instancia, true);
                                    throw "ERRO";
                                }

                                // Verifica viabilidade da nova rota
                                double distNovaRota = testaRota(evRouteAux, evRouteAux.routeSize, instancia, false, solucao.satTempoChegMax[satId], min - 1, nullptr);
                                bool exit = false;

                                if(distNovaRota < 0.0)
                                {
                                    bool viavel = viabilizaRotaEv(evRouteAux, instancia, instancia.bestInsViabRotaEv, insercaoEstacao, (evRoute.distancia - INCREM_DIST),
                                                                  false, solucao.satTempoChegMax[satId]);

                                    if(!viavel)
                                    {

                                        #if PRINT_MV_SHIFIT_INTRA
                                            cout<<"Inviavel\n";
                                        #endif

                                        continue;
                                    }

                                    distNovaRota = insercaoEstacao.distanciaRota;
                                }
                                else
                                {
                                    double d0 = abs(distNovaRota - (evRoute.distancia+incDist));
                                    double d1 = abs((evRoute.distancia+incDist) - distNovaRota);
                                    double dif = (d0 < d1) ? d0 : d1;

                                    if(dif > TOLERANCIA_DIF_ROTAS)
                                    {

                                        #if PRINT_MV_SHIFIT_INTRA
                                            string strRota;
                                            string strRotaNova;

                                            evRoute.print(strRota, instance, true);
                                            evRouteAux.print(strRotaNova, instance, true);

                                            PRINT_DEBUG("", "ERRO CALCULO DA NOVA ROTA. CALC: "<<(evRoute.distancia+incDist)<<"; REAL: "<<distNovaRota<<"; DIF: "<<dif<<"\nROTA ORIGINAL: "<<strRota<<"\nNOVA ROTA: "<<strRotaNova<<"\ni: "<<i<<"; pos: "<<pos);

                                            //throw "ERRO";
                                        #endif

                                    }
                                }

                                if(distNovaRota < evRoute.distancia)
                                {
                                    double dif = evRoute.distancia - distNovaRota;
                                    bool contB = true;

                                    if(dif < INCREM_DIST)
                                    {

                                        #if PRINT_MV_SHIFIT_INTRA
                                            string strRota;
                                            string strRotaNova;

                                            evRoute.print(strRota, instance, true);
                                            evRouteAux.print(strRotaNova, instance, true);

                                            PRINT_DEBUG("", "ERRO NOVA ROTA EH MAIOR. ROTA ORIGINAL "<<(evRoute.distancia)<<"; NOVA ROTA: "<<distNovaRota<<"\nROTA ORIGINAL: "<<strRota<<"\nNOVA ROTA: "<<strRotaNova<<"\ni: "<<i<<"; pos: "<<pos);
                                            //throw "ERRO";
                                        #endif


                                        contB = false;

                                    }

                                    #if PRINT_MV_SHIFIT_INTRA
                                        cout<<"\t\tATUALIZACAO, INC: "<<dif<<"\n";
                                    #endif

                                    if(SELECAO_PRIMEIRO && contB)
                                    {

                                        solucao.distancia -= evRoute.distancia;
                                        solucao.distancia += distNovaRota;

                                        satelite->distancia -= evRoute.distancia;
                                        satelite->distancia += distNovaRota;

                                        if(insercaoEstacao.pos != -1)
                                        {
                                            insereEstacaoRota(evRouteAux, insercaoEstacao, instancia, solucao.satTempoChegMax[satId]);
                                            evRoute.copia(evRouteAux);
                                            evRoute.vetRecarga[insercaoEstacao.estacao - instancia.getFirstRechargingSIndex()].utilizado += 1;

                                            return true;
                                        }
                                        else
                                        {
                                            evRoute.copia(evRouteAux);
                                        }

                                        evRoute.distancia = testaRota(evRoute, evRoute.routeSize, instancia, true, solucao.satTempoChegMax[satId], 0, nullptr);
                                        if(evRoute.distancia < 0)
                                        {
                                            PRINT_DEBUG("", "ERRO FUNC testaRota");
                                            throw "ERRO";
                                        }
                                        return true;
                                    }

                                }

                                #if PRINT_MV_SHIFIT_INTRA
                                    else
                                    {
                                        cout<<"Viavel, > rota orig\n";
                                    }
                                #endif

/*                                else
                                {

                                    string strRota;
                                    string strRotaNova;

                                    evRoute.print(strRota, instance, true);
                                    evRouteAux.print(strRotaNova, instance, true);

                                    PRINT_DEBUG("", "ERRO NOVA ROTA EH MAIOR. ROTA ORIGINAL "<<(evRoute.distancia)<<"; NOVA ROTA: "<<distNovaRota<<"\nROTA ORIGINAL: "<<strRota<<"\nNOVA ROTA: "<<strRotaNova<<"\ni: "<<i<<"; pos: "<<pos);
                                    throw "ERRO";
                                }*/


                            }
                        }
                    }
                }
            }
        }
    }

    return false;

}

/**
 *
 * @param evRoute     rota original
 * @param evRouteAux  rota nova
 * @param i           possicao do cliente que sofrera o shifit
 * @param pos         pos+1 eh a nova possicao do cliente
 * @return            i < pos+1 ? i:pos+1, ou -1 em caso de erro
 */
int NS_LocalSearch::setRotaMvEvShifitIntraRota(EvRoute &evRoute, EvRoute &evRouteAux, int i, int pos, Instance &instancia)
{

    bool igual = false;
    if(i == pos)
    {
        #if PRINT_MV_SHIFIT_INTRA
            cout<<"i=pos= "<<pos<<"\n";
        #endif

        int novoI   = i+1;
        int novaPos = pos-1;

        i   = novoI;
        pos = novaPos;
    }

    #if PRINT_MV_SHIFIT_INTRA
        cout<<"setRotaMvEvShifitIntraRota\n";
    #endif

    const int cliente = evRoute[i].cliente;

    int j_evRoute     = 0;
    int j_evRouteAux  = 0;
    int min = (i < (pos+1)) ? i:(pos+1);

    while((j_evRoute < evRoute.routeSize) && (j_evRouteAux < evRoute.routeSize))
    {

        if(j_evRoute == i)
        {
            j_evRoute += 1;
        }
        else if(j_evRoute == pos+1)
        {
            evRouteAux[j_evRouteAux].cliente = cliente;
            j_evRouteAux += 1;
        }

        if(j_evRoute >= evRoute.routeSize || j_evRouteAux >= evRoute.routeSize)
            return -1;

        if(j_evRoute >= min)
            evRouteAux[j_evRouteAux].cliente = evRoute[j_evRoute].cliente;
        else
            evRouteAux[j_evRouteAux] = evRoute[j_evRoute];

        j_evRoute += 1;
        j_evRouteAux += 1;

    }

    evRouteAux.routeSize = evRoute.routeSize;
    int routeSize = evRoute.routeSize;

    for(int t=0; t < (routeSize-1); ++t)
    {
        if(evRouteAux[t].cliente == evRouteAux[t+1].cliente)
        {
            for(int p=t; p < routeSize; ++p)
                evRouteAux[p].cliente = evRouteAux[p+1].cliente;

            routeSize -= 1;
            if((t+1) == min)
                min = t;

            igual = true;
        }
    }

    evRouteAux.routeSize = routeSize;

/*    if(igual)
    {
        string str;
        evRouteAux.print(str, instancia, true);

        cout<<"NOVA ROTA: "<<str<<"\n";
        exit(-1);
    }*/

    return min;

}

void NS_LocalSearch::insereEstacaoRota(EvRoute &evRoute, NameViabRotaEv::InsercaoEstacao &insercaoEstacao, Instance &instance, double tempoSaida)
{
    if(insercaoEstacao.pos == -1)
        return;

    shiftVectorClienteDir(evRoute.route, insercaoEstacao.pos+1, 1, evRoute.routeSize);
    evRoute[insercaoEstacao.pos+1].cliente = insercaoEstacao.estacao;
    evRoute.routeSize += 1;
    evRoute.distancia = testaRota(evRoute, evRoute.routeSize, instance, true, tempoSaida, 0, nullptr);

    if(evRoute.distancia < 0.0)
    {
        string str;
        evRoute.print(str, instance, true);

        PRINT_DEBUG("\t", "ERRO, ROTA DEVERIA SER VIAVEL: "<<str);
        throw "ERRO";
    }

}



bool NS_LocalSearch::mvEvSwapIntraRota(Solucao &solucao, Instance &instancia, EvRoute &evRouteAux)
{

    // Percorre os satelites
    for(int sat=instancia.getFirstSatIndex(); sat <= instancia.getEndSatIndex(); ++sat)
    {
        Satelite &satelite = solucao.satelites[sat];
        evRouteAux.satelite = sat;
        evRouteAux[0].cliente = sat;

        // Percorre os EVs
        for(int ev=0; ev < instancia.getN_Evs(); ++ev)
        {
            EvRoute &evRoute = satelite.vetEvRoute[ev];

            // Verifica se ev eh diferente de vazio e se existe mais de um cliente na rota
            if(evRoute.routeSize > 3)
            {
                bool copiaEvRouteAux = false;

                /* ****************************************************************************************************
                 * ****************************************************************************************************
                 * i, j percorrem evRoute realizando a troca(swap) de i,j
                 *
                 *  Verificar(se i < j) a partir de uma posicao anterior a i se existem duas
                 * estacoes de recarga consecutivas.
                 *
                 *  Calculo da distancia:
                 *
                 *      Rota: 0 k i l ... m j n 0
                 *      Dist(nova rota): dist(rota) -(k,i) -(i, l) -(m, j) -(j, n) +(k, j) +(j, l) +(m, i) +(i, n)
                 *
                 *      Rota: 0 k i j n 0
                 *      Dist(nova rota): dist(rota) -(k,i) -(j,n) +(k, j) +(k, j) +(i, n)
                 * ****************************************************************************************************
                 * ****************************************************************************************************
                 */

                // i tem que ser menor ou igual que a penultima possicao de j
                for(int i=1; i <= (evRoute.routeSize-3); ++i)
                {

                    for(int j=(i+1); j <= (evRoute.routeSize-2); ++j)
                    {
                        // Calcula a nova distancia
                        double novaDist = evRoute.distancia;

                        // Verifica se a rota eh diferente de ex: 0 k i j n 0
                        if((i+1) != j)
                        {
                            novaDist += -instancia.getDistance(evRoute[i-1].cliente, evRoute[i].cliente) +
                                        -instancia.getDistance(evRoute[i].cliente, evRoute[i+1].cliente) +
                                        -instancia.getDistance(evRoute[j-1].cliente, evRoute[j].cliente) +
                                        -instancia.getDistance(evRoute[j].cliente, evRoute[j+1].cliente) +
                                        +instancia.getDistance(evRoute[i-1].cliente, evRoute[j].cliente) +
                                        +instancia.getDistance(evRoute[j].cliente, evRoute[i+1].cliente) +
                                        +instancia.getDistance(evRoute[j-1].cliente, evRoute[i].cliente) +
                                        +instancia.getDistance(evRoute[i].cliente, evRoute[j+1].cliente);
                        }
                        else
                        {
                            //novaDist += -instancia.getDistance(evRoute[i-1].cliente, evRoute[i].cliente)+
                            //          -instancia.getDistance()
                        }
                    }
                }
            }
        }
    }
}


/*
bool NS_LocalSearch::mvShifitIntraRota(Solucao &solution, const Instance &instance)
{

    LocalSearch localSearchBest;


    // Shifit intra rota
    for(int satId = 0; satId < instance.getNSats(); ++satId)
    {
        //PRINT_DEBUG("", "ANTES DE PEGAR SATELLITE");


        Satelite *satelite = &solution.satelites[satId];

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
                                            if(remainingBatt < -TOLERANCIA_BATERIA)
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
                                    if(remainingBatt >= -TOLERANCIA_BATERIA && viavel)
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
        CandidatoEV &insertion = localSearchBest.inser0;
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
            if(evRoute.vetRemainingBattery[i] < -TOLERANCIA_BATERIA)
            {
                std::cerr <<"ERRO\nARQUIVO: "<<__FILE__<<"\nLINHA: "<<__LINE__<<"\nvetRemaingBattery["<<i<<"] = "<<evRoute.vetRemainingBattery[i] <<
                            " < -"<<TOLERANCIA_BATERIA<<" ID: "<<evRoute.route[i]<<"\n";

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



        // PRINT_DEBUG("", "INCREMENTO DISTANCIA: "<<localSearchBest.incrementoDistancia);
        // PRINT_DEBUG("", "DISTANCIA: "<<evRoute.distance);
        // PRINT_DEBUG("", "POS: "<<localSearchBest.inser0.pos);
        // PRINT_DEBUG("", "CLIENTE: "<<localSearchBest.inser0.clientId);

        //PRINT_DEBUG("", "MOVIMENTO SHIFIT INTRA ROTA");

        solution.mvShiftIntraRota = true;

        return true;

    }
    else
        return false;

}
*/

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

bool NS_LocalSearch::interSatelliteSwap(Solucao &Sol, const Instance &Inst, float &improvement) {
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
                                                    if(bestLs.incrementoDistancia < TOLERANCIA_BATERIA) {
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
void NS_LocalSearch::swapMov(Solucao& Sol, const LocalSearch2& mov, const Instance& Inst){
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


/*
bool NS_LocalSearch::mvShiftInterRotasIntraSatellite(Solucao &solution, const Instance &instance)
{

    LocalSearch localSearchBest;

    // Percorre os satellites
    for(int satId0 = 0; satId0 < instance.getNSats(); ++satId0)
    {

        Satelite *satelite0 = solution.satelites[satId0];

        // Percorre todas as rotas do satellite0
        for(int routeId0 = 0; routeId0 < satelite0->getNRoutes(); ++routeId0)
        {

            EvRoute *evRoute0 = &satelite0->vetEvRoute[routeId0];

            //for(int satId1 = satId0; satId1 < instance.getNSats(); ++satId1)
            {
                //Satelite *satelite1 = solution.satelites[satId1];

                for(int routeId1 = routeId0+1; routeId1 < satelite0->getNRoutes(); ++routeId1)
                {

                    EvRoute *evRoute1 = &satelite0->vetEvRoute[routeId1];

                    shifitInterRotasMvDuasRotas({satId0, satId0}, {routeId0, routeId1}, *evRoute0, *evRoute1, localSearchBest, instance);
                    shifitInterRotasMvDuasRotas({satId0, satId0}, {routeId1, routeId0}, *evRoute1, *evRoute0, localSearchBest, instance);

                }
            }
        }
    }


    // Verifica se movimento eh de melhora
    if(localSearchBest.incrementoDistancia < -INCREM_DIST)
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

        // Atualiza rota0

        evRoute0.distance += -instance.getDistance(evRoute0.route[pos], evRoute0.route[pos+1]) + instance.getDistance(evRoute0.route[pos], clienteId) +
                instance.getDistance(clienteId, evRoute0.route[pos+1]);

        string strRoute0;
        evRoute0.print(strRoute0, instance);

        shiftVectorDir(evRoute0.route, pos+1, 1, evRoute0.size());
        //shiftVectorDir(evRoute0.vetRemainingBattery, pos+1, 1,evRoute0.size());
        //shiftVectorDir(evRoute0.rechargingStationRoute, pos, 1,evRoute0.size());

        evRoute0.routeSize += 1;

        evRoute0.route[pos+1] = clienteId;
        //evRoute0.rechargingStationRoute[pos+1] = false;


        if(localSearchBest.insercaoEstacaoRota0.estacao >= 0)
        {
            int posEst = localSearchBest.insercaoEstacaoRota0.pos;
            shiftVectorDir(evRoute0.route, posEst+1, 1, evRoute0.routeSize);
            evRoute0.route[posEst+1] = localSearchBest.insercaoEstacaoRota0.estacao;
            evRoute0.routeSize += 1;
        }

        evRoute0.distance = testaRota(evRoute0.route, evRoute0.routeSize, instance, &evRoute0.vetRemainingBattery);

        if(evRoute0.distance <= 0.0)
        {
            PRINT_DEBUG("", "ROTA: "<<printVector(evRoute0.route, evRoute0.routeSize)<<"; EH INVIAVEL");
            throw "ERRO";
        }

// / *        for(int i=pos; (i+1) < evRoute0.size(); ++i)
        {
            remainingBattery -= instance.getDistance(evRoute0.route[i], evRoute0.route[i+1]);

            if(remainingBattery < -TOLERANCIA_BATERIA)
            {
                string e;
                evRoute0.print(e, instance);
                PRINT_DEBUG("", "ERRO BATERIA!!; remainingBattery: "<<remainingBattery<<"; Rota: "<<e);
                cout<<"ROTA ANTES: "<<strRoute0<<"\n\n";
                throw "erro";
            }

            evRoute0.vetRemainingBattery[i+1] = remainingBattery;

            if(instance.isRechargingStation(evRoute0.route[i+1]))
            {
                evRoute0.vetRemainingBattery[i + 1] = instance.getEvBattery();

            }

        }// * /

        // Atualiza rota1

        evRoute1.distance += -instance.getDistance(evRoute1[posClienteRoute1-1], evRoute1[posClienteRoute1]) - instance.getDistance(evRoute1[posClienteRoute1], evRoute1[posClienteRoute1+1]) +
                instance.getDistance(evRoute1[posClienteRoute1-1], evRoute1[posClienteRoute1+1]);


        for(int i=posClienteRoute1; (i+1) < evRoute1.routeSize; ++i)
        {

            evRoute1[i] = evRoute1[i + 1];
        }

        evRoute1.routeSize -= 1;

        if(evRoute1.routeSize > 2)
            evRoute1.distance = testaRota(evRoute1.route, evRoute1.routeSize, instance, &evRoute1.vetRemainingBattery);
        else
            evRoute1.distance = 0.0;


        if(evRoute1.distance < 0.0)
        {
            PRINT_DEBUG("", "ROTA: "<<printVector(evRoute1.route, evRoute1.routeSize)<<"; EH INVIAVEL");
            cout<<"ClienteId: "<<clienteId<<"\n";
            throw "ERRO";
        }

        solution.mvShiftInterRotas = true;
        float demanda =  instance.getDemand(clienteId);
        evRoute0.totalDemand += demanda;
        evRoute1.totalDemand -= demanda;

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

    static EvRoute evRouteAux(instance);

    evRouteAux[0] = evRouteAux[1] = evRoute0[0];
    evRouteAux.routeSize = 2;

    // Percorre as possicoes de rota0
    for(int posRoute0 = 0; posRoute0 < (evRoute0.routeSize-1); ++posRoute0)
    {

        evRouteAux[posRoute0] = evRoute0[posRoute0];

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

                    evRouteAux[posRoute0+1] = clienteId;
                    evRouteAux[posRoute0+2] = evRoute0[posRoute0+1];
                    int posEvRouteAux = posRoute0+3;

                    const float novaDistanciaTotal = distanciaRoute0+distanciaRoute1;

                    // distSol??
                    const float incremento = novaDistanciaTotal - distanciaRotas;

                    // Verifica se o movimento melhora a solucao


                    //if(incremento < -1E-4 && incremento < localSearchBest.incrementoDistancia)
                    {
                        // Verifica a viabilidade da rota

                        float cargaBateria = evRoute0.vetRemainingBattery[posRoute0];

                        cargaBateria += -instance.getDistance(evRoute0.route[posRoute0], clienteId) - instance.getDistance(clienteId, evRoute0.route[posRoute0+1]);
                        int p = posRoute0+1;
                        bool viavel = true;

                        while((p+1) < evRoute0.routeSize)
                        {

// / *                            if(cargaBateria >= -TOLERANCIA_BATERIA)
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
                            // * /

                            evRouteAux[posEvRouteAux] = evRoute0[p+1];

                            p += 1;
                            posEvRouteAux += 1;
                        }

                        viavel = (testaRota(evRouteAux.route, posEvRouteAux, instance, nullptr) != -1);
                        InsercaoEstacao insercaoEstacao;

                        evRouteAux.routeSize = posEvRouteAux;

                        if(viavel && incremento < -INCREM_DIST && incremento < localSearchBest.incrementoDistancia)
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

                            localSearchBest.insercaoEstacaoRota0 = insercaoEstacao;
                        }
                        else if(!viavel && incremento < -INCREM_DIST && incremento < localSearchBest.incrementoDistancia)
                        {

                            if(viabilizaRotaEv(evRouteAux.route, evRouteAux.routeSize, instance, true, insercaoEstacao))
                            {
                                float incremento = (insercaoEstacao.distanciaRota + distanciaRoute1) - distanciaRotas;
                                if(incremento < -INCREM_DIST && incremento < localSearchBest.incrementoDistancia)
                                {

                                    localSearchBest.inser0.pos = posRoute0;
                                    localSearchBest.inser0.clientId = clienteId;
                                    localSearchBest.inser0.satId = satId0;
                                    localSearchBest.inser0.routeId = routeId0;

                                    localSearchBest.incrementoDistancia = incremento;

                                    localSearchBest.inser1.pos = posClieRoute1;
                                    localSearchBest.inser1.clientId = clienteId;
                                    localSearchBest.inser1.satId = satId1;
                                    localSearchBest.inser1.routeId = routeId1;

                                    localSearchBest.insercaoEstacaoRota0 = insercaoEstacao;
                                }
                            }

                        }
                    }

                }
            }
        }
    }

}

/ **
 *
 * Descricao ...
 *
 * @param solution      Solucao ; Pode ser alterado
 * @param instance      Instancia do problema ; Constante
 * @param evRoute0      Rota0 do veiculo eletrico, utilizada no movimento ; Pode ser alterado
 * @param evRoute1      Rota1 do veiculo eletrico, utilizada no movimento ; Pode ser alterado
 * @return              Retorna se conseguiu realizar o movimento
 // * /

bool NS_LocalSearch::mvCrossIntraSatellite(Solucao &solution, const Instance &instance)
{


    //cout<<"\nMOVIMENTO CROSS\n\n";

    string str;
    //solution.print(str, instance);
    //cout<<str<<"\n*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#\n*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#\n\n";





    LocalSearch localSearchBest;

    for(int satId=0; satId < solution.getNSatelites(); ++satId)
    {
        Satelite *sat = solution.getSatelite(satId);

        for(int routeId0 = 0; routeId0  < sat->getNRoutes(); ++routeId0)
        {
            EvRoute *evRouteSol0 = &sat->getRoute(routeId0);

            for(int routeId1 = routeId0+1; routeId1 < sat->getNRoutes(); ++routeId1)
            {
                if(routeId1 == routeId0)
                    continue;

                EvRoute *evRouteSol1 = &sat->getRoute(routeId1);
                crossAux({satId, satId}, {routeId0, routeId1}, evRouteSol0, evRouteSol1, localSearchBest, instance);

            }
        }
    }

    if(localSearchBest.incrementoDistancia < 0.0)
    {

        //PRINT_DEBUG("\t\t", "Atualiza solucao");


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


        // / *
        if(localSearchBest.insercaoEstacaoRota0.estacao != -1 || localSearchBest.insercaoEstacaoRota1.estacao != -1)
        {
            PRINT_DEBUG("", "SOLUCAO ANTES DO MOVIMENTO:");
            cout << "Rota0: ";
            evRouteSol0->print(instance);

            cout << "\n\n";
            cout << "Rota1: ";
            evRouteSol1->print(instance);

        }
        // * /



        // Copia o vetor evRoute0
        vector<int> route0Aux(evRouteSol0->route);
        const int tamEvRoute0 = evRouteSol0->routeSize;

        float distAcumRota0 = calculaDistanciaAcumulada(evRouteSol0->route, localSearchBest.inser0.pos, instance);
        float distRota0 = calculaNovaDistanciaRoute0Cross(evRouteSol0, evRouteSol1->route, evRouteSol1->routeSize, vectorEstacoesEmComun, localSearchBest.inser0.pos,
                                                     localSearchBest.inser1.pos, distAcumRota0, instance, true, false, localSearchBest.insercaoEstacaoRota0);

        float distAcumRota1 = calculaDistanciaAcumulada(evRouteSol1->route, localSearchBest.inser1.pos, instance);
        float distRota1 = calculaNovaDistanciaRoute0Cross(evRouteSol1, route0Aux, tamEvRoute0, vectorEstacoesEmComun, localSearchBest.inser1.pos,
                                                          localSearchBest.inser0.pos, distAcumRota1, instance, true, true, localSearchBest.insercaoEstacaoRota1);

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

// / *
        if(localSearchBest.insercaoEstacaoRota0.estacao != -1 || localSearchBest.insercaoEstacaoRota1.estacao != -1)
        {

            cout<<"estacaoRota0: "<<localSearchBest.insercaoEstacaoRota0.estacao<<"\n";
            cout<<"estacaoRota1: "<<localSearchBest.insercaoEstacaoRota1.estacao<<"\n";

            cout
                    << "\n\n******************************************************************************************************************************************************\n";
            PRINT_DEBUG("", "SOLUCAO APOS O MOVIMENTO:");

            cout << "Rota0: ";
            evRouteSol0->print(instance);

            cout << "\n\n";
            cout << "Rota1: ";
            evRouteSol1->print(instance);

        }
        // * /




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

        if(bateriaRestante < -TOLERANCIA_BATERIA)
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

                NameViabRotaEv::InsercaoEstacao insercaoEstacaoRota0;
                float novaDistanciaRoute0 = calculaNovaDistanciaRoute0Cross(evRoute0, evRoute1->route, evRoute1->routeSize, vectorEstacoesEmComum, posEvRoute0,
                                                                            posEvRoute1, distanciaAcumRoute0, instance, false, false, insercaoEstacaoRota0);
                if(novaDistanciaRoute0 < 0.0)
                    continue;

                NameViabRotaEv::InsercaoEstacao insercaoEstacaoRota1;
                float novaDistanciaRoute1 = calculaNovaDistanciaRoute0Cross(evRoute1, evRoute0->route, evRoute0->routeSize, vectorEstacoesEmComum, posEvRoute1,
                                                                            posEvRoute0, distanciaAcumRoute1, instance, false, true, insercaoEstacaoRota1);
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

                    localSearchBest.insercaoEstacaoRota0 = insercaoEstacaoRota0;
                    localSearchBest.insercaoEstacaoRota1 = insercaoEstacaoRota1;

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

// / **
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
 // * /

//Calcula a distancia de evRoute0 apos o movimento cross na pos0 de evRoute0 e pos1 de evRoute1. A funcao tambem verifica o combustivel
float NS_LocalSearch::calculaNovaDistanciaRoute0Cross(EvRoute *evRoute0, const std::vector<int> &evRoute1, const int tamEvRoute1, std::vector<PosRota0Rota1Estacao> &vectorEstacoesEmComun, const int pos0, const int pos1,
                                                      const float distanciaAcumRota0, const Instance &instance, const bool escreveRoute0, const bool inverteRotaEmVectorEstacoesEmComun, NameViabRotaEv::InsercaoEstacao &insercaoEstacao)
{

// / * Exemplo:
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
 // * /

    static EvRoute evRouteAux(instance);

    evRouteAux[0] = evRouteAux[1] = evRoute0->route[0];
    evRouteAux.routeSize = 2;



    if(!escreveRoute0)
    {
        for(int i=0; i <= pos0; ++i)
            evRouteAux[i] = evRoute0->route[i];
    }

    float distanciaRota0 = distanciaAcumRota0;
    float bateriaRestante = evRoute0->vetRemainingBattery[pos0];

    int ultimoCliente = (*evRoute0)[pos0];
    int auxPos0 = pos0+1;
    int auxPos1 = pos1+1;

    bool inviavel = false;

    if(escreveRoute0 && insercaoEstacao.estacao != -1)
        inviavel = true;

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

        if(!inviavel)
        {
            distanciaRota0 += distAux;
            bateriaRestante -= distAux;

            if(bateriaRestante < -TOLERANCIA_BATERIA)
            {
                inviavel = true;
            }

            if(cliePos1RecS && !inviavel)
                bateriaRestante = instance.getEvBattery();

        }

        if(escreveRoute0)
        {
            (*evRoute0)[auxPos0] = evRoute1[auxPos1];
            evRoute0->vetRemainingBattery[auxPos0] = bateriaRestante;
            //evRoute0->rechargingStationRoute[auxPos0] = cliePos1RecS;
        }
        else
        {
            evRouteAux[auxPos0] = evRoute1[auxPos1];
        }

        ultimoCliente = evRoute1[auxPos1];

        auxPos1 += 1;
        auxPos0 += 1;

    }

    if(!inviavel)
    {

        distanciaRota0 += instance.getDistance(ultimoCliente, (*evRoute0)[0]);
        bateriaRestante -= instance.getDistance(ultimoCliente, (*evRoute0)[0]);

        if(bateriaRestante < -TOLERANCIA_BATERIA)
            inviavel = true;
    }

    if(inviavel)
    {



        if(escreveRoute0 && insercaoEstacao.estacao == -1)
        {   (*evRoute0)[auxPos0] = (*evRoute0)[0];
            inviavel = !viabilizaRotaEv(evRoute0->route, auxPos0+1, instance, true, insercaoEstacao);
        }
        else if(escreveRoute0 && insercaoEstacao.estacao != -1)
        {
            shiftVectorDir(evRoute0->route, insercaoEstacao.pos+1, 1, auxPos0);
            evRoute0->routeSize = auxPos0+2;
            evRoute0->route[insercaoEstacao.pos+1] = insercaoEstacao.estacao;
            (*evRoute0)[auxPos0+1] = (*evRoute0)[0];
            evRoute0->distance = testaRota(evRoute0->route, evRoute0->routeSize, instance, &evRoute0->vetRemainingBattery);
            evRoute0->totalDemand = calculaCargaEv(*evRoute0, instance);

            if(evRoute0->distance == -1.0)
            {
                string aux;
                evRoute0->print(aux, instance);
                PRINT_DEBUG("", "ROTA: "<<aux<<"; DEVERIA SER VIAVEL.");
                throw "ERRO";
            }

            return evRoute0->distance;
        }
        else
        {   evRouteAux[auxPos0] = evRouteAux[0];
            inviavel = !viabilizaRotaEv(evRouteAux.route, auxPos0+1, instance, true, insercaoEstacao);
        }

        if(!inviavel)
        {
            distanciaRota0 = insercaoEstacao.distanciaRota;
            //PRINT_DEBUG("", "VIABILIZOU ROTA DO MV CROSS");
        }

    }


    if(escreveRoute0 && !inviavel)
    {
        evRoute0->vetRemainingBattery[auxPos0] = bateriaRestante;
        //evRoute0->rechargingStationRoute[auxPos0] = false;
        (*evRoute0)[auxPos0] = (*evRoute0)[0];
        evRoute0->routeSize = auxPos0+1;
        evRoute0->distance = distanciaRota0;
        evRoute0->totalDemand = NS_LocalSearch::calculaCargaEv(*evRoute0, instance);
    }


    if(inviavel)
        return -1.0;


    return distanciaRota0;
}

float NS_LocalSearch::calculaCargaEv(const EvRoute &rota, const Instance &instance)
{
    float carga = 0.0;
    for(int i=0; i < rota.routeSize-1; ++i)
        carga += instance.getDemand(rota[i]);

    return carga;
}

 */