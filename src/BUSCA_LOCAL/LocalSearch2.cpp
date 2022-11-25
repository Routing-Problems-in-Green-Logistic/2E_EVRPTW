/* ****************************************
 * ****************************************
 *  Nome:    Igor de Andrade Junqueira
 *  Data:    25/11/22
 *  Arquivo: LocalSearch2.cpp
 * ****************************************
 * ****************************************/

#include "LocalSearch2.h"
#include "../LocalSearch.h"
#include "../Auxiliary.h"
#include "../ViabilizadorRotaEv.h"

using namespace NS_LocalSearch;
using namespace NS_LocalSearch2;
using namespace NS_Auxiliary;
using namespace NS_viabRotaEv;

bool NS_LocalSearch2::mvEvShifit2Nos_interRotasIntraSat(Solucao &solucao, Instancia &instancia, EvRoute &evRouteAux0, EvRoute &evRouteAux1)
{


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


        // clientes em ((posEvRoute0+1) e (posEvRoute0+2)) ira para (posEvRoute1+1) da rota evRoute1
        // evRoute0 nao pode ser vazio e verificar nova carga de evRoute1

        if((evRoute0.routeSize <= 2))// || instancia.isRechargingStation(evRoute0[(posEvRoute0+1)].cliente))
            return false;

        if(evRoute1.routeSize <=2 && posEvRoute1 != 0)
            return false;

        if(evRoute0.routeSize == 3 && evRoute1.routeSize == 2)
            return false;

        if(posEvRoute0 > (evRoute0.routeSize-4))
            return false;

        const int cliente0 = evRoute0[posEvRoute0+1].cliente;
        const int cliente1 = evRoute0[posEvRoute0+2].cliente;

        if(instancia.isRechargingStation(cliente0) && instancia.isRechargingStation(cliente1))
            return false;

        // Verifica carga de evRoute1
        const double novaCargaEvRoute1 = evRoute1.demanda+instancia.vectCliente[cliente0].demanda+instancia.vectCliente[cliente1].demanda;
        if(novaCargaEvRoute1 > instancia.vectVeiculo[evRoute1.idRota].capacidade)
            return false;


        // Calcula distancia
        const double distOrig = evRoute0.distancia+evRoute1.distancia;

        // Calcula nova distancia
        double novaDist = distOrig;

//cout<<"evRoute0[posEvRoute0+2].cliente: "<<evRoute0[posEvRoute0+2].cliente<<"\n";

        novaDist += -(instancia.getDistance(evRoute0[posEvRoute0].cliente, cliente0) +
                      instancia.getDistance(cliente1, evRoute0[posEvRoute0+3].cliente));

        novaDist += instancia.getDistance(evRoute0[posEvRoute0].cliente, evRoute0[posEvRoute0+3].cliente);

        novaDist += -(instancia.getDistance(evRoute1[posEvRoute1].cliente, evRoute1[posEvRoute1+1].cliente));

        novaDist +=   instancia.getDistance(evRoute1[posEvRoute1].cliente, cliente0);

        novaDist +=   instancia.getDistance(cliente1, evRoute1[posEvRoute1+1].cliente);

        // Verifica se novaDist < distOrig
        if(menor(novaDist,distOrig))
        {
//cout<<"novaDist("<<novaDist<<") < distOrig("<<distOrig<<")!\n";

            // Copia evRoute1 para evRouteAux1 e add cliente
            evRouteAux1.copia(evRoute1, true, &instancia);
            shiftVectorClienteDir(evRouteAux1.route, (posEvRoute1+1), 2, evRouteAux1.routeSize);
            evRouteAux1[posEvRoute1+1].cliente = cliente0;
            evRouteAux1[posEvRoute1+2].cliente = cliente1;
            evRouteAux1.routeSize += 2;

            removeRS_Repetido(evRouteAux1, instancia, false);


/*
string strRota1;
evRoute1.print(strRota1, instancia, true);
cout<<"evRoute1: "<<strRota1<<"\n";
*/

/*string strRota1 = "";
evRouteAux1.print(strRota1, instancia, true);
cout<<"nova evRoute1: "<<strRota1<<"\n";*/


            double distNovaRota1 = testaRota(evRouteAux1, evRouteAux1.routeSize, instancia, false, tempoSaidaSat, 0, nullptr);
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
                estNovaDist0 += -(instancia.getDistance(evRoute0[posEvRoute0].cliente, cliente0) +
                                  instancia.getDistance(cliente1, evRoute0[posEvRoute0+3].cliente));
                estNovaDist0 += instancia.getDistance(evRoute0[posEvRoute0].cliente, evRoute0[posEvRoute0+3].cliente);
                double limNovaDist1 = evRoute0.distancia+evRoute1.distancia - estNovaDist0;



                novaRota1Viavel = viabilizaRotaEv(evRouteAux1, instancia, false, insercaoEstacao, limNovaDist1, false, tempoSaidaSat);
                if(novaRota1Viavel)
                {
                    distNovaRota1 = insercaoEstacao.distanciaRota;
                    rotaViabilizada = true;
                }

            }
            else
            {
                distNovaRota1 = testaRota(evRouteAux1, evRouteAux1.routeSize, instancia, true, tempoSaidaSat, 0, nullptr);
                evRouteAux1.distancia = distNovaRota1;
            }

            if(novaRota1Viavel)
            {
/*strRota = "";
evRouteAux1.print(strRota, instancia, false);
cout<<"evRoute0: "<<strRota<<"\n";*/

                evRouteAux0.copia(evRoute0, true, &instancia);
                shiftVectorClienteEsq(evRouteAux0.route, posEvRoute0+2, evRouteAux0.routeSize);
                evRouteAux0.routeSize -= 1;

                shiftVectorClienteEsq(evRouteAux0.route, posEvRoute0+1, evRouteAux0.routeSize);
                evRouteAux0.routeSize -= 1;

                removeRS_Repetido(evRouteAux0, instancia, false);

/*strRota="";
evRouteAux0.print(strRota, instancia, false);
cout<<"nova evRoute0: "<<strRota<<"\n\n";*/

                double distNovaRota0 = testaRota(evRouteAux0, evRouteAux0.routeSize, instancia, true, tempoSaidaSat, 0, nullptr);
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
                    evRoute0.atualizaParametrosRota(instancia);

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
                    evRoute1.atualizaParametrosRota(instancia);

                    return true;

                }

                else
                    return false;
            }
        }

        return false;
    };



    for(int sat = 1; sat <= instancia.getNSats(); ++sat)
    {

        for(int ev0=0; ev0 < instancia.getN_Evs(); ++ev0)
        {
            EvRoute &evRoute0 = solucao.satelites[sat].vetEvRoute[ev0];

            for(int ev1=0; ev1 < instancia.getN_Evs(); ++ev1)
            {
                if(ev0 == ev1)
                    continue;

//cout<<"ev0: "<<evSat0<<"; ev1: "<<evSat1<<"\n";

                EvRoute &evRoute1 = solucao.satelites[sat].vetEvRoute[ev1];

                // Selecionar as posicoes das rotas
                for(int posEv0=0; posEv0 < (evRoute0.routeSize-1); ++posEv0)
                {

                    for(int posEv1=0; posEv1 < (evRoute1.routeSize-1); ++posEv1)
                    {

                        /* ******************************************************************************************
                         * ******************************************************************************************
                         *  1º
                         *     O cliente na posicao (posEv0+1) do ev0 ira para a posicao (posEv1+1)
                         *   ev0 nao pode estar vazio,  cliente nao pode ser rs, verificar a nova carga de evSat1
                         *
                         *  2º
                         *     O cliente na posicao (posEv1+1) do ev ev1 ira para a posicao (posEv0+1)
                         *   ev1 nao pode estar vazio, e cliente nao pode ser rs, verificar a nova carga de ev0
                         *
                         * ********************************************************************************************
                         * ********************************************************************************************/


                        const double distOrig    = evRoute0.distancia + evRoute1.distancia;
                        const double demandaOrig = evRoute0.demanda + evRoute1.demanda;

                        bool resutado = realizaMv(instancia, evRoute0, posEv0, evRoute1, posEv1, evRouteAux0, evRouteAux1, evRoute0[0].tempoSaida);

//cout<<"\n\n############################################\n\n";

                        //if(!resutado)
                        //    resutado = realizaMv(instancia, evRouteSat1, posEvSat1, evRouteSat0, posEvSat0, evRouteAux0, evRouteAux1, evRouteSat1[0].tempoSaida);

                        if(resutado)
                        {
                            double novaDist = evRoute0.distancia + evRoute1.distancia;
                            solucao.distancia += -distOrig + novaDist;
                            solucao.satelites[sat].distancia += -distOrig + novaDist;
                            //cout<<"MV UPDATE\n";

                            double novaDemanda = evRoute0.demanda + evRoute1.demanda;
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

        } // End for(evSat0)

    } // End for(sat1)


    return false;
}


/**
 *
 *  rota0 -> rota1
 *                 *
 *  rota0: 1 5  6  7  8  9  1
 *  rota1: 1 11 14 16 17 18 1
 *
 *  posRota0 = 0; :: clientes 5 6
 *  posRota0 <= 3
 *  posRota0 <= (size(rota0)-4)
 *
 *
 *
 *
 */