/*
 * ./run ../instancias/2e-vrp-tw/Customer_5/C103_C5x.txt 1657747507

 */

#include "LocalSearch.h"
#include "../mersenne-twister.h"
#include <memory>

using namespace NS_LocalSearch;
using namespace NS_Auxiliary;
using namespace NS_Construtivo;
using namespace NS_viabRotaEv;

#define PRINT_MV_SHIFIT_INTRA FALSE
#define PRINT_MV_2OPT FALSE


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

bool NS_LocalSearch::mvEvShifitIntraRota(Solucao &solucao, Instancia &instancia, EvRoute &evRouteAux, const int selecao)
{

    const int mvId = MV_EV_SHIFIT_INTRA_ROTA;

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

#if UTILIZA_MAT_MV
            if(solucao.vetMatSatEvMv[satId](routeId, mvId) == 1)
                continue;
#endif

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
                                double distNovaRota = testaRota(evRouteAux, evRouteAux.routeSize, instancia, false,
                                                                solucao.satTempoChegMax[satId], 0, nullptr, nullptr);
                                bool exit = false;

                                if(distNovaRota < 0.0)
                                {
                                    bool viavel = viabilizaRotaEv(evRouteAux, instancia, instancia.bestInsViabRotaEv,
                                                                  insercaoEstacao, (evRoute.distancia - INCREM_DIST),
                                                                  false, solucao.satTempoChegMax[satId], nullptr);

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
/*                                {
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
                                }*/


                                //if(distNovaRota < evRoute.distancia)
                                if(menor(distNovaRota, evRoute.distancia))
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
                                            evRoute.copia(evRouteAux, true, &instancia);
                                            evRoute.vetRecarga[insercaoEstacao.estacao - instancia.getFirstRS_index()].utilizado += 1;
                                            solucao.rotaEvAtualizada(satId, routeId);

                                            return true;
                                        }
                                        else
                                        {
                                            evRoute.copia(evRouteAux, true, &instancia);
                                        }

                                        evRoute.distancia = testaRota(evRoute, evRoute.routeSize, instancia, true,
                                                                      solucao.satTempoChegMax[satId], 0, nullptr,
                                                                      nullptr);
                                        if(evRoute.distancia < 0)
                                        {
                                            PRINT_DEBUG("", "ERRO FUNC testaRota");
                                            throw "ERRO";
                                        }
                                        //solucao.recalculaDist();

#if UTILIZA_MAT_MV
                                        solucao.rotaEvAtualizada(satId, routeId);
#endif
                                        return true;
                                    }

                                }

                                #if PRINT_MV_SHIFIT_INTRA
                                    else
                                    {
                                        cout<<"Viavel, > rota orig\n";
                                    }
                                #endif

                            }
                        }
                    }
                }
            }


#if UTILIZA_MAT_MV
            solucao.vetMatSatEvMv[satId](routeId, mvId) = 1;
#endif
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
int NS_LocalSearch::setRotaMvEvShifitIntraRota(EvRoute &evRoute, EvRoute &evRouteAux, int i, int pos, Instancia &instancia)
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

void NS_LocalSearch::insereEstacaoRota(EvRoute &evRoute, NS_viabRotaEv::InsercaoEstacao &insercaoEstacao, Instancia &instance, double tempoSaida)
{
    if(insercaoEstacao.pos == -1)
        return;

    shiftVectorClienteDir(evRoute.route, insercaoEstacao.pos+1, 1, evRoute.routeSize);
    evRoute[insercaoEstacao.pos+1].cliente = insercaoEstacao.estacao;
    evRoute.routeSize += 1;
    removeRS_Repetido(evRoute, instance, false);
    evRoute.distancia = testaRota(evRoute, evRoute.routeSize, instance, true, tempoSaida, 0, nullptr, nullptr);

    if(evRoute.distancia < 0.0)
    {
        string str;
        evRoute.print(str, instance, true);

        PRINT_DEBUG("\t", "ERRO, ROTA DEVERIA SER VIAVEL: "<<str);
        throw "ERRO";
    }

}


#define PRINT_MV_SWAP FALSE

