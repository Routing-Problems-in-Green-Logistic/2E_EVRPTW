/* ****************************************
 * ****************************************
 *  Nome: ***********************
 *  Data:    25/11/22
 *  Arquivo: LocalSearch2.cpp
 * ****************************************
 * ****************************************/

#include "LocalSearch2.h"
#include "LocalSearch.h"
#include "../ViabilizadorRotaEv.h"
#include "../mersenne-twister.h"
#include "Construtivo.h"

#define PRINT_CROSS FALSE
#define PRINT_SPLIT_CARGA TRUE

using namespace NS_LocalSearch;
using namespace NS_LocalSearch2;
using namespace NS_Auxiliary;
using namespace NS_viabRotaEv;

bool NS_LocalSearch2::mvEvShifit2Nos_interRotasIntraSat(Solucao &solucao, Instancia &instancia, EvRoute &evRouteAux0, EvRoute &evRouteAux1)
{

    const int mv = MV_EV_SHIFIT_2CLIENTES_INTER_ROTAS_INTRA_SAT;

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


            double distNovaRota1 = testaRota(evRouteAux1, evRouteAux1.routeSize, instancia, false, tempoSaidaSat, 0,
                                             nullptr, nullptr);
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



                novaRota1Viavel = viabilizaRotaEv(evRouteAux1, instancia, false, insercaoEstacao, limNovaDist1, false,
                                                  tempoSaidaSat, nullptr);
                if(novaRota1Viavel)
                {
                    distNovaRota1 = insercaoEstacao.distanciaRota;
                    rotaViabilizada = true;
                }

            }
            else
            {
                distNovaRota1 = testaRota(evRouteAux1, evRouteAux1.routeSize, instancia, true, tempoSaidaSat, 0,
                                          nullptr, nullptr);
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

                double distNovaRota0 = testaRota(evRouteAux0, evRouteAux0.routeSize, instancia, true, tempoSaidaSat, 0,
                                                 nullptr, nullptr);
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

#if UTILIZA_MAT_MV
                if(solucao.vetMatSatEvMv[sat](ev0, mv) == 1 && solucao.vetMatSatEvMv[sat](ev1, mv) == 1)
                    continue;
#endif
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

                        if(resutado)
                        {
                            double novaDist = evRoute0.distancia + evRoute1.distancia;
                            solucao.distancia += -distOrig + novaDist;
                            solucao.satelites[sat].distancia += -distOrig + novaDist;

                            solucao.rotaEvAtualizada(sat, ev0);
                            solucao.rotaEvAtualizada(sat, ev1);

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

#if UTILIZA_MAT_MV
            solucao.vetMatSatEvMv[sat](ev0, mv) = 1;
#endif

        } // End for(evSat0)

    } // End for(sat1)


    return false;
}


bool NS_LocalSearch2::mvEvShifit2Nos_interRotasInterSats(Solucao &solucao, Instancia &instancia, EvRoute &evRouteAux0,
                                                         EvRoute &evRouteAux1, const float beta)

{

    const int mv = MV_EV_SHIFIT_2CLIENTES_INTER_ROTAS_INTER_SATS;

    if(instancia.numSats == 1)
        return false;

    static Solucao solucaoAux(instancia);
    solucaoAux.copia(solucao);

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


            double distNovaRota1 = testaRota(evRouteAux1, evRouteAux1.routeSize, instancia, false, tempoSaidaSat, 0,
                                             nullptr, nullptr);
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



                novaRota1Viavel = viabilizaRotaEv(evRouteAux1, instancia, false, insercaoEstacao, limNovaDist1, false,
                                                  tempoSaidaSat, nullptr);
                if(novaRota1Viavel)
                {
                    distNovaRota1 = insercaoEstacao.distanciaRota;
                    rotaViabilizada = true;
                }

            }
            else
            {
                distNovaRota1 = testaRota(evRouteAux1, evRouteAux1.routeSize, instancia, true, tempoSaidaSat, 0,
                                          nullptr, nullptr);
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

                double distNovaRota0 = testaRota(evRouteAux0, evRouteAux0.routeSize, instancia, true, tempoSaidaSat, 0,
                                                 nullptr, nullptr);
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



    for(int sat0 = 1; sat0 <= instancia.getNSats(); ++sat0)
    {

        for(int sat1=1; sat1 <= instancia.getNSats(); ++sat1)
        {
            if(sat0 == sat1)
                continue;

            for(int evSat0 = 0; evSat0 < instancia.getN_Evs(); ++evSat0)
            {
                EvRoute &evRouteSat0 = solucaoAux.satelites[sat0].vetEvRoute[evSat0];

                for(int evSat1 = 0; evSat1 < instancia.getN_Evs(); ++evSat1)
                {

#if UTILIZA_MAT_MV
                    if(solucao.vetMatSatEvMv[sat0](evSat0, mv) == 1 && solucao.vetMatSatEvMv[sat1](evSat1, mv) == 1)
                        continue;
#endif
//cout<<"ev0: "<<evSat0<<"; ev1: "<<evSat1<<"\n";

                    EvRoute &evRouteSat1 = solucaoAux.satelites[sat1].vetEvRoute[evSat1];

                    // Selecionar as posicoes das rotas
                    for(int posEvSat0 = 0; posEvSat0 < (evRouteSat0.routeSize - 1); ++posEvSat0)
                    {

                        for(int posEvSat1 = 0; posEvSat1 < (evRouteSat1.routeSize - 1); ++posEvSat1)
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


                            const double distOrig = evRouteSat0.distancia + evRouteSat1.distancia;
                            const double demandaOrig = evRouteSat0.demanda + evRouteSat1.demanda;

                            bool resutado = realizaMv(instancia, evRouteSat0, posEvSat0, evRouteSat1, posEvSat1, evRouteAux0,
                                                      evRouteAux1, evRouteSat0[0].tempoSaida);

//cout<<"\n\n############################################\n\n";

                            //if(!resutado)
                            //    resutado = realizaMv(instancia, evRouteSat1, posEvSat1, evRouteSat0, posEvSat0, evRouteAux0, evRouteAux1, evRouteSat1[0].tempoSaida);

                            if(resutado)
                            {
                                solucaoAux.recalculaDistSat(instancia);
                                //solucaoAux.distSat();

                                solucaoAux.resetaPrimeiroNivel(instancia);
                                construtivoPrimeiroNivel(solucaoAux, instancia, beta, false, Split);

                                if(solucaoAux.viavel)
                                {
                                    if(menor(solucaoAux.distancia, solucao.distancia))
                                    {
                                        //cout<<"ATUALIZACAO: "<<((solucaoAux.distancia-solucao.distancia)/solucao.distancia)*100.0<<"\n";
                                        solucao.copia(solucaoAux);
                                        solucao.rotaEvAtualizada(sat0, evSat0);
                                        solucao.rotaEvAtualizada(sat1, evSat1);

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

        }// End for(sat1)

    } // End for(sat0)


    return false;
}

int NS_LocalSearch2::cross(Instancia &instancia, EvRoute &evRoute0, int posEvRoute0, EvRoute &evRoute1, int posEvRoute1, EvRoute &evRouteAux0,
                           EvRoute &evRouteAux1, const double tempoSaidaSatRoute0, const double tempoSaidaSatRoute1)
{



    /* Todos os clientes a partir da pos (posEvRoute0+1) irão para a pos (posEvRoute1+1), e o mesmo ocorre
     * com evRoute1 na pos (posEvRoute1+1) irão para (posEvRoute0+1).
     *
     * evRoute0 nao pode ser vazio, evRoute1 pode ser vazio */

    if(evRoute0.routeSize <= 2)
        return MV_EV_ROUTE0_INVIAVEL;

    if(posEvRoute0 == posEvRoute1 && evRoute0[0].cliente == evRoute1[0].cliente)
        return MV_INVIAVEL;

    const bool evRoute1Vazio = (evRoute1.routeSize <= 2);

    if(evRoute1Vazio && posEvRoute1 != 0)
        return MV_POS_EV_ROUTE1_INVIAVEL;

    double novaCargaEvRoute0 = evRoute0.demanda;
    double novaCargaEvRoute1 = evRoute1.demanda;


    double novaDistEvRoute0 = evRoute0.distancia;
    double novaDistEvRoute1 = evRoute1.distancia;

    novaDistEvRoute0 -= instancia.getDistance(evRoute0[posEvRoute0].cliente, evRoute0[posEvRoute0+1].cliente);

    // Calcula a nova demanda e dist da evRoute0
    for(int i=(posEvRoute0+1); i < (evRoute0.routeSize-1); ++i)
    {
        novaCargaEvRoute0 -= instancia.getDemand(evRoute0[i].cliente);
        novaDistEvRoute0 -= instancia.getDistance(evRoute0[i].cliente, evRoute0[i+1].cliente);
    }

    if(!evRoute1Vazio)
    {
        novaDistEvRoute0 += instancia.getDistance(evRoute1[posEvRoute1].cliente, evRoute0[posEvRoute0+1].cliente);

        for(int i=(posEvRoute1+1); i < (evRoute1.routeSize-1); ++i)
        {
            novaCargaEvRoute0 += instancia.getDemand(evRoute1[i].cliente);
            if((i+1) < (evRoute1.routeSize-1))
                novaDistEvRoute0  += instancia.getDistance(evRoute1[i].cliente, evRoute1[i+1].cliente);
        }

        novaDistEvRoute0 += instancia.getDistance(evRoute1[evRoute1.routeSize-2].cliente, evRoute0[0].cliente);

    }

    if(novaCargaEvRoute0 > instancia.getEvCap(evRoute0.idRota))
        return MV_INVIAVEL;


    if(!evRoute1Vazio)
    {

        novaDistEvRoute1 -= instancia.getDistance(evRoute1[posEvRoute1].cliente, evRoute1[posEvRoute1+1].cliente);

        // Calcula a nova demanda e dist da evRoute1
        for(int i = (posEvRoute1 + 1); i < (evRoute1.routeSize - 1); ++i)
        {
            novaCargaEvRoute1 -= instancia.getDemand(evRoute1[i].cliente);
            novaDistEvRoute1  -= instancia.getDistance(evRoute1[i].cliente, evRoute1[i+1].cliente);
        }
    }


    for(int i=(posEvRoute0+1); i < (evRoute0.routeSize-1); ++i)
    {
        novaCargaEvRoute1 += instancia.getDemand(evRoute0[i].cliente);
        if((i+1) < (evRoute0.routeSize-1))
            novaDistEvRoute1 += instancia.getDistance(evRoute0[i].cliente, evRoute0[i+1].cliente);
    }

    novaDistEvRoute1 += instancia.getDistance(evRoute0[evRoute0.routeSize-2].cliente, evRoute1[0].cliente);


    if(novaCargaEvRoute1 > instancia.getEvCap(evRoute1.idRota))
        return MV_INVIAVEL;

    const double distRotas = evRoute0.distancia+evRoute1.distancia;

    // Cargas estao corretas

#if PRINT_CROSS
cout<<"\tDIST ORIG: "<<distRotas<<"; NOVA DIST ESTIMADA: "<<(novaDistEvRoute0+novaDistEvRoute1)<<"\n";
#endif

    // Verifica se existe melhora
    if(!menor((novaDistEvRoute0+novaDistEvRoute1), distRotas))
        return MV_INVIAVEL;
    {
#if PRINT_CROSS
cout<<"POS EV ROUTE0: "<<posEvRoute0<<"\n";
cout<<"POS EV ROUTE1: "<<posEvRoute1<<"\n";

string strRota;
evRoute0.print(strRota, instancia, true);
cout<<"\tROTA0: "<<strRota<<"\n\n";

strRota = "";
evRoute1.print(strRota, instancia, true);
cout<<"\tROTA1: "<<strRota<<"\n\n";
#endif

        // Cria a nova rota0
        // Copia evRoute0 para evRouteAux0 de 0 ate posEvRoute0(inclusive)
        copiaCliente(evRoute0.route, evRouteAux0.route, (posEvRoute0+1), 0);

#if PRINT_CROSS
evRouteAux0.routeSize = posEvRoute0+1;
strRota = "";
evRouteAux0.print(strRota, instancia, true);
cout<<"\tNOVA ROTA0 ATE POS("<<posEvRoute0<<"): "<<strRota<<"\n\n";

        // 1º Parte

cout<<"COPIA EV ROUTE1 PARA EV ROUTE AUX0:\n";
#endif

        // Copia evRoute1 para evRouteAux0 de (posEvRoute0+1) em evRouteAux0, evRoute1 apartir de (posEvRoute1+1)
        evRouteAux0.routeSize = copiaCliente(evRoute1.route, evRouteAux0.route, (posEvRoute1+1), (evRoute1.routeSize-1), (posEvRoute0+1));
        evRouteAux0[evRouteAux0.routeSize-1].cliente = evRouteAux0[0].cliente;

        // Cria a nova rota1
        // Copia evRoute1 para evRouteAux1 de 0 ate posEvRoute1(inclusive)
        copiaCliente(evRoute1.route, evRouteAux1.route, (posEvRoute1+1), 0);

#if PRINT_CROSS
evRouteAux1.routeSize = posEvRoute1+1;
strRota = "";
evRouteAux1.print(strRota, instancia, true);
cout<<"\tNOVA ROTA1 ATE POS("<<posEvRoute1<<"): "<<strRota<<"\n\n";

cout<<"COPIA EV ROUTE0 PARA EV ROUTE AUX1:\n";
#endif
        // Copia evRoute0 para evRouteAux1 de (posEvRoute1+1) em evRouteAux1, evRoute0 apartir de (posEvRoute0+1)
        evRouteAux1.routeSize = copiaCliente(evRoute0.route, evRouteAux1.route, (posEvRoute0+1), (evRoute0.routeSize-1), (posEvRoute1+1));
        evRouteAux1[evRouteAux1.routeSize-1].cliente = evRouteAux1[0].cliente;

#if PRINT_CROSS
cout<<"\n#############################################################\n\n";

strRota = "";
evRouteAux0.print(strRota, instancia, true);
cout<<"\tNOVA ROTA0: "<<strRota<<"\n";
cout<<"\tPOS ROTA0: "<<posEvRoute0<<"\n\n";


strRota = "";
evRouteAux1.print(strRota, instancia, true);
cout<<"\tNOVA ROTA1: "<<strRota<<"\n";
cout<<"\tPOS ROTA1: "<<posEvRoute1<<"\n\n";
#endif

        removeRS_Repetido(evRouteAux0, instancia, false);
        removeRS_Repetido(evRouteAux1, instancia, false);


        // Testa as rotas sem escrever as mudancas

        double distNovaRota0 = 0.0;
        double distNovaRota1 = 0.0;

        if(evRouteAux0.routeSize > 2)
            distNovaRota0 = testaRota(evRouteAux0, evRouteAux0.routeSize, instancia, false, tempoSaidaSatRoute0, 0,
                                      nullptr, nullptr);

        if(evRouteAux1.routeSize > 2)
            distNovaRota1 = testaRota(evRouteAux1, evRouteAux1.routeSize, instancia, false, tempoSaidaSatRoute1, 0,
                                      nullptr, nullptr);

        if(distNovaRota0 >= 0.0 && distNovaRota1 >= 0.0)
        {
#if PRINT_CROSS
cout<<"\tROTAS VIAVEIS; NOVA DIST: "<<(distNovaRota0+distNovaRota1)<<"\n\n";
#endif
            if(menor((distNovaRota0+distNovaRota1), distRotas))
            {
#if PRINT_CROSS
cout<<"\tNOVA DIST EH MENOR! ATUALIZANDO ROTAS AUX\n\n";
#endif
                // Escreve as mudancas nas rotas

                if(evRouteAux0.routeSize > 2)
                    distNovaRota0 = testaRota(evRouteAux0, evRouteAux0.routeSize, instancia, true, tempoSaidaSatRoute0,
                                              0, nullptr, nullptr);

                if(evRouteAux1.routeSize > 2)
                    distNovaRota1 = testaRota(evRouteAux1, evRouteAux1.routeSize, instancia, true, tempoSaidaSatRoute1,
                                              0, nullptr, nullptr);

                evRouteAux0.distancia = distNovaRota0;
                evRouteAux1.distancia = distNovaRota1;

                return MV_VIAVEL;
            }
            else
            {
#if PRINT_CROSS
cout<<"\tNOVA DIST EH MAIOR!\n";
#endif
                return MV_INVIAVEL;
            }
        }
        else if(distNovaRota0 >= 0.0 && distNovaRota1 <= 0.0)
        {
#if PRINT_CROSS
cout<<"\t\tNOVA ROTA0 EH VIAVEL. DIST: "<<distNovaRota0<<"\n";
cout<<"\t\tNOVA ROTA1 EH INVIAVEL, TENTANDO VIABILIZAR...\n";
#endif
            // Calcular a sobra para a viabilidade da novaRota1
            // distNovaRota1Max < (evRoute0.distancia+evRoute1.distancia) - (distNovaRota0)
            double distNovaRota1Max = distRotas - distNovaRota0;

#if PRINT_CROSS
cout<<"\t\tDIST NOVA ROTA1 MAX: "<<distNovaRota1Max<<"\n\n";
#endif
            InsercaoEstacao insercaoEstacao;
            bool viavel = viabilizaRotaEv(evRouteAux1, instancia, false, insercaoEstacao, distNovaRota1Max, false,
                                          tempoSaidaSatRoute1, nullptr);

            if(viavel)
            {
                if(menor((distNovaRota0+insercaoEstacao.distanciaRota), distRotas))
                {

                    if(evRouteAux0.routeSize > 2)
                        distNovaRota0 = testaRota(evRouteAux0, evRouteAux0.routeSize, instancia, true,
                                                  tempoSaidaSatRoute0, 0, nullptr, nullptr);

                    evRouteAux0.distancia = distNovaRota0;
                    insereEstacaoRota(evRouteAux1, insercaoEstacao, instancia, tempoSaidaSatRoute1);
#if PRINT_CROSS
cout<<"\t\tNOVA ROTA1 VIABILIZADA; DIST: "<<insercaoEstacao.distanciaRota<<"\n";


strRota = "";
evRouteAux1.print(strRota, instancia, true);
cout<<"\tNOVA ROTA1 VIABILIZADA: "<<strRota<<"\n\n";
#endif

                    return MV_VIAVEL;
                }
                else
                {
#if PRINT_CROSS
cout<<"\t\tNOVA ROTA1 VIABILIZADA; POREM, SUM DAS DISTS DAS NOVAS ROTAS EH MAIOR DO QUE A DIST ORIG!\n";
cout<<"\t\tDIST NOVA ROTA1: "<<insercaoEstacao.distanciaRota<<"\n";
#endif
                    return MV_INVIAVEL;
                }
            }
            else
            {
#if PRINT_CROSS
cout<<"\t\tNAO FOI POSSIVEL VIABILIZAR NOVA ROTA1\n\n";
#endif
                return MV_INVIAVEL;
            }

        }
        else if(distNovaRota1 >= 0.0 && distNovaRota0 <= 0.0)
        {
#if PRINT_CROSS
cout<<"\t\tNOVA ROTA1 EH VIAVEL. DIST: "<<distNovaRota1<<"\n";
cout<<"\t\tNOVA ROTA0 EH INVIAVEL, TENTANDO VIABILIZAR...\n";
#endif
            // Calcular a sobra para a viabilidade da novaRota0

            // distNovaRota1Max < (evRoute0.distancia+evRoute1.distancia) - (distNovaRota0)
            double distNovaRota0Max = distRotas - distNovaRota1;

#if PRINT_CROSS
cout<<"\t\tDIST NOVA ROTA1 MAX: "<<distNovaRota0Max<<"\n\n";
#endif
            InsercaoEstacao insercaoEstacao;
            bool viavel = viabilizaRotaEv(evRouteAux0, instancia, false, insercaoEstacao, distNovaRota0Max, false,
                                          tempoSaidaSatRoute0, nullptr);

            if(viavel)
            {
                if(menor((distNovaRota1+insercaoEstacao.distanciaRota), distRotas))
                {

                    if(evRouteAux1.routeSize > 2)
                        distNovaRota1 = testaRota(evRouteAux1, evRouteAux1.routeSize, instancia, true,
                                                  tempoSaidaSatRoute1, 0, nullptr, nullptr);

                    evRouteAux1.distancia = distNovaRota1;
                    insereEstacaoRota(evRouteAux0, insercaoEstacao, instancia, tempoSaidaSatRoute0);

#if PRINT_CROSS
cout<<"\t\tNOVA ROTA0 VIABILIZADA; DIST: "<<insercaoEstacao.distanciaRota<<"\n";

strRota = "";
evRouteAux0.print(strRota, instancia, true);
cout<<"\t\tNOVA ROTA0 VIABILIZADA: "<<strRota<<"\n\n";
#endif
                    return MV_VIAVEL;
                }
                else
                {
#if PRINT_CROSS
cout<<"\t\tNOVA ROTA0 VIABILIZADA; POREM, SUM DAS DISTS DAS NOVAS ROTAS EH MAIOR DO QUE A DIST ORIG!\n";
cout<<"\t\tDIST NOVA ROTA0: "<<insercaoEstacao.distanciaRota<<"\n";
#endif
                    return MV_INVIAVEL;
                }
            }
            else
                return MV_INVIAVEL;
        }
        else if(distNovaRota0 < 0.0 && distNovaRota1 < 0.0)
        {

#if PRINT_CROSS
cout<<"NOVAS ROTAS SAO INVIAVEL, TENTANDO VIABILIZAR AMBAS\n";
#endif
            // Tenta viabilizar rota0
            InsercaoEstacao insercaoEstacaoRota0;
            bool viavel = viabilizaRotaEv(evRouteAux0, instancia, true, insercaoEstacaoRota0, DOUBLE_MAX, false,
                                          tempoSaidaSatRoute0, nullptr);
            if(viavel)
            {
#if PRINT_CROSS
cout<<"\t\tNOVA ROTA0 VIABILIZADA; DIST: "<<insercaoEstacaoRota0.distanciaRota<<"\n";
strRota = "";
evRouteAux0.print(strRota, instancia, true);
cout<<"\t\tNOVA ROTA0 VIABILIZADA: "<<strRota<<"\n\n";
#endif

                double distNovaRota1Max = distRotas - insercaoEstacaoRota0.distanciaRota;

#if PRINT_CROSS
cout<<"\t\tTENTANDO VIABILIZAR NOVA ROTA1 COM DIST MAX: "<<distNovaRota1Max<<"\n";
#endif
                InsercaoEstacao insercaoEstacaoRota1;
                viavel = viabilizaRotaEv(evRouteAux1, instancia, false, insercaoEstacaoRota1, distNovaRota1Max, false,
                                         tempoSaidaSatRoute1, nullptr);

                if(viavel)
                {
                    double novaDist = insercaoEstacaoRota0.distanciaRota+insercaoEstacaoRota1.distanciaRota;
                    if(menor(novaDist, distRotas))
                    {
#if PRINT_CROSS
cout<<"\t\tNOVA ROTA1 VIABILIZADA E SUM DAS NOVAS ROTAS EH MENOR\n";
cout<<"\t\tDIST NOVA ROTA1: "<<insercaoEstacaoRota1.distanciaRota<<"\n\n";
#endif
                        insereEstacaoRota(evRouteAux0, insercaoEstacaoRota0, instancia, tempoSaidaSatRoute0);
                        insereEstacaoRota(evRouteAux1, insercaoEstacaoRota1, instancia, tempoSaidaSatRoute1);
#if PRINT_CROSS
strRota = "";
evRouteAux0.print(strRota, instancia, true);
cout<<"\t\tNOVA ROTA0 VIABILIZADA: "<<strRota<<"\n\n";
strRota = "";
evRouteAux0.print(strRota, instancia, true);
cout<<"\t\tNOVA ROTA0 VIABILIZADA: "<<strRota<<"\n\n";
#endif
                        return MV_VIAVEL;

                    }
                    else
                    {

#if PRINT_CROSS
cout<<"\t\tNOVA ROTA1 VIABILIZADA, POREM SUM NOVAS ROTAS EH MAIOR QUE DIST ORIG\n";
#endif
                    }

                }
                else
                {
#if PRINT_CROSS
cout<<"\t\tNAO FOI POSSIVEL VIABILIZAR NOVA ROTA1\n\n";
#endif
                }
            }
            else
            {
#if PRINT_CROSS
cout<<"\t\tNAO FOI POSSIVEL VIABILIZAR NOVA ROTA0\n\n";
#endif
            }

            if(!viavel)
                return MV_INVIAVEL;
        }
        else
        {
            PRINT_DEBUG("", "");
            cout<<"ERRO , CODIGO NAO DEVERIA ATINGIR ESSE ELSE!!\n";
            throw "ERRO";
        }

    }
    //else
    {

//cout<<"\tNOVA DIST ESTIMADA EH MAIOR QUE A DIST ORIG\n";
        return MV_INVIAVEL;
    }

    return MV_INVIAVEL;

}

void NS_LocalSearch2::copiaCliente(const Vector<EvNo> &vet0, Vector<EvNo> &vetDest, const int tam, const int ini)
{
    for(int i=ini; i < tam; ++i)
        vetDest[i].cliente = vet0[i].cliente;
}


int NS_LocalSearch2::copiaCliente(const Vector<EvNo> &vet0, Vector<EvNo> &vetDest, const int iniVet0, const int fimVet0, const int iniVetDest)
{
    int iVet0    = iniVet0;
    int iVetDest = iniVetDest;

#if PRINT_CROSS
cout<<"COPIA CLIENTE\n\n";
cout<<"VET0: ";
for(int i=0; i < iniVet0; ++i)
    cout<<vet0[i].cliente<<" ";
cout<<"\n\n";

cout<<"VET DEST [0:"<<iniVetDest-1<<"]: ";
for(int i=0; i < iniVetDest; ++i)
    cout<<vetDest[i].cliente<<" ";
cout<<"\n\n";
#endif

    while(iVet0 <= fimVet0)
    {
        vetDest[iVetDest].cliente = vet0[iVet0].cliente;

        ++iVet0;
        ++iVetDest;
    }

    return iVetDest;
}

// MV
bool NS_LocalSearch2::mvShifitEvs_interRotasInterSats(Solucao &solucao, Instancia &instancia, EvRoute &evRouteAux, float beta)
{
    if(instancia.numSats <= 1)
        return false;

    //cout<<"\n\n**************************\n\n";

    int sat = 1 + rand_u32()%instancia.numSats;
    const int satIni = sat;
    InsercaoEstacao insercaoEstacao;
    static Solucao tempSolucao(instancia);

    //cout<<"Num de satellites: "<<instancia.numSats<<"\n\n";

    // Percorre os satellites da solucao
    do
    {
        //cout<<"sat: "<<sat<<"\n";
        Satelite &satelite = solucao.satelites[sat];
        if(satelite.vazio())
        {
            sat += 1;
            sat = 1 + sat%instancia.numSats;
            continue;
        }

        // Percorre os EVs do sat
        for(int ev=0; ev < instancia.numEv; ++ev)
        {
            EvRoute &evRoute = satelite.vetEvRoute[ev];
            if(evRoute.routeSize <= 2)
                continue;

            const int primCli = evRoute[1].cliente;
            const int ultCli  = evRoute[evRoute.routeSize-2].cliente;

            // Dist de evRoute menos a dist ao satelite
            const double dist = evRoute.distancia - (instancia.getDistance(sat, primCli) + instancia.getDistance(ultCli, sat));

            // Percorre os sat da solucao
            for(int satId=0; satId < (instancia.numSats-1); ++satId)
            {
                const int satOutro = (instancia.vetVetDistSatSat[sat])[satId].satelite;
                Satelite &sateliteOutro = solucao.satelites[satOutro];

                if(satOutro <= 0 || satOutro > instancia.numSats)
                {
                    PRINT_DEBUG("", "");
                    cout<<"sat("<<satOutro<<") eh invalido!\n";
                    throw "ERRO";
                }

                double tempTempoSaidaSat = instancia.getDistance(0, satOutro);
                if(!sateliteOutro.vazio())
                {
                    for(int ev=0; ev < instancia.numEv; ++ev)
                    {
                        if(sateliteOutro.vetEvRoute[ev].routeSize > 2)
                        {
                            tempTempoSaidaSat = sateliteOutro.vetEvRoute[ev].route[0].tempoSaida;
                            break;
                        }
                    }
                }

                const double tempoSaidaSat = tempTempoSaidaSat;

                const double newDist = dist + instancia.getDistance(satOutro, primCli) + instancia.getDistance(ultCli, satOutro);
//cout<<"nova dist: "<<newDist<<"; dist evRoute: "<<evRoute.distancia<<"\n";
                if(!menor(newDist, evRoute.distancia))
                    continue;

                // Verifica se a nova rota eh viavel
                evRouteAux.copia(evRoute, false, nullptr);
                evRouteAux.route[0].cliente = satOutro;
                evRouteAux.route[evRoute.routeSize-1].cliente = satOutro;
                evRouteAux.satelite = satOutro;
                double distNovaRota = testaRota(evRouteAux, evRouteAux.routeSize, instancia, true, tempoSaidaSat, 0, nullptr, nullptr);
//cout<<"\tapos testar rota: "<<distNovaRota<<"\n\n";
                if(distNovaRota <= 0.0)
                {
                    if(viabilizaRotaEv(evRouteAux, instancia, false, insercaoEstacao, evRoute.distancia, false, tempTempoSaidaSat, nullptr))
                    {
                        distNovaRota = insercaoEstacao.distanciaRota;
                        if(!menor(distNovaRota, evRoute.distancia))
                            continue;

                        // Adiciona estacao a evRouteAux
                        insereEstacaoRota(evRouteAux, insercaoEstacao, instancia, tempTempoSaidaSat);

                    }
                    else
                        continue;
                }

                evRouteAux.distancia = distNovaRota;

                // Testar se o primeiro nivel eh viavel
                tempSolucao.copia(solucao);

                // Encontra um Ev vazio em outroSat
                int evVazio = -1;
                Satelite &satOutroTempSol = tempSolucao.satelites[satOutro];

                for(int i=0; i < instancia.numEv; ++i)
                {
                    EvRoute &evRouteTemp = satOutroTempSol.vetEvRoute[i];
                    if(evRouteTemp.routeSize <= 2)
                    {
                        evVazio = i;
                        break;
                    }
                }

                if(evVazio == -1)
                    continue;

                // Corrigi os satelites
                Satelite &satTempSol = tempSolucao.satelites[sat];

                // Corrigir evRoute de sat
                EvRoute &evRouteTempSol = satTempSol.vetEvRoute[ev];
                satTempSol.distancia    -= evRouteTempSol.distancia;
                satTempSol.demanda      -= evRouteTempSol.demanda;

                tempSolucao.distancia   -= evRouteTempSol.distancia;
                evRouteTempSol.resetaEvRoute();

                // Copiar evRouteTemp para evVazio em temoSolucao
                EvRoute &evRouteTempOutro = satOutroTempSol.vetEvRoute[evVazio];
                const int idRota = evRouteTempOutro.idRota;
                evRouteTempOutro.copia(evRouteAux, false, nullptr);
                evRouteTempOutro.idRota = idRota;

                satOutroTempSol.demanda += evRouteTempOutro.demanda;
                satOutroTempSol.distancia += evRouteTempOutro.distancia;
                tempSolucao.distancia     += evRouteTempOutro.distancia;

                tempSolucao.reseta1Nivel(instancia);
                NS_Construtivo::construtivoPrimeiroNivel(tempSolucao, instancia, beta, false, Split);

                if(!tempSolucao.viavel)
                {   //cout<<"Primeiro nivel inviavel\n";
                    continue;
                }

                tempSolucao.recalculaDist();

                if(menor(tempSolucao.distancia, solucao.distancia))
                {
                    string strRota;
                    evRouteAux.print(strRota, instancia, true);
                    //cout<<"Rota alterada: "<<strRota<<"\n\n";

                    string strSol;
                    tempSolucao.print(strSol, instancia, false);
                    //cout<<"Nova SOLUCAO: "<<strSol<<"\n";

                    solucao.copia(tempSolucao);
                    //cout<<"sat: "<<sat<<"; satNovo: "<<satOutro<<"\n\n";
                    return true;
                }
                else
                {
                    //cout<<"Nova sol eh Maior: "<<tempSolucao.distancia<<"; "<<solucao.distancia<<"\n";
                }
            }
        }

        sat += 1;
        sat = 1 + sat%instancia.numSats;
    }
    while(sat != satIni);


    return false;
}
