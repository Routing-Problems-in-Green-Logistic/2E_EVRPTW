/* ****************************************
 * ****************************************
 *  Nome: ***********************
 *  Data:    22/02/23
 *  Arquivo: Construtivo3.cpp
 * ****************************************
 * ****************************************/

#include "Construtivo4.h"
#include "../Auxiliary.h"
#include <set>
#include <cfloat>
#include <chrono>
#include "../mersenne-twister.h"
#include "../ViabilizadorRotaEv.h"

using namespace NS_Construtivo4;
using namespace std;
using namespace NS_Auxiliary;
using namespace NS_viabRotaEv;


#define PRINT_DEBUG_CONST FALSE

typedef std::list<CandidatoEV>::iterator ItListCand;

// Roteamento dos veiculos eletricos
bool NS_Construtivo4::construtivoSegundoNivelEV(Solucao &sol, Instancia &instance, const float alfaSeg,
                                                const Matrix<int> &matClienteSat, bool listaRestTam,
                                                const Vector<int> &satUtilizados, bool print,
                                                Vector<int> &vetInviabilidade, const bool torneio)
{
#if PRINT_DEBUG_CONST
    //print = true;
    cout<<"**********************************************INICIO CONSTRUTIVO4**********************************************\n\n";
    cout<<"NUM EVs: "<<sol.numEv<<"\n";
    //cout<<"TEMPO SAIDA SAT 2: "<<instance.vetTempoSaida[2]<<"\n";
#endif

    Vector<int8_t> &visitedClients = sol.vetClientesAtend;

    const int FistIdClient  = instance.getFirstClientIndex();
    const int LastIdClient  = instance.getEndClientIndex();
    const auto ItEnd        = visitedClients.begin() + instance.getNSats() + instance.getNClients();
    const Vector<double> &vetTempoSaida = instance.vetTempoSaida;
    EvRoute evRouteAux(-1, -1, instance.getEvRouteSizeMax(), instance);

    std::list<CandidatoEV> listaCandidatos;
    std::list<int> clientesSemCandidato;

    const int numLinhasMat      = instance.getN_Evs();
    const int numColMat         = instance.getNClients();
    const int idPrimeiroCliente = instance.getFirstClientIndex();
    const int idPrimeiroEv      = instance.getFirstEvIndex();
    auto transformaIdCliente  = [&](const int id){return (id - idPrimeiroCliente);};
    auto transformaIdEv       = [&](const int id){return (id-idPrimeiroEv);};


    // POSSUI SOMENTE O NUMERO DE CLIENTES! UTILIZAR transformaIdCliente
    //Vector<std::list<ItListCand>> vetCliItListCand(instance.getNClients(), std::list<ItListCand>());

    //COLUNAS DA MATRIZ POSSUEM SOMENTE A QUANTIDADE DE CLIENTES!!  UTILIZAR transformaIdCliente
    //static Vector<Matrix<ItListCand>> matCandidato(1 + instance.getNSats());
    //std::fill(matCandidato.begin(), matCandidato.end(), Matrix<ItListCand>(numLinhasMat, numColMat, listaCandidatos.end()));

    auto criaListaCandidatosP_Cliente = [&](const int cliente, const bool numEvMax) -> std::list<CandidatoEV>
    {
        if(visitedClients[cliente] == int8_t(1))
            return {};


#if PRINT_DEBUG_CONST
cout<<"INICIO criaListaCandidatosP_Cliente CLIENTE: "<<cliente<<"\n\n";
#endif

        // Guarda as todas insercoes para de cliente para cada rota
        std::list<CandidatoEV> listaCandCliente;

        const double cargaCliente = instance.getDemand(cliente);

        for(int satId = instance.getFirstSatIndex(); satId <= instance.getEndSatIndex(); ++satId)
        {
            if(matClienteSat(cliente, (satId)) != 1 || satUtilizados[satId] == 0)
                continue;

            const double tempoSaidaSat = instance.vetTempoSaida[satId];

            Satelite *sat = sol.getSatelite(satId);
            bool rotaVazia = false;
            for(int routeId = 0; routeId < instance.getN_Evs(); ++routeId)
            {
                EvRoute &evRoute = sat->getRoute(routeId);

                if((evRoute.demanda+cargaCliente) > instance.getEvCap(evRoute.idRota))
                {
                    vetInviabilidade[Inv_Carga] += 1;
                    continue;
                }

                if(evRoute.routeSize <= 2)
                {
                    if(numEvMax)
                        rotaVazia = true;

                    if(rotaVazia)
                        continue;
                    else
                        rotaVazia = true;
                }


                //copiaVector(evRoute.route, evRouteAux.route, evRoute.routeSize);
                evRouteAux.copia(evRoute, false, nullptr);
                shiftVectorDir(evRouteAux.route, 1, 1, evRoute.routeSize);
                evRouteAux.routeSize = evRoute.routeSize+1;
                evRouteAux[0].bateriaRestante = instance.getEvBattery(evRoute.idRota);
                evRouteAux[0].tempoSaida = tempoSaidaSat;
                evRouteAux.idRota = evRoute.idRota;

                for(int k=0; k < (evRoute.routeSize-1); ++k)
                {
                    evRouteAux[k+1].cliente = cliente;


#if PRINT_DEBUG_CONST
    string strRota;
    evRouteAux.print(strRota, instance, true);
    cout<<"\t"<<strRota<<"\n";
#endif

                    double custo = testaRota(evRouteAux, evRouteAux.routeSize, instance, false, tempoSaidaSat, 0,
                                             nullptr, &vetInviabilidade);

                    if(custo > 0.0)
                    {

                        const double inc = custo-evRoute.distancia;
                        if(inc < -0.1)
                        {
                            PRINT_DEBUG("", "");
                            std::cout<<"ERRO, INCREMENTO: "<<inc<<" NAO DEVERIA SER NEGATIVO\n";
                            std::string strRota;
                            evRoute.print(strRota, instance, true);
                            std::cout<<"ROTA ORIGINAL: "<<strRota;
                            strRota = "";
                            evRouteAux.print(strRota, instance, true);
                            std::cout<<"ROTA COM CLIENTE("<<cliente<<"): "<<strRota<<"\n";
                            std::cout<<"CUSTO: "<<custo<<"\n";
                            throw "ERRO";
                        }

                        CandidatoEV candidatoEvAux;
                        candidatoEvAux.satId = satId;
                        candidatoEvAux.routeId = routeId;
                        candidatoEvAux.clientId = cliente;
                        candidatoEvAux.incremento = inc;
                        candidatoEvAux.penalidade = 0.0;
                        candidatoEvAux.atualizaPenalidade();
                        candidatoEvAux.pos = k;
                        candidatoEvAux.demand = cargaCliente;
                        listaCandCliente.push_back(candidatoEvAux);

                    }
                    else
                    {
                        InsercaoEstacao insercaoEstacao;
                        if(viabilizaRotaEv(evRouteAux, instance, false, insercaoEstacao, DOUBLE_MAX,
                                           true, tempoSaidaSat, &vetInviabilidade))
                        {
                            const double inc = insercaoEstacao.distanciaRota-evRoute.distancia;
                            if(inc < -0.1)
                            {
                                PRINT_DEBUG("", "");
                                std::cout<<"ERRO, INCREMENTO: "<<inc<<" NAO DEVERIA SER NEGATIVO\n";
                                std::string strRota;
                                evRoute.print(strRota, instance, true);
                                std::cout<<"ROTA ORIGINAL: "<<strRota;
                                strRota = "";
                                evRouteAux.print(strRota, instance, true);
                                std::cout<<"ROTA COM CLIENTE("<<cliente<<"): "<<strRota<<"\n";
                                throw "ERRO";
                            }


                            CandidatoEV candidatoEvAux;
                            candidatoEvAux.satId = satId;
                            candidatoEvAux.routeId = routeId;
                            candidatoEvAux.clientId = cliente;
                            candidatoEvAux.incremento = inc;
                            candidatoEvAux.penalidade = 0.0;
                            candidatoEvAux.atualizaPenalidade();
                            candidatoEvAux.pos = k;
                            candidatoEvAux.demand = cargaCliente;
                            candidatoEvAux.insercaoEstacao = insercaoEstacao;
                            listaCandCliente.push_back(candidatoEvAux);
                        }
                    }

                    if((k+1) < (evRoute.routeSize-1))
                        evRouteAux[k+1].cliente = evRouteAux[k+2].cliente;
                }
                //cout<<"\n\n";
            }
        }


#if PRINT_DEBUG_CONST
cout<<"FIM criaListaCandidatosP_Cliente\n\n";
#endif

        return listaCandCliente;
    };


    // Cria um candidato para cada cliente e armazena em matCandidato

    /*
    for(int clientId = FistIdClient; clientId <= LastIdClient; ++clientId)
    {
        if(visitedClients[clientId] == int8_t(1))
            continue;

        if(!instance.isClient(clientId))
        {
            cout<<"ERRO, "<<clientId<<" NAO EH CLIENTE\n";
            throw "ERRO";
        }

        auto listaCandCliente = criaListaCandidatosP_Cliente(clientId, (sol.numEv >= sol.numEvMax));

        if(!listaCandCliente.empty())
        {

            listaCandidatos.splice(listaCandidatos.end(), listaCandCliente);

#if PRINT_DEBUG_CONST
string strRota;
Satelite *sat = sol.getSatelite(candPtr->satId);
EvRoute &evRoute = sat->getRoute(candPtr->routeId);
evRoute.print(strRota, instance, true);
cout<<"CLIENTE("<<clientId<<") ROTA ESCOLHIDA: "<<strRota<<"; pos: "<<candPtr->pos<<"; posIt: "<<itCandEscolhido->pos<<"\n\n";
#endif
        } else
            clientesSemCandidato.push_back(clientId);
    }
    */

#if PRINT_DEBUG_CONST
cout<<"\nNUMERO DE CANDIDATOS: "<<listaCandidatos.size()<<"\n\n";
#endif

    bool numEvMaxAcionado = false;
    bool printTam = true;
    bool primeiraChamada = true;

    while(!visitAllClientes(visitedClients, instance))
    {
        //Vector<int8_t> clientesVisitadosAux = Vector(visitedClients);
        listaCandidatos = std::list<CandidatoEV>();
        //int clienteCand = instance.getFirstClientIndex() + (rand_u32()%instance.numClients);

/*        do
        {

            const int priCliente = clienteCand;
            do
            {
                clienteCand = instance.getFirstClientIndex() + ((clienteCand + 1) % instance.numClients);
            } while(clienteCand != priCliente && clientesVisitadosAux[clienteCand] == int8_t(1));

            if(clientesVisitadosAux[clienteCand] == int8_t(1))
                break;

            listaCandidatos = criaListaCandidatosP_Cliente(clienteCand, sol.numEv >= sol.numEvMax);
            clientesVisitadosAux[clienteCand] = int8_t(1);

        }
        while(listaCandidatos.empty());*/


        for(int i = instance.getFirstClientIndex(); i <= instance.getEndClientIndex(); ++i)
        {
            if(visitedClients[i] == int8_t(0))
            {
                listaCandidatos.splice(listaCandidatos.end(),
                                       criaListaCandidatosP_Cliente(i, (sol.numEv >= sol.numEvMax)));
            }
        }


        if(listaCandidatos.empty())
        {
            //std::cout<<"LISTA VAZIA\n";
            break;
        }

        if(primeiraChamada)
        {
            VarAuxiliaresIgNs::sumQuantCand += listaCandidatos.size();
            VarAuxiliaresIgNs::num_sumQuantCand += 1;

            primeiraChamada = false;
        }

        /*
        if(primeiraChamada)
        {
            primeiraChamada = false;
            const int tam = listaCandidatos.size();

            if(tam <= 10)
                alpha = 0.9;
            else if(tam <= 40)
                alpha = 0.85;
            else if(tam <= 80)
                alpha = 0.80;
            else if(tam <= 100)
                alpha = 0.75;
            else if(tam <= 140)
                alpha = 0.7;
            else if(tam <= 180)
                alpha = 0.65;
            else if(tam <= 200)
                alpha = 0.6;
            else// if(tam <= 240)
                alpha = 0.55;


        }
        */

        listaCandidatos.sort();

/*        if(printTam)
        {
            cout<<"\t\tTAM LISTA DE CANDIDATOS: "<<listaCandidatos.size()<<"\n";
            printTam = false;
        }*/

#if PRINT_DEBUG_CONST
        cout<<"LISTA DE CANDIDATOS: \n";
        for(const auto& it:listaCandidatos)
            cout<<"("<<it.clientId<<", "<<it.routeId<<", "<<it.pos<<") ";
        cout<<"\n\n";


        cout<<"NUMERO DE CANDIDATOS: "<<listaCandidatos.size()<<"\n\n";

        /*cout<<"LISTA DE CANDIDATOS COM EV VAZIO: \n";
        for(auto it:listaCandidatos)
        {
            Satelite *sat = sol.getSatelite(it.satId);
            EvRoute &evRoute = sat->getRoute(it.routeId);

            if(evRoute.routeSize <= 2)
                cout << it.clientId << ", ";
        }*/
        cout<<"\n\n";
#endif


        const int size = max(int(alfaSeg * listaCandidatos.size()), 1);
        auto topItem = listaCandidatos.begin();

        if(torneio)
        {

            auto funcGetIncremento = [&](const int i) -> double
            {
                auto it = std::next(listaCandidatos.begin(), i);
                return it->incremento;
            };

            auto funcGetItCand = [&](auto itI, auto itJ, auto itK)
            {
                const auto itI_Primeiro = itI;
                const int clienteJ = itJ->clientId;
                const int clienteK = itK->clientId;

                if(!(itI->clientId == clienteJ || itI->clientId == clienteK))
                    return itI;

                do
                {

                    ++itI;
                    if(itI == listaCandidatos.end())
                        itI = listaCandidatos.begin();

                } while((itI->clientId == clienteJ || itI->clientId == clienteK) && itI != itI_Primeiro);

                return itI;

            };


            const int escolhido0 = rand_u32() % size;
            const int escolhido1 = rand_u32() % size;
            const int escolhido2 = rand_u32() % size;

            auto it0 = std::next(listaCandidatos.begin(), escolhido0);
            auto it1 = std::next(listaCandidatos.begin(), escolhido1);
            auto it2 = std::next(listaCandidatos.begin(), escolhido2);

            it1 = funcGetItCand(it1, it0, it2);
            it2 = funcGetItCand(it2, it0, it1);

            topItem = it0;

            const double val0 = it0->incremento;
            const double val1 = it1->incremento;
            const double val2 = it2->incremento;

            if(val1 < val0 && val1 < val2)
                topItem = it1;
            else if(val2 < val0 && val2 < val1)
                topItem = it2;


        }
        else
        {
            int randIndex = rand_u32() % size;
            topItem = std::next(listaCandidatos.begin(), randIndex);
        }
#if PRINT_DEBUG_CONST
cout<<"\tESCOLHIDO: "<<topItem->clientId<<";"<<topItem->incrP<<"\n\n";
#endif

       // CandidatoEV *candEvPtr = &(*topItem);

        if(visitedClients[topItem->clientId] == Int8(1))
        {
            PRINT_DEBUG("", "");
            cout<<"ERRO, CLIENTE: "<<topItem->clientId<<" JA ESTA NA SOLUCAO\n";
            throw "ERRO";
        }

        visitedClients[topItem->clientId] = int8_t(1);
        sol.vetClientesAtend[topItem->clientId] = int8_t(1);
        Satelite *satelite = sol.getSatelite(topItem->satId);
        satelite->demanda += topItem->demand;
        EvRoute &evRoute = satelite->getRoute(topItem->routeId);

        const bool evRouteCandVazio = (evRoute.routeSize <= 2);

/*#if PRINT_DEBUG_CONST
string strRota;
evRoute.print(strRota, instance, true);
cout<<"\tROTA: "<<strRota<<"\n\n";
#endif*/

        bool resultadoTopItem = insert(evRoute, *topItem, instance, vetTempoSaida[topItem->satId], sol);
        sol.rotaEvAtualizada(topItem->satId, topItem->routeId);

        if(!resultadoTopItem)
        {
            PRINT_DEBUG("", "ERRO, INSERT RETORNOU FALSE!!!\n\n");
            throw "ERRO";
        }


        //const bool numEvMax = sol.numEv >= sol.numEvMax;

        // Remove candidatos com ev vazio

        /*
        if(numEvMax && !numEvMaxAcionado)
        {
            numEvMaxAcionado = true;
            for(ItListCand it = listaCandidatos.begin(); it != listaCandidatos.end(); )
            {
                bool incIt = true;
                if(it->clientId != topItem->clientId)
                {
//cout<<"satId: "<<it->satId<<"\n";
//cout<<"clienteId: "<<it->clientId<<"\n\n";

                    Satelite &sateliteTemp = sol.satelites[it->satId];
                    EvRoute &evRouteTemp   = sateliteTemp.vetEvRoute[it->routeId];

                    if(evRouteTemp.routeSize <= 2)
                    {
                        //matCandidato[it->satId](it->routeId, transformaIdCliente(it->clientId)) = listaCandidatos.end();
                        it = listaCandidatos.erase(it);
                        incIt = false;
                        //cout<<"ERASE CANDIDATO COM EV VAZIO!\n";
                    }
                }

                if(incIt)
                    ++it;
            }
        }
        */

        // Corrigi a lista de candidatos
        // 1º Os candidatos que estao no mesmo satelite e na mesma rota precisao ser reavaliados
        // 2º Os clientes que nao possuem candidato so tem que ser avaliados na rota que houve mudanca
        // 3º Os candidatos que nao estao na mesma rota nao sao reavaliados


/*        Satelite *sat = sol.getSatelite(candEvPtr->satId);
        EvRoute &evRouteEsc = sat->vetEvRoute[candEvPtr->routeId];
        const bool numEVsMax = (sol.numEv >= sol.numEvMax);*/



        const bool clientesAntendidos = visitAllClientes(visitedClients, instance);

        /*
        if(!clientesAntendidos)
        {
            const int evIdCand  = candEvPtr->routeId;
            const int satIdCand = candEvPtr->satId;
            const int cliIdCand = candEvPtr->clientId;

            for(int i=instance.getFirstClientIndex(); i <= instance.getEndClientIndex(); ++i)
            {

                if(visitedClients[i] == int8_t(1) || i == cliIdCand)
                    continue;

                //ItListCand candItAux = matCandidato[satIdCand](evIdCand, transformaIdCliente(i));
                //if(candItAux == listaCandidatos.end())
                //    continue;

                // Gera a melhor insersao para i na nova rota
                EvRoute &evRouteCand = sol.satelites[satIdCand].vetEvRoute[evIdCand];
                (*candItAux) = CandidatoEV(evIdCand);

                canInsert(evRouteCand, i, instance, *candItAux, satIdCand,
                          vetTempoSaida[satIdCand], evRouteAux, vetInviabilidade);

                if(candItAux->pos != -1)
                    candItAux->routeId = evIdCand;
                else
                {
                    //vetCliItListCand[transformaIdCliente(i)].remove(candItAux);
                    listaCandidatos.erase(candItAux);
                    matCandidato[satIdCand](evIdCand, transformaIdCliente(i)) = listaCandidatos.end();
                }

                // Caso a rota antiga era vazia, um novo candidato em uma rota vazia eh criado
                if(evRouteCandVazio && !numEvMax)
                {

                    // Encontra uma rota vazia
                    int idRotaVazia = -1;
                    for(int ev=0; ev < instance.numEv; ++ev)
                    {
                        EvRoute &evRoute1 = sat->vetEvRoute[ev];
                        if(evRoute1.routeSize <= 2)
                        {
                            idRotaVazia = ev;
                            break;
                        }
                    }

                    if(idRotaVazia == -1)
                        continue;

                    EvRoute &evRouteVazia = sat->vetEvRoute[idRotaVazia];
                    CandidatoEV candidatoEv(idRotaVazia);
                    canInsert(evRouteVazia, i, instance, candidatoEv, satIdCand,
                              vetTempoSaida[satIdCand], evRouteAux, vetInviabilidade);

                    if(candidatoEv.pos != -1)
                    {
                        candidatoEv.routeId = idRotaVazia;
                        listaCandidatos.push_front(candidatoEv);
                        ItListCand it = listaCandidatos.begin();
                        matCandidato[satIdCand](idRotaVazia, transformaIdCliente(i)) = it;
                        //vetCliItListCand[transformaIdCliente(i)].push_front(it);
                    }
                }
            }
        }
        */


        const int cliente = topItem->clientId;

        /*
        {
            ItListCand it0 = listaCandidatos.begin();
            ItListCand it1 = it0;
            ++it1;

            if(it1 != listaCandidatos.end())
            {
                while(it0 != listaCandidatos.end())
                {
                    if(it0->clientId == cliente)
                    {
                        it0 = listaCandidatos.erase(it0);
                        ++it1;
                    }
                    else
                    {
                        ++it0;
                        ++it1;
                    }
                }
            }
            else
            {
                if(it0->clientId == cliente)
                    listaCandidatos.erase(it0);
            }
        }
        */
        //vetCliItListCand[transformaIdCliente(cliente)] = std::list<ItListCand>();

        if(clientesAntendidos)
        {
            break;
        }


    } // End while(!visitAllClientes)


    sol.viavel = visitAllClientes(visitedClients, instance);
    if(!instance.nivel2Viavel && sol.viavel)
        instance.nivel2Viavel = true;


#if PRINT_DEBUG_CONST
    //print = true;
    cout<<"**********************************************FIM CONSTRUTIVO4**********************************************\n\n";

#endif
    return sol.viavel;
}


