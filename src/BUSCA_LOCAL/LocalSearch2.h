/* ****************************************
 * ****************************************
 *  Nome: ***********************
 *  Data:    25/11/22
 *  Arquivo: LocalSearch2.h
 * ****************************************
 * ****************************************/

#ifndef INC_2E_EVRP_LOCALSEARCH2_H
#define INC_2E_EVRP_LOCALSEARCH2_H

#define MV_INVIAVEL                 0
#define MV_VIAVEL                   1
#define MV_POS_EV_ROUTE0_INVIAVEL   2
#define MV_POS_EV_ROUTE1_INVIAVEL   3
#define MV_EV_ROUTE0_INVIAVEL       4
#define MV_EV_ROUTE1_INVIAVEL       5

#include "../Solucao.h"
#include "Instancia.h"
#include "../CONSTRUTIVO/Construtivo.h"

namespace NS_LocalSearch2
{
    bool mvShifitEvs_interRotasInterSats(Solucao &solucao, Instancia &instancia, EvRoute &evRouteAux, float beta);

    bool mvEvShifit2Nos_interRotasIntraSat(Solucao &solucao, Instancia &instancia, EvRoute &evRouteAux0, EvRoute &evRouteAux1);
    bool mvEvShifit2Nos_interRotasInterSats(Solucao &solucao, Instancia &instancia, EvRoute &evRouteAux0,
                                            EvRoute &evRouteAux1, float beta);

    int cross(Instancia &instancia, EvRoute &evRoute0, int posEvRoute0, EvRoute &evRoute1, int posEvRoute1, EvRoute &evRouteAux0,
              EvRoute &evRouteAux1, double tempoSaidaSatRoute0, double tempoSaidaSatRoute1);

    void copiaCliente(const Vector<EvNo> &vet0, Vector<EvNo> &vetDest, int tam, int ini=0);
    int copiaCliente(const Vector<EvNo> &vet0, Vector<EvNo> &vetDest, int iniVet0, int fimVet0, int iniVetDest);

    template<typename Func>
    bool mvInterRotasIntraSat(Solucao &solucao, Instancia &instancia, EvRoute &evRouteAux0, EvRoute &evRouteAux1, Func func, const int mv)
    {

        int resutado = 0;

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
                    EvRoute &evRoute1 = solucao.satelites[sat].vetEvRoute[ev1];

                    // Selecionar as posicoes das rotas
                    for(int posEv0=0; posEv0 < (evRoute0.routeSize-1); ++posEv0)
                    {

                        for(int posEv1=0; posEv1 < (evRoute1.routeSize-1); ++posEv1)
                        {

                            const double distOrig    = evRoute0.distancia+evRoute1.distancia;
                            const double demandaOrig = evRoute0.demanda+evRoute1.demanda;

                            resutado = func(instancia, evRoute0, posEv0, evRoute1, posEv1, evRouteAux0, evRouteAux1, evRoute0[0].tempoSaida, evRoute1[0].tempoSaida);

                            if(resutado == MV_POS_EV_ROUTE0_INVIAVEL || resutado == MV_POS_EV_ROUTE1_INVIAVEL)
                                break;

                            if(resutado == 1)
                            {
                                const double novaDist = evRouteAux0.distancia + evRouteAux1.distancia;
                                solucao.distancia += -distOrig + novaDist;
                                solucao.satelites[sat].distancia += -distOrig + novaDist;

                                evRouteAux0.satelite = evRoute0.satelite;
                                evRouteAux0.idRota   = evRoute0.idRota;


                                evRouteAux1.satelite = evRoute1.satelite;
                                evRouteAux1.idRota   = evRoute1.idRota;

                                evRoute0.copia(evRouteAux0, true, &instancia);
                                evRoute1.copia(evRouteAux1, true, &instancia);

                                evRoute0.distancia = evRouteAux0.distancia;
                                evRoute1.distancia = evRouteAux1.distancia;

                                evRoute0.atualizaParametrosRota(instancia);
                                evRoute1.atualizaParametrosRota(instancia);


#if UTILIZA_MAT_MV
                                solucao.rotaEvAtualizada(sat, ev0);
                                solucao.rotaEvAtualizada(sat, ev1);
#endif

//cout<<"MV UPDATE\n";

                                double novaDemanda = evRoute0.demanda + evRoute1.demanda;
                                if(novaDemanda != demandaOrig)
                                {
                                    PRINT_DEBUG("","");
                                    cout<<"ERRO! NOVA DEMANDA("<<novaDemanda<<") != DEMANDA ORIGINAL("<<demandaOrig<<")\n";
                                    throw "ERRO";
                                }
//cout<<"\n\n****************************************************\n****************************************************\n\n";
                                return true;
                            }



                        } // End for(posEvSat1)


                        if(resutado == MV_POS_EV_ROUTE0_INVIAVEL || resutado == MV_POS_EV_ROUTE1_INVIAVEL)
                            break;

                    } // End for(posEvSat0)


                    if(resutado == MV_POS_EV_ROUTE0_INVIAVEL)
                        break;

                    if(resutado == MV_POS_EV_ROUTE1_INVIAVEL)
                        continue;

                } // End for(evSat1)


#if UTILIZA_MAT_MV
                solucao.vetMatSatEvMv[sat](ev0, mv) = 1;
#endif

                if(resutado == MV_POS_EV_ROUTE0_INVIAVEL)
                    continue;

            } // End for(evSat0)

        } // End for(sat1)

        return false;
    }

    template<typename Func>
    bool mvInterRotasInterSats(Solucao &solucao, Instancia &instancia, EvRoute &evRouteAux0, EvRoute &evRouteAux1, Func func, const float beta, const int mv)
    {


        int resutado = 0;

        for(int sat0 = 1; sat0 <= instancia.getNSats(); ++sat0)
        {


            for(int sat1 = 1; sat1 <= instancia.getNSats(); ++sat1)
            {
                if(sat0 == sat1)
                    continue;

                for(int evSat0=0; evSat0 < instancia.getN_Evs(); ++evSat0)
                {
                    EvRoute &evRouteSat0 = solucao.satelites[sat0].vetEvRoute[evSat0];


                        for(int evSat1 = 0; evSat1 < instancia.getN_Evs(); ++evSat1)
                        {

#if UTILIZA_MAT_MV
                            if(solucao.vetMatSatEvMv[sat0](evSat0, mv) == 1 && solucao.vetMatSatEvMv[sat1](evSat1, mv) == 1)
                                continue;
#endif

                            EvRoute &evRouteSat1 = solucao.satelites[sat1].vetEvRoute[evSat1];

                            // Selecionar as posicoes das rotas
                            for(int posEv0 = 0; posEv0 < (evRouteSat0.routeSize - 1); ++posEv0)
                            {

                                for(int posEv1 = 0; posEv1 < (evRouteSat1.routeSize - 1); ++posEv1)
                                {

                                    const double distOrig = evRouteSat0.distancia + evRouteSat1.distancia;
                                    const double demandaOrig = evRouteSat0.demanda + evRouteSat1.demanda;

                                    resutado = MV_INVIAVEL;
                                    resutado = func(instancia, evRouteSat0, posEv0, evRouteSat1, posEv1, evRouteAux0, evRouteAux1,
                                                    evRouteSat0[0].tempoSaida, evRouteSat1[0].tempoSaida);

                                    if(resutado == MV_POS_EV_ROUTE0_INVIAVEL || resutado == MV_POS_EV_ROUTE1_INVIAVEL)
                                        break;

                                    if(resutado == 1)
                                    {
                                        static Solucao solucaoCopia(instancia);
                                        solucaoCopia.copia(solucao);

                                        EvRoute &evRoute0 = solucaoCopia.satelites[sat0].vetEvRoute[evSat0];
                                        EvRoute &evRoute1 = solucaoCopia.satelites[sat1].vetEvRoute[evSat1];

                                        const double novaDist = evRouteAux0.distancia + evRouteAux1.distancia;
                                        solucaoCopia.distancia += -distOrig + novaDist;
                                        solucaoCopia.satelites[sat0].distancia += -distOrig + novaDist;

                                        evRouteAux0.satelite = evRoute0.satelite;
                                        evRouteAux0.idRota = evRoute1.idRota;


                                        evRouteAux1.satelite = evRoute1.satelite;
                                        evRouteAux1.idRota = evRoute1.idRota;

                                        evRoute0.copia(evRouteAux0, true, &instancia);
                                        evRoute1.copia(evRouteAux1, true, &instancia);

                                        evRoute0.distancia = evRouteAux0.distancia;
                                        evRoute1.distancia = evRouteAux1.distancia;

                                        evRoute0.atualizaParametrosRota(instancia);
                                        evRoute1.atualizaParametrosRota(instancia);


//cout<<"MV UPDATE\n";

                                        double novaDemanda = evRoute0.demanda + evRoute1.demanda;
                                        if(novaDemanda != demandaOrig)
                                        {
                                            PRINT_DEBUG("", "");
                                            cout << "ERRO! NOVA DEMANDA(" << novaDemanda << ") != DEMANDA ORIGINAL("
                                                 << demandaOrig << ")\n";

                                            throw "ERRO";
                                        }

                                        solucaoCopia.recalculaDistSat(instancia);
                                        solucaoCopia.resetaPrimeiroNivel(instancia);
                                        NS_Construtivo::construtivoPrimeiroNivel(solucaoCopia, instancia, beta, false,
                                                                                 true);

                                        if(solucaoCopia.viavel && NS_Auxiliary::menor(solucaoCopia.distancia, solucao.distancia))
                                        {
//cout<<"MV UPDATE\n";
                                            solucao.copia(solucaoCopia);


#if UTILIZA_MAT_MV
                                            solucao.rotaEvAtualizada(sat0, evSat0);
                                            solucao.rotaEvAtualizada(sat1, evSat1);
#endif
                                            return true;
                                        }
                                    }

                                } // End for(posEvSat1)

                                if(resutado == MV_POS_EV_ROUTE0_INVIAVEL || resutado == MV_POS_EV_ROUTE1_INVIAVEL)
                                    break;

                            } // End for(posEvSat0)


                            if(resutado == MV_POS_EV_ROUTE0_INVIAVEL)
                                break;

                            if(resutado == MV_POS_EV_ROUTE1_INVIAVEL)
                                continue;

                        } // End for(evSat1)


#if UTILIZA_MAT_MV
                        solucao.vetMatSatEvMv[sat0](evSat0, mv) = 1;
#endif

                        if(resutado == MV_POS_EV_ROUTE0_INVIAVEL)
                            continue;

                    } // End for(evSat0)

                } // End for(sat1)

        } // End for(sat0)

        return false;
    }

    //bool mvSplitCarga(Solucao &satId, Instancia &instancia);

    class RotaTempoCh
    {
    public:
        int rotaId   = -1;
        double tempoCh = 0.0;
        RotaTempoCh()=default;
        RotaTempoCh(int rota, double tempo){rotaId = rota; tempoCh = tempo;}

        friend bool operator < (const RotaTempoCh &rota0, const RotaTempoCh &rota1)
        {
            return rota0.tempoCh > rota1.tempoCh;
        }
    };

    class VetRotaTempoCh
    {
    public:
        Vector<RotaTempoCh> vet;
        VetRotaTempoCh()
        {
            vet.reserve(2);
        }
        void sort()
        {
            std::sort(vet.begin(), vet.end());
        }
    };
}

#endif //INC_2E_EVRP_LOCALSEARCH2_H
