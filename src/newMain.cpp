/* ****************************************
 * ****************************************
 *  Nome: ***********************
 *  Data:    14/09/22
 *  Arquivo: newMain.cpp
 * ****************************************
 * ****************************************/

// Parametros para inst de 100 clientes:
// --alphaSeg 0.05 --betaPrim 0.75 --difBest 0.05 --numItIG 2500 --torneio 1 --taxaRm 0.25
// MIP:
// --alphaSeg 0.35 --betaPrim 0.15 --difBest 0.03 --numItIG 3000 --torneio 1 --taxaRm 0.2 --fatNumCh 3 --mip_presolve -1 --mip_cuts 1 --mip_restTempo 1 --mip_gap 0.00

// Parametros para inst menor de 15 clientes:
// --alphaSeg 0.9 --betaPrim 0.9 --difBest 0.03 --numItIG 2500 --torneio 0 --taxaRm 0.6
// INSTANCIA: C103_21x     SEMENTE: 1686762490     Wed Jun 14 14:08:10 2023

// INSTANCIA: C105_21x     SEMENTE: 1686762520     Wed Jun 14 14:08:40 2023

#include <iostream>
#include <chrono>
#include <limits>
#include <unordered_set>
#include <fstream>
#include "Parametros.h"
#include "Instancia.h"
#include "Aco.h"
#include "PreProcessamento.h"
#include "k_means.h"
#include "Vnd.h"
#include "VetorHash.h"
//#include "HASH/Hash.h"
#include "Grasp.h"
#include "Construtivo.h"
#include "IG.h"
#include <sys/stat.h>

using namespace std;
using namespace NS_parametros;
using namespace NameS_Grasp;
using namespace N_PreProcessamento;
using namespace N_Aco;
using namespace N_k_means;
using namespace NS_VetorHash;
//using namespace NS_Hash;
using namespace NameS_IG;


void aco(Instancia &instancia, Parametros &parametros, ParametrosGrasp &parm, Solucao &best);
void grasp(Instancia &instancia, Parametros &parametros, Solucao &best, ParametrosSaida &parametrosSaida);
void ig(Instancia &instancia, Parametros &parametros, Solucao &best, ParametrosSaida &parametrosSaida);
void setParamGrasp(Instancia &instancia, ParametrosGrasp &parametrosGrasp, const Parametros &parametros);
void escreveDistFile(double dist, double tempo, const std::string &file);


namespace N_gamb
{
    BoostC::vector<NS_vnd::MvValor> vetMvValor;
    BoostC::vector<NS_vnd::MvValor> vetMvValor1Nivel;
}

int main(int argc, char* argv[])
{

    Parametros parametros;
    const string commite = "4ffc0ced8eccac582746329dcc38708ed68e7fc8";
    parametros.commit = commite;
    clock_t startA = clock();

    try
    {

        if(argc == 1)
        {
            cerr<<"NUMERO DE ARGUMENTOS EH INSUFICIENTE.\n";
            cerr<<"COMMITE: "<<commite<<"\n";
            cerr<<"COMPILADO EM: "<<__DATE__<<", "<<__TIME__<<".\n";
            //cout<<"\nDESCRICAO IG: "<<NameS_IG::strDescricaoIg<<"\n\n";

            cerr<<string(argv[0])<<"\n";
            exit(-1);
        }

        caregaParametros(parametros, argc-1, &argv[1]);

        Instancia instancia(parametros.instancia, parametros.nomeInstancia);

        dijkstraSatCli(instancia);
        instancia.calculaVetVoltaRS_sat();

        ParametrosGrasp parametrosGrasp;
        setParamGrasp(instancia, parametrosGrasp, parametros);
        Solucao best(instancia);

        ParametrosSaida parametrosSaida = getParametros();

        //auto start = std::chrono::high_resolution_clock::now();
        clock_t start = clock();

        ig(instancia, parametros, best, parametrosSaida);

        clock_t end = clock();
        //auto end = std::chrono::high_resolution_clock::now();

        if(best.viavel)
        {


            string erro;
            if(!best.checkSolution(erro, instancia))
            {
                cout<<"ERRO SOL BEST\n";
                cout << erro << "\n\n";
                best.viavel = false;
            }

        }
        else
            cout<<"SOLUCAO INVIAVEL!\n";


        //std::chrono::duration<double> tempoAux = end - start;
        double tempo = double(end-start)/CLOCKS_PER_SEC;

//#if !AJUSTE_DE_PARAMETRO
        if constexpr(!AjusteDeParametro)
        {
            setParametrosSaida(parametrosSaida, parametros, best, start, end, N_gamb::vetMvValor, N_gamb::vetMvValor1Nivel);
            saida(parametros, parametrosSaida, best, instancia);
        }
        else
        {
//#else
            escreveDistFile(best.distancia, tempo, parametros.paramIg.fileSaida);
        }
//#endif
        

    }
    catch(const char *erro)
    {
        cerr<<"SEMENTE: "<< parametros.semente<<"\n";

        return -1;

        if(ValBestNs::distBest <= 0.0)
        {
            return -1;
        }
        else
        {

            clock_t end = clock();
            double tempo = double(end-startA)/CLOCKS_PER_SEC;
            escreveDistFile(ValBestNs::distBest, tempo, parametros.paramIg.fileSaida);
        }
    }


    return 0;
}

