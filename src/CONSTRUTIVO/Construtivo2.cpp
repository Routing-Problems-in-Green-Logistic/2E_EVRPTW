/* ****************************************
 * ****************************************
 *  Nome: ***********************
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
bool NS_Construtivo2::construtivo2SegundoNivelEV(Solucao &sol, Instancia &instancia, const float alpha, const Matrix<int> &matClienteSat,
                                                 bool listaRestTam, const float beta, const Vector<int> &satUtilizados)
{

    if(sol.numEv == sol.numEvMax)
        return false;

    //cout<<"**********************************************CONSTRUTIVO2**********************************************\n\n";

    Vector <int8_t> &visitedClients = sol.vetClientesAtend;

    // Indice: igual a do cliente
    Vector<int8_t> vetClientesVisitados = Vector<int8_t>(visitedClients);

/*
cout<<"VET CLIENTES VISITADOS: \n";
for(int i=0; i < visitedClients.size(); ++i)
    cout<<int(visitedClients[i])<<" ";
cout<<"\n\n";
*/

    const Vector<double> &vetTempoSaida = instancia.vetTempoSaida;
    EvRoute evRouteAux(-1, -1, instancia.getEvRouteSizeMax(), instancia);

    //std::list <CandidatoEV> listaCandidatos;
    Vector<CandidatoEV> vetCandidatos;
    std::list<int> clientesSemCandidato;

    //instancia.vetVetDistClienteSatelite
    std::vector<DistClienteSat> vetClienteSat;
    int proxIdCliente = 0;

    for(int cliente=instancia.getFirstClientIndex(); cliente <= instancia.getEndClientIndex(); ++cliente)
    {
        if(vetClientesVisitados[cliente] == int8_t(CLIENTE_NAO_VISITADO) && cliente != 0)
        {
            DistSatelite distSatelite = (instancia.vetVetDistClienteSatelite[cliente])[0];
            vetClienteSat.emplace_back(cliente, distSatelite.dist);
        }
    }

    std::sort(vetClienteSat.begin(), vetClienteSat.end());



    do
    {
/*
EvRoute &evRouteTemp = sol.satelites[6].vetEvRoute[0];
string str;
evRouteTemp.print(str, instancia, true);
cout<<"\t\t\tROTA: "<<str<<"\n\n";
*/
        // Seleciona aleatoriamente um cliente que nao foi atendido

        if(proxIdCliente == vetClienteSat.size())
            break;

        int cliente = vetClienteSat[proxIdCliente].cliente;
        proxIdCliente += 1;

        if(vetClientesVisitados[cliente] != int8_t(CLIENTE_NAO_VISITADO))
        {
            PRINT_DEBUG("", "");
            cout<<"ERRO, CLIENTE("<<cliente<<") DEVERIA SER NAO VISITADO!\n\n";
            throw "ERRO";
        }

//cout<<"CLIENTE SELECIONADO: "<<cliente<<"\n";

        // Cliente Selecionando

        vetCandidatos = Vector<CandidatoEV>();
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
/*
if(cliente == 82 && sat == 6)
{
    string strRota;
    evRoute.print(strRota, instancia, true);
    cout<<"\t\tROTA: "<<strRota<<"\n";
}
*/
                // Retorna a melhor insercao do cliente para evRoute
                if(!canInsert(evRoute, cliente, instancia, candidatoEv, sat, instancia.vetTempoSaida[sat], evRouteAux,
                              nullptr))
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

//cout<<"\tCANDIDATO: CLIENTE: "<<candEscolhido.clientId<<"; SAT: "<<candEscolhido.satId<<"; ROUTE ID: "<<candEscolhido.routeId<<"\n\n";
//cout<<"\n\n\t**********************\n\n";

        EvRoute &evRoute = sol.satelites[candEscolhido.satId].vetEvRoute[candEscolhido.routeId];
        string strRotaAntes;
        evRoute.print(strRotaAntes, instancia, true);

        if(!insert(evRoute, candEscolhido, instancia, instancia.vetTempoSaida[candEscolhido.satId], sol))
        {
            PRINT_DEBUG("", "ERRO!, CLIENTE("<<candEscolhido.clientId<<") JA FOI TESTADO NESSA ROTA, DEVERIA SER VIAVEL!!");
            throw "ERRO";
        }
/*
if(evRoute[1].cliente == 56)
{
    string strRota;
    evRoute.print(strRota, instancia, true);
    cout<<"ROTA COM 56: "<<strRota<<"\n";
    cout<<"ROTA ANTES: "<<strRotaAntes<<"\n";
    cout<<"ID ROTA: "<<evRoute.idRota<<"\n\n";
}

//PRINT_DEBUG("","");
cout<<"ADD CLIENTE "<<candEscolhido.clientId<<" A SOLUCAO\n";
cout<<"*********************************\n\n";
*/
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
    for(int i=instancia.getFirstClientIndex(); i <= instancia.getEndClientIndex(); ++i)
    {
        if(vetClientesVisitados[i] != int8_t(1))
            visitedClients[i] = int8_t(0);
        else
            visitedClients[i] = 1;

    }

    return sol.viavel;
}

void NS_Construtivo2::construtivo2(Solucao &sol, Instancia &instancia, const float alpha, const float beta,
                                   const Matrix<int> &matClienteSat, bool listaRestTam, bool iniSatUtil)
{

//cout<<"INICIO CONSTRUTIVO2\n\n";

    //instancia.vetVetDistClienteSatelite

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

    bool segundoNivel = construtivo2SegundoNivelEV(sol, instancia, alpha, matClienteSat, listaRestTam, beta, satUtilizados);


    Matrix<int> matClienteSat2 = matClienteSat;
    const int zero_max = max(1, instancia.numSats-2);

    if(segundoNivel)
    {
        construtivoPrimeiroNivel(sol, instancia, beta, listaRestTam, true);

        if(!sol.viavel && instancia.numSats > 2)
        {
            //cout<<"CARGAS: ";
            int numSatZero = 0;

            Vector<double> vetCargaSat;

            while(!sol.viavel)
            {

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
                sol.reseta1Nivel(instancia);

//cout<<"\tSAT "<<satMin<<" NAO SERA UTILIZADO\n";

                //sol = Solucao(instancia);
                numSatZero += 1;

/*
cout<<"VET CLIENTES VISITADOS: \n";

for(int i=0; i < sol.vetClientesAtend.size(); ++i)
    cout<<int(sol.vetClientesAtend[i])<<" ";
cout<<"\n\n";
*/

                segundoNivel = construtivo2SegundoNivelEV(sol, instancia, alpha, matClienteSat2, listaRestTam, beta, satUtilizados);
                if(segundoNivel)
                {
                    construtivoPrimeiroNivel(sol, instancia, beta, listaRestTam, true);
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
