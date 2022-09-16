//
// Created by igor on 14/09/22.
//

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

using namespace std;
using namespace NS_parametros;
using namespace NameS_Grasp;
using namespace N_PreProcessamento;
using namespace N_Aco;


int main(int argc, char* argv[])
{
    Parametros parametros;

    try
    {

        if(argc == 1)
        {
            cout<<"NUMERO DE ARGUMENTOS EH INSUFICIENTE.\n";
            cout<<string(argv[0])<<"\n";
            exit(-1);
        }

        caregaParametros(parametros, argc-1, &argv[1]);
        Instance instancia(parametros.instancia, parametros.nomeInstancia);

        dijkstraSatCli(instancia);
        instancia.calculaVetVoltaRS_sat();

        const std::vector<float> vetAlfa{0.1, 0.3, 0.5, 0.7, 0.9};
        int num = min(instancia.getN_Evs() / 2, 8);
        if(num == 0)
            num = 1;

        ParametrosGrasp parm(1000, 200, vetAlfa, 150, num, 0.1);
        AcoParametros acoParm;
        AcoEstatisticas acoEst;
        Estatisticas est;
        Solucao best(instancia);

        auto start = std::chrono::high_resolution_clock::now();

            N_Aco::acoSol(instancia, acoParm, acoEst, parm, est, best);

        auto end = std::chrono::high_resolution_clock::now();

        ParametrosSaida parametrosSaida;
        parametrosSaida.setTempo(start, end);

        parametrosSaida.mapNoSaida["dist"] = NoSaida("dist");
        parametrosSaida.mapNoSaida["t(s)"] = NoSaida("t(s)");
        parametrosSaida.mapNoSaida["sem"] = NoSaida("sem");


        parametrosSaida.mapNoSaida["dist"](best.distancia);
        parametrosSaida.mapNoSaida["t(s)"](parametrosSaida.tempo);
        parametrosSaida.mapNoSaida["sem"].addSemente(parametros.semente);

        parametrosSaida.mapNoSaida["dist"].addSaida(SAIDA_EXEC_VAL);
        parametrosSaida.mapNoSaida["t(s)"].addSaida(SAIDA_EXEC_VAL);
        parametrosSaida.mapNoSaida["sem"].addSaida(SAIDA_EXEC_SEM);


        //cout<<parametrosSaida.mapNoSaida["dist"].nome<<"\n";

        string cab;
        parametrosSaida.mapNoSaida["dist"].getCabecalho(cab);
        //cout<<cab<<"\n";

        string val;
        parametrosSaida.mapNoSaida["dist"].getVal(val);
        //cout<<val<<"\n";

        //cout<<"tempo: "<<parametrosSaida.tempo<<"\n";
        saida(parametros, parametrosSaida, best, instancia);




/*        escreveSolCompleta(parametros, best, instancia);
        escreveSolParaPrint(parametros, best, instancia);*/
    }
    catch(const char *erro)
    {

        std::cerr<<"CATCH ERRO\n";
        std::cerr<<erro<<"\n\n";
        //std::cerr<<"Semente: "<<semente<<"\n";
        exit(-1);
    }

    return 0;
}