void aco(Instancia &instancia, Parametros &parametros, ParametrosGrasp &parm, Solucao &best)
{
    //cout<<"ACO\n\n";
    AcoParametros acoParm;
    AcoEstatisticas acoEst;
    N_Aco::acoSol(instancia, acoParm, acoEst, parm, best);
}

void grasp(Instancia &instancia, Parametros &parametros, Solucao &best, ParametrosSaida &parametrosSaida)
{
    //cout<<"GRASP\n\n";

    ParametrosGrasp parametrosGrasp;
    setParamGrasp(instancia, parametrosGrasp, parametros);
    Estatisticas estatisticas;

    BoostC::vector<int> vetSatAtendCliente(instancia.numNos, -1);
    BoostC::vector<int> satUtilizado(instancia.numSats+1, 0);
    const ublas::matrix<int> matClienteSat =  k_means(instancia, vetSatAtendCliente, satUtilizado, false);
    Solucao *solGrasp = grasp(instancia, parametrosGrasp, estatisticas, false, matClienteSat, N_gamb::vetMvValor,
                              N_gamb::vetMvValor1Nivel, parametrosSaida);
    best.copia(*solGrasp);

    delete solGrasp;

}

void ig(Instancia &instancia, Parametros &parametros, Solucao &best, ParametrosSaida &parametrosSaida)
{

    ParametrosGrasp parametrosGrasp;
    setParamGrasp(instancia, parametrosGrasp, parametros);
    Estatisticas estatisticas;

    BoostC::vector<int> vetSatAtendCliente(instancia.numNos, -1);
    BoostC::vector<int> satUtilizado(instancia.numSats+1, 0);
    ublas::matrix<int> matClienteSat =  k_means(instancia, vetSatAtendCliente, satUtilizado, false);
    Solucao *solGrasp = iteratedGreedy(instancia, parametrosGrasp, estatisticas, matClienteSat, N_gamb::vetMvValor,
                                       N_gamb::vetMvValor1Nivel, parametrosSaida, parametros);
    best.copia(*solGrasp);

    delete solGrasp;
}

void setParamGrasp(Instancia &instancia, ParametrosGrasp &parametrosGrasp, const Parametros &parametros)
{
    const BoostC::vector<float> vetAlfa{0.05, 0.07, 0.1, 0.15, 0.2, 0.3, 0.35, 0.4, 0.5, 0.6};
    //const BoostC::vector<float> vetAlfa{0.1, 0.3, 0.5, 0.9};
    int num = min(instancia.getN_Evs() / 2, 8);
    if(num == 0)
        num = 1;

    parametrosGrasp = ParametrosGrasp(parametros.numItTotal, 300, vetAlfa, 250, num, 0.1, parametros.numItTotal+100);
}

void escreveDistFile(double dist, double tempo, const std::string &fileStr)
{

    struct stat buffer;
    if(stat(fileStr.c_str(), &buffer) == 0)
    {
        cout<<"File: "<<fileStr<<" ja existe!!\n\n";
        exit(-1);
    }

    fstream file(fileStr, ios::out);
    if(!file.is_open())
    {
        cout<<"Nao foi possivel abrir arquivo: "<<fileStr<<"\n";
        exit(-1);
    }

    file<<dist<<" "<<tempo<<"\n";
    file.close();
}
