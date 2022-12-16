/* ****************************************
 * ****************************************
 *  Nome:    Igor de Andrade Junqueira
 *  Data:    13/12/22
 *  Arquivo: Construtivo2.cpp
 * ****************************************
 * ****************************************/

#include "Construtivo2.h"
#include "../mersenne-twister.h"

using namespace NS_Construtivo;
using namespace NS_Construtivo2;


// Roteamento dos veiculos eletricos
bool NS_Construtivo2::construtivo2SegundoNivelEV(Solucao &sol, Instancia &instance, const float alpha, const ublas::matrix<int> &matClienteSat,
                                                 bool listaRestTam, const float beta, const BoostC::vector<int> &satUtilizados)
{
    if(sol.numEv == sol.numEvMax)
        return false;

    //cout<<"**********************************************CONSTRUTIVO**********************************************\n\n";

    BoostC::vector<int8_t> &visitedClients = sol.vetClientesAtend;

    const int FistIdClient  = instance.getFirstClientIndex();
    const int LastIdClient  = instance.getEndClientIndex();
    const auto ItEnd        = visitedClients.begin() + instance.getNSats() + instance.getNClients();
    const BoostC::vector<double> &vetTempoSaida = instance.vetTempoSaida;
    EvRoute evRouteAux(-1, -1, instance.getEvRouteSizeMax(), instance);

    std::list<CandidatoEV> listaCandidatos;
    std::list<int> clientesSemCandidato;

    //COLUNAS DA MATRIZ POSSUEM SOMENTE A QUANTIDADE DE CLIENTES!!
    static BoostC::vector<ublas::matrix<CandidatoEV*>> vetMatCand(1 + instance.getNSats());
    static bool primeiraChamada = true;

    // Guarda candidato para cada cliente
    BoostC::vector<CandidatoEV*> vetCandPtr(instance.getNClients(), nullptr);

    const int numLinhasMat      = instance.getN_Evs();
    const int numColMat         = instance.getNClients();
    const int idPrimeiroCliente = instance.getFirstClientIndex();
    const int idPrimeiroEv      = instance.getFirstEvIndex();

    auto transformaIdCliente  = [&](const int id){return (id - idPrimeiroCliente);};
    auto transformaIdEv       = [&](const int id){return (id-idPrimeiroEv);};

    std::fill(vetMatCand.begin(), vetMatCand.end(), ublas::zero_matrix<CandidatoEV*>(numLinhasMat, numColMat));

    const int fistSat = instance.getFirstSatIndex();
//    int satId = sat;

    // Cria um candidato para cada cliente e armazena em matCandidato
    for(int clientId = FistIdClient; clientId <= LastIdClient; ++clientId)
    {
        if(visitedClients.at(clientId) == int8_t(0))
        {

            CandidatoEV candidatoEv;
            candidatoEv.clientId = clientId;

            for(int satId = instance.getFirstSatIndex(); satId <= instance.getEndSatIndex(); ++satId)
            {

                //cout<<clientId<<", "<<satId<<"\n";
                if(matClienteSat(clientId, (satId)) != 1 || satUtilizados[satId] == 0)
                    continue;

                Satelite *sat = sol.getSatelite(satId);
                bool rotaVazia = false;
                for(int routeId = 0; routeId < instance.getN_Evs(); ++routeId)
                {
                    EvRoute &route = sat->getRoute(routeId);
                    if(route.routeSize <= 2 && !rotaVazia)
                        rotaVazia = true;
                    else if(route.routeSize <= 2 && rotaVazia)
                        continue;

                    CandidatoEV candidatoEvAux(candidatoEv);
                    candidatoEvAux.satId = satId;
                    candidatoEvAux.routeId = routeId;

                    canInsert(route, clientId, instance, candidatoEvAux, satId, vetTempoSaida.at(satId), evRouteAux);

                    if(candidatoEv.pos != -1 && candidatoEvAux.pos != -1)
                    {
                        if(candidatoEvAux < candidatoEv)
                            candidatoEv = candidatoEvAux;
                    }
                    else if(candidatoEvAux != -1)
                        candidatoEv = candidatoEvAux;

                }

            }

            if(candidatoEv.pos != -1)
            {

                listaCandidatos.push_back(candidatoEv);
                CandidatoEV *candPtr = &listaCandidatos.back();
                vetMatCand[(candPtr->satId)](transformaIdEv(candPtr->routeId), transformaIdCliente(candPtr->clientId)) = candPtr;
                vetCandPtr[transformaIdCliente(candPtr->clientId)] = candPtr;

            } else
                clientesSemCandidato.push_back(clientId);
        }

    }


    while(!visitAllClientes(visitedClients, instance) && !listaCandidatos.empty())
    {
        listaCandidatos.sort();
        int size = listaCandidatos.size();

        if(listaRestTam)
        {
            size = max(int(alpha * listaCandidatos.size()), 1);
        }
        else
        {
            double limite = (1.0+alpha)*(listaCandidatos.begin()->incrP);
            auto candIt = listaCandidatos.begin();
            int temp = 0;

            while(candIt->incrP <= limite && candIt != listaCandidatos.end())
            {
                //cout<<candIt->clientId<<"\n";
                ++candIt;
                temp += 1;
            }

            size = max(temp, 1);
        }

        int randIndex = rand_u32()%size;

        //cout<<"size(lista): "<<listaCandidatos.size()<<"; size*alfa: "<<size<<"; rand: "<<randIndex<<"\n";

        auto topItem = std::next(listaCandidatos.begin(), randIndex);
        //cout<<"\tESCOLHIDO: "<<topItem->clientId<<";"<<topItem->incrP<<"\n\n";
        CandidatoEV *candEvPtr = &(*topItem);
        visitedClients.at(topItem->clientId) = 1;
        sol.vetClientesAtend.at(topItem->clientId) = 1;

        Satelite *satelite = sol.getSatelite(topItem->satId);
        satelite->demanda += topItem->demand;


        EvRoute &evRoute = satelite->getRoute(transformaIdEv(topItem->routeId));

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

        bool resultadoTopItem = insert(evRoute, *topItem, instance, vetTempoSaida[topItem->satId], sol);
        if(!resultadoTopItem)
        {
            PRINT_DEBUG("", "ERRO, INSERT RETORNOU FALSE!!!\n\n");
            throw "ERRO";
        }

        vetMatCand.at(topItem->satId)(transformaIdEv(topItem->routeId), transformaIdCliente(topItem->clientId)) = nullptr;

        // Corrigi a lista de candidatos
        // 1º Os candidatos que estao no mesmo satelite e na mesma rota precisao ser avaliados novamente
        // 2º Os clientes que nao possuem candidato so tem que ser avaliados na rota que houve mudanca
        // 3º Os candidatos que nao estao na mesma rota so precisao ser reavaliados na rota que houve mudanca

        Satelite *sat = sol.getSatelite(candEvPtr->satId);
        EvRoute &evRouteEsc = sat->vetEvRoute.at(transformaIdEv(candEvPtr->routeId));
        const bool numEVsMax = (sol.numEv >= sol.numEvMax) && instance.numSats > 1;

        /* ********************************************************************************
         * Se numEvMax=true, entao, excluir os candidados que sao de veiculos vazios
         * Verificar se existe somente um sat
         * ********************************************************************************/

        if(listaCandidatos.size() > 1)
        {

            for(int clientId = instance.getFirstClientIndex(); clientId <= instance.getEndClientIndex(); ++clientId)
            {

                CandidatoEV *candidatoEvPtrAux = vetMatCand.at(sat->sateliteId)(transformaIdEv(evRouteEsc.idRota), transformaIdCliente(clientId));

                if(numEVsMax && candidatoEvPtrAux)
                {
                    // Se numEvMax=True e evRouteTemp eh VAZIO, entao eh necessario reavaliar o cliente em todas as rotas

                    EvRoute &evRouteTemp = sat->getRoute(transformaIdEv(candidatoEvPtrAux->routeId));
                    if(evRouteTemp.routeSize <= 2)
                        candidatoEvPtrAux = nullptr;
                }

                // 1º Os candidatos que estao no mesmo satelite e na mesma rota precisao ser avaliados novamente em todos os sat e rotas
                if(((!visitedClients.at(clientId)) && (matClienteSat(clientId, sat->sateliteId) == 1 && satUtilizados[sat->sateliteId]==1) && clientId != topItem->clientId) &&
                   (candidatoEvPtrAux || (!candidatoEvPtrAux && numEVsMax)))
                {

                    vetMatCand.at(sat->sateliteId)(transformaIdEv(evRouteEsc.idRota), transformaIdCliente(clientId)) = nullptr;
                    CandidatoEV candidatoEv;
                    candidatoEv.clientId = clientId;

                    for(int satId = instance.getFirstSatIndex(); satId <= instance.getEndSatIndex(); satId++)
                    {
                        if(matClienteSat(clientId, satId) == 1 && satUtilizados[satId]==1)
                        {
                            bool routeEmpty = false;

                            for(int routeId = instance.getFirstEvIndex(); routeId <= instance.getEndEvIndex(); routeId++)
                            {
                                Satelite *satAux = sol.getSatelite(satId);
                                EvRoute &route = satAux->getRoute(transformaIdEv(routeId));

                                if(route.routeSize <= 2 && (routeEmpty || numEVsMax))
                                    continue;

                                canInsert(route, clientId, instance, candidatoEv, satId, vetTempoSaida.at(satId), evRouteAux);

                                if(route.routeSize <= 2 && !routeEmpty)
                                    routeEmpty = true;

                            }
                        }

                    }

                    if(!candidatoEvPtrAux)
                        candidatoEvPtrAux = vetCandPtr.at(transformaIdCliente(candidatoEv.clientId));

                    if(candidatoEv.pos != -1)
                    {
                        // Encontrou uma rota viavel
                        if(candidatoEvPtrAux == nullptr)
                        {
                            clientesSemCandidato.remove(candidatoEv.clientId);
                            listaCandidatos.push_back(candidatoEv);
                            vetCandPtr.at(transformaIdCliente(candidatoEv.clientId)) = &listaCandidatos.back();
                            candidatoEvPtrAux = vetCandPtr.at(transformaIdCliente(candidatoEv.clientId));
                        }
                        else
                        {

                            *candidatoEvPtrAux = candidatoEv;
                        }

                        vetMatCand.at(candidatoEv.satId)(transformaIdEv(candidatoEv.routeId), transformaIdCliente(candidatoEv.clientId)) = candidatoEvPtrAux;
                    }
                    else
                    {

                        // Nao existe uma rota viavel para o cliente

                        if(candidatoEvPtrAux)
                        {
                            listaCandidatos.remove(*candidatoEvPtrAux);
                            vetCandPtr.at(transformaIdCliente(clientId)) = nullptr;
                            candidatoEvPtrAux = nullptr;
                            clientesSemCandidato.push_back(clientId);
                        }
                    }

                }
                    // 3º Os candidatos que nao estao na mesma rota so precisao ser reavaliados na rota que houve mudanca
                else if((!visitedClients[clientId]) && !candidatoEvPtrAux && clientId != topItem->clientId)
                {

                    candidatoEvPtrAux = vetCandPtr.at(transformaIdCliente(clientId));
                    if(candidatoEvPtrAux)
                    {
                        if(matClienteSat(clientId, topItem->satId) == 1 && satUtilizados[topItem->satId] == 1)
                        {

                            CandidatoEV candCopia = *candidatoEvPtrAux;

                            const int satIdTemp = topItem->satId;
                            const int routeId = topItem->routeId;

                            bool resultado = canInsert(sol.satelites.at(satIdTemp).getRoute(transformaIdEv(routeId)), clientId, instance,
                                                       *candidatoEvPtrAux, candidatoEvPtrAux->satId, vetTempoSaida.at(satIdTemp), evRouteAux);

                            EvRoute &evRouteTemp = sol.satelites.at(satIdTemp).getRoute(transformaIdEv(routeId));

                            if(evRouteTemp.routeSize <= 2)
                            {
                                PRINT_DEBUG("", "evRouteTemp eh VAZIO\n");
                                throw "ERRO";
                            }

                            if(resultado)
                            {

                                vetMatCand.at(candCopia.satId)(transformaIdEv(candCopia.routeId), transformaIdCliente(candCopia.clientId)) = nullptr;
                                vetMatCand.at(satIdTemp)(transformaIdEv(candidatoEvPtrAux->routeId),
                                                         transformaIdCliente(candidatoEvPtrAux->clientId)) = candidatoEvPtrAux;
                            }
                        }
                    }
                }
            }
        }

        bool clientesAntendidos = visitAllClientes(visitedClients, instance);

        if(!clientesAntendidos)
        {
            // Tenta inserir os clientes sem candidato na nova rota

            auto itCliente = clientesSemCandidato.begin();
            int cliente = -1;

            while(itCliente != clientesSemCandidato.end())
            {
                if(matClienteSat((*itCliente), candEvPtr->satId) == 1 && satUtilizados[candEvPtr->satId] == 1)
                {
                    cliente = *itCliente;
                    CandidatoEV candidatoEv;

                    const int satId = topItem->satId;
                    const int routeId = topItem->routeId;

                    bool resultado = canInsert(sol.satelites.at(satId).getRoute(transformaIdEv(routeId)), cliente,
                                               instance, candidatoEv, satId, vetTempoSaida.at(satId), evRouteAux);

                    if(resultado)
                    {

                        listaCandidatos.push_back(candidatoEv);
                        CandidatoEV *candPtr = &listaCandidatos.back();
                        vetMatCand.at(satId)(transformaIdEv(routeId), transformaIdCliente(cliente)) = candPtr;
                        vetCandPtr.at(transformaIdCliente(cliente)) = candPtr;
                        ++itCliente;
                        clientesSemCandidato.remove(cliente);
                    } else
                        ++itCliente;

                }
            }
        }

        // Remove candidato escolhido
        vetCandPtr.at(transformaIdCliente(topItem->clientId)) = nullptr;
        vetMatCand.at(topItem->satId)(transformaIdEv(topItem->routeId), transformaIdCliente(topItem->clientId)) = nullptr;
        listaCandidatos.erase(topItem);

        if(listaCandidatos.empty())
        {
            break;
        }


    } // End while(!visitAllClientes)


    sol.viavel = visitAllClientes(visitedClients, instance);
    if(!instance.nivel2Viavel && sol.viavel)
        instance.nivel2Viavel = true;

    return sol.viavel;
}

