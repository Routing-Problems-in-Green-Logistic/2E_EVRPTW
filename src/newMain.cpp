// Erro MV 1:
// run ../../instancias/2e-vrp-tw/Customer_15/C202_C15x.txt --execTotal 2 --pasta 'resultados' --resulCSV 'resultados.csv' --execAtual 0 --mt G --seed 1663599262

/* ****************************************
 * ****************************************
 *  Nome:    Igor de Andrade Junqueira
 *  Data:    14/09/22
 *  Arquivo: newMain.cpp
 * ****************************************
 * ****************************************/

#include <iostream>
#include <chrono>
#include "Parametros.h"
#include "Instance.h"
#include "Aco.h"
#include "PreProcessamento.h"
#include "k_means.h"

using namespace std;
using namespace NS_parametros;
using namespace NameS_Grasp;
using namespace N_PreProcessamento;
using namespace N_Aco;
using namespace N_k_means;

void aco(Instance &instancia, Parametros &parametros, ParametrosGrasp &parm, Solucao &best);
void grasp(Instance &instancia, Parametros &parametros, Solucao &best);
void setParamGrasp(Instance &instancia, ParametrosGrasp &parametrosGrasp);

int main(int argc, char* argv[])
{
    Parametros parametros;
    string commite = "4894234d73f2188c280429fc9c1c6d5174c0f212";
    parametros.commit = commite;

    try
    {

        if(argc == 1)
        {
            cout<<"NUMERO DE ARGUMENTOS EH INSUFICIENTE.\n";
            cout<<"COMMITE: "<<commite<<"\n";
            cout<<string(argv[0])<<"\n";
            exit(-1);
        }

        caregaParametros(parametros, argc-1, &argv[1]);
        Instance instancia(parametros.instancia, parametros.nomeInstancia);


        dijkstraSatCli(instancia);
        instancia.calculaVetVoltaRS_sat();



/*            vector<int> vetSatAtendCliente(instancia.numNos, -1);
            vector<int> satUtilizado(instancia.numSats+1, 0);

            const ublas::matrix<int> matClienteSat =  k_means(instancia, vetSatAtendCliente, satUtilizado, false);

            cout<<"\t";
            for(int sat=instancia.getFirstSatIndex(); sat <= instancia.getEndSatIndex(); ++sat)
                cout<<sat<<"\t";

            cout<<"\n\n";

            for(int i=instancia.getEndSatIndex()+1; i < instancia.numNos; ++i)
            {
                cout<<i<<":\t";
                for(int sat=instancia.getFirstSatIndex(); sat <= instancia.getEndSatIndex(); ++sat)
                {
                    cout<<matClienteSat(i, sat)<<"\t";
                }

                cout<<"\n";
            }

            return 0;*/


            //calculaRaioSatSeedK_means(instancia);
            //return 0;

        ParametrosGrasp parametrosGrasp;
        setParamGrasp(instancia, parametrosGrasp);
        Solucao best(instancia);

        auto start = std::chrono::high_resolution_clock::now();

        switch(parametros.metodo)
        {
            case METODO_ACO:
                aco(instancia, parametros, parametrosGrasp, best);
                break;

            case METODO_GRASP:
                grasp(instancia, parametros, best);
                break;
            default:
                cout<<"METODO: "<<parametros.metodo<<" NAO EXISTE\n"<<parametros.getParametros()<<"\n\n";
                throw "ERRO";
                break;
        }

        auto end = std::chrono::high_resolution_clock::now();

        ParametrosSaida parametrosSaida = getParametros();
        setParametrosSaida(parametrosSaida, parametros, best, start, end);
        saida(parametros, parametrosSaida, best, instancia);


    }
    catch(const char *erro)
    {
        cout<<"\n\n*************************************************\n\nCOMMITE: "<<parametros.commit<<"\n";
        cout<<"Compilado em: "<<__DATE__<<", "<<__TIME__<<".\n";
        cout<<"SEMENTE: "<<parametros.semente<<"\n\n";

        std::cerr<<"CATCH ERRO\n";
        std::cerr<<erro<<"\n\n";
        //std::cerr<<"Semente: "<<semente<<"\n";
        exit(-1);
    }

    return 0;
}

void aco(Instance &instancia, Parametros &parametros, ParametrosGrasp &parm, Solucao &best)
{
    //cout<<"ACO\n\n";

    AcoParametros acoParm;
    AcoEstatisticas acoEst;
    Estatisticas est;

    N_Aco::acoSol(instancia, acoParm, acoEst, parm, est, best);


}

void grasp(Instance &instancia, Parametros &parametros, Solucao &best)
{
    //cout<<"GRASP\n\n";

    ParametrosGrasp parametrosGrasp;
    setParamGrasp(instancia, parametrosGrasp);
    Estatisticas estatisticas;


    vector<int> vetSatAtendCliente(instancia.numNos, -1);
    vector<int> satUtilizado(instancia.numSats+1, 0);
    const ublas::matrix<int> matClienteSat =  k_means(instancia, vetSatAtendCliente, satUtilizado, false);

    Solucao *solGrasp = grasp(instancia, parametrosGrasp, estatisticas, false, matClienteSat);
    best.copia(*solGrasp);
    delete solGrasp;

}

void setParamGrasp(Instance &instancia, ParametrosGrasp &parametrosGrasp)
{
    const std::vector<float> vetAlfa{0.1, 0.3, 0.5, 0.7, 0.9};
    int num = min(instancia.getN_Evs() / 2, 8);
    if(num == 0)
        num = 1;

    parametrosGrasp = ParametrosGrasp(1000, 200, vetAlfa, 150, num, 0.1);
}
