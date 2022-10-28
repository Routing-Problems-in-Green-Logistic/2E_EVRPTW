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
#include "Instancia.h"
#include "Aco.h"
#include "PreProcessamento.h"
#include "k_means.h"
#include "Vnd.h"

using namespace std;
using namespace NS_parametros;
using namespace NameS_Grasp;
using namespace N_PreProcessamento;
using namespace N_Aco;
using namespace N_k_means;

void aco(Instancia &instancia, Parametros &parametros, ParametrosGrasp &parm, Solucao &best);
void grasp(Instancia &instancia, Parametros &parametros, Solucao &best);
void setParamGrasp(Instancia &instancia, ParametrosGrasp &parametrosGrasp);

namespace N_gamb
{
    std::vector<NS_vnd::MvValor> vetMvValor;
    std::vector<NS_vnd::MvValor> vetMvValor1Nivel;
}

int main(int argc, char* argv[])
{
    Parametros parametros;
    string commite = "2232777be04a2a6c6d68d2d89697a5acdfe0d36a";
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
        Instancia instancia(parametros.instancia, parametros.nomeInstancia);

        dijkstraSatCli(instancia);
        instancia.calculaVetVoltaRS_sat();

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
        setParametrosSaida(parametrosSaida, parametros, best, start, end, N_gamb::vetMvValor, N_gamb::vetMvValor1Nivel);
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

void aco(Instancia &instancia, Parametros &parametros, ParametrosGrasp &parm, Solucao &best)
{
    //cout<<"ACO\n\n";

    AcoParametros acoParm;
    AcoEstatisticas acoEst;
    Estatisticas est;

    N_Aco::acoSol(instancia, acoParm, acoEst, parm, est, best);


}

void grasp(Instancia &instancia, Parametros &parametros, Solucao &best)
{
    //cout<<"GRASP\n\n";

    ParametrosGrasp parametrosGrasp;
    setParamGrasp(instancia, parametrosGrasp);
    Estatisticas estatisticas;


    vector<int> vetSatAtendCliente(instancia.numNos, -1);
    vector<int> satUtilizado(instancia.numSats+1, 0);
    const ublas::matrix<int> matClienteSat =  k_means(instancia, vetSatAtendCliente, satUtilizado, false);
    Solucao *solGrasp = grasp(instancia, parametrosGrasp, estatisticas, false, matClienteSat, N_gamb::vetMvValor, N_gamb::vetMvValor1Nivel);
    best.copia(*solGrasp);

    delete solGrasp;

}

void setParamGrasp(Instancia &instancia, ParametrosGrasp &parametrosGrasp)
{
    const std::vector<float> vetAlfa{0.01, 0.05, 0.1, 0.15, 0.2, 0.25, 0.3, 0.35, 0.4, 0.45, 0.5, 0.6, 0.7, 0.9};
    int num = min(instancia.getN_Evs() / 2, 8);
    if(num == 0)
        num = 1;

    parametrosGrasp = ParametrosGrasp(1000, 260, vetAlfa, 250, num, 0.1);
}