bool NS_LocalSearch::mvEvSwapIntraRota(Solucao &solucao, Instancia &instancia, EvRoute &evRouteAux)
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

    const int mvId = MV_EV_SWAP_INTRA_ROTA;

    // Percorre os satelites
    for(int sat=instancia.getFirstSatIndex(); sat <= instancia.getEndSatIndex(); ++sat)
    {
        Satelite &satelite = solucao.satelites[sat];
        evRouteAux.satelite = sat;
        evRouteAux[0].cliente = sat;

        // Percorre os EVs
        for(int ev=0; ev < satelite.getNRoutes(); ++ev)
        {

#if UTILIZA_MAT_MV
            if(solucao.vetMatSatEvMv[sat](ev, mvId) == 1)
                continue;
#endif

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

                        if(clienteI != clienteJ)
                        {
                            // Calcula a nova distancia
                            double novaDist = evRoute.distancia;

                            // Verifica se a rota eh diferente de ex: 0 k i j n 0
                            if(((i + 1) != j))
                            {
                                novaDist += -instancia.getDistance(evRoute[i - 1].cliente, evRoute[i].cliente) +
                                            -instancia.getDistance(evRoute[i].cliente, evRoute[i + 1].cliente) +
                                            -instancia.getDistance(evRoute[j - 1].cliente, evRoute[j].cliente) +
                                            -instancia.getDistance(evRoute[j].cliente, evRoute[j + 1].cliente) +
                                            +instancia.getDistance(evRoute[i - 1].cliente, evRoute[j].cliente) +
                                            +instancia.getDistance(evRoute[j].cliente, evRoute[i + 1].cliente) +
                                            +instancia.getDistance(evRoute[j - 1].cliente, evRoute[i].cliente) +
                                            +instancia.getDistance(evRoute[i].cliente, evRoute[j + 1].cliente);
                            } else
                            {
                                // Se rota eh do tipo: 0 k i j n 0
                                novaDist += -instancia.getDistance(evRoute[i - 1].cliente, clienteI) +
                                            -instancia.getDistance(clienteJ, evRoute[j + 1].cliente) +
                                            +instancia.getDistance(evRoute[i - 1].cliente, clienteJ) +
                                            +instancia.getDistance(clienteI, evRoute[j + 1].cliente);
                            }

                            // Verifica se existe melhora
                            //if((novaDist+10E-2) < evRoute.distancia)
                            if(menor(novaDist, evRoute.distancia))
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

                                //if((novaDist+10E-2) < evRoute.distancia)
                                if(menor(novaDist, evRoute.distancia))
                                {
                                    // Testa nova rota:
                                    double distReal = testaRota(evRouteAux, evRouteAux.routeSize, instancia, false,
                                                                evRoute[0].tempoSaida, ultimoValIndice, nullptr,
                                                                nullptr);

                                    // Verifica se a nova rota eh viavel
                                    //if(distReal > 0.0 && ((distReal+10E-2) < evRoute.distancia))
                                    if(distReal > 0.0 && menor(distReal, evRoute.distancia))
                                    {

#if PRINT_MV_SWAP == TRUE
                                        cout<<"\t\t\tRota viavel. Dist: "<<distReal<<"\n\n";
#endif
                                        double distOrig = evRoute.distancia;
                                        distReal = testaRota(evRouteAux, evRouteAux.routeSize, instancia, true,
                                                             evRoute[0].tempoSaida, ultimoValIndice, nullptr, nullptr);

                                        if(!menor(distReal, evRoute.distancia))
                                        {
                                            PRINT_DEBUG("","");
                                            cout<<"ERRO, ROTA REAL EH MAIOR:\n";
                                            cout<<"DIST REAL: "<<distReal<<"\nROTA DE REF.: "<<evRoute.distancia<<"\n\n";
                                            exit(-1);
                                        }

                                        double dif = (distReal - distOrig) / distOrig;

//cout<<"MELHORA: "<<100.0*dif<<"%\n";

                                        satelite.distancia -= evRoute.distancia;
                                        solucao.distancia -= evRoute.distancia;
                                        evRouteAux.distancia = distReal;

                                        //cout<<"EV ROUTE ANTES: \n";
                                        //evRoute.print(instancia, true);

                                        evRoute.copia(evRouteAux, true, &instancia);


                                        satelite.distancia += distReal;
                                        solucao.distancia += distReal;
                                        solucao.recalculaDist();

#if UTILIZA_MAT_MV
                                        solucao.rotaEvAtualizada(sat, ev);
#endif

                                        return true;
                                    }
                                    else if(distReal < 0.0)
                                    {
                                        // Tenta viabilizar rota
                                        if(viabilizaRotaEv(evRouteAux, instancia, false, insereEstacao,
                                                           (evRoute.distancia - 10E-2), false, evRoute[0].tempoSaida,
                                                           nullptr))
                                        {

                                            //if((evRouteAux.distancia+10E-2) < evRoute.distancia)
                                            if(menor(insereEstacao.distanciaRota, evRoute.distancia))
                                            {
                                                double distOrig = evRoute.distancia;
                                                insereEstacaoRota(evRouteAux, insereEstacao, instancia, evRoute[0].tempoSaida);
                                                double dif = (evRouteAux.distancia - distOrig) / distOrig;


                                                if(!menor(evRouteAux.distancia, evRoute.distancia))
                                                {
                                                    PRINT_DEBUG("", "ERRO!\n");
                                                    cout<<"menor("<<evRouteAux.distancia<<", "<<evRoute.distancia<<"): 0\n";
                                                    throw(-1);
                                                }


//cout << "MELHORA VIAB.: " << 100.0 * dif << "%\n";

                                                solucao.distancia -= evRoute.distancia;
                                                satelite.distancia -= evRoute.distancia;
                                                evRoute.copia(evRouteAux, true, &instancia);
                                                evRoute.vetRecarga[insereEstacao.estacao - instancia.getFirstRS_index()].utilizado += 1;
                                                evRoute.atualizaParametrosRota(instancia);

                                                solucao.distancia += evRoute.distancia;
                                                satelite.distancia += evRoute.distancia;
                                                solucao.recalculaDist();


#if UTILIZA_MAT_MV
                                                solucao.rotaEvAtualizada(sat, ev);
#endif
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
            }

#if PRINT_MV_SWAP == TRUE
cout<<"*******************\n";
#endif

        }
    }

    return false;
}

bool NS_LocalSearch::mvEv2opt(Solucao &solucao, Instancia &instancia, EvRoute &evRouteAux)
{

/*PRINT_DEBUG("", "RETURN FALSE");
    return false;*/

    const int mv = MV_EV_2OPT;

    // Copia evRoute para evRouteAux sem repeticao de RS, realizando a troca de i com j. Retorna a distancia e o ultimo indice em que a rota ainda eh valida
    auto copiaEvRoute = [&](const EvRoute &evRoute, int &ultimoValIndice, int posI, int posJ)
    {

        ultimoValIndice   = posI-1;
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
        std::copy(evRoute.route.begin(), evRoute.route.begin()+posI, evRouteAux.route.begin());

        // copia de posJ ate posI
        for(; posEvRoute >= posI; )
        {
            //cout<<evRouteAux[posEvRouteAux-1].cliente<<" != "<<evRoute.route[posEvRoute].cliente<<"\n";
            if(evRouteAux[posEvRouteAux-1].cliente != evRoute.route[posEvRoute].cliente)
            {
                evRouteAux[posEvRouteAux].cliente = evRoute.route[posEvRoute].cliente;
                //cout<<evRouteAux[posEvRouteAux].cliente<<" ";
                posEvRouteAux += 1;
            }

            posEvRoute -= 1;
        }

        //cout<<"\n\n";

        // copia de posJ+1 ate o fim
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
    for(int sat = instancia.getFirstSatIndex(); sat <= instancia.getEndSatIndex(); ++sat)
    {
        Satelite &satelite = solucao.satelites[sat];
        evRouteAux.satelite = sat;
        evRouteAux[0].cliente = sat;

        // Percorre os EVs
        for(int ev = 0; ev < satelite.getNRoutes(); ++ev)
        {

#if UTILIZA_MAT_MV
            if(solucao.vetMatSatEvMv[sat](ev, mv) == 1)
                continue;
#endif

#if PRINT_MV_2OPT == TRUE
            cout<<"EV: "<<ev<<"\n";
#endif
            EvRoute &evRoute = satelite.vetEvRoute[ev];

            // Verifica se ev eh diferente de vazio e se existe mais de um cliente na rota
            if(evRoute.routeSize <= 4)
                continue;


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
             *      Dist(nova rota): dist(rota) + (-(k,i) -(j,n)) + (+(k,j) +(i,n))
             *
             *    *   *
             *  0 1 2 3 4 5 6 7 8 9 0
             *
             * ****************************************************************************************************
             * ****************************************************************************************************
             */

            // i tem que ser menor ou igual que a ant penultima possicao de j
            for(int i = 1; i <= (evRoute.routeSize - 4); ++i)
            {

                const int clienteI_menos1 = evRoute[i - 1].cliente;
                const int clienteI = evRoute[i].cliente;
                //const int clienteI_mais1  = evRoute[i+1].cliente;


#if PRINT_MV_2OPT == TRUE
                cout<<"clienteI: "<<clienteI<<"("<<i<<")\n";
#endif

                for(int j = (i + 3); j <= (evRoute.routeSize - 2); ++j)
                {

                    //const int clienteJ_menos1 = evRoute[j-1].cliente;
                    const int clienteJ = evRoute[j].cliente;
                    const int clienteJ_mais1 = evRoute[j + 1].cliente;


#if PRINT_MV_2OPT == TRUE
                    cout<<"clienteJ: "<<clienteJ<<"("<<j<<")\n";
#endif

                    // Calcula a nova distancia
                    double novaDist = evRoute.distancia;

                    novaDist += -instancia.getDistance(clienteI_menos1, clienteI) +
                                -instancia.getDistance(clienteJ, clienteJ_mais1) +
                                +instancia.getDistance(clienteI_menos1, clienteJ) +
                                +instancia.getDistance(clienteI, clienteJ_mais1);

                    // Verifica se existe melhora

                    //if(novaDist < evRoute.distancia)
                    if(menor(novaDist, evRoute.distancia))
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
cout<<"\tnovaDistC("<<novaDist<<"); dist rota("<<evRoute.distancia<<")\n";
#endif


                        //if(novaDist < evRoute.distancia)
                        if(menor(novaDist, evRoute.distancia))
                        {
                            // Testa nova rota:
                            double distReal = testaRota(evRouteAux, evRouteAux.routeSize, instancia, false,
                                                        evRoute[0].tempoSaida, ultimoValIndice, nullptr, nullptr);

                            // Verifica se a nova rota eh viavel
                            //if(distReal > 0.0 && (distReal+1e-3) < evRoute.distancia)
                            if(distReal > 0.0 && menor(distReal, evRoute.distancia))
                            {

#if PRINT_MV_2OPT == TRUE
                                cout<<"\t\t\tRota viavel. Dist: "<<distReal<<"\n\n";
#endif
                                double distOrig = evRoute.distancia;
                                distReal = testaRota(evRouteAux, evRouteAux.routeSize, instancia, true,
                                                     evRoute[0].tempoSaida, ultimoValIndice, nullptr, nullptr);

                                if(!menor(distReal, evRoute.distancia))
                                {
                                    PRINT_DEBUG("", "");
                                    cout << "ERRO, ROTA REAL EH MAIOR:\n";
                                    cout << "DIST REAL: " << distReal << "\nROTA DE REF.: " << evRoute.distancia
                                         << "\n\n";
                                    exit(-1);

                                }

#if PRINT_MV_2OPT == TRUE
                                double dif = (distReal-distOrig)/distOrig;
cout<<"\t\t\tMELHORA: "<<100.0*dif<<"%\n";
#endif

                                satelite.distancia -= evRoute.distancia;
                                solucao.distancia -= evRoute.distancia;

                                evRouteAux.distancia = distReal;
                                evRoute.copia(evRouteAux, true, &instancia);

                                satelite.distancia += distReal;
                                solucao.distancia += distReal;
                                //solucao.recalculaDist();

#if UTILIZA_MAT_MV
                                solucao.rotaEvAtualizada(sat, ev);
#endif

                                return true;
                            } else if(distReal <= 0.0)
                            {
                                // Tenta viabilizar rota
                                if(viabilizaRotaEv(evRouteAux, instancia, true, insereEstacao,
                                                   (evRoute.distancia - 1E-3), false, evRoute[0].tempoSaida, nullptr))
                                {

                                    //if((evRouteAux.distancia+10E-3) < evRoute.distancia)
                                    if(menor(insereEstacao.distanciaRota, evRoute.distancia))
                                    {

                                        double distOrig = evRoute.distancia;
                                        insereEstacaoRota(evRouteAux, insereEstacao, instancia, evRoute[0].tempoSaida);


#if PRINT_MV_2OPT == TRUE
                                        double dif = (evRouteAux.distancia - distOrig) / distOrig;
cout << "\t\t\tMELHORA VIAB.: " << 100.0 * dif << "%\n";
#endif
                                        solucao.distancia -= evRoute.distancia;
                                        satelite.distancia -= evRoute.distancia;
                                        evRoute.copia(evRouteAux, true, &instancia);
                                        evRoute.vetRecarga[insereEstacao.estacao -
                                                           instancia.getFirstRS_index()].utilizado += 1;

                                        solucao.distancia += evRoute.distancia;
                                        satelite.distancia += evRoute.distancia;
                                        //solucao.recalculaDist();

#if UTILIZA_MAT_MV
                                        solucao.rotaEvAtualizada(sat, ev);
#endif

                                        return true;
                                    }

#if PRINT_MV_2OPT == TRUE
                                    else
cout<<"\t\t\tROTA VIABILIZADA > DO QUE ROTA ORIGINAL\n";
#endif

                                }

#if PRINT_MV_2OPT == TRUE
                                else
                                {
cout<<"\t\t\tRota inviavel\n\n";

                                }
#endif

                            }
                        }

#if PRINT_MV_2OPT == TRUE
                        else
cout<<"\t\t\tDIST REAL EH MAIOR!!\n";
#endif
                    }
                }
            }


#if PRINT_MV_2OPT == TRUE
            cout<<"*******************\n";
#endif

#if UTILIZA_MAT_MV
            solucao.vetMatSatEvMv[sat](ev, mv) = 1;
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
 ** @return                 true se encontou sol melhor, false caso contrario
 **
 ************************************************************************************************************
 ************************************************************************************************************/

bool NS_LocalSearch::mvEvShifitInterRotasIntraSat(Solucao &solucao, Instancia &instancia, EvRoute &evRouteAux0, EvRoute &evRouteAux1)
{

    //cout<<"mvEvShifitInterRotas\n";

    const int mv = MV_EV_SHIFIT_INTER_ROTAS_INTRA_SAT;

    const bool interSat = false;

    if(instancia.numEv == 1)
        return false;

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
                EvRoute &evRouteSat0 = solucao.satelites[sat0].vetEvRoute[evSat0];

                for(int evSat1=0; evSat1 < instancia.getN_Evs(); ++evSat1)
                {

                    if(!interSat && evSat0==evSat1)
                        continue;


#if UTILIZA_MAT_MV
                    if(solucao.vetMatSatEvMv[sat1](evSat0, mv) == 1 && solucao.vetMatSatEvMv[sat1](evSat1, mv) == 1)
                        continue;
#endif

//cout<<"ev0: "<<evSat0<<"; ev1: "<<evSat1<<"\n";

                    EvRoute &evRouteSat1 = solucao.satelites[sat1].vetEvRoute[evSat1];

                    // Selecionar as posicoes das rotas
                    for(int posEvSat0=0; posEvSat0 < (evRouteSat0.routeSize-1); ++posEvSat0)
                    {

                         for(int posEvSat1=0; posEvSat1 < (evRouteSat1.routeSize-1); ++posEvSat1)
                         {

                             /* ******************************************************************************************
                              * ******************************************************************************************
                              *  1º
                              *     O cliente na posicao (posEvSat0+1) do ev evSat0 ira para a posicao (posEvSat1+1)
                              *   evSat0 nao pode estar vazio,  cliente nao pode ser rs, verificar a nova carga de evSat1
                              *
                              *  2º
                              *     O cliente na posicao (posEvSat1+1) do ev evSat1 ira para a posicao (posEvSat0+1)
                              *   evSat1 nao pode estar vazio, e cliente nao pode ser rs, verificar a nova carga de evSat0
                              *
                              * ********************************************************************************************
                              * ********************************************************************************************/

                             auto realizaMv=[](Instancia &instancia, EvRoute &evRoute0, int posEvRoute0, EvRoute &evRoute1, int posEvRoute1,
                                               EvRoute &evRouteAux0, EvRoute &evRouteAux1, const double tempoSaidaSat)
                             {

/*PRINT_DEBUG("", "");

string strRota;
evRoute0.print(strRota, instancia, true);
cout<<"evRoute0: "<<strRota<<"\n";
cout<<"posEvRoute0: "<<posEvRoute0<<"\n\n";

strRota = "";
evRoute1.print(strRota, instancia, true);
cout<<"evRoute1: "<<strRota<<"\n";
cout<<"posEvRooute1: "<<posEvRoute1<<"\n\n";

cout<<"********************************************\n********************************************\n\n";*/


                                 // cliente em (posEvRoute0+1) ira para (posEvRoute1+1) da rota evRoute1
                                 // evRoute nao pode ser vazio e verificar nova carga de evRoute1

                                 if((evRoute0.routeSize <= 2) || instancia.isRechargingStation(evRoute0[(posEvRoute0+1)].cliente))
                                     return false;

                                 if(evRoute1.routeSize <=2 && posEvRoute1 != 0)
                                     return false;

                                 if(evRoute0.routeSize == 3 && evRoute1.routeSize == 2)
                                     return false;

                                 if(!(posEvRoute0 < (evRoute0.routeSize-3)))
                                     return false;

                                 const int cliente = evRoute0[posEvRoute0+1].cliente;

                                 // Verifica carga de evRoute1
                                 if((evRoute1.demanda+instancia.getDemand(cliente)) > instancia.vectVeiculo[evRoute1.idRota].capacidade)
                                     return false;


                                 // Calcula distancia
                                 const double distOrig = evRoute0.distancia+evRoute1.distancia;

                                 // Calcula nova distancia
                                 double novaDist = distOrig;

//cout<<"evRoute0[posEvRoute0+2].cliente: "<<evRoute0[posEvRoute0+2].cliente<<"\n";

                                 novaDist += -(instancia.getDistance(evRoute0[posEvRoute0].cliente, cliente)+
                                               instancia.getDistance(cliente, evRoute0[posEvRoute0+2].cliente));
                                 novaDist += instancia.getDistance(evRoute0[posEvRoute0].cliente, evRoute0[posEvRoute0+2].cliente);
                                 novaDist += -(instancia.getDistance(evRoute1[posEvRoute1].cliente, evRoute1[posEvRoute1+1].cliente));
                                 novaDist +=   instancia.getDistance(evRoute1[posEvRoute1].cliente, cliente);
                                 novaDist +=   instancia.getDistance(cliente, evRoute1[posEvRoute1+1].cliente);

                                 // Verifica se novaDist < distOrig
                                 if(menor(novaDist,distOrig))
                                 {
//cout<<"novaDist("<<novaDist<<") < distOrig("<<distOrig<<")!\n";

                                     // Copia evRoute1 para evRouteAux1 e add cliente
                                     evRouteAux1.copia(evRoute1, true, &instancia);
                                     shiftVectorClienteDir(evRouteAux1.route, (posEvRoute1+1), 1, evRouteAux1.routeSize);
                                     evRouteAux1[posEvRoute1+1].cliente = cliente;
                                     evRouteAux1.routeSize += 1;


/*
string strRota1;
evRoute1.print(strRota1, instancia, true);
cout<<"evRoute1: "<<strRota1<<"\n";

strRota1 = "";
evRouteAux1.print(strRota1, instancia, true);
cout<<"nova evRoute1: "<<strRota1<<"\n";
*/

                                     double distNovaRota1 = testaRota(evRouteAux1, evRouteAux1.routeSize, instancia,
                                                                      false, tempoSaidaSat, 0, nullptr, nullptr);
                                     InsercaoEstacao insercaoEstacao;
                                     bool novaRota1Viavel = true;
                                     bool rotaViabilizada = false;

                                     // Verifica se a nova rota eh viavel
                                     if(distNovaRota1 <= 0.0)
                                     {
                                         /*
                                          * Rota0 custo c0
                                          * Rota1 custo c1
                                          *
                                          * Nova Rota0 custo nc0; nc0 < c0
                                          * Nova Rota1 custo nc1; nc1 > c1
                                          *
                                          * c0+c1 > nc0+nc1
                                          * c0+c1 -nc0 > nc1
                                          * |_nc1_| = (c0+c1 -nc0)
                                          */
                                         double estNovaDist0 = evRoute0.distancia;
                                         estNovaDist0 += -(instancia.getDistance(evRoute0[posEvRoute0].cliente, cliente)+
                                                           instancia.getDistance(cliente, evRoute0[posEvRoute0+2].cliente));
                                         estNovaDist0 += instancia.getDistance(evRoute0[posEvRoute0].cliente, evRoute0[posEvRoute0+2].cliente);
                                         double limNovaDist1 = evRoute0.distancia+evRoute1.distancia - estNovaDist0;



                                        novaRota1Viavel = viabilizaRotaEv(evRouteAux1, instancia, false,
                                                                          insercaoEstacao, limNovaDist1, false,
                                                                          tempoSaidaSat, nullptr);
                                        if(novaRota1Viavel)
                                        {
                                            distNovaRota1 = insercaoEstacao.distanciaRota;
                                            rotaViabilizada = true;
                                        }

                                     }
                                     else
                                     {
                                         distNovaRota1 = testaRota(evRouteAux1, evRouteAux1.routeSize, instancia, true,
                                                                   tempoSaidaSat, 0, nullptr, nullptr);
                                         evRouteAux1.distancia = distNovaRota1;
                                     }

                                     if(novaRota1Viavel)
                                     {
/*string strRota;
evRoute0.print(strRota, instancia, false);
cout<<"evRoute0: "<<strRota<<"\n";*/

                                         evRouteAux0.copia(evRoute0, true, &instancia);
                                         shiftVectorClienteEsq(evRouteAux0.route, posEvRoute0+1, evRouteAux0.routeSize);
                                         evRouteAux0.routeSize -= 1;
                                         removeRS_Repetido(evRouteAux0, instancia, false);

/*strRota="";
evRouteAux0.print(strRota, instancia, false);
cout<<"nova evRoute0: "<<strRota<<"\n\n";*/

                                         double distNovaRota0 = testaRota(evRouteAux0, evRouteAux0.routeSize, instancia,
                                                                          true, tempoSaidaSat, 0, nullptr, nullptr);
                                         evRouteAux0.distancia = distNovaRota0;

                                         // Verifica Viabilidade
                                         if(distNovaRota0 <= 0.0)
                                         {
                                             //PRINT_DEBUG("", "ERRO, ROTA0 DEVERIA SER VIAVEL!");
                                             return false;
                                         }

                                         if(menor((distNovaRota0+distNovaRota1), distOrig))
                                         {
                                             evRoute0.copia(evRouteAux0, true, &instancia);

                                             if(rotaViabilizada)
                                             {
                                                 try
                                                 {
                                                     insereEstacaoRota(evRouteAux1, insercaoEstacao, instancia, tempoSaidaSat);
                                                 } catch(const char *erro)
                                                 {
                                                     PRINT_DEBUG("", "ERRO ROTRA JA FOI TESTADA COM RESULTADO TRUE, ...");
                                                     return false;
                                                 }
                                             }

                                             evRoute1.copia(evRouteAux1, true, &instancia);
                                             evRoute0.atualizaParametrosRota(instancia);
                                             evRoute1.atualizaParametrosRota(instancia);
/*
                                             cout<<"ATUAL.\n";
                                             string strRota;
                                             evRoute0.print(strRota, instancia, true);
                                             cout<<"NOVA ROTA0: "<<strRota<<"\n";

                                             strRota = "";

                                             evRoute1.print(strRota, instancia, true);
                                             cout<<"NOVA ROTA1: "<<strRota<<"\n";
*/

                                             return true;

                                         }

                                         else
                                             return false;
                                     }
                                 }

                                 return false;
                             };

                             const double distOrig    = evRouteSat0.distancia+evRouteSat1.distancia;
                             const double demandaOrig = evRouteSat0.demanda+evRouteSat1.demanda;

                             bool resutado = realizaMv(instancia, evRouteSat0, posEvSat0, evRouteSat1, posEvSat1, evRouteAux0, evRouteAux1, evRouteSat0[0].tempoSaida);
                             //if(!resutado)
                             //    resutado = realizaMv(instancia, evRouteSat1, posEvSat1, evRouteSat0, posEvSat0, evRouteAux0, evRouteAux1, evRouteSat1[0].tempoSaida);

                             if(resutado)
                             {
                                 double novaDist = evRouteSat0.distancia+evRouteSat1.distancia;
                                 solucao.distancia += -distOrig + novaDist;
                                 solucao.satelites[sat0].distancia += -distOrig + novaDist;


#if UTILIZA_MAT_MV
                                 solucao.rotaEvAtualizada(sat1, evSat0);
                                 solucao.rotaEvAtualizada(sat1, evSat1);
#endif

                                 //cout<<"MV UPDATE\n";

                                 double novaDemanda = evRouteSat0.demanda+evRouteSat1.demanda;
                                 if(novaDemanda != demandaOrig)
                                 {
                                     PRINT_DEBUG("","");
                                     cout<<"ERRO! NOVA DEMANDA("<<novaDemanda<<") != DEMANDA ORIGINAL("<<demandaOrig<<")\n";
                                     throw "ERRO";
                                 }

                                 return true;
                             }

                         } // End for(posEvSat1)

                    } // End for(posEvSat0)

                } // End for(evSat1)

#if UTILIZA_MAT_MV
                solucao.vetMatSatEvMv[sat1](evSat0, mv) = 1;
#endif

            } // End for(evSat0)

        } // End for(sat1)

    } // End for(sat0)

    return false;

}

void NS_LocalSearch::removeRS_Repetido(EvRoute &evRoute, Instancia &instancia, bool atualizaParam)
{

    if(evRoute.routeSize <= 2)
        return;

    for(int i=2; i < (evRoute.routeSize-1); )
    {
        if(evRoute[i].cliente == evRoute[i-1].cliente)
        {
            shiftVectorClienteEsq(evRoute.route, i, evRoute.routeSize);
            evRoute.routeSize -= 1;
        }
        else
            i += 1;
    }


    if(atualizaParam)
        evRoute.atualizaParametrosRota(instancia);
}

/************************************************************************************************************
 ************************************************************************************************************
 **
 ** @param solucao          Modificado caso encontre um sol vizinha de melhora
 ** @param instancia        Nao modificado
 ** @param evRouteAux0      Rota aux; modificado
 ** @param evRouteAux1      Rota aux; modificado
 *  @param beta             Parametro beta para a heuristica do primeiro nivel
 ** @return                 true se encontou sol melhor, false caso contrario
 **
 ************************************************************************************************************
 ************************************************************************************************************/

bool NS_LocalSearch::mvEvShifitInterRotasInterSats(Solucao &solucao, Instancia &instancia, EvRoute &evRouteAux0, EvRoute &evRouteAux1, const float beta)
{
    //cout<<"mvEvShifitInterRotas\n";

    const bool interSat = true;

    if(instancia.numSats == 1)
        return false;

    static Solucao solucaoAux(instancia);
    solucaoAux.copia(solucao);

    const int mv = MV_EV_SHIFIT_INTER_ROTAS_INTER_SAT;


    for(int sat0=instancia.getFirstSatIndex(); sat0 <= instancia.getEndSatIndex(); ++sat0)
    {

        for(int sat1=instancia.getFirstSatIndex(); sat1 <= instancia.getEndSatIndex(); ++sat1)
        {

            if(sat0 == sat1)
                continue;

            for(int evSat0=0; evSat0 < instancia.getN_Evs(); ++evSat0)
            {
                EvRoute &evRouteSat0 = solucaoAux.satelites[sat0].vetEvRoute[evSat0];

                for(int evSat1=0; evSat1 < instancia.getN_Evs(); ++evSat1)
                {

#if UTILIZA_MAT_MV
                    if(solucao.vetMatSatEvMv[sat0](evSat0, mv) == 1 && solucao.vetMatSatEvMv[sat1](evSat1, mv) == 1)
                        continue;
#endif

                    EvRoute &evRouteSat1 = solucaoAux.satelites[sat1].vetEvRoute[evSat1];

                    // Selecionar as posicoes das rotas
                    for(int posEvSat0=0; posEvSat0 < (evRouteSat0.routeSize-1); ++posEvSat0)
                    {
                        for(int posEvSat1=0; posEvSat1 < (evRouteSat1.routeSize-1); ++posEvSat1)
                        {

                            /* ******************************************************************************************
                             * ******************************************************************************************
                             *  1º
                             *     O cliente na posicao (posEvSat0+1) do ev evSat0 ira para a posicao (posEvSat1+1)
                             *   evSat0 nao pode estar vazio,  cliente nao pode ser rs, verificar a nova carga de evSat1
                             *
                             *  2º
                             *     O cliente na posicao (posEvSat1+1) do ev evSat1 ira para a posicao (posEvSat0+1)
                             *   evSat1 nao pode estar vazio, e cliente nao pode ser rs, verificar a nova carga de evSat0
                             *
                             * ********************************************************************************************
                             * ********************************************************************************************/

                            auto realizaMv=[](Instancia &instancia, EvRoute &evRoute0, int posEvRoute0, EvRoute &evRoute1, int posEvRoute1,
                                              EvRoute &evRouteAux0, EvRoute &evRouteAux1, const double tempoSaidaSat)
                            {

/*PRINT_DEBUG("", "");

string strRota;
evRoute0.print(strRota, instancia, true);
cout<<"evRoute0: "<<strRota<<"\n";
cout<<"posEvRoute0: "<<posEvRoute0<<"\n\n";

strRota = "";
evRoute1.print(strRota, instancia, true);
cout<<"evRoute1: "<<strRota<<"\n";
cout<<"posEvRooute1: "<<posEvRoute1<<"\n\n";

cout<<"********************************************\n********************************************\n\n";*/


                                // cliente em (posEvRoute0+1) ira para (posEvRoute1+1) da rota evRoute1
                                // evRoute nao pode ser vazio e verificar nova carga de evRoute1

                                if((evRoute0.routeSize <= 2) || instancia.isRechargingStation(evRoute0[(posEvRoute0+1)].cliente))
                                    return false;

                                if(evRoute1.routeSize <=2 && posEvRoute1 != 0)
                                    return false;

                                if(evRoute0.routeSize == 3 && evRoute1.routeSize == 2)
                                    return false;

                                if(!(posEvRoute0 < (evRoute0.routeSize-3)))
                                    return false;

                                const int cliente = evRoute0[posEvRoute0+1].cliente;

                                // Verifica carga de evRoute1
                                if((evRoute1.demanda+instancia.getDemand(cliente)) > instancia.vectVeiculo[evRoute1.idRota].capacidade)
                                    return false;


                                // Calcula distancia
                                const double distOrig = evRoute0.distancia+evRoute1.distancia;

                                // Calcula nova distancia
                                double novaDist = distOrig;

//cout<<"evRoute0[posEvRoute0+2].cliente: "<<evRoute0[posEvRoute0+2].cliente<<"\n";

                                novaDist += -(instancia.getDistance(evRoute0[posEvRoute0].cliente, cliente)+
                                              instancia.getDistance(cliente, evRoute0[posEvRoute0+2].cliente));
                                novaDist += instancia.getDistance(evRoute0[posEvRoute0].cliente, evRoute0[posEvRoute0+2].cliente);
                                novaDist += -(instancia.getDistance(evRoute1[posEvRoute1].cliente, evRoute1[posEvRoute1+1].cliente));
                                novaDist +=   instancia.getDistance(evRoute1[posEvRoute1].cliente, cliente);
                                novaDist +=   instancia.getDistance(cliente, evRoute1[posEvRoute1+1].cliente);

                                // Verifica se novaDist < distOrig
                                if(menor(novaDist,distOrig))
                                {
//cout<<"novaDist("<<novaDist<<") < distOrig("<<distOrig<<")!\n";

                                    // Copia evRoute1 para evRouteAux1 e add cliente
                                    evRouteAux1.copia(evRoute1, true, &instancia);
                                    shiftVectorClienteDir(evRouteAux1.route, (posEvRoute1+1), 1, evRouteAux1.routeSize);
                                    evRouteAux1[posEvRoute1+1].cliente = cliente;
                                    evRouteAux1.routeSize += 1;


/*
string strRota1;
evRoute1.print(strRota1, instancia, true);
cout<<"evRoute1: "<<strRota1<<"\n";

strRota1 = "";
evRouteAux1.print(strRota1, instancia, true);
cout<<"nova evRoute1: "<<strRota1<<"\n";
*/

                                    double distNovaRota1 = testaRota(evRouteAux1, evRouteAux1.routeSize, instancia,
                                                                     false, tempoSaidaSat, 0, nullptr, nullptr);
                                    InsercaoEstacao insercaoEstacao;
                                    bool novaRota1Viavel = true;
                                    bool rotaViabilizada = false;

                                    // Verifica se a nova rota eh viavel
                                    if(distNovaRota1 <= 0.0)
                                    {
                                        /*
                                         * Rota0 custo c0
                                         * Rota1 custo c1
                                         *
                                         * Nova Rota0 custo nc0; nc0 < c0
                                         * Nova Rota1 custo nc1; nc1 > c1
                                         *
                                         * c0+c1 > nc0+nc1
                                         * c0+c1 -nc0 > nc1
                                         * |_nc1_| = (c0+c1 -nc0)
                                         */
                                        double estNovaDist0 = evRoute0.distancia;
                                        estNovaDist0 += -(instancia.getDistance(evRoute0[posEvRoute0].cliente, cliente)+
                                                          instancia.getDistance(cliente, evRoute0[posEvRoute0+2].cliente));
                                        estNovaDist0 += instancia.getDistance(evRoute0[posEvRoute0].cliente, evRoute0[posEvRoute0+2].cliente);
                                        double limNovaDist1 = evRoute0.distancia+evRoute1.distancia - estNovaDist0;



                                        novaRota1Viavel = viabilizaRotaEv(evRouteAux1, instancia, false,
                                                                          insercaoEstacao, limNovaDist1, false,
                                                                          tempoSaidaSat, nullptr);
                                        if(novaRota1Viavel)
                                        {
                                            distNovaRota1 = insercaoEstacao.distanciaRota;
                                            rotaViabilizada = true;
                                        }

                                    }
                                    else
                                    {
                                        distNovaRota1 = testaRota(evRouteAux1, evRouteAux1.routeSize, instancia, true,
                                                                  tempoSaidaSat, 0, nullptr, nullptr);
                                        evRouteAux1.distancia = distNovaRota1;
                                    }

                                    if(novaRota1Viavel)
                                    {
/*string strRota;
evRoute0.print(strRota, instancia, false);
cout<<"evRoute0: "<<strRota<<"\n";*/

                                        evRouteAux0.copia(evRoute0, true, &instancia);
                                        shiftVectorClienteEsq(evRouteAux0.route, posEvRoute0+1, evRouteAux0.routeSize);
                                        evRouteAux0.routeSize -= 1;
                                        removeRS_Repetido(evRouteAux0, instancia, false);

/*strRota="";
evRouteAux0.print(strRota, instancia, false);
cout<<"nova evRoute0: "<<strRota<<"\n\n";*/

                                        double distNovaRota0 = testaRota(evRouteAux0, evRouteAux0.routeSize, instancia,
                                                                         true, tempoSaidaSat, 0, nullptr, nullptr);
                                        evRouteAux0.distancia = distNovaRota0;

                                        // Verifica Viabilidade
                                        if(distNovaRota0 <= 0.0)
                                        {
                                            //PRINT_DEBUG("", "ERRO, ROTA0 DEVERIA SER VIAVEL!");
                                            return false;
                                        }

                                        if(menor((distNovaRota0+distNovaRota1), distOrig))
                                        {
                                            evRoute0.copia(evRouteAux0, true, &instancia);

                                            if(rotaViabilizada)
                                            {
                                                try
                                                {
                                                    insereEstacaoRota(evRouteAux1, insercaoEstacao, instancia, tempoSaidaSat);
                                                } catch(const char *erro)
                                                {
                                                    PRINT_DEBUG("", "ERRO ROTRA JA FOI TESTADA COM RESULTADO TRUE, ...");
                                                    return false;
                                                }
                                            }


                                            evRoute1.copia(evRouteAux1, true, &instancia);
                                            evRoute0.atualizaParametrosRota(instancia);
                                            evRoute1.atualizaParametrosRota(instancia);

                                            return true;
                                        }
                                        else
                                            return false;
                                    }
                                }

                                return false;
                            };

                            const double distOrig    = evRouteSat0.distancia+evRouteSat1.distancia;
                            const double demandaOrig = evRouteSat0.demanda+evRouteSat1.demanda;

                            bool resutado = realizaMv(instancia, evRouteSat0, posEvSat0, evRouteSat1, posEvSat1, evRouteAux0, evRouteAux1, evRouteSat0[0].tempoSaida);
                            //if(!resutado)
                            //    resutado = realizaMv(instancia, evRouteSat1, posEvSat1, evRouteSat0, posEvSat0, evRouteAux0, evRouteAux1, evRouteSat1[0].tempoSaida);

                            if(resutado)
                            {
                                solucaoAux.recalculaDistSat(instancia);
                                solucaoAux.resetaPrimeiroNivel(instancia);
                                construtivoPrimeiroNivel(solucaoAux, instancia, beta, false, Split);

                                if(solucaoAux.viavel)
                                {
                                    if(menor(solucaoAux.distancia, solucao.distancia))
                                    {
                                        //cout<<"ATUALIZACAO: "<<solucaoAux.distancia<<" "<<solucao.distancia<<"\n";
                                        solucao.copia(solucaoAux);

#if UTILIZA_MAT_MV
                                        solucao.rotaEvAtualizada(sat0, evSat0);
                                        solucao.rotaEvAtualizada(sat1, evSat1);
#endif

                                        return true;
                                    }
                                    else
                                    {
                                        solucaoAux.copia(solucao);
                                        //evRouteSat1.copia(solucaoAux.satelites[sat1].vetEvRoute[evSat1], true, &instancia);
                                        //evRouteSat0.copia(solucaoAux.satelites[sat0].vetEvRoute[evSat0], true, &instancia);
                                    }
                                }

                            }

                        } // End for(posEvSat1)

                    } // End for(posEvSat0)

                } // End for(evSat1)

#if UTILIZA_MAT_MV
                solucao.vetMatSatEvMv[sat0](evSat0, mv) = 1;
#endif

            } // End for(evSat0)

        } // End for(sat1)

    } // End for(sat0)


    return false;
}

bool NS_LocalSearch::mvEvSwapInterRotasIntraSat(Solucao &solucao, Instancia &instancia, EvRoute &evRouteAux0, EvRoute &evRouteAux1)
{

    const bool interSat = false;

    if(instancia.numSats == 1 && instancia.numEv == 1)
        return false;

    const int mv = MV_EV_SWAP_INTER_ROTAS_INTRA_SAT;

    for(int sat = instancia.getFirstSatIndex(); sat <= instancia.getEndSatIndex(); ++sat)
    {


        for(int ev0 = 0; ev0 < instancia.getN_Evs(); ++ev0)
        {
            EvRoute &evRoute0 = solucao.satelites[sat].vetEvRoute[ev0];
            if(evRoute0.routeSize <= 2)
                continue;

            for(int ev1 = 0; ev1 < instancia.getN_Evs(); ++ev1)
            {
                if(ev0 == ev1)
                    continue;


#if UTILIZA_MAT_MV
                if(solucao.vetMatSatEvMv[sat](ev0, mv) == 1 && solucao.vetMatSatEvMv[sat](ev1, mv) == 1)
                    continue;
#endif

//cout<<"ev0: "<<evSat0<<"; ev1: "<<evSat1<<"\n";

                EvRoute &evRoute1 = solucao.satelites[sat].vetEvRoute[ev1];
                if(evRoute1.routeSize <= 2)
                    continue;
                // 0 1 2  3  4  5
                // 1 9 10 11 12 1

                // tam = 6
                //

                // Selecionar as posicoes das rotas
                for(int posEv0 = 0; posEv0 < (evRoute0.routeSize-2); ++posEv0)
                {
                    if(instancia.isRechargingStation(evRoute0[posEv0+1].cliente))
                        continue;

                    for(int posEv1 = 0; posEv1 < (evRoute1.routeSize-2); ++posEv1)
                    {

                        if(instancia.isRechargingStation(evRoute1[posEv1+1].cliente))
                            continue;

                        /* ******************************************************************************************
                         * ******************************************************************************************
                         *  1º
                         *     O cliente na posicao (posEvSat0+1) do ev evSat0 ira para a posicao (posEvSat1+1)
                         *   evSat0 nao pode estar vazio,  cliente nao pode ser rs, verificar a nova carga de evSat1
                         *
                         *  2º
                         *     O cliente na posicao (posEvSat1+1) do ev evSat1 ira para a posicao (posEvSat0+1)
                         *   evSat1 nao pode estar vazio, e cliente nao pode ser rs, verificar a nova carga de evSat0
                         *
                         * ********************************************************************************************
                         * ********************************************************************************************/

                        auto realizaMv = [](Instancia &instancia, EvRoute &evRoute0, int posEvRoute0, EvRoute &evRoute1, int posEvRoute1,
                                            EvRoute &evRouteAux0, EvRoute &evRouteAux1, const double tempoSaidaSat)
                        {

/*PRINT_DEBUG("", "");

string strRota;
evRoute0.print(strRota, instancia, true);
cout<<"evRoute0: "<<strRota<<"\n";
cout<<"posEvRoute0: "<<posEvRoute0<<"\n\n";

strRota = "";
evRoute1.print(strRota, instancia, true);
cout<<"evRoute1: "<<strRota<<"\n";
cout<<"posEvRooute1: "<<posEvRoute1<<"\n\n";

cout<<"********************************************\n********************************************\n\n";*/


                            const int clienteRoute0 = evRoute0[posEvRoute0+1].cliente;
                            const int clienteRoute1 = evRoute1[posEvRoute1+1].cliente;

                            // Verifica carga de evRoute0
                            if((evRoute0.demanda - instancia.getDemand(clienteRoute0) + instancia.getDemand(clienteRoute1)) >
                               instancia.vectVeiculo[evRoute1.idRota].capacidade)
                                return false;

                            // Verifica carga de evRoute1
                            if((evRoute1.demanda - instancia.getDemand(clienteRoute1) + instancia.getDemand(clienteRoute0)) >
                               instancia.vectVeiculo[evRoute0.idRota].capacidade)
                                return false;


                            // Calcula distancia
                            const double distOrig = evRoute0.distancia + evRoute1.distancia;

                            // Calcula nova distancia
                            double distNovaRota0Prev = evRoute0.distancia;
                            double distNovaRota1Prev = evRoute1.distancia;

//cout<<"evRoute0[posEvRoute0+2].cliente: "<<evRoute0[posEvRoute0+2].cliente<<"\n";

                            // Retira clienteRoute0 e clienteRoute1
                            distNovaRota0Prev += -(instancia.getDistance(evRoute0[posEvRoute0].cliente, clienteRoute0) +
                                                   instancia.getDistance(clienteRoute0, evRoute0[posEvRoute0 + 2].cliente));

                            distNovaRota1Prev += -(instancia.getDistance(evRoute1[posEvRoute1].cliente, clienteRoute1) +
                                                   instancia.getDistance(clienteRoute1, evRoute1[posEvRoute1 + 2].cliente));

                            // Insere clienteRoute0 em evRoute1
                            distNovaRota1Prev += instancia.getDistance(evRoute1[posEvRoute1].cliente, clienteRoute0) +
                                                 instancia.getDistance(clienteRoute0, evRoute1[posEvRoute1+2].cliente);


                            // Insere clienteRoute1 em evRoute0
                            distNovaRota0Prev += instancia.getDistance(evRoute0[posEvRoute0].cliente, clienteRoute1) +
                                                 instancia.getDistance(clienteRoute1, evRoute0[posEvRoute0+2].cliente);


                            double novaDist = distNovaRota0Prev+distNovaRota1Prev;

                            // Verifica se novaDist < distOrig
                            if(menor(novaDist, distOrig))
                            {
//cout<<"novaDist("<<novaDist<<") < distOrig("<<distOrig<<")!\n";

                                // Copia evRoute1 para evRouteAux1 e add clienteRoute0
                                evRouteAux1.copia(evRoute1, true, &instancia);
                                evRouteAux1[posEvRoute1+1].cliente = clienteRoute0;


                                // Copia evRoute0 para evRouteAux0 e add clienteRoute1
                                evRouteAux0.copia(evRoute0, true, &instancia);
                                evRouteAux0[posEvRoute0+1].cliente = clienteRoute1;

/*
string strRota1;
evRoute1.print(strRota1, instancia, true);
cout<<"evRoute1: "<<strRota1<<"\n";

strRota1 = "";
evRouteAux1.print(strRota1, instancia, true);
cout<<"nova evRoute1: "<<strRota1<<"\n";
*/

                                // Testa as novas rotas
                                double distNovaRota1 = testaRota(evRouteAux1, evRouteAux1.routeSize, instancia, false,
                                                                 tempoSaidaSat, 0, nullptr, nullptr);
                                double distNovaRota0 = testaRota(evRouteAux0, evRouteAux0.routeSize, instancia, false,
                                                                 tempoSaidaSat, 0, nullptr, nullptr);

                                InsercaoEstacao inserEstRota0;
                                InsercaoEstacao inserEstRota1;

                                bool novaRota0Viavel = distNovaRota0 > 0.0;
                                bool novaRota1Viavel = distNovaRota1 > 0.0;

                                bool rota0Viabilizada = false;
                                bool rota1Viabilizada = false;

                                // Verifica se as duas novas rotas sao inviaveis
                                if(!novaRota0Viavel && !novaRota1Viavel)
                                {

                                    rota0Viabilizada = viabilizaRotaEv(evRouteAux0, instancia, true, inserEstRota0,
                                                                       -1.0, false, tempoSaidaSat, nullptr);
                                    if(rota0Viabilizada)
                                    {
                                        novaDist = distNovaRota1+inserEstRota0.distanciaRota;

                                        // Verificar se a viabizacao ainda pode ser menor que a dist original
                                        if(menor(novaDist, distOrig))
                                        {
                                            /* Calcula o valor limite para a rota1 viabilizada
                                             *
                                             * distRota0        = 50
                                             * distRota1        = 50
                                             * distOrig         = 100
                                             * **********************
                                             * novaRota0Prev    = 40
                                             * novaRota1Prev    = 40
                                             *                  = novaRota0Prev + novaRota1Prev
                                             * distPrev         = 80
                                             *
                                             *                  = distOrig    distPrev
                                             * melhora          = 100       - 80
                                             *                  = 20
                                             *
                                             * novaRota0        = 45
                                             * distPrev         = novaRota0 + novaRota1Prev
                                             *                  = 85
                                             *
                                             * novaRota0 + novaRota1 < rotaOrig
                                             * novaRota1 < (distOrig - novaRota0)
                                             * novaRota1 < (100 - 45)
                                             * novaRota1 < 55
                                             */

                                            const double distLimRota1 = distOrig-inserEstRota0.distanciaRota;
                                            rota1Viabilizada = viabilizaRotaEv(evRouteAux1, instancia, false,
                                                                               inserEstRota1, distLimRota1, false,
                                                                               tempoSaidaSat, nullptr);
                                            if(!rota1Viabilizada)
                                                return false;       // Nao existem outras opcoes

                                            novaDist = inserEstRota0.distanciaRota+inserEstRota1.distanciaRota;

                                        }
                                        else
                                            return false;           // Nao existem outras opcoes


                                    }
                                    else
                                        return false;   // Nao existem outras opcoes

                                }
                                // Verifica se uma das rotas eh inviavel
                                else if(!novaRota0Viavel || !novaRota1Viavel)
                                {

                                    if(!novaRota0Viavel)
                                    {
                                        rota0Viabilizada = viabilizaRotaEv(evRouteAux0, instancia, false, inserEstRota0,
                                                                           distOrig - distNovaRota1, false,
                                                                           tempoSaidaSat, nullptr);
                                        if(!rota0Viabilizada)
                                            return false;

                                        novaDist = distNovaRota1 + inserEstRota0.distanciaRota;
                                    }
                                    else
                                    {
                                        rota1Viabilizada = viabilizaRotaEv(evRouteAux1, instancia, false, inserEstRota1,
                                                                           distOrig - distNovaRota0, false,
                                                                           tempoSaidaSat, nullptr);
                                        if(!rota1Viabilizada)
                                            return false;

                                        novaDist = distNovaRota0 + inserEstRota1.distanciaRota;
                                    }
                                }


                                if(!menor(novaDist, distOrig))
                                    return false;


                                if(novaRota0Viavel)
                                {
                                    distNovaRota0 = testaRota(evRouteAux0, evRouteAux0.routeSize, instancia, true,
                                                              tempoSaidaSat, 0, nullptr, nullptr);
                                    evRouteAux0.distancia = distNovaRota0;
                                    if(distNovaRota0 <= 0.0)
                                    {
                                        PRINT_DEBUG("", "ROTA JA ERA VIAVEL, ...");
                                        return false;
                                    }

                                    evRoute0.copia(evRouteAux0, true, &instancia);
                                    evRoute0.atualizaParametrosRota(instancia);

                                }
                                else if(rota0Viabilizada)
                                {
                                    insereEstacaoRota(evRouteAux0, inserEstRota0, instancia, tempoSaidaSat);
                                    evRoute0.copia(evRouteAux0, true, &instancia);
                                    evRoute0.atualizaParametrosRota(instancia);
                                }
                                else
                                {
                                    PRINT_DEBUG("", "ERRO ROTA DEVERIA SER VIAVEL NESSE PONTO!");
                                    return false;
                                }



                                if(novaRota1Viavel)
                                {
                                    distNovaRota1 = testaRota(evRouteAux1, evRouteAux1.routeSize, instancia, true,
                                                              tempoSaidaSat, 0, nullptr, nullptr);
                                    evRouteAux1.distancia = distNovaRota1;
                                    if(distNovaRota1 <= 0.0)
                                    {
                                        PRINT_DEBUG("", "ROTA JA ERA VIAVEL, ...");
                                        return false;
                                    }

                                    evRoute1.copia(evRouteAux1, true, &instancia);
                                    evRoute1.atualizaParametrosRota(instancia);

                                }
                                else if(rota1Viabilizada)
                                {
                                    insereEstacaoRota(evRouteAux1, inserEstRota1, instancia, tempoSaidaSat);
                                    evRoute1.copia(evRouteAux1, true, &instancia);
                                    evRoute1.atualizaParametrosRota(instancia);
                                }
                                else
                                {
                                    PRINT_DEBUG("", "ERRO ROTA DEVERIA SER VIAVEL NESSE PONTO!");
                                    return false;
                                }

                                return true;

                            }
                            else
                                return false;

                        }; // Fim funcao realizaMv


                        const double distOrig = evRoute0.distancia + evRoute1.distancia;
                        const double demandaOrig = evRoute0.demanda + evRoute1.demanda;

                        bool resutado = realizaMv(instancia, evRoute0, posEv0, evRoute1, posEv1, evRouteAux0, evRouteAux1, evRoute0[0].tempoSaida);

                        //if(!resutado)
                        //    resutado = realizaMv(instancia, evRoute1, posEv1, evRoute0, posEv0, evRouteAux0, evRouteAux1, evRoute1[0].tempoSaida);

                        if(resutado)
                        {

                            double novaDist = evRoute0.distancia + evRoute1.distancia;
                            solucao.distancia += -distOrig + novaDist;
                            solucao.satelites[sat].distancia += -distOrig + novaDist;


#if UTILIZA_MAT_MV
                            solucao.rotaEvAtualizada(sat, ev0);
                            solucao.rotaEvAtualizada(sat, ev1);
#endif

                            //cout<<"MV UPDATE\n";
                            //cout<<distOrig<<" : "<<novaDist<<"\n\n";

                            double novaDemanda = evRoute0.demanda + evRoute1.demanda;
                            if(novaDemanda != demandaOrig)
                            {
                                PRINT_DEBUG("", "");
                                cout << "ERRO! NOVA DEMANDA(" << novaDemanda << ") != DEMANDA ORIGINAL(" << demandaOrig<< ")\n";
                                throw "ERRO";
                            }

                            return true;
                        }

                    } // End for(posEv1)

                } // End for(posEv0)

            } // End for(ev1)

#if UTILIZA_MAT_MV
            solucao.vetMatSatEvMv[sat](ev0, mv) = 1;
#endif
        } // End for(ev0)

    } // End for(sat)

    return false;

}

bool NS_LocalSearch::mvEvSwapInterRotasInterSats(Solucao &solucao, Instancia &instancia, EvRoute &evRouteAux0, EvRoute &evRouteAux1, const float beta)
{

    const int mv = MV_EV_SWAP_INTER_ROTAS_INTER_SAT;

    if(instancia.numSats == 1 && instancia.numEv == 1)
        return false;


    static Solucao solucaoAux(instancia);
    solucaoAux.copia(solucao);

    for(int sat0 = instancia.getFirstSatIndex(); sat0 <= instancia.getEndSatIndex(); ++sat0)
    {

        for(int sat1 = instancia.getFirstSatIndex(); sat1 <= instancia.getEndSatIndex(); ++sat1)
        {
            if(sat0 == sat1)
                continue;

            for(int evSat0 = 0; evSat0 < instancia.getN_Evs(); ++evSat0)
            {
                std::reference_wrapper<EvRoute> refWEvRoute0 = solucaoAux.satelites[sat0].vetEvRoute[evSat0];

                if(refWEvRoute0.get().routeSize <= 2)
                    continue;

                for(int evSat1 = 0; evSat1 < instancia.getN_Evs(); ++evSat1)
                {

#if UTILIZA_MAT_MV
                    if(solucao.vetMatSatEvMv[sat0](evSat0, mv) == 1 && solucao.vetMatSatEvMv[sat1](evSat1, mv) == 1)
                        continue;
#endif

//cout<<"ev0: "<<evSat0<<"; ev1: "<<evSat1<<"\n";

                    std::reference_wrapper<EvRoute> refWEvRoute1 = solucaoAux.satelites[sat1].vetEvRoute[evSat1];

                    if((refWEvRoute1.get()).routeSize <= 2)
                        continue;
                    // 0 1 2  3  4  5
                    // 1 9 10 11 12 1

                    // tam = 6
                    //

                    // Selecionar as posicoes das rotas
                    for(int posEvSat0 = 0; posEvSat0 < (refWEvRoute0.get().routeSize - 2); ++posEvSat0)
                    {
                        if(instancia.isRechargingStation(refWEvRoute0.get()[posEvSat0 + 1].cliente))
                            continue;

                        for(int posSatEv1 = 0; posSatEv1 < (refWEvRoute1.get().routeSize - 2); ++posSatEv1)
                        {

                            if(instancia.isRechargingStation(refWEvRoute1.get()[posSatEv1 + 1].cliente))
                                continue;

                            /* ******************************************************************************************
                             * ******************************************************************************************
                             *  1º
                             *     O cliente na posicao (posEvSat0+1) do ev evSat0 ira para a posicao (posEvSat1+1)
                             *   evSat0 nao pode estar vazio,  cliente nao pode ser rs, verificar a nova carga de evSat1
                             *
                             *  2º
                             *     O cliente na posicao (posEvSat1+1) do ev evSat1 ira para a posicao (posEvSat0+1)
                             *   evSat1 nao pode estar vazio, e cliente nao pode ser rs, verificar a nova carga de evSat0
                             *
                             * ********************************************************************************************
                             * ********************************************************************************************/

                            auto realizaMv = [](Instancia &instancia, EvRoute &evRoute0, int posEvRoute0, EvRoute &evRoute1,
                                                int posEvRoute1, EvRoute &evRouteAux0, EvRoute &evRouteAux1)
                            {

                                const double tempoSaidaRoute0 = evRoute0[0].tempoSaida;
                                const double tempoSaidaRoute1 = evRoute1[0].tempoSaida;

/*PRINT_DEBUG("", "");

string strRota;
evRoute0.print(strRota, instancia, true);
cout<<"evRoute0: "<<strRota<<"\n";
cout<<"posEvRoute0: "<<posEvRoute0<<"\n\n";

strRota = "";
evRoute1.print(strRota, instancia, true);
cout<<"evRoute1: "<<strRota<<"\n";
cout<<"posEvRooute1: "<<posEvRoute1<<"\n\n";

cout<<"********************************************\n********************************************\n\n";*/


                                const int clienteRoute0 = evRoute0[posEvRoute0 + 1].cliente;
                                const int clienteRoute1 = evRoute1[posEvRoute1 + 1].cliente;

                                // Verifica carga de evRoute0
                                if((evRoute0.demanda - instancia.getDemand(clienteRoute0) +
                                    instancia.getDemand(clienteRoute1)) >
                                   instancia.vectVeiculo[evRoute1.idRota].capacidade)
                                    return false;

                                // Verifica carga de evRoute1
                                if((evRoute1.demanda - instancia.getDemand(clienteRoute1) +
                                    instancia.getDemand(clienteRoute0)) >
                                   instancia.vectVeiculo[evRoute0.idRota].capacidade)
                                    return false;


                                // Calcula distancia
                                const double distOrig = evRoute0.distancia + evRoute1.distancia;

                                // Calcula nova distancia
                                double distNovaRota0Prev = evRoute0.distancia;
                                double distNovaRota1Prev = evRoute1.distancia;

//cout<<"evRoute0[posEvRoute0+2].cliente: "<<evRoute0[posEvRoute0+2].cliente<<"\n";

                                // Retira clienteRoute0 e clienteRoute1
                                distNovaRota0Prev += -(
                                        instancia.getDistance(evRoute0[posEvRoute0].cliente, clienteRoute0) +
                                        instancia.getDistance(clienteRoute0, evRoute0[posEvRoute0 + 2].cliente));

                                distNovaRota1Prev += -(
                                        instancia.getDistance(evRoute1[posEvRoute1].cliente, clienteRoute1) +
                                        instancia.getDistance(clienteRoute1, evRoute1[posEvRoute1 + 2].cliente));

                                // Insere clienteRoute0 em evRoute1
                                distNovaRota1Prev +=
                                        instancia.getDistance(evRoute1[posEvRoute1].cliente, clienteRoute0) +
                                        instancia.getDistance(clienteRoute0, evRoute1[posEvRoute1 + 2].cliente);


                                // Insere clienteRoute1 em evRoute0
                                distNovaRota0Prev +=
                                        instancia.getDistance(evRoute0[posEvRoute0].cliente, clienteRoute1) +
                                        instancia.getDistance(clienteRoute1, evRoute0[posEvRoute0 + 2].cliente);


                                double novaDist = distNovaRota0Prev + distNovaRota1Prev;

                                // Verifica se novaDist < distOrig
                                if(menor(novaDist, distOrig))
                                {
//cout<<"novaDist("<<novaDist<<") < distOrig("<<distOrig<<")!\n";

                                    // Copia evRoute1 para evRouteAux1 e add clienteRoute0
                                    evRouteAux1.copia(evRoute1, true, &instancia);
                                    evRouteAux1[posEvRoute1 + 1].cliente = clienteRoute0;


                                    // Copia evRoute0 para evRouteAux0 e add clienteRoute1
                                    evRouteAux0.copia(evRoute0, true, &instancia);
                                    evRouteAux0[posEvRoute0 + 1].cliente = clienteRoute1;

/*
string strRota1;
evRoute1.print(strRota1, instancia, true);
cout<<"evRoute1: "<<strRota1<<"\n";

strRota1 = "";
evRouteAux1.print(strRota1, instancia, true);
cout<<"nova evRoute1: "<<strRota1<<"\n";
*/

                                    // Testa as novas rotas
                                    double distNovaRota1 = testaRota(evRouteAux1, evRouteAux1.routeSize, instancia,
                                                                     false, tempoSaidaRoute1, 0, nullptr, nullptr);
                                    double distNovaRota0 = testaRota(evRouteAux0, evRouteAux0.routeSize, instancia,
                                                                     false, tempoSaidaRoute0, 0, nullptr, nullptr);

                                    InsercaoEstacao inserEstRota0;
                                    InsercaoEstacao inserEstRota1;

                                    bool novaRota0Viavel = distNovaRota0 > 0.0;
                                    bool novaRota1Viavel = distNovaRota1 > 0.0;

                                    bool rota0Viabilizada = false;
                                    bool rota1Viabilizada = false;

                                    // Verifica se as duas novas rotas sao inviaveis
                                    if(!novaRota0Viavel && !novaRota1Viavel)
                                    {

                                        rota0Viabilizada = viabilizaRotaEv(evRouteAux0, instancia, true, inserEstRota0,
                                                                           -1.0, false, tempoSaidaRoute0, nullptr);
                                        if(rota0Viabilizada)
                                        {
                                            novaDist = distNovaRota1 + inserEstRota0.distanciaRota;

                                            // Verificar se a viabizacao ainda pode ser menor que a dist original
                                            if(menor(novaDist, distOrig))
                                            {
                                                /* Calcula o valor limite para a rota1 viabilizada
                                                 *
                                                 * distRota0        = 50
                                                 * distRota1        = 50
                                                 * distOrig         = 100
                                                 * **********************
                                                 * novaRota0Prev    = 40
                                                 * novaRota1Prev    = 40
                                                 *                  = novaRota0Prev + novaRota1Prev
                                                 * distPrev         = 80
                                                 *
                                                 *                  = distOrig    distPrev
                                                 * melhora          = 100       - 80
                                                 *                  = 20
                                                 *
                                                 * novaRota0        = 45
                                                 * distPrev         = novaRota0 + novaRota1Prev
                                                 *                  = 85
                                                 *
                                                 * novaRota0 + novaRota1 < rotaOrig
                                                 * novaRota1 < (distOrig - novaRota0)
                                                 * novaRota1 < (100 - 45)
                                                 * novaRota1 < 55
                                                 */

                                                const double distLimRota1 = distOrig - inserEstRota0.distanciaRota;
                                                rota1Viabilizada = viabilizaRotaEv(evRouteAux1, instancia, false,
                                                                                   inserEstRota1, distLimRota1, false,
                                                                                   tempoSaidaRoute1, nullptr);
                                                if(!rota1Viabilizada)
                                                    return false;       // Nao existem outras opcoes

                                                novaDist = inserEstRota0.distanciaRota + inserEstRota1.distanciaRota;

                                            } else
                                                return false;           // Nao existem outras opcoes


                                        } else
                                            return false;   // Nao existem outras opcoes

                                    }
                                        // Verifica se uma das rotas eh inviavel
                                    else if(!novaRota0Viavel || !novaRota1Viavel)
                                    {

                                        if(!novaRota0Viavel)
                                        {
                                            rota0Viabilizada = viabilizaRotaEv(evRouteAux0, instancia, false,
                                                                               inserEstRota0, distOrig - distNovaRota1,
                                                                               false, tempoSaidaRoute0, nullptr);
                                            if(!rota0Viabilizada)
                                                return false;

                                            novaDist = distNovaRota1 + inserEstRota0.distanciaRota;
                                        } else
                                        {
                                            rota1Viabilizada = viabilizaRotaEv(evRouteAux1, instancia, false,
                                                                               inserEstRota1, distOrig - distNovaRota0,
                                                                               false, tempoSaidaRoute1, nullptr);
                                            if(!rota1Viabilizada)
                                                return false;

                                            novaDist = distNovaRota0 + inserEstRota1.distanciaRota;
                                        }
                                    }


                                    if(!menor(novaDist, distOrig))
                                        return false;


                                    if(novaRota0Viavel)
                                    {
                                        distNovaRota0 = testaRota(evRouteAux0, evRouteAux0.routeSize, instancia, true,
                                                                  tempoSaidaRoute0, 0, nullptr, nullptr);
                                        evRouteAux0.distancia = distNovaRota0;
                                        if(distNovaRota0 <= 0.0)
                                        {
                                            PRINT_DEBUG("", "ROTA JA ERA VIAVEL, ...");
                                            return false;
                                        }

                                        evRoute0.copia(evRouteAux0, true, &instancia);
                                        evRoute0.atualizaParametrosRota(instancia);

                                    } else if(rota0Viabilizada)
                                    {
                                        insereEstacaoRota(evRouteAux0, inserEstRota0, instancia, tempoSaidaRoute0);
                                        evRoute0.copia(evRouteAux0, true, &instancia);
                                        evRoute0.atualizaParametrosRota(instancia);
                                    } else
                                    {
                                        PRINT_DEBUG("", "ERRO ROTA DEVERIA SER VIAVEL NESSE PONTO!");
                                        return false;
                                    }


                                    if(novaRota1Viavel)
                                    {
                                        distNovaRota1 = testaRota(evRouteAux1, evRouteAux1.routeSize, instancia, true,
                                                                  tempoSaidaRoute1, 0, nullptr, nullptr);
                                        evRouteAux1.distancia = distNovaRota1;
                                        if(distNovaRota1 <= 0.0)
                                        {
                                            PRINT_DEBUG("", "ROTA JA ERA VIAVEL, ...");
                                            return false;
                                        }

                                        evRoute1.copia(evRouteAux1, true, &instancia);
                                        evRoute1.atualizaParametrosRota(instancia);

                                    } else if(rota1Viabilizada)
                                    {
                                        insereEstacaoRota(evRouteAux1, inserEstRota1, instancia, tempoSaidaRoute1);
                                        evRoute1.copia(evRouteAux1, true, &instancia);
                                        evRoute1.atualizaParametrosRota(instancia);
                                    }
                                    else
                                    {
                                        PRINT_DEBUG("", "ERRO ROTA DEVERIA SER VIAVEL NESSE PONTO!");
                                        return false;
                                    }

                                    return true;

                                } else
                                    return false;

                            }; // Fim funcao realizaMv


                            const double distOrig = refWEvRoute0.get().distancia + refWEvRoute1.get().distancia;
                            const double demandaOrig = refWEvRoute0.get().demanda + refWEvRoute1.get().demanda;
                            bool resutado = realizaMv(instancia, refWEvRoute0, posEvSat0, refWEvRoute1.get(), posSatEv1, evRouteAux0, evRouteAux1);

                            if(resutado)
                            {

                                double novaDist = refWEvRoute0.get().distancia + refWEvRoute1.get().distancia;
                                bool resetaSolucaE_rotas = false;

                                if(menor(novaDist, distOrig))
                                {
                                    solucaoAux.recalculaDistSat(instancia);
                                    solucaoAux.resetaPrimeiroNivel(instancia);
                                    construtivoPrimeiroNivel(solucaoAux, instancia, beta, false, Split);

                                    if(solucaoAux.viavel)
                                    {

                                        if(menor(solucaoAux.distancia, solucao.distancia))
                                        {
                                            //cout<<"ATUALIZACAO: "<<solucaoAux.distancia<<" "<<solucao.distancia<<"\n";
/*                                            const double dist1NivelNovo = solucaoAux.getDist1Nivel();
                                            const double dist1NivelOrig = solucao.getDist1Nivel();

                                            if(menor(dist1NivelNovo, dist1NivelOrig))
                                            {
                                                cout<<dist1NivelNovo<<" : "<<dist1NivelOrig<<"\n";
                                            }*/

                                            solucao.copia(solucaoAux);

#if UTILIZA_MAT_MV
                                            solucao.rotaEvAtualizada(sat0, evSat0);
                                            solucao.rotaEvAtualizada(sat1, evSat1);
#endif
                                            return true;
                                        }
                                        else
                                            resetaSolucaE_rotas = true;

                                    }
                                    else
                                        resetaSolucaE_rotas = true;

                                }
                                else
                                    resetaSolucaE_rotas = true;


                                if(resetaSolucaE_rotas)
                                {

                                    // CORRIGIR

                                    solucaoAux.copia(solucao);
                                    refWEvRoute1 = solucaoAux.satelites[sat1].vetEvRoute[evSat1];
                                    refWEvRoute0 = solucaoAux.satelites[sat0].vetEvRoute[evSat0];
                                }
                            }

                        } // End for(posEv1)

                    } // End for(posEv0)

                } // End for(evSat1)


#if UTILIZA_MAT_MV
                solucao.vetMatSatEvMv[sat0](evSat0, mv) = 1;
#endif
            } // End for(evSat0)



        } // End for(sat1)


    } // End for(sat0)

    return false;
}



/**
 *
 * @param solucao
 * @param sat0Temp
 * @param novaCargaSat0Temp         Incremento de carga no sat0
 * @param sat1Temp
 * @param novaCargaSat1Temp         Incremento de carga no sat0
 * @return
 */
bool NS_LocalSearch::conserta1Nivel(Solucao &solucao, const int sat0Temp, const double novaCargaSat0Temp, const int sat1Temp, const double novaCargaSat1Temp)
{

    if(sat1Temp == sat0Temp)
    {
        PRINT_DEBUG("", "ERRO, SAT0("<<sat0Temp<<") == SAT1("<<sat1Temp<<")");
        return false;
    }

    /* ********************************************************************
     * ********************************************************************
     *
     *  sat0 tem incremento positivo, sat1 tem incremento negativo
     *
     * ********************************************************************
     * ********************************************************************/

     int sat0, sat1;
     sat0=sat1=-1;

     double novaCargaSat0, novaCargaSat1;
     novaCargaSat0=novaCargaSat1=0.0;

     if(novaCargaSat0Temp > 0.0)
     {
         sat0 = sat0Temp;
         sat1 = sat1Temp;

         novaCargaSat0 = novaCargaSat0Temp;
         novaCargaSat1 = novaCargaSat1Temp;
     }
     else
     {
         sat0 = sat1Temp;
         sat1 = sat0Temp;

         novaCargaSat0 = novaCargaSat1Temp;
         novaCargaSat1 = novaCargaSat0Temp;
     }

}