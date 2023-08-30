/*
const int Tam =10000;

#include <chrono>
#include "Teste.h"
#include "Construtivo.h"
#include "LocalSearch.h"
#include "mersenne-twister.h"


using namespace NameTeste;
using namespace NS_Construtivo;



void NameTeste::testeMovimentos(string &saida, const Instance &instance, long semente, int k)
{

    seed(semente);

    saida = "";
    MovimentoEst vetMv[4];


    for(int i=0; i<Tam; ++i)
    {


        auto start = std::chrono::high_resolution_clock::now();

            Solucao copia(instance);
            construtivo(copia, instance, 0.4, 0.4);

            string erro;
            if(copia.viavel)
            {
                if(!copia.checkSolution(erro, instance))
                {
                    cout << erro << "\n\nERRO*********\n";
                    //solution.print(erro);
                    //cout << "\n\n\n"<<erro << "\n\nERRO*********\n";
                    continue;
                }
            }
auto start = std::chrono::high_resolution_clock::now();
        auto end = std::chrono::high_resolution_clock::now();

        std::chrono::duration<double> tempoAux = end - start;
        double tempo = tempoAux.count();

        if(copia.viavel)
        {
            vetMv[T_HEURISTICA].soma(copia.calcCost(instance), tempo);

            //for(int k=1; k < T_TAM; ++k)
            if(k > 0)
            {


                bool mv = true;
                float somaTempo = 0.0;
                bool erroB = false;

                while(mv)
                {

                    start = std::chrono::high_resolution_clock::now();
                    switch(k)
                    {
                        case T_MV_SHIFIT_INTRA_ROTA:
                            mv = NS_LocalSearch::mvShifitIntraRota(copia, instance);
                            break;

                        case T_MV_SHIFIT_INTER_ROTAS_INTRA_SAT:
                            mv = NS_LocalSearch::mvShiftInterRotasIntraSatellite(copia, instance);
                            break;

                        case T_MV_CROSS_INTRA_SAT:
                            mv = NS_LocalSearch::mvCrossIntraSatellite(copia, instance);
                            break;

                    }
                    end = std::chrono::high_resolution_clock::now();


                    std::chrono::duration<double> tempoAux = end - start;
                    somaTempo += tempoAux.count() + tempo;

                    erro = "";
                    if(!copia.checkSolution(erro, instance))
                    {
                        cout << erro << "\n\nERRO*********\n";
                        //solution.print(erro);
                        //cout << "\n\n\n"<<erro << "\n\nERRO*********\n";
                        erroB = true;
                        break;
                    }

                }

                if(!erroB)
                {

                    vetMv[k].soma(copia.calcCost(instance), somaTempo);
                }

            }

        }



    }

    saida = "";
    //saida += vetMv[0].print("heuristica");

    //for(int k=0; k < T_TAM; ++k)
    {

        switch(k)
        {

            case T_HEURISTICA:
                saida += vetMv[k].print("heuristica");
                break;

            case T_MV_SHIFIT_INTRA_ROTA:
                saida += vetMv[k].print("shifit_intra_rota");
                break;

            case T_MV_SHIFIT_INTER_ROTAS_INTRA_SAT:
                saida += vetMv[k].print("shifit_inter_rotas_intra_sat");
                break;

            case T_MV_CROSS_INTRA_SAT:
                saida += vetMv[k].print("cross_intra_sat");
                break;

        }
    }


}

*/