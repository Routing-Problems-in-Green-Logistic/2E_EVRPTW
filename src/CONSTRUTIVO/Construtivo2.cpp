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
                candidatoEv.routeId = ev;

//cout<<"\tADD CANDIDATO CLIENTE: "<<cliente<<"; SAT: "<<sat<<"; EV ROUTE: "<<ev<<"\n";

                vetCandidatos.push_back(candidatoEv);

            }
        }

        if(vetCandidatos.empty())
        {
            vetClientesVisitados[cliente] = CLIENTE_INVIAVEL;
            continue;
        }

        std::sort(vetCandidatos.begin(), vetCandidatos.end());

        int temp = vetCandidatos.size()*beta;
        const int sizeVet = max(temp, 1);

        const int candId = rand_u32()%sizeVet;
        CandidatoEV &candEscolhido = vetCandidatos[candId];

//cout<<"CANDIDATO: CLIENTE: "<<candEscolhido.clientId<<"; SAT: "<<candEscolhido.satId<<"; ROUTE ID: "<<candEscolhido.routeId<<"\n\n";
//cout<<"\n\n**********************\n\n";

        EvRoute &evRoute = sol.satelites[candEscolhido.satId].vetEvRoute[candEscolhido.routeId];
        if(!insert(evRoute, candEscolhido, instancia, instancia.vetTempoSaida[candEscolhido.satId], sol))
        {
            PRINT_DEBUG("", "ERRO!, CLIENTE("<<candEscolhido.clientId<<") JA FOI TESTADO NESSA ROTA, DEVERIA SER VIAVEL!!");
            throw "ERRO";
        }

//PRINT_DEBUG("","");
//cout<<"ADD CLIENTE "<<candEscolhido.clientId<<" A SOLUCAO\n";

        vetClientesVisitados[candEscolhido.clientId] = int8_t(CLIENTE_VISITADO);


    }while(!visitAllClientes(vetClientesVisitados, instancia));

    for(int sat=instancia.getFirstSatIndex(); sat <= instancia.getEndSatIndex(); ++sat)
    {

        Satelite &satelite = sol.satelites[sat];
        double demandaSat = 0.0;

        for(int ev=0; ev < instancia.numEv; ++ev)
        {
            EvRoute &evRoute = satelite.vetEvRoute[ev];
            demandaSat += evRoute.demanda;
        }

        satelite.demanda = demandaSat;
    }

    if(visitAllClientes(vetClientesVisitados, instancia))
        sol.viavel = true;
    else
        sol.viavel = false;

    // Converte vetClientesVisitados para visitClientes
    for(int i=0; i < visitedClients.size(); ++i)
    {
        if(vetClientesVisitados[i] != int8_t(1))
            visitedClients[i] = int8_t(0);
    }

    return sol.viavel;
}

void NS_Construtivo2::construtivo2(Solucao &sol, Instancia &instancia, const float alpha, const float beta, const ublas::matrix<int> &matClienteSat,
                                   bool listaRestTam)
{


    BoostC::vector<int> satUtilizados(instancia.numSats+1, 0);
    BoostC::vector<int> clientesSat(instancia.getEndClientIndex()+1, 0);

    std::fill(satUtilizados.begin()+1, satUtilizados.end(), 1);

    bool segundoNivel = construtivo2SegundoNivelEV(sol, instancia, alpha, matClienteSat, listaRestTam, beta, satUtilizados);

    ublas::matrix<int> matClienteSat2 = matClienteSat;
    const int zero_max = max(1, instancia.numSats-2);

    if(segundoNivel)
    {
        construtivoPrimeiroNivel(sol, instancia, beta, listaRestTam);

        if(!sol.viavel && instancia.numSats > 2)
        {
            //cout<<"CARGAS: ";
            int numSatZero = 0;

            BoostC::vector<double> vetCargaSat;

            while(!sol.viavel)
            {

                //cout<<"CARGAS: ";
                vetCargaSat = BoostC::vector<double>(1 + instancia.numSats, 0.0);

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
                sol.reseta1Nivel(instancia);

                //sol = Solucao(instancia);
                numSatZero += 1;

                segundoNivel = construtivo2SegundoNivelEV(sol, instancia, alpha, matClienteSat2, listaRestTam, beta, satUtilizados);
                if(segundoNivel)
                {
                    construtivoPrimeiroNivel(sol, instancia, beta, listaRestTam);
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
