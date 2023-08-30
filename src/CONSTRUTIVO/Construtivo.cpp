#include "Construtivo.h"
#include "../Auxiliary.h"
#include <set>
#include <cfloat>
#include <chrono>
#include "../mersenne-twister.h"
#include "../ViabilizadorRotaEv.h"

using namespace NS_Construtivo;
using namespace std;
using namespace NS_Auxiliary;
using namespace NS_viabRotaEv;


// Roteamento dos veiculos eletricos
bool NS_Construtivo::construtivoSegundoNivelEV(Solucao &sol, Instancia &instance, const float alpha,
                                               const Matrix<int> &matClienteSat, bool listaRestTam,
                                               const float beta, const Vector<int> &satUtilizados, bool print,
                                               Vector<int> *vetInviabilidate)
{
    if(sol.numEv == sol.numEvMax)
        return false;

    //cout<<"**********************************************CONSTRUTIVO**********************************************\n\n";

    Vector<int8_t> &visitedClients = sol.vetClientesAtend;

    const int FistIdClient  = instance.getFirstClientIndex();
    const int LastIdClient  = instance.getEndClientIndex();
    const auto ItEnd        = visitedClients.begin() + instance.getNSats() + instance.getNClients();
    const Vector<double> &vetTempoSaida = instance.vetTempoSaida;
    EvRoute evRouteAux(-1, -1, instance.getEvRouteSizeMax(), instance);

    std::list<CandidatoEV> listaCandidatos;
    std::list<int> clientesSemCandidato;

    //COLUNAS DA MATRIZ POSSUEM SOMENTE A QUANTIDADE DE CLIENTES!!
    static Vector<Matrix<CandidatoEV*>> matCandidato(1 + instance.getNSats());
    static bool primeiraChamada = true;
    Vector<CandidatoEV*> vetCandPtr(instance.getNClients(), nullptr);

    const int numLinhasMat      = instance.getN_Evs();
    const int numColMat         = instance.getNClients();
    const int idPrimeiroCliente = instance.getFirstClientIndex();
    const int idPrimeiroEv      = instance.getFirstEvIndex();

    auto transformaIdCliente  = [&](const int id){return (id - idPrimeiroCliente);};
    auto transformaIdEv       = [&](const int id){return (id-idPrimeiroEv);};

    std::fill(matCandidato.begin(), matCandidato.end(), Matrix<CandidatoEV*>(numLinhasMat, numColMat, nullptr));

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

                    canInsert(route, clientId, instance, candidatoEvAux, satId, vetTempoSaida.at(satId), evRouteAux,
                              vetInviabilidate);

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
                matCandidato.at(candPtr->satId)(transformaIdEv(candPtr->routeId), transformaIdCliente(candPtr->clientId)) = candPtr;
                vetCandPtr.at(transformaIdCliente(candPtr->clientId)) = candPtr;

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
        auto topItem = std::next(listaCandidatos.begin(), randIndex);

if(print)
    cout<<"\tESCOLHIDO: "<<topItem->clientId<<";"<<topItem->incrP<<"\n\n";

        CandidatoEV *candEvPtr = &(*topItem);
        visitedClients.at(topItem->clientId) = 1;
        sol.vetClientesAtend.at(topItem->clientId) = 1;
        Satelite *satelite = sol.getSatelite(topItem->satId);
        satelite->demanda += topItem->demand;
        EvRoute &evRoute = satelite->getRoute(transformaIdEv(topItem->routeId));

        bool resultadoTopItem = insert(evRoute, *topItem, instance, vetTempoSaida[topItem->satId], sol);
        sol.rotaEvAtualizada(topItem->satId, transformaIdEv(topItem->routeId));

        if(!resultadoTopItem)
        {
            PRINT_DEBUG("", "ERRO, INSERT RETORNOU FALSE!!!\n\n");
            throw "ERRO";
        }

        matCandidato.at(topItem->satId)(transformaIdEv(topItem->routeId), transformaIdCliente(topItem->clientId)) = nullptr;

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
                CandidatoEV *candidatoEvPtrAux = matCandidato.at(sat->sateliteId)(transformaIdEv(evRouteEsc.idRota), transformaIdCliente(clientId));

                if(numEVsMax && candidatoEvPtrAux)
                {
                    // Se numEvMax=True e evRouteTemp eh VAZIO, entao eh necessario reavaliar o cliente em todas as rotas

                    EvRoute &evRouteTemp = sat->getRoute(transformaIdEv(candidatoEvPtrAux->routeId));
                    if(evRouteTemp.routeSize <= 2)
                        candidatoEvPtrAux = nullptr;
                }

                // 1º Os candidatos que estao no mesmo satelite e na mesma rota precisao ser avaliados novamente em todos os sat e rotas
                if(((!visitedClients.at(clientId)) && (matClienteSat(clientId, sat->sateliteId) == 1 && satUtilizados[sat->sateliteId]==1) &&
                clientId != topItem->clientId) && (candidatoEvPtrAux || (!candidatoEvPtrAux && numEVsMax)))
                {

                    matCandidato.at(sat->sateliteId)(transformaIdEv(evRouteEsc.idRota), transformaIdCliente(clientId)) = nullptr;
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

                                canInsert(route, clientId, instance, candidatoEv, satId, vetTempoSaida.at(satId),
                                          evRouteAux, vetInviabilidate);

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
                            *candidatoEvPtrAux = candidatoEv;

                        matCandidato.at(candidatoEv.satId)(transformaIdEv(candidatoEv.routeId), transformaIdCliente(candidatoEv.clientId)) = candidatoEvPtrAux;
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
                            bool resultado = canInsert(sol.satelites.at(satIdTemp).getRoute(transformaIdEv(routeId)),
                                                       clientId, instance, *candidatoEvPtrAux, candidatoEvPtrAux->satId,
                                                       vetTempoSaida.at(satIdTemp), evRouteAux, vetInviabilidate);

                            EvRoute &evRouteTemp = sol.satelites.at(satIdTemp).getRoute(transformaIdEv(routeId));

                            if(evRouteTemp.routeSize <= 2)
                            {
                                PRINT_DEBUG("", "evRouteTemp eh VAZIO\n");
                                throw "ERRO";
                            }

                            if(resultado)
                            {
                                matCandidato.at(candCopia.satId)(transformaIdEv(candCopia.routeId), transformaIdCliente(candCopia.clientId)) = nullptr;
                                matCandidato.at(satIdTemp)(transformaIdEv(candidatoEvPtrAux->routeId),
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
                                               instance, candidatoEv, satId, vetTempoSaida.at(satId), evRouteAux,
                                               vetInviabilidate);

                    if(resultado)
                    {

                        listaCandidatos.push_back(candidatoEv);
                        CandidatoEV *candPtr = &listaCandidatos.back();
                        matCandidato.at(satId)(transformaIdEv(routeId), transformaIdCliente(cliente)) = candPtr;
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
        matCandidato.at(topItem->satId)(transformaIdEv(topItem->routeId), transformaIdCliente(topItem->clientId)) = nullptr;
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


bool NS_Construtivo::visitAllClientes(Vector<int8_t> &visitedClients, const Instancia &instance)
{

    int i=instance.getFirstClientIndex();

    for(; i <= instance.getEndClientIndex(); ++i)
    {

        if(visitedClients.at(i) != int8_t(1))
            return false;
    }

    return true;

}

void NS_Construtivo::construtivoPrimeiroNivel(Solucao &sol, Instancia &instance, const float beta, bool listaRestTam, const bool split)
{

    // Cria o vetor com a demanda de cada satellite

    Vector<double> demandaNaoAtendidaSat;
    demandaNaoAtendidaSat.reserve(sol.getNSatelites()+1);
    int satId = 1;
    demandaNaoAtendidaSat.push_back(0.0);

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
        for(int sat=1; sat < NumSatMaisDep; ++sat)
        {
            Satelite &satelite = sol.satelites.at(sat);

            // Verifica se a demanda não atendida eh positiva
            if(demandaNaoAtendidaSat[sat] > 0.0)
            {

                // Percorre todas as rotas
                for(int rotaId = 0; rotaId < sol.primeiroNivel.size(); ++rotaId)
                {
                    static const double truckCap = instance.getTruckCap(rotaId);

                    Route &route = sol.primeiroNivel[rotaId];

                    // Verifica se veiculo esta 100% da capacidade
                    if(route.totalDemand < instance.getTruckCap(rotaId))
                    {
                        // Calcula a capacidade restante do veiculo
                        double capacidade = instance.getTruckCap(rotaId) - route.totalDemand;
                        double demandaAtendida = capacidade;

                        if(demandaNaoAtendidaSat.at(sat) < capacidade)
                            demandaAtendida = demandaNaoAtendidaSat[sat];
                        else if(!split && demandaNaoAtendidaSat[sat] < truckCap)
                            continue;


                        CandidatoVeicComb candidato(rotaId, sat, demandaAtendida, DOUBLE_MAX);

                        // Percorre todas as posicoes da rota
                        for(int p=0; (p+1) < route.routeSize; ++p)
                        {
                            double incrementoDist = 0.0;

                            // Realiza a insercao do satellite entre as posicoes p e p+1 da rota
                            const RouteNo &clienteP =  route.rota[p];
                            const RouteNo &clientePP = route.rota[p+1];

                            // Calcula o incremento da distancia (Sempre positivo, desigualdade triangular)
                            incrementoDist -= instance.getDistance(clienteP.satellite, clientePP.satellite);
                            incrementoDist = incrementoDist + instance.getDistance(clienteP.satellite, sat) + instance.getDistance(sat, clientePP.satellite);

                            if(incrementoDist < candidato.incrementoDistancia)
                            {

                                // Calcula o tempo de chegada e verifica a janela de tempo
                                const double tempoChegCand = clienteP.tempoChegada + instance.getDistance(clienteP.satellite, sat);

                                bool satViavel = true;

                                if(verificaViabilidadeSatelite(tempoChegCand, satelite, instance, false))
                                {
                                    double tempoChegTemp = tempoChegCand + instance.getDistance(sat, clientePP.satellite);

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
                                {
                                    satViavel = false;
                                }

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

            if(listaRestTam)
            {
                size = max(int(beta * listaCandidatos.size()), 1);
            }
            else
            {
                double limite = (1.0+beta)*listaCandidatos.begin()->incrementoDistancia;
                auto candIt = listaCandidatos.begin();
                int temp = 0;

                while(candIt != listaCandidatos.end() && candIt->incrementoDistancia <= limite)
                {
                    ++candIt;
                    temp += 1;
                }

                size = max(temp, 1);
            }

            int tam = size;
            int escolhido = rand_u32() % tam;
            auto it = listaCandidatos.begin();

            std::advance(it, escolhido);
            CandidatoVeicComb &candidato = *it;

            // Insere candidato na solucao
            Route &route = sol.primeiroNivel.at(candidato.rotaId);

            shiftVectorDir(route.rota, candidato.pos + 1, 1, route.routeSize);

            route.rota[candidato.pos+1].satellite = candidato.satelliteId;
            route.rota[candidato.pos+1].tempoChegada = candidato.tempoSaida;
            route.routeSize += 1;
            double tempoSaida = candidato.tempoSaida;

            for(int i=candidato.pos+1; (i+1) < route.routeSize; ++i)
            {
                const int satTemp = route.rota[i].satellite;

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

            demandaNaoAtendidaSat[candidato.satelliteId] -= candidato.demand;

        }
        else
        {
            sol.viavel = false;
            break;
        }
    }

    if(sol.viavel)
        sol.atualizaVetSatTempoChegMax(instance);

}

// Com o tempo de chegada ao satelite, eh verificado se as rotas dos EV's podem sair apos o tempo de chegada do veic a combustao
bool NS_Construtivo::verificaViabilidadeSatelite(const double tempoChegada, Satelite &satelite, const Instancia &instance, const bool modificaSatelite)
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
        return false;

}



bool NS_Construtivo::existeDemandaNaoAtendida(Vector<double> &demandaNaoAtendida)
{
    for(double dem:demandaNaoAtendida)
    {
        if(dem > 0.0)
            return true;
    }

    return false;
}

void NS_Construtivo::setSatParaCliente(Instancia &instancia, vector<int> &vetSatAtendCliente, vector<int> &satUtilizado, ParametrosGrasp &param)
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
 * @param alpha           Parametro de aleatoriedade  do segundo nivel
 * @param beta            Parametro de aleatoriedade  do primeiro nivel
 * @param matClienteSat
 * @param listaRestTam
 * @param iniSatUtil      Indica se os satelites devem ser zerados de acordo com a sol parcial (Para utilizacao do IG)
 */
void NS_Construtivo::construtivo(Solucao &sol, Instancia &instancia, const float alpha, const float beta,
                                 const Matrix<int> &matClienteSat, bool listaRestTam, bool iniSatUtil,
                                 bool print, Vector<int> *vetInviabilidate, const bool split)
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
    bool segundoNivel = construtivoSegundoNivelEV(sol, instancia, alpha, matClienteSat, listaRestTam, beta,
                                                  satUtilizados, print, vetInviabilidate);
    Matrix<int> matClienteSat2 = matClienteSat;
    const int zero_max = max(1, instancia.numSats-2);

    if(segundoNivel)
    {
        construtivoPrimeiroNivel(sol, instancia, beta, listaRestTam, split);

        if(!sol.viavel && instancia.numSats > 2)
        {
            int numSatZero = 0;
            Vector<double> vetCargaSat;

            while(!sol.viavel)
            {
                if(vetInviabilidate)
                    (*vetInviabilidate)[Inv_1_Nivel] += 1;

                vetCargaSat = Vector<double>(1 + instancia.numSats, 0.0);

                for(int i = 1; i <= instancia.getEndSatIndex(); ++i)
                {
                    vetCargaSat[i] = sol.satelites[i].demanda;
                    if(vetCargaSat[i] == 0.0)
                        satUtilizados[i] = 0;
                }

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
                sol.reseta1Nivel(instancia);

                //sol = Solucao(instancia);
                numSatZero += 1;

                segundoNivel = construtivoSegundoNivelEV(sol, instancia, alpha, matClienteSat2, listaRestTam, beta,
                                                         satUtilizados, false, vetInviabilidate);
                if(segundoNivel)
                {
                    construtivoPrimeiroNivel(sol, instancia, beta, listaRestTam, split);
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



bool NS_Construtivo::canInsert(EvRoute &evRoute, int cliente, Instancia &instance, CandidatoEV &candidatoEv, const int satelite,
                               const double tempoSaidaSat, EvRoute &evRouteAux, Vector<int> *vetInviabilidate)
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
        if(vetInviabilidate)
            (*vetInviabilidate)[Inv_Carga] += 1;
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

            const double custo = testaRota(evRouteAux, evRouteAux.routeSize, instance, false, tempoSaidaSat, 0, nullptr,
                                           vetInviabilidate);

            if(custo > 0.0)
            {

                bestIncremento = distanceAux;
                candidatoEv = CandidatoEV(pos, cliente, distanceAux, demand, 0.0, evRoute.idRota, evRoute.satelite, -1, -1, {});
                candidatoEv.penalidade = 0.0;
                candidatoEv.atualizaPenalidade();

                if(cliente == 86)
                {
                    strRota = "";
                    evRouteAux.print(strRota, instance, false);
                }

                viavel = true;

            }
            else if(viabilizaRotaEv(evRouteAux, instance, false, insercaoEstacao, (bestIncremento - distanceAux),
                                    true, tempoSaidaSat, vetInviabilidate))
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

Vector<double> NS_Construtivo::calculaTempoSaidaInicialSat(Instancia &instance, const float beta)
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

bool NS_Construtivo::insert(EvRoute &evRoute, CandidatoEV &insertion, const Instancia &instance, const double tempoSaidaSat, Solucao &sol)
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

    //cout<<"EV ROUTE ID: "<<evRoute.idRota<<"\n";
    //evRoute.print(instance, true);

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
