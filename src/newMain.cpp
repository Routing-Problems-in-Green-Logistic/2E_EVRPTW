/* ****************************************
 * ****************************************
 *  Nome:    Igor de Andrade Junqueira
 *  Data:    14/09/22
 *  Arquivo: newMain.cpp
 * ****************************************
 * ****************************************/

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
#include "HASH/Hash.h"
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
using namespace NS_Hash;
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
    const string commite = "0dae18891ab35b7374b97cb9357e49d01e164aad";
    parametros.commit = commite;
    auto startA = std::chrono::high_resolution_clock::now();

    try
    {

        if(argc == 1)
        {
            cout<<"NUMERO DE ARGUMENTOS EH INSUFICIENTE.\n";
            cout<<"COMMITE: "<<commite<<"\n";
            cout<<"COMPILADO EM: "<<__DATE__<<", "<<__TIME__<<".\n";
            cout<<"\nDESCRICAO IG: "<<NameS_IG::strDescricaoIg<<"\n\n";

            cout<<string(argv[0])<<"\n";
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

        auto start = std::chrono::high_resolution_clock::now();

        ig(instancia, parametros, best, parametrosSaida);

        /*switch(parametros.metodo)
        {
            case METODO_ACO:
                aco(instancia, parametros, parametrosGrasp, best);
                break;

            case METODO_GRASP:
                cout<<"GRASP!!\n";
                grasp(instancia, parametros, best, parametrosSaida);
                break;

            case METODO_IG:
                ig(instancia, parametros, best, parametrosSaida);
                break;

            default:
                cout<<"METODO: "<<parametros.metodo<<" NAO EXISTE\n"<<parametros.getParametros()<<"\n\n";
                throw "ERRO";
                break;
        }*/

        auto end = std::chrono::high_resolution_clock::now();

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


        std::chrono::duration<double> tempoAux = end - start;
        double tempo = tempoAux.count();

        setParametrosSaida(parametrosSaida, parametros, best, start, end, N_gamb::vetMvValor, N_gamb::vetMvValor1Nivel);
        saida(parametros, parametrosSaida, best, instancia);
        //cout<<best.distancia<<"\n";
        //escreveDistFile(best.distancia, tempo, parametros.paramIg.fileSaida);

       // cout<<"TEMPO CPU: "<<parametrosSaida.tempo<<" S\n";
        //string numSol;
        //parametrosSaida.mapNoSaida["numSol"].getVal(numSol);
        //cout<<"NUM SOL: "<<numSol<<"\n";
        

    }
    catch(const char *erro)
    {
/*        cout<<"\n\n*************************************************\n\nCOMMITE: "<<parametros.commit<<"\n";
        cout<<"Compilado em: "<<__DATE__<<", "<<__TIME__<<".\n";
        cout<<"SEMENTE: "<<parametros.semente<<"\n\n";

        std::cerr<<"CATCH ERRO\n";
        std::cerr<<erro<<"\n\n";
        //std::cerr<<"Semente: "<<semente<<"\n";
        exit(-1);*/

        if(ValBestNs::distBest <= 0.0)
        {
            return -1;
        }
        else
        {
            auto end = std::chrono::high_resolution_clock::now();

            std::chrono::duration<double> tempoAux = end - startA;
            double tempo = tempoAux.count();

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
    const ublas::matrix<int> matClienteSat =  k_means(instancia, vetSatAtendCliente, satUtilizado, false);
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

    parametrosGrasp = ParametrosGrasp(parametros.numItTotal, 300, vetAlfa,
                                      250, num, 0.1, parametros.numItTotal+100);
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
