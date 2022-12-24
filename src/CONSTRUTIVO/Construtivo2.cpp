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


#define CLIENTE_INVIAVEL        (-1)
#define CLIENTE_NAO_VISITADO    0
#define CLIENTE_VISITADO        1



// Roteamento dos veiculos eletricos
bool NS_Construtivo2::construtivo2SegundoNivelEV(Solucao &sol, Instancia &instancia, const float alpha, const ublas::matrix<int> &matClienteSat,
                                                 bool listaRestTam, const float beta, const BoostC::vector<int> &satUtilizados)
{
    if(sol.numEv == sol.numEvMax)
        return false;

    //cout<<"**********************************************CONSTRUTIVO**********************************************\n\n";

    BoostC::vector <int8_t> &visitedClients = sol.vetClientesAtend;

    // Indice: igual a do cliente
    BoostC::vector<int8_t> vetClientesVisitados = BoostC::vector<int8_t>(visitedClients);

    const int FistIdClient = instancia.getFirstClientIndex();
    const int LastIdClient = instancia.getEndClientIndex();
    const auto ItEnd = visitedClients.begin() + instancia.getNSats() + instancia.getNClients();
    const BoostC::vector<double> &vetTempoSaida = instancia.vetTempoSaida;
    EvRoute evRouteAux(-1, -1, instancia.getEvRouteSizeMax(), instancia);

    //std::list <CandidatoEV> listaCandidatos;
    BoostC::vector<CandidatoEV> vetCandidatos;
    std::list<int> clientesSemCandidato;

    //COLUNAS DA MATRIZ POSSUEM SOMENTE A QUANTIDADE DE CLIENTES!!
    static BoostC::vector <ublas::matrix<CandidatoEV *>> vetMatCand(1 + instancia.getNSats());
    static bool primeiraChamada = true;

    // Guarda candidato para cada cliente
    BoostC::vector <CandidatoEV*> vetCandPtr(instancia.getNClients(), nullptr);

    const int numLinhasMat = instancia.getN_Evs();
    const int numColMat = instancia.getNClients();
    const int idPrimeiroCliente = instancia.getFirstClientIndex();
    const int idPrimeiroEv = instancia.getFirstEvIndex();

    auto transformaIdCliente = [&](const int id)
    { return (id - idPrimeiroCliente); };
    auto transformaIdEv = [&](const int id)
    { return (id - idPrimeiroEv); };

    std::fill(vetMatCand.begin(), vetMatCand.end(), ublas::zero_matrix<CandidatoEV *>(numLinhasMat, numColMat));

    const int fistSat = instancia.getFirstSatIndex();
//    int satId = sat;

    // Cria um candidato para cada cliente e armazena em matCandidato
    /*
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
                    } else if(candidatoEvAux != -1)
                        candidatoEv = candidatoEvAux;

                }

            }

            if(candidatoEv.pos != -1)
            {

                listaCandidatos.push_back(candidatoEv);
                CandidatoEV *candPtr = &listaCandidatos.back();
                vetMatCand[(candPtr->satId)](transformaIdEv(candPtr->routeId),
                                             transformaIdCliente(candPtr->clientId)) = candPtr;
                vetCandPtr[transformaIdCliente(candPtr->clientId)] = candPtr;

            } else
                clientesSemCandidato.push_back(clientId);
        }

    }
    */

    do
    {
        int cliente = instancia.getFirstClientIndex() + (rand_u32()%instancia.numClients);
        const int clienteIni = cliente;

        do
        {
            if(vetClientesVisitados[cliente] == int8_t(CLIENTE_NAO_VISITADO))
                break;
            cliente += 1;
            cliente = instancia.getFirstClientIndex() + cliente%instancia.numClients;
        }
        while(cliente != clienteIni);

        if(vetClientesVisitados[cliente] != int8_t(CLIENTE_NAO_VISITADO))
        {
            sol.viavel = false;
            break;
        }

        vetCandidatos = BoostC::vector<CandidatoEV>();
        vetCandidatos.reserve(instancia.numEv+instancia.numSats);

        for(int sat=1; sat <= instancia.numSats; ++sat)
        {
            Satelite &satelite = sol.satelites[sat];

            bool evVazio = false;
            for(int ev=0; ev < instancia.numEv; ++ev)
            {
                EvRoute &evRoute = satelite.vetEvRoute[ev];
                if(evRoute.routeSize <= 2 && evVazio)
                    continue;

                CandidatoEV candidatoEv;
                candidatoEv.satId = sat;
                candidatoEv.routeId = ev;

                canInsert(evRoute, cliente, instancia, candidatoEv, sat, )


            }
        }

    }
    while(!visitAllClientes(visitedClients, instancia));





}