bool NS_Construtivo4::visitAllClientes(Vector<int8_t> &visitedClients, const Instancia &instance)
{

    int i=instance.getFirstClientIndex();

    for(; i <= instance.getEndClientIndex(); ++i)
    {

        if(visitedClients.at(i) != int8_t(1))
            return false;
    }

    return true;

}

void
NS_Construtivo4::construtivoPrimeiroNivel(Solucao &sol, Instancia &instance, const float betaPrim, bool listaRestTam, const bool split)
{
    sol.reseta1Nivel(instance);
    //cout<<"*******************************INICIO CONSTRUTIVO3 1º NIVEL*******************************\n\n";
    // Cria o vetor com a demanda de cada satellite
    Vector<double> demandaNaoAtendidaSat;
    demandaNaoAtendidaSat.reserve(sol.getNSatelites()+1);
    int satId = 1;
    demandaNaoAtendidaSat.push_back(0.0);


/*    {

        Route &route = sol.primeiroNivel[0];
        string strRota;
        std::cout<<"ROUTE 0: ";
        route.print(strRota);
    }*/

    for(int sat=instance.getFirstSatIndex(); sat <= instance.getEndSatIndex(); ++sat)
    {
        demandaNaoAtendidaSat.push_back(sol.satelites.at(sat).demanda);
    }

    const int NumSatMaisDep = sol.getNSatelites()+1;


    // Enquanto existir um satellite com demanda > 0, continua
    while(existeDemandaNaoAtendida(demandaNaoAtendidaSat))
    {
        // Cria a lista de candidatos
        std::list<CandidatoVeicComb> listaCandidatos;

        // Percorre os satellites
        for(int i=1; i < NumSatMaisDep; ++i)
        {
            Satelite &satelite = sol.satelites.at(i);
/*            if(i==1)
            {

                std::cout<<"1º: DEM NAO ATEND(1): "<<demandaNaoAtendidaSat[1]<<"\n\n";
            }*/
            // Verifica se a demanda não atendida eh positiva
            if(demandaNaoAtendidaSat.at(i) > 0.0)
            {
                // Percorre todas as rotas
                for(int rotaId = 0; rotaId < sol.primeiroNivel.size(); ++rotaId)
                {
                    Route &route = sol.primeiroNivel.at(rotaId);

                    // Verifica se veiculo esta 100% da capacidade
                    if(route.totalDemand < instance.getTruckCap(rotaId))
                    {
                        // Calcula a capacidade restante do veiculo
                        double capacidade = instance.getTruckCap(rotaId) - route.totalDemand;
                        double demandaAtendida = capacidade;

                        if(demandaNaoAtendidaSat.at(i) < capacidade)
                            demandaAtendida = demandaNaoAtendidaSat.at(i);
                        else if(!split)
                            continue;

                        CandidatoVeicComb candidato(rotaId, i, demandaAtendida, DOUBLE_MAX);

                        // Percorre todas as posicoes da rota
                        for(int p=0; (p+1) < route.routeSize; ++p)
                        {
                            double incrementoDist = 0.0;

                            // Realiza a insercao do satellite entre as posicoes p e p+1 da rota
                            const RouteNo &clienteP =  route.rota[p];
                            const RouteNo &clientePP = route.rota[p+1];

                            // Calcula o incremento da distancia (Sempre positivo, desigualdade triangular)
                            incrementoDist -= instance.getDistance(clienteP.satellite, clientePP.satellite);
                            incrementoDist = incrementoDist + instance.getDistance(clienteP.satellite, i) + instance.getDistance(i, clientePP.satellite);

                            if(incrementoDist < candidato.incrementoDistancia)
                            {
                                // Calcula o tempo de chegada e verifica a janela de tempo
                                const double tempoChegCand = clienteP.tempoChegada + instance.getDistance(clienteP.satellite, i);

                                bool satViavel = true;

                                if(verificaViabilidadeSatelite(tempoChegCand, satelite, instance, false))
                                {
                                    double tempoChegTemp = tempoChegCand + instance.getDistance(i, clientePP.satellite);

                                    // Verificar viabilidade dos outros satelites
                                    for(int t=p+1; (t+1) < (route.routeSize); ++t)
                                    {
                                        Satelite &sateliteTemp = sol.satelites.at(route.rota.at(t).satellite);

                                        if(!verificaViabilidadeSatelite(tempoChegTemp, sateliteTemp, instance, false))
                                        {
                                            satViavel = false;
                                            break;
                                        }

                                        tempoChegTemp += instance.getDistance(route.rota.at(t).satellite, route.rota.at(t+1).satellite);
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

            listaCandidatos.sort();

            int sizeTemp = min(2, int(listaCandidatos.size()));
            if(instance.numSats > 2)
                sizeTemp = max(int(betaPrim * listaCandidatos.size()), 1);

            const int size = sizeTemp;

            int escolhido = rand_u32() % size;
            auto it = listaCandidatos.begin();

            std::advance(it, escolhido);
            CandidatoVeicComb &candidato = *it;

            // Insere candidato na solucao
            Route &route = sol.primeiroNivel.at(candidato.rotaId);
            // AQUI

/*            string strRota;
            route.print(strRota);
            std::cout<<"ROTA ID: "<<candidato.rotaId<<"\n";
            std::cout<<strRota<<"\n";
            std::cout<<"DEM SAT(1): "<<sol.satelites[1].demanda<<"\n";
            std::cout<<"DEM NAO ATEND(1): "<<demandaNaoAtendidaSat[1]<<"\n\n";*/

            shiftVectorDir(route.rota, candidato.pos + 1, 1, route.routeSize);
            route.rota.at(candidato.pos+1).satellite = candidato.satelliteId;
            route.rota.at(candidato.pos+1).tempoChegada = candidato.tempoSaida;
            route.routeSize += 1;
            double tempoSaida = candidato.tempoSaida;

            for(int i=candidato.pos+1; (i+1) < route.routeSize; ++i)
            {
                const int satTemp = route.rota.at(i).satellite;

                if(!verificaViabilidadeSatelite(tempoSaida, sol.satelites[satTemp], instance, true))
                {


                    sol.viavel = false;
                    break;

                    string satStr;
                    sol.satelites[satTemp].print(satStr, instance, false);


                    sol.print(instance);

                    throw "ERRO!";
                }

                route.rota[i].tempoChegada = tempoSaida;
                tempoSaida += instance.getDistance(satTemp, route.rota[i+1].satellite);
            }

            route.rota[route.routeSize-1].tempoChegada = tempoSaida;

            // Atualiza demanda, vetor de demanda e distancia
            route.totalDemand += candidato.demand;
            sol.distancia += candidato.incrementoDistancia;
            route.satelliteDemand[candidato.satelliteId] = candidato.demand;
            route.totalDistence += candidato.incrementoDistancia;

            demandaNaoAtendidaSat.at(candidato.satelliteId) -= candidato.demand;

        }
        else
        {
            sol.viavel = false;
            break;
        }
    }

    if(sol.viavel)
        sol.atualizaVetSatTempoChegMax(instance);


    //cout<<"*******************************FIM CONSTRUTIVO3 1º NIVEL*******************************\n\n";

}

// Com o tempo de chegada ao satelite, eh verificado se as rotas dos EV's podem sair apos o tempo de chegada do veic a combustao
bool NS_Construtivo4::verificaViabilidadeSatelite(const double tempoChegada, Satelite &satelite, const Instancia &instance, const bool modificaSatelite)
{

    bool viavel = true;

    if(satelite.sateliteId == 0)
        return true;


    // Verifica se os tempos de saida das rotas dos EV's eh maior que o tempo de chegada do veic a combustao
    for(int evId = 0; evId < instance.getN_Evs(); ++evId)
    {
        EvRoute &evRoute = satelite.vetEvRoute[evId];
        //TempoSaidaEvRoute &tempoSaidaEv = satelite.vetTempoSaidaEvRoute[evId];
        const double tempoEv = evRoute.route[0].tempoSaida;


        if(evRoute.routeSize > 2)
        {

            // Verifica se o tempo de saida do ev eh inconpativel com o tempo de chegada do veic a comb
            if(tempoEv < tempoChegada)
            {
                //string rotaStr;
                //tempoSaidaEv.evRoute->print(rotaStr, instance, true);

                // Verifica se eh possivel realizar um shift na rota

                const int indice = evRoute.route[0].posMenorFolga;
                const int cliente = evRoute.route[indice].cliente;
                const double twFim = instance.vectCliente[cliente].fimJanelaTempo;

                // calcula a menor diferenca da rota
                double diferenca = twFim - evRoute.route[indice].tempoCheg;
                if(diferenca < 0.0)
                    diferenca = 0.0;

                if(!((tempoEv + diferenca) >= tempoChegada))
                {
                    viavel = false;
                    break;
                }

            }

        }
    }


    if(!modificaSatelite)
    {
        return viavel;
    }
    else if(viavel)
    {
        //cout<<"\t\tVIAVEL\n";

        for(EvRoute &evRoute:satelite.vetEvRoute)
        {

            const double tempoEv = evRoute.route[0].tempoSaida;

            if(evRoute.routeSize > 2)
            {
                if(tempoEv >= tempoChegada)
                {
                    //cout<<"\t\tNAO FAZ NADA\n";
                    //return true;

                } else
                {
                    // Verifica se eh possivel realizar um shift na rota

                    const int indice = evRoute.route[0].posMenorFolga;
                    const int cliente = evRoute.route[indice].cliente;
                    const double twFim = instance.vectCliente[cliente].fimJanelaTempo;

                    double diferenca = twFim - evRoute.route[indice].tempoCheg;
                    if(diferenca < 0.0)
                        diferenca = 0.0;

                    if(!((tempoEv + diferenca) >= tempoChegada))
                    {

                        return false;
                    } else
                    {
                        if(!evRoute.alteraTempoSaida(tempoChegada, instance, false))
                        {
                            string str;
                            evRoute.print(str, instance, false);
                            int pos = evRoute.route[0].posMenorFolga;

                            cout<<"\n\nCLIENTE COM MENOR FOLGA: "<<evRoute.route[pos].cliente<<"\n";
                            cout<<"ROTA: "<<str<<"\n";

                            PRINT_DEBUG("", "ERRO AO ALTERAR O TEMPO DE SAIDA DA ROTA EV DE ID: "
                                    << evRoute.idRota << " DO SATELITE: " << satelite.sateliteId
                                    << "\n\n");


                            //return false;
                            cout<<"\n\n***********************************************************************VERIFICA VIABILIDADE SAT FIM**************************************************************************************\n\n";
                            throw "ERRO";
                        }

                    }

                }

            }
        }

        return true;

    }
    else
    {
        return false;
    }

}



bool NS_Construtivo4::existeDemandaNaoAtendida(Vector<double> &demandaNaoAtendida)
{
    for(double dem:demandaNaoAtendida)
    {
        if(dem > 0.0)
            return true;
    }

    return false;
}

void NS_Construtivo4::setSatParaCliente(Instancia &instancia, vector<int> &vetSatAtendCliente, vector<int> &satUtilizado, ParametrosGrasp &param)
{
    std::fill(satUtilizado.begin()+1, satUtilizado.end(), 0);
    std::fill(vetSatAtendCliente.begin()+1, vetSatAtendCliente.end(), -1);

    satUtilizado[0] = -1;

    if(instancia.getNSats() > 1)
    {
        vector<double> vetProb(instancia.getNSats() + 1);

        for(int i = instancia.getFirstClientIndex(); i <= instancia.getEndClientIndex(); ++i)
        {

            double soma = 0.0;
            int numSize = 1;
            double limite = (1.0+param.paramFuncSetSatParaCliente) * (instancia.vetVetDistClienteSatelite[i])[0].dist;
            for(; numSize < instancia.getNSats(); ++numSize)
            {
                if((instancia.vetVetDistClienteSatelite[i])[numSize].dist > limite)
                    break;
            }

            int sat = rand_u32() % numSize;
            vetSatAtendCliente[i] = (instancia.vetVetDistClienteSatelite[i])[sat].satelite;
            satUtilizado[vetSatAtendCliente[i]] += 1;

        }
    }
    else
    {

        int sat = instancia.getFirstSatIndex();

        for(int i = instancia.getFirstClientIndex(); i <= instancia.getEndClientIndex(); ++i)
        {
            vetSatAtendCliente[i] = sat;
        }

        satUtilizado[1] = instancia.numClients;
    }
}

/**
 *
 * @param sol
 * @param instancia
 * @param alfaSeg           Parametro de aleatoriedade  do segundo nivel
 * @param betaPrim            Parametro de aleatoriedade  do primeiro nivel
 * @param matClienteSat
 * @param listaRestTam
 * @param iniSatUtil      Indica se os satelites devem ser zerados de acordo com a sol parcial (Para utilizacao do IG)
 */
void NS_Construtivo4::construtivo(Solucao &sol, Instancia &instancia, const float alfaSeg, const float betaPrim,
                                  const Matrix<int> &matClienteSat, bool listaRestTam, bool iniSatUtil,
                                  bool print, Vector<int> *vetInviabilidate, const bool torneio,
                                  const bool split)
{


    Vector<int> satUtilizados(instancia.numSats+1, 0);
    Vector<int> clientesSat(instancia.getEndClientIndex()+1, 0);

    if(!iniSatUtil)
        std::fill(satUtilizados.begin()+1, satUtilizados.end(), 1);
    else
    {
        for(int sat=1; sat <= instancia.numSats; ++sat)
        {
            if(sol.satelites[sat].demanda > 0.001)
                satUtilizados[sat] = 1;
        }
    }
    std::fill(satUtilizados.begin()+1, satUtilizados.end(), 1);

/*    if(instancia.numSats == 8)
        satUtilizados[8] = 0;*/
    //satUtilizados[1] = 0;
    //satUtilizados[3] = 0;


    bool segundoNivel = construtivoSegundoNivelEV(sol, instancia, alfaSeg, matClienteSat, listaRestTam, satUtilizados,
                                                  print, *vetInviabilidate, torneio);

    Matrix<int> matClienteSat2 = matClienteSat;
    const int zero_max = max(1, instancia.numSats-2);

    if(segundoNivel)
    {
        construtivoPrimeiroNivel(sol, instancia, betaPrim, listaRestTam, split);

        if(!sol.viavel && instancia.numSats > 2)
        {
            (*vetInviabilidate)[Inv_1_Nivel_unico] += 1;

//            if(iniSatUtil)
//cout<<"\t\t1º NIVEL INV\n";

            //cout<<"CARGAS: ";
            int numSatZero = 0;

            Vector<double> vetCargaSat;

            while(!sol.viavel)
            {

                if(vetInviabilidate)
                    (*vetInviabilidate)[Inv_1_Nivel] += 1;

                //cout<<"CARGAS: ";
                vetCargaSat = Vector<double>(1 + instancia.numSats, 0.0);

                for(int i = 1; i <= instancia.getEndSatIndex(); ++i)
                {
                    vetCargaSat[i] = sol.satelites[i].demanda;
                    //cout<<i<<": "<<vetCargaSat[i]<<" ";

                    if(vetCargaSat[i] == 0.0)
                        satUtilizados[i] = 0;
                }

                //cout<<"\n\n";

                int satMin = -1;
                double min = DOUBLE_INF;

                for(int i = 1; i <= instancia.getEndSatIndex(); ++i)
                {
                    if(vetCargaSat[i] < min && vetCargaSat[i] > 0.0)
                    {
                        satMin = i;
                        min = vetCargaSat[i];
                    }

                }

                satUtilizados[satMin] = 0;
                sol.resetaSat(satMin, instancia, clientesSat);

                //sol = Solucao(instancia);
                numSatZero += 1;

                segundoNivel = construtivoSegundoNivelEV(sol, instancia, alfaSeg, matClienteSat2, listaRestTam,
                                                         satUtilizados, false, *vetInviabilidate, torneio);
                if(segundoNivel)
                {
                    //std::cout<<"1 NIVEL INVIAVEL!!!\n\n";
                    construtivoPrimeiroNivel(sol, instancia, betaPrim, listaRestTam, split);
                }
                else
                {
                    sol.viavel = false;
                    break;
                }

                if(numSatZero == zero_max)
                    break;


            }

            if(sol.viavel)
            {
                sol.recalculaDist();
            }
        }
    }
    else
        sol.viavel = false;

}



bool NS_Construtivo4::canInsert(EvRoute &evRoute, int cliente, Instancia &instance, CandidatoEV &candidatoEv, const int satelite,
                                const double tempoSaidaSat, EvRoute &evRouteAux, Vector<int> &vetInviabilidade)
{

    double demand = instance.getDemand(cliente);
    double bestIncremento = candidatoEv.incrP;
    bool viavel = false;

    evRouteAux.satelite = satelite;
    evRouteAux.idRota = evRoute.idRota;
    evRouteAux.routeSizeMax = evRoute.routeSizeMax;

    string strRota;

    InsercaoEstacao insercaoEstacao;


    if((evRoute.getDemand() + demand) > instance.getEvCap(evRoute.idRota))
    {
        vetInviabilidade[Inv_Carga] += 1;
        return false;
    }


    copiaVector(evRoute.route, evRouteAux.route, evRoute.routeSize);
    shiftVectorDir(evRouteAux.route, 1, 1, evRoute.routeSize);
    evRouteAux.routeSize = evRoute.routeSize+1;
    evRouteAux[0].bateriaRestante = instance.getEvBattery(evRoute.idRota);
    evRouteAux[0].tempoSaida = tempoSaidaSat;


    double distanciaRota  = 0.0;
    if(evRoute.routeSize > 2)
    {
        for(int i=0; (i+1) < evRoute.routeSize; ++i)
            distanciaRota += instance.getDistance(evRoute[i].cliente, evRoute[i + 1].cliente);
    }


    /* *****************************************************JANELA DE TEMPO******************************************************
     * **************************************************************************************************************************
     *
     * 0 1 2 3 0
     *
     *      Para verificar a janela de tempo ao inserir o cliente 4 eh necessario verificar se eh possivel chegar ao cliente 4,
     * verifica-se a viabilidade: <CLIENTE MENOR> acresentando a (diferença do tempo de chegada do proximo cliente).
     *
     *
     * **************************************************************************************************************************
     * **************************************************************************************************************************
     */

    const ClienteInst &instNode = instance.vectCliente[cliente];

    for(int pos = 0; pos < evRoute.routeSize-1; pos++)
    {

        evRouteAux.route[pos+1].cliente = cliente;
        double distanceAux = instance.getDistance(evRouteAux[pos].cliente, cliente) + instance.getDistance(cliente, evRouteAux[pos + 2].cliente) -
                             instance.getDistance(evRouteAux[pos].cliente, evRouteAux[pos+2].cliente);


        if(distanceAux < bestIncremento)
        {

            const double custo = testaRota(evRouteAux, evRouteAux.routeSize, instance, false, tempoSaidaSat, 0,
                                           nullptr, &vetInviabilidade);

            if(custo > 0.0)
            {

                bestIncremento = distanceAux;
                candidatoEv = CandidatoEV(pos, cliente, distanceAux, demand, 0.0, evRoute.idRota, evRoute.satelite, -1, -1, {});
                candidatoEv.penalidade = 0.0;
                candidatoEv.atualizaPenalidade();

/*                if(cliente == 86)
                {
                    strRota = "";
                    evRouteAux.print(strRota, instance, false);
                }*/

                viavel = true;

            }
            else if(viabilizaRotaEv(evRouteAux, instance, false, insercaoEstacao, (bestIncremento - distanceAux),
                                    true, tempoSaidaSat, &vetInviabilidade))
            {

                double insertionCost = insercaoEstacao.distanciaRota - distanciaRota;

                if(evRoute.routeSize == 2)
                    insertionCost += instance.penalizacaoDistEv;

                if(insertionCost < bestIncremento)
                {

                    bestIncremento = insertionCost;
                    candidatoEv = CandidatoEV(pos, cliente, (insercaoEstacao.distanciaRota - distanciaRota), demand, 0.0, evRoute.idRota, evRoute.satelite, -1, -1, insercaoEstacao);
                    candidatoEv.atualizaPenalidade(0.0);
                    viavel = true;
                }
            }
        }

        evRouteAux[pos+1] = evRouteAux[pos+2];

    }

    if(viavel)
        candidatoEv.atualizaPenalidade(0.0);

    return viavel;
}

Vector<double> NS_Construtivo4::calculaTempoSaidaInicialSat(Instancia &instance, const float beta)
{

    const int NumSatMaisDep = instance.getNSats()+1;

    Vector<double> vetTempoSaida(NumSatMaisDep, 0.0);
    Vector<int> vetSatAtendido(NumSatMaisDep, 0);

    auto existeSatNaoAtendido = [&](Vector<int> &vetSatAtendido)
    {
        for(int i=1; i <= instance.getEndSatIndex(); ++i)
        {
            if(vetSatAtendido[i] == 0)
                return true;
        }

        return false;
    };

    Solucao sol(instance);


    // Enquanto existir um satellite com demanda > 0, continua
    while(existeSatNaoAtendido(vetSatAtendido))
    {
        // Cria a lista de candidatos
        std::list<CandidatoVeicComb> listaCandidatos;

        // Percorre os satellites
        for(int i=1; i < NumSatMaisDep; ++i)
        {
            Satelite &satelite = sol.satelites.at(i);

            // Verifica se a demanda não atendida eh positiva
            if(vetSatAtendido[i] == 0)
            {

                // Percorre todas as rotas
                for(int rotaId = 0; rotaId < sol.primeiroNivel.size(); ++rotaId)
                {
                    Route &route = sol.primeiroNivel.at(rotaId);

                    // Verifica se veiculo esta 100% da capacidade
                    //if(route.totalDemand < instance.getTruckCap(rotaId))
                    {

                        CandidatoVeicComb candidato(rotaId, i, 0.0, DOUBLE_MAX);

                        // Percorre todas as posicoes da rota
                        for(int p=0; (p+1) < route.routeSize; ++p)
                        {
                            double incrementoDist = 0.0;

                            // Realiza a insercao do satellite entre as posicoes p e p+1 da rota
                            const RouteNo &clienteP =  route.rota[p];
                            const RouteNo &clientePP = route.rota[p+1];

                            // Calcula o incremento da distancia (Sempre positivo, desigualdade triangular)
                            incrementoDist -= instance.getDistance(clienteP.satellite, clientePP.satellite);
                            incrementoDist = incrementoDist + instance.getDistance(clienteP.satellite, i) + instance.getDistance(i, clientePP.satellite);

                            if(incrementoDist < candidato.incrementoDistancia)
                            {
                                // Calcula o tempo de chegada e verifica a janela de tempo
                                const double tempoChegCand = clienteP.tempoChegada + instance.getDistance(clienteP.satellite, i);
                                bool satViavel = true;

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

            listaCandidatos.sort();

            // Escolhe o candidado da lista restrita
            int size = listaCandidatos.size();
            size = max(int(beta * listaCandidatos.size()), 1);

            int tam = size;
            int escolhido = rand_u32() % tam;
            auto it = listaCandidatos.begin();

            std::advance(it, escolhido);
            CandidatoVeicComb &candidato = *it;

            // Insere candidato na solucao
            Route &route = sol.primeiroNivel.at(candidato.rotaId);
            shiftVectorDir(route.rota, candidato.pos + 1, 1, route.routeSize);

            route.rota.at(candidato.pos+1).satellite = candidato.satelliteId;
            route.rota.at(candidato.pos+1).tempoChegada = candidato.tempoSaida;
            route.routeSize += 1;
            double tempoSaida = candidato.tempoSaida;
            vetTempoSaida[candidato.pos] = candidato.tempoSaida;

            for(int i=candidato.pos+1; (i+1) < route.routeSize; ++i)
            {
                const int satTemp = route.rota.at(i).satellite;
                vetTempoSaida[i] = tempoSaida;

                tempoSaida += instance.getDistance(satTemp, route.rota[i + 1].satellite);
            }

            // Atualiza demanda, vetor de demanda e distancia
            route.totalDemand += candidato.demand;
            sol.distancia += candidato.incrementoDistancia;
            route.satelliteDemand[candidato.satelliteId] = candidato.demand;
            route.totalDistence += candidato.incrementoDistancia;
            vetSatAtendido[candidato.satelliteId] = 1;

        }
        else
        {
            sol.viavel = false;
            break;
        }
    }


    for(int i=1; i <= instance.getEndSatIndex(); ++i)
    {
        if(vetSatAtendido[i] == 0)
        {
            vetTempoSaida[i] = instance.getDistance(0, i);
        }
    }

    static bool printTempo = false;

    if(!printTempo)
    {
        for(int i = 1; i <= instance.getEndSatIndex(); ++i)
        {
            cout<<i<<": "<<vetTempoSaida[i]<<"; ";
        }
        cout<<"\n\n";

        for(int i = 1; i <= instance.getEndSatIndex(); ++i)
            cout<<i<<": "<<instance.getDistance(0, i)<<"; ";

        cout<<"\n";
        printTempo = true;

    }

    return vetTempoSaida;

}

bool NS_Construtivo4::insert(EvRoute &evRoute, CandidatoEV &insertion, const Instancia &instance, const double tempoSaidaSat, Solucao &sol)
{

    // Atualiza o numero de EVs
    if(evRoute.routeSize <= 2)
    {
        sol.numEv += 1;

        if(sol.numEv > sol.numEvMax)
        {
            PRINT_DEBUG("\n", "SOLUCAO POSSUI NUMERO MAIOR DE EVs("<<sol.numEv<<") DO QUE MAX("<<sol.numEvMax<<")");
            throw "ERRO";
        }
    }

    const int pos = insertion.pos;
    const int node = insertion.clientId;

    if(pos < 0 )
        return false;

    if(node < 0)
        return false;


    /* *********************************************************************************************
     * *********************************************************************************************
     *  checar se consegue com a capacidade atual
     *  checar se consegue com a bateria atual (a bateria do veiculo antes da proxima recarga.
     *  inserir
     *  atualizar capacidade
     *  atualizar bateria
     *  atualizar custo
     * *********************************************************************************************
     * *********************************************************************************************
     */

    int k = pos;

    string rotaAntes;
    evRoute.print(rotaAntes, instance, false);

    shiftVectorDir(evRoute.route, pos+1, 1, evRoute.routeSize);
    evRoute.route.at(pos+1).cliente = node;
    evRoute.routeSize += 1;

    if(insertion.insercaoEstacao.pos >= 0)
    {
        shiftVectorDir(evRoute.route, insertion.insercaoEstacao.pos+1, 1, evRoute.routeSize);
        evRoute.route.at(insertion.insercaoEstacao.pos+1).cliente = insertion.insercaoEstacao.estacao;
        evRoute.routeSize += 1;
    }


    if(evRoute.routeSize > 2)
    {
        sol.distancia += -evRoute.distancia;
        sol.satelites[evRoute.satelite].distancia += -evRoute.distancia;
    }

    string rotaDebug;
    evRoute.distancia = testaRota(evRoute, evRoute.routeSize, instance, true, tempoSaidaSat, 0, &rotaDebug, nullptr);
    evRoute.demanda += insertion.demand;
    sol.distancia += evRoute.distancia;


    sol.satelites[evRoute.satelite].distancia += evRoute.distancia;

    if(evRoute.distancia <= 0.0)
    {
        string rotaStr;
        evRoute.print(rotaStr, instance, false);

        cout<<"NO: "<<node<<"\n";
        cout<<"ERRO: "<<rotaDebug<<"\n\n";
        cout<<"ROTA ANTES: "<<rotaAntes<<"\n\n";

        PRINT_DEBUG("", "ERRO NA FUNCAO GreedyAlgNS::insert, BATERIA DA ROTA EH INVIAVEL: "<<rotaStr<<"\n\n");
        cout<<"FUNCAO TESTA ROTA RETORNOU DISTANCIA NEGATIVA, ROTA: ";
        //evRoute.print(instance, false);
        sol.viavel = false;

        evRoute.print(instance, true);
        cout<<" EH INVALIDA\nFILE: "<<__FILE__<<"\nLINHA: "<<__LINE__<<"\n\n";

        sol.print(instance);

        throw "ERRO";
    }


    return true;
}
