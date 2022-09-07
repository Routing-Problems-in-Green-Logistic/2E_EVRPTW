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
#define PRINT_MV_2OPT TRUE

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
                                            evRoute.copia(evRouteAux, false, nullptr);
                                            evRoute.vetRecarga[insercaoEstacao.estacao - instancia.getFirstRS_index()].utilizado += 1;

                                            return true;
                                        }
                                        else
                                        {
                                            evRoute.copia(evRouteAux, false, nullptr);
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


#define PRINT_MV_SWAP FALSE

bool NS_LocalSearch::mvEvSwapIntraRota(Solucao &solucao, Instance &instancia, EvRoute &evRouteAux)
{

//PRINT_DEBUG("", "RETURN FALSE");
//    return false;

    // Copia evRoute para evRouteAux sem repeticao de RS, realizando a troca de i com j. Retorna a distancia e o ultimo indice em que a rota ainda eh valida
    auto copiaEvRoute = [&](const EvRoute &evRoute, int &ultimoValIndice, int posI, int posJ)
    {

        ultimoValIndice = -1;
        int i           = 1;
        double dist     = 0.0;
        int indiceEvAux = 1;
        evRouteAux[0]   = evRoute.route[0];
        bool copiaAll   = true;

        if(posJ < posI)
        {
            int temp = posI;
            posI = posJ;
            posJ = temp;
        }

        for(; i < evRoute.routeSize; ++i)
        {

            if(evRoute.route[i].cliente != evRoute.route[i-1].cliente)
            {
                if(copiaAll)
                {
                    if(i == posI)
                    {
                        evRouteAux[indiceEvAux].cliente = evRoute.route[posJ].cliente;
                        copiaAll = false;
                        ultimoValIndice = indiceEvAux - 1;
                    } else
                        evRouteAux[indiceEvAux] = evRoute.route[i];
                } else
                {
                    if(i == posI)
                        evRouteAux[indiceEvAux].cliente = evRoute.route[posJ].cliente;

                    else if(i == posJ)
                        evRouteAux[indiceEvAux].cliente = evRoute.route[posI].cliente;

                    else
                        evRouteAux[indiceEvAux].cliente = evRoute.route[i].cliente;
                }

                if(evRouteAux[indiceEvAux-1].cliente != evRouteAux[indiceEvAux].cliente)
                {
                    dist += instancia.getDistance(evRouteAux[indiceEvAux - 1].cliente, evRouteAux[indiceEvAux].cliente);
                    indiceEvAux += 1;
                }
            }
            else
            {
                if(copiaAll)
                {
                    copiaAll = false;
                    ultimoValIndice = i - 1;
                }
            }
        }

        evRouteAux.routeSize = indiceEvAux;
        return dist;
    };

    InsercaoEstacao insereEstacao;

    // Percorre os satelites
    for(int sat=instancia.getFirstSatIndex(); sat <= instancia.getEndSatIndex(); ++sat)
    {
        Satelite &satelite = solucao.satelites[sat];
        evRouteAux.satelite = sat;
        evRouteAux[0].cliente = sat;

        // Percorre os EVs
        for(int ev=0; ev < satelite.getNRoutes(); ++ev)
        {

#if PRINT_MV_SWAP == TRUE
cout<<"EV: "<<ev<<"\n";
#endif
            EvRoute &evRoute = satelite.vetEvRoute[ev];

            // Verifica se ev eh diferente de vazio e se existe mais de um cliente na rota
            if(evRoute.routeSize > 3)
            {

#if PRINT_MV_SWAP == TRUE
string strRota;
evRoute.print(strRota, instancia, true);
cout<<"SWAP ROTA: "<<strRota<<"\n";
#endif
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
                    const int clienteI = evRoute[i].cliente;

                    for(int j=(i+1); j <= (evRoute.routeSize-2); ++j)
                    {
                        const int clienteJ = evRoute[j].cliente;


                        // Calcula a nova distancia
                        double novaDist = evRoute.distancia;

                        // Verifica se a rota eh diferente de ex: 0 k i j n 0
                        if(((i+1) != j))
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
                            // Se rota eh do tipo: 0 k i j n 0
                            novaDist += -instancia.getDistance(evRoute[i-1].cliente, clienteI)+
                                        -instancia.getDistance(clienteJ, evRoute[j+1].cliente)+
                                        +instancia.getDistance(evRoute[i-1].cliente, clienteJ)+
                                        +instancia.getDistance(clienteI, evRoute[j+1].cliente);
                        }

                        // Verifica se existe melhora
                        if(novaDist < evRoute.distancia)
                        {


#if PRINT_MV_SWAP == TRUE
cout<<"\tClienteI: "<<clienteI<<"\n";
cout<<"\n\t\tClienteJ: "<<clienteJ<<"\n";
cout<<"\t\t\tNova dist: "<<novaDist<<"\n";
#endif

                            int ultimoValIndice = 0;

                            // Copia rota
                            novaDist = copiaEvRoute(evRoute, ultimoValIndice, i, j);

#if PRINT_MV_SWAP == TRUE
string strRota1;
evRouteAux.print(strRota1, instancia, true);
cout<<"\t\t\tNova rota: "<<strRota1<<"\n\n";
#endif

                            if(novaDist < evRoute.distancia)
                            {
                                // Testa nova rota:
                                double distReal = testaRota(evRouteAux, evRouteAux.routeSize, instancia, false, evRoute[0].tempoSaida,
                                                            ultimoValIndice, nullptr);

                                // Verifica se a nova rota eh viavel
                                if(distReal > 0.0 && (distReal+1e-3) < evRoute.distancia)
                                {

#if PRINT_MV_SWAP == TRUE
cout<<"\t\t\tRota viavel. Dist: "<<distReal<<"\n\n";
#endif
                                    double distOrig = evRoute.distancia;
                                    distReal = testaRota(evRouteAux, evRouteAux.routeSize, instancia, true, evRoute[0].tempoSaida,
                                                            ultimoValIndice, nullptr);

                                    double dif = (distReal-distOrig)/distOrig;

//cout<<"MELHORA: "<<100.0*dif<<"%\n";

                                    satelite.distancia -= evRoute.distancia;
                                    solucao.distancia  -= evRoute.distancia;

                                    evRouteAux.distancia = distReal;
                                    evRoute.copia(evRouteAux, true, &instancia);

                                    satelite.distancia += distReal;
                                    solucao.distancia  += distReal;

                                    return true;
                                }
                                else
                                {
                                    // Tenta viabilizar rota
                                    if(viabilizaRotaEv(evRouteAux, instancia, false, insereEstacao, (evRoute.distancia-1E-3), false, evRoute[0].tempoSaida))
                                    {

                                        if((evRouteAux.distancia+10E-3) < evRoute.distancia)
                                        {

                                            double distOrig = evRoute.distancia;

                                            insereEstacaoRota(evRouteAux, insereEstacao, instancia,
                                                              evRoute[0].tempoSaida);
                                            double dif = (evRouteAux.distancia - distOrig) / distOrig;

//cout << "MELHORA VIAB.: " << 100.0 * dif << "%\n";

                                            solucao.distancia -= evRoute.distancia;
                                            satelite.distancia -= evRoute.distancia;

                                            evRoute.copia(evRouteAux, true, &instancia);

                                            solucao.distancia += evRoute.distancia;
                                            satelite.distancia += evRoute.distancia;

                                            return true;
                                        }
                                    }
                                }

#if PRINT_MV_SWAP == TRUE
                                else
cout<<"\t\t\tRota inviavel\n\n";
#endif

                            }

                        }
                    }
                }
            }

#if PRINT_MV_SWAP == TRUE
cout<<"*******************\n";
#endif

        }
    }

    return false;
}

bool NS_LocalSearch::mvEv2opt(Solucao &solucao, Instance &instancia, EvRoute &evRouteAux)
{

PRINT_DEBUG("", "RETURN FALSE");
    return false;

    // Copia evRoute para evRouteAux sem repeticao de RS, realizando a troca de i com j. Retorna a distancia e o ultimo indice em que a rota ainda eh valida
    auto copiaEvRoute = [&](const EvRoute &evRoute, int &ultimoValIndice, int posI, int posJ)
    {

        ultimoValIndice   = -1;
        int posEvRoute    = posJ;
        int posEvRouteAux = posI;
        double dist       = 0.0;
        int indiceEvAux   = 1;
        evRouteAux[0]     = evRoute.route[0];
        bool copiaAll     = true;

        if(posJ < posI)
        {
            int temp = posI;
            posI = posJ;
            posJ = temp;
        }

        // copia ate posI - 1
        std::copy(evRoute.route.begin(), evRoute.route.begin()+(posI-1), evRouteAux.route.begin());

        // copia de posJ ate posI
        for(; posEvRoute <= posI; )
        {
            if(evRouteAux[posEvRouteAux-1].cliente != evRoute.route[posEvRoute].cliente)
            {
                evRouteAux[posEvRouteAux].cliente = evRoute.route[posEvRoute].cliente;
                posEvRouteAux += 1;
            }

            posEvRoute -= 1;
        }

        // copia de posJ+1 ate o fim
        //std::copy(evRoute.route.begin()+posJ+1, evRoute.route.begin()+evRoute.routeSize, evRouteAux.route.begin()+posEvRouteAux);

        posEvRoute = posJ+1;
        for(; posEvRoute < evRoute.routeSize; ++posEvRoute)
        {
            if(evRoute.route[posEvRoute].cliente != evRouteAux[posEvRouteAux-1].cliente)
            {
                evRouteAux[posEvRouteAux].cliente = evRoute.route[posEvRoute].cliente;
                posEvRouteAux += 1;
            }
        }

        evRouteAux.routeSize = posEvRouteAux;
        posEvRouteAux = 1;
        dist = 0.0;

        // Calcula a distancia e o tamanho da nova rota
        for(; posEvRouteAux < evRouteAux.routeSize; ++posEvRouteAux)
            dist += instancia.getDistance(evRouteAux[posEvRouteAux-1].cliente, evRouteAux[posEvRouteAux].cliente);

        return dist;
    };

    InsercaoEstacao insereEstacao;

    // Percorre os satelites
    for(int sat=instancia.getFirstSatIndex(); sat <= instancia.getEndSatIndex(); ++sat)
    {
        Satelite &satelite = solucao.satelites[sat];
        evRouteAux.satelite = sat;
        evRouteAux[0].cliente = sat;

        // Percorre os EVs
        for(int ev=0; ev < satelite.getNRoutes(); ++ev)
        {

#if PRINT_MV_2OPT == TRUE
            cout<<"EV: "<<ev<<"\n";
#endif
            EvRoute &evRoute = satelite.vetEvRoute[ev];

            // Verifica se ev eh diferente de vazio e se existe mais de um cliente na rota
            if(evRoute.routeSize > 4)
            {

#if PRINT_MV_2OPT == TRUE
                string strRota;
evRoute.print(strRota, instancia, true);
cout<<" ROTA: "<<strRota<<"\n";
#endif
                /* ****************************************************************************************************
                 * ****************************************************************************************************
                 * i, j percorrem evRoute realizando 2opt nos postos  i e j
                 *
                 *  Verificar(se (i+2) > j)
                 *
                 *  Calculo da distancia:
                 *
                 *      Rota: 0 k i l ... m j n 0
                 *      Dist(nova rota): dist(rota) + (-(k,i) -(i,l) -(m,j) -(j,n)) + (+(k,j) +(j,l) +(m,i) +(i,n))
                 *
                 *    *   *
                 *  0 1 2 3 4 5 6 7 8 9 0
                 *
                 * ****************************************************************************************************
                 * ****************************************************************************************************
                 */

                // i tem que ser menor ou igual que a ant penultima possicao de j
                for(int i=1; i <= (evRoute.routeSize-4); ++i)
                {

                    const int clienteI_menos1 = evRoute[i-1].cliente;
                    const int clienteI        = evRoute[i].cliente;
                    const int clienteI_mais1  = evRoute[i+1].cliente;


#if PRINT_MV_2OPT == TRUE
cout<<"clienteI: "<<clienteI<<"\n";
#endif

                    for(int j=(i+3); j <= (evRoute.routeSize-1); ++j)
                    {

                        const int clienteJ_menos1 = evRoute[j-1].cliente;
                        const int clienteJ        = evRoute[j].cliente;
                        const int clienteJ_mais1  = evRoute[j+1].cliente;


#if PRINT_MV_2OPT == TRUE
cout<<"clienteJ: "<<clienteJ<<"\n";
#endif

                        // Calcula a nova distancia
                        double novaDist = evRoute.distancia;

                        novaDist += -instancia.getDistance(clienteI_menos1, clienteI) -instancia.getDistance(clienteI, clienteI_mais1) +
                                    -instancia.getDistance(clienteJ_menos1, clienteJ) -instancia.getDistance(clienteJ, clienteJ_mais1) +
                                    +instancia.getDistance(clienteI_menos1, clienteJ) +instancia.getDistance(clienteJ, clienteI_mais1)+
                                    +instancia.getDistance(clienteJ_menos1, clienteI) +instancia.getDistance(clienteI, clienteJ_mais1);


                        // Verifica se existe melhora
                        if(novaDist < evRoute.distancia)
                        {

#if PRINT_MV_2OPT == TRUE
                            cout<<"\tClienteI: "<<clienteI<<"\n";
cout<<"\n\t\tClienteJ: "<<clienteJ<<"\n";
cout<<"\t\t\tNova dist: "<<novaDist<<"\n";
#endif

                            int ultimoValIndice = 0;

                            // Copia rota
                            novaDist = copiaEvRoute(evRoute, ultimoValIndice, i, j);

#if PRINT_MV_2OPT == TRUE
                            string strRota1;
evRouteAux.print(strRota1, instancia, true);
cout<<"\t\t\tNova rota: "<<strRota1<<"\n\n";
#endif

                            if(novaDist < evRoute.distancia)
                            {
                                // Testa nova rota:
                                double distReal = testaRota(evRouteAux, evRouteAux.routeSize, instancia, false, evRoute[0].tempoSaida,
                                                            ultimoValIndice, nullptr);

                                // Verifica se a nova rota eh viavel
                                if(distReal > 0.0 && (distReal+1e-3) < evRoute.distancia)
                                {

#if PRINT_MV_2OPT == TRUE
cout<<"\t\t\tRota viavel. Dist: "<<distReal<<"\n\n";
#endif
                                    double distOrig = evRoute.distancia;
                                    distReal = testaRota(evRouteAux, evRouteAux.routeSize, instancia, true, evRoute[0].tempoSaida,
                                                         ultimoValIndice, nullptr);

                                    double dif = (distReal-distOrig)/distOrig;

cout<<"MELHORA: "<<100.0*dif<<"%\n";

                                    satelite.distancia -= evRoute.distancia;
                                    solucao.distancia  -= evRoute.distancia;

                                    evRouteAux.distancia = distReal;
                                    evRoute.copia(evRouteAux, true, &instancia);

                                    satelite.distancia += distReal;
                                    solucao.distancia  += distReal;

                                    return true;
                                }
                                else
                                {
                                    // Tenta viabilizar rota
                                    if(viabilizaRotaEv(evRouteAux, instancia, false, insereEstacao, (evRoute.distancia-1E-3), false, evRoute[0].tempoSaida))
                                    {

                                        if((evRouteAux.distancia+10E-3) < evRoute.distancia)
                                        {

                                            double distOrig = evRoute.distancia;

                                            insereEstacaoRota(evRouteAux, insereEstacao, instancia,
                                                              evRoute[0].tempoSaida);
                                            double dif = (evRouteAux.distancia - distOrig) / distOrig;

cout << "MELHORA VIAB.: " << 100.0 * dif << "%\n";

                                            solucao.distancia -= evRoute.distancia;
                                            satelite.distancia -= evRoute.distancia;

                                            evRoute.copia(evRouteAux, true, &instancia);

                                            solucao.distancia += evRoute.distancia;
                                            satelite.distancia += evRoute.distancia;

                                            return true;
                                        }
                                    }

#if PRINT_MV_2OPT == TRUE
                                    else
                                    {
cout<<"\t\t\tRota inviavel\n\n";

                                    }
#endif

                                }
                            }
                        }
                    }
                }
            }

#if PRINT_MV_2OPT == TRUE
            cout<<"*******************\n";
#endif

        }
    }

    return false;
}


/************************************************************************************************************
 ************************************************************************************************************
 **
 ** @param solucao          Modificado caso encontre um sol vizinha de melhora
 ** @param instancia        Nao modificado
 ** @param evRouteAux0      Rota aux; modificado
 ** @param evRouteAux1      Rota aux; modificado
 ** @param interSat         Se false: mov so eh realizado em rotas do mesmo sat. Se true: mov so eh realizado em rotas de sats diferentes
 ** @return                 true se encontou sol melhor, false caso contrario
 **
 ************************************************************************************************************
 ************************************************************************************************************/
bool NS_LocalSearch::mvEvShifitInterRotas(Solucao &solucao, Instance &instancia, EvRoute &evRouteAux0, EvRoute &evRouteAux1, const bool interSat)
{

    auto geraSequecia = [&](const EvRoute &evRouteRef, EvRoute &evRoute, const int pos, const int clienteAdd = -1)
    {
        const int fistCliente = instancia.getFirstClientIndex();

        if(clienteAdd >= instancia.getFirstClientIndex() && evRouteRef.route[pos].cliente != clienteAdd)
        {
            PRINT_DEBUG("", "ERRO, cliente(pos("<<pos<<"), "<<evRouteRef.route[pos].cliente<<"),  != clienteAdd("<<clienteAdd<<")");
            throw "ERRO";
        }

        // Copia evRouteRef para evRoute ate pos-1
        evRoute.copiaN(evRouteRef, pos, true, &instancia);
        int posProxEvRoute = pos;

        // Copia pos
        if(clienteAdd >= fistCliente)
        {
            evRoute[pos] = evRouteRef.route[pos];
            evRoute.distancia += instancia.getDistance(evRoute[pos-1].cliente, evRoute[pos].cliente);
            posProxEvRoute += 1;
            evRoute[posProxEvRoute].cliente = clienteAdd;
            posProxEvRoute += 1;
            evRoute.demanda += instancia.getDemand(clienteAdd);
            evRoute.distancia += instancia.getDistance(evRoute[pos].cliente, evRoute[pos+1].cliente);
        }

        if(clienteAdd >= instancia.getFirstClientIndex())
        {

        }
        else
        {

        }
    };

    for(int sat0=instancia.getFirstSatIndex(); sat0 <= instancia.getEndSatIndex(); ++sat0)
    {
        int sat1 = sat0;
        if(interSat)
            sat1 += 1;

        int satFim = instancia.getEndSatIndex();
        if(!interSat)
            satFim = sat0;

        for(; sat1 <= satFim; ++sat1)
        {

            for(int evSat0=0; evSat0 < instancia.getN_Evs(); ++evSat0)
            {
                for(int evSat1=0; evSat1 < instancia.getN_Evs(); ++evSat1)
                {
                    if(!interSat && evSat0==evSat1)
                        continue;

                    /* **************************************************************************************
                     * **************************************************************************************
                     *
                     * **************************************************************************************
                     * **************************************************************************************/

                    // Selecionar as posicoes das rotas

                } // End for(evSat1)

            } // End for(evSat0)

        } // End for(sat1)

    } // End for(sat0)

}

