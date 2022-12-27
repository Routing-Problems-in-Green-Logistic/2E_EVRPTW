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

    /*
    const int numLinhasMat = instancia.getN_Evs();
    const int numColMat = instancia.getNClients();
    const int idPrimeiroCliente = instancia.getFirstClientIndex();
    const int idPrimeiroEv = instancia.getFirstEvIndex();
    auto transformaIdCliente = [&](const int id)
    { return (id - idPrimeiroCliente); };
    auto transformaIdEv = [&](const int id)
    { return (id - idPrimeiroEv); };
    */

    std::fill(vetMatCand.begin(), vetMatCand.end(), ublas::zero_matrix<CandidatoEV *>(numLinhasMat, numColMat));


    do
    {
        // Seleciona aleatoriamente um cliente que nao foi atendido
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
        // Cliente Selecionando

        vetCandidatos = BoostC::vector<CandidatoEV>();
        vetCandidatos.reserve(instancia.numEv+instancia.numSats);

        // Candidato eh <cliente, ev> sendo a insercao mais barata de cada ev
        for(int sat=1; sat <= instancia.numSats; ++sat)
        {
            if(!satUtilizados[sat])
                continue;

            Satelite &satelite = sol.satelites[sat];
            // Somente um ev vazio eh testado para cada sat
            bool evVazio = false;

            for(int ev=0; ev < instancia.numEv; ++ev)
            {
                EvRoute &evRoute = satelite.vetEvRoute[ev];
                if(evRoute.routeSize <= 2 && evVazio)
                    continue;

                if(evRoute.routeSize <= 2 && sol.numEv == sol.numEvMax)
                {
                    evVazio = true;
                    continue;
                }

                if(evRoute.routeSize <= 2)
                    evVazio = true;

                CandidatoEV candidatoEv;
                candidatoEv.satId = sat;
                candidatoEv.routeId = ev;

                // Retorna a melhor insercao do cliente para evRoute
                if(!canInsert(evRoute, cliente, instancia, candidatoEv, sat, instancia.vetTempoSaida[sat], evRouteAux))
                    continue;

                vetCandidatos.push_back(candidatoEv);

            }
        }

        if(vetCandidatos.empty())
        {
            vetClientesVisitados[cliente] = CLIENTE_INVIAVEL;
            continue;
        }





    }
    while(!visitAllClientes(visitedClients, instancia));





}

