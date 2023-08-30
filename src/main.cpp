/* ****************************************************************
 * ****************************************************************
 *
 * NAO EXECUTAR MULTIPLAS INSTANCIAS NA MESMA EXECUCAO!!!
 * Variaveis estaticas!!
 *
 * Considerando dist(i,j) == dist(j,i) !!
 *
 * ****************************************************************
 * ****************************************************************
 */


#include <iostream>
#include "Instancia.h"
#include "Solucao.h"
#include <fstream>
#include <sstream>
#include <cmath>
#include <ctime>
#include "Construtivp.h"
#include "Auxiliary.h"
#include "LocalSearch.h"
#include <cfloat>
#include <memory>
#include "mersenne-twister.h"
#include "Vnd.h"
#include <chrono>
#include <iomanip>

#include <filesystem>
#include "Teste.h"
#include "Grasp.h"
#include "PreProcessamento.h"
//#include "ParametrosEntrada.h"

#include "Aco.h"

using namespace std;
using namespace GreedyAlgNS;
using namespace NS_vnd;
using namespace NameTeste;
using namespace NameS_Grasp;
using namespace N_PreProcessamento;
using namespace N_Aco;
//using namespace NS_parametros;

void routine(char** filenames, int nFileNames);
float distance(std::pair<float, float> p1, std::pair<float,float> p2);
Instancia* getInstanceFromFile(std::string &fileName);
void saveSolutionToFile(const Solucao& Sol, const std::string& fileName="solution.txt");
string getNomeInstancia2(string str);
void escreveInstancia(const Instancia &instance, string file);
void escreveSolucao(Solucao &solution, Instancia &instance, string &file);
void leSolucao(Solucao &solucao, Instancia &instancia, string &file);

#define NUM_EXEC 1000

#define MAIN_METODO     0
#define MAIN_DIST       1
#define MAIN_TESTE      2
#define MAIN_METODO_2   3
#define MAIN_SOMA_CARGA 4
#define MAIN_ACO        5
#define MAIN_ACO_SOL    6

//#define MAIN MAIN_METODO_2
//#define MAIN MAIN_ACO
#define MAIN MAIN_DIST

#define PRINT_RESULT FALSE



#if MAIN == MAIN_METODO_2
int main(int argc, char* argv[])
{
/*    ParametrosEntrada parametrosEntrada;
    caregaParametros(parametrosEntrada, argc-1, &argv[1]);*/

    if(argc != 2 && argc != 3)
    {

        cout<<"Compilado em: "<<__DATE__<<", "<<__TIME__<<".\n";
        cout<<"Commit: \tf7d1f0fbd4de1288fc027810a969822e3ba4e3dc\n\n";
        std::cerr<<"FORMATO: a.out file.txt\n";
        return -1;
    }

    uint32_t semente = 0;
    string fileResultado = "resultado.csv";

    if(argc == 3)
    {
        semente = atoll(argv[2]);

        //cout<<"SEMENTE: \t"<<semente<<"\n";
    }



    if(semente == 0)
    {
        //auto aux = std::chrono::high_resolution_clock::now();
        semente = duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();

        if(argc == 3)
            fileResultado = string(argv[2]);

    }


    seed(semente);
    std::string file(argv[1]);

    const string nomeInst = getNomeInstancia(file);

    Instance instance(file, nomeInst);

    //cout<<nomeInst<<" ; "<<instance.numRechargingS<<" ; "<<instance.numSats<<" ; "<<instance.numEv<<" ; "<<instance.numTruck<<"\n";

    //instance.print();
    //return 0;
    //cout<<"fist evRoute: "<<instance.getFirstEvIndex()<<"\n";
    //cout<<"last evRoute: "<<instance.getEndEvIndex()<<"\n";
    //return 0;

    try
    {

        string arquivo = "/home/igor/Documentos/Projetos/2E-EVRP-TW/Código/utils/instanciasMod/" + nomeInst + ".txt";
        string arquivoSol = "/home/igor/Documentos/Projetos/2E-EVRP-TW/Código/utils/solucao/" + nomeInst + "_2.txt";
        std::time_t result = std::time(nullptr);
        auto data = std::asctime(std::localtime(&result));

        string sementeStr;
        sementeStr += "INSTANCIA: " + string(nomeInst) + "\t";
        sementeStr += "SEMENTE: " + to_string(semente)  + "\t"+data;
        cout<<sementeStr;

/*        string arquivoInst = "/home/igor/Documentos/Projetos/2E-EVRP-TW/Código/ultimaExec.txt";
        ofstream escrita;
        escrita.open(arquivoInst, ios::out);
        if(escrita.is_open())
        {
            escrita<<sementeStr;
            escrita.close();
        }
        else
        {
            cout<<"NAO FOI POSSIVEL ABRIR ARQUIVO: "<<arquivoInst<<"\n";
            //int p;
            //cin>>p;
        }*/


/*        escreveInstancia(instance, arquivo);
        return 0;*/

        double tempo = 0.0;
        //instance.print();

        //const Vector<float> vetAlfa{0.1, 0.2, 0.3, 0.4, 0.6, 0.7, 0.9};
        const Vector<float> vetAlfa{0.1, 0.3, 0.5, 0.7, 0.9};


        int num = min(instance.getN_Evs()/2, 8);
        if(num == 0)
            num = 1;

        //Parametros parametros(NUM_EXEC, 110, vetAlfa, 100, num);
        Parametros parametros(NUM_EXEC, 200, vetAlfa, 150, num, 0.1);

        auto start = std::chrono::high_resolution_clock::now();


            dijkstraSatCli(instance);
            instance.calculaVetVoltaRS_sat();

            Estatisticas estat;
            Solucao *solBest = nullptr;
            solBest = grasp(instance, parametros, estat, false);


        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> tempoAux = end - start;
        tempo = tempoAux.count();

        if(tempo < 1.0)
            sleep(1);

/*        cout<<"\nINST \t\tDISTANCIA_MEDIA \tBEST \t\tNUM \tTEMPO\n";
        if(solBest->viavel)
            cout<<nomeInst<<";\t"<<estat.media()<<";\t\t"<<solBest->distancia<<";\t"<<estat.numSol<<";\t"<<tempo<<"\n";
        else
            cout<<nomeInst<<";\t*; \t\t\t*; \t\t"<<estat.numSol<<";\t"<<tempo<<"\n";

        cout<<"viavel: "<<solBest->viavel<<"\n";*/

        escreveSolucao(*solBest, instance, arquivoSol);

        bool exists = std::filesystem::exists(fileResultado);
        std::ofstream outfile;
        outfile.open(fileResultado, std::ios_base::app);
        if(!exists)
        {
            string dataStr;
            string temp(data);

            for(auto ch:temp)
            {
                if(ch != '\n')
                    dataStr +=  ch;
            }

            outfile<<dataStr<<";\t;\t;\t;\t;\t;\t;\n";
            outfile<<"nomeInst;\tmedia; \t\tbest; \t\tnumSol;\ttempo;\t1° nivel;\t2° nivel;\tultimaA;\ttempoViab;\tnumEVs\n";
        }

        string tempoPocStr;


#if TEMPO_FUNC_VIABILIZA_ROTA_EV
        double val = 100.0*(NameViabRotaEv::global_tempo/tempo);
        //cout<<"\nTEMPO TOTAL FUNC VIABILIZA ROTA EV: "<<NameViabRotaEv::global_tempo<<"  "<<100.0*(NameViabRotaEv::global_tempo/tempo)<<" % DO TEMPO TOTAL\n";
        tempoPocStr = to_string(int(val)) + "%";
#endif

        if(solBest->viavel)
        {
            double dist1Nivel = solBest->getDist1Nivel();
            double dist2Nivel = solBest->getDist2Nivel();

            string saida = str(boost::format("%.2f; \t%.2f; \t%d;\t%.2f;\t%.2f;\t\t%.2f;\t\t%d;") % float(estat.media()) % float(solBest->distancia) % estat.numSol % float(tempo) % float(dist1Nivel/solBest->distancia) % float(dist2Nivel/solBest->distancia) % estat.ultimaAtualizacaoBest);
            saida += "\t\t"+tempoPocStr + ";\t\t"+ to_string(solBest->numEv);

            //string saida = str(boost::format("%.2f") % float(estat.media()));
            //outfile << nomeInst << ";\t" << estat.media() << ";\t " << solBest->distancia << ";\t" << estat.numSol<< ";\t" << tempo << "\n";
            outfile << nomeInst << ";\t" <<saida<<";\n";//\t\t"<<tempoPocStr<<"\n";

#if PRINT_RESULT
            cout<<nomeInst << ";\t" <<saida<<"\n";//<<";\t\t"<<tempoPocStr<<"\n";
#endif

/*            cout<<"\n\n";

            for(Route &route:solBest->primeiroNivel)
            {
                if(route.routeSize > 2)
                {
                    for(int i=0; i < route.routeSize; ++i)
                        cout<<route.rota[i].satellite<<"("<<route.satelliteDemand[route.rota[i].satellite]<<") ";

                    cout<<"\n";
                }
            }*/

        }
        else
        {
            outfile << nomeInst << ";\t*;\t*;\t" << estat.numSol << ";\t" << tempo<<"; \t; \t; \t\n";

#if PRINT_RESULT
            cout << nomeInst << ";\t*;\t*;\t" << estat.numSol << ";\t" << tempo << "\n";
#endif

        }

        outfile.close();

        //solBest->print(instance);
        //cout<<"numEVs: "<<solBest->numEv<<"\n\n";

        delete solBest;
        return 0;


    }
    catch(std::exception &e)
    {

        //instance.print();

        cout<<"EXCEPTION:\n";
        cout<<e.what()<<"\n";

        cout<<"SEMENTE: \t"<<semente<<"\n";
        return -1;
    }
    catch(char const* exception)
    {
        //instance.print();

        cout<<"EXCEPTION:\n"<<exception<<"\n\n";
        cout<<"SEMENTE: \t"<<semente<<"\n";
        return -1;
    }


    return 0;


}

#endif

#if MAIN == MAIN_ACO_SOL

int main(int argc, char* argv[])
{
    int semente = 0;

    if(argc == 3)
        semente = atoi(argv[2]);
    else
        semente = duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();


    seed(semente);

    if(argc == 1 || argc > 3)
    {
        cout << "Compilado em: \t" << __DATE__ << ", " << __TIME__ << "\n";
        cout << "Commit: \t532370b0f5da08a84c88b4a4649d417032c87fff\n\n";
        std::cerr << "FORMATO: ./a.out instancia.txt\n";
        return -1;
    }

    //cout<<"SEMENTE: "<<semente<<"\n";


    std::string file(argv[1]);
    const string nomeInst = getNomeInstancia2(file);

    std::time_t result2 = std::time(nullptr);
    auto data2 = std::asctime(std::localtime(&result2));

    string sementeStr;
    sementeStr += "INSTANCIA: " + string(nomeInst) + "\t";
    sementeStr += "SEMENTE: " + to_string(semente) + "\t" + data2;
    cout << sementeStr;


    Instance instancia(file, nomeInst);
    dijkstraSatCli(instancia);
    instancia.calculaVetVoltaRS_sat();

    const Vector<float> vetAlfa{0.1, 0.3, 0.5, 0.7, 0.9};
    int num = min(instancia.getN_Evs() / 2, 8);
    if(num == 0)
        num = 1;

    ParametrosGrasp parm(NUM_EXEC, 200, vetAlfa, 150, num, 0.1);
    AcoParametros acoParm;
    AcoEstatisticas acoEst;
    Estatisticas est;
    Solucao best(instancia);

    auto start = std::chrono::high_resolution_clock::now();

        bool viavel = N_Aco::acoSol(instancia, acoParm, acoEst, parm, est, best);

    auto end = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> tempoAux = end - start;
    double tempo = tempoAux.count();


    string fileResultado = "resultadoACO.csv";

    bool exists = std::filesystem::exists(fileResultado);
    std::ofstream outfile;
    outfile.open(fileResultado, std::ios_base::app);
    if(!exists)
    {

        std::time_t result = std::time(nullptr);
        auto data = std::asctime(std::localtime(&result));
        string dataStr;
        string temp(data);

        for(auto ch:temp)
        {
            if(ch != '\n')
                dataStr +=  ch;
        }

        outfile<<dataStr<<";\t;\t\n";
        //outfile<<"nomeInst;\tmedia; \t\tbest; \t\tnumSol;\ttempo;\t1° nivel;\t2° nivel;\tultimaA;\ttempoViab;\tnumEVs\n";
        //outfile<<"nomeInst;\tbestAntDist;\ttempo\n";

        outfile<<"inst; \t\t distSol; \t tempo\n";
    }

    outfile<<nomeInst<<"; \t ";

    if(best.viavel)
    {
        //outfile<<solucao.distancia<<";\t";


        outfile<<str(boost::format("%.2f; \t %.2f\n") % best.distancia % tempo);
    }
    else
    {
        outfile<<"*; *\n";
    }

    //outfile<<tempo<<"\n";
    outfile.close();

}
#endif

#if MAIN ==MAIN_ACO

int main(int argc, char* argv[])
{

    int semente = 0;

    if(argc == 3)
        semente = atoi(argv[2]);
    else
        semente = duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();


    seed(semente);

    if(argc == 1 || argc > 3)
    {
        cout<<"Compilado em: \t"<<__DATE__<<", "<<__TIME__<<"\n";
        cout<<"Commit: \tf7d1f0fbd4de1288fc027810a969822e3ba4e3dc\n\n";
        std::cerr<<"FORMATO: ./a.out instancia.txt\n";
        return -1;
    }

    //cout<<"SEMENTE: "<<semente<<"\n";


    std::string file(argv[1]);
    const string nomeInst = getNomeInstancia(file);

    std::time_t result2 = std::time(nullptr);
    auto data2 = std::asctime(std::localtime(&result2));

    string sementeStr;
    sementeStr += "INSTANCIA: " + string(nomeInst) + "\t";
    sementeStr += "SEMENTE: " + to_string(semente)  + "\t"+data2;
    cout<<sementeStr;


    Instance instancia(file, nomeInst);
    dijkstraSatCli(instancia);
    instancia.calculaVetVoltaRS_sat();

    if(instancia.getNSats() > 1)
    {
        cout<<"ACO SO FUNCIONA COM 1 SATELITE!\n";
        return -1;
    }


    const Vector<float> vetAlfa{0.1, 0.3, 0.5, 0.7, 0.9};
    int num = min(instancia.getN_Evs()/2, 8);
    if(num == 0)
        num = 1;

    Parametros parm(NUM_EXEC, 200, vetAlfa, 150, num, 0.1);
    AcoParametros acoParm;
    AcoEstatisticas acoEst;
    Satelite satelite(instancia, 1);

    vector<int> vetSatAtendCliente(instancia.numNos, -1);
    vector<int> satUtilizado(instancia.numSats+1, 0);
    setSatParaCliente(instancia, vetSatAtendCliente, satUtilizado, parm);

    Parametros parametros(NUM_EXEC, 200, vetAlfa, 150, num, 0.1);
    Estatisticas est;
    Solucao solucao(instancia);


    auto start = std::chrono::high_resolution_clock::now();


    if(aco(instancia, acoParm, acoEst, 1, satelite, vetSatAtendCliente, parametros, est))
    {
        solucao.satelites[1].copia(satelite);

        for(EvRoute &evRoute:solucao.satelites[1].vetEvRoute)
        {
            if(evRoute.routeSize > 2)
            {
                for(int i=1; i < (evRoute.routeSize-1); ++i)
                    solucao.vetClientesAtend[evRoute[i].cliente] += 1;
            }
        }
        solucao.distancia = satelite.distancia;

        GreedyAlgNS::firstEchelonGreedy(solucao, instancia, parametros.vetAlfa[0]);

        if(solucao.viavel)
        {
            string erroSol;
            if(solucao.checkSolution(erroSol, instancia))
            {

                auto end = std::chrono::high_resolution_clock::now();
                std::chrono::duration<double> tempoAux = end - start;
                double tempo = tempoAux.count();

                /*
                cout<<"sol viavel\n";
                cout<<"DIST: "<<solucao.distancia<<"\n\n";

                cout<<"inst \t antsMedia \t antBest \t antsViaveis \t numAntsTotal \t numAntsPorIt \t numIt \t numMedioAntsViaveisP_it \t tempo\n";
                cout<<acoEst.sumDistAntsViaveis<<" \t "<<acoEst.distBestAnt<<" \t "<<acoEst.nAntViaveis<<" \t\t "<<acoEst.nAntGeradas<<" \t\t "
                <<acoParm.numAnts<<" \t\t "<<acoParm.numIteracoes<<" \t "<<acoEst.mediaAntsViaveisPorIt<<" \t\t\t\t "<<tempo<<"\n";
                */

            }
            else
            {
                cout << "sol inviavel\n";
                cout<<"ERRO: "<<erroSol<<"\n\n";
                solucao.viavel = false;
            }
        }

    }
    else
    {
        solucao.satelites[1].copia(satelite);
        solucao.viavel = false;
    }

    string arquivoSol = "/home/igor/Documentos/Projetos/2E-EVRP-TW/Código/utils/solucao/" + nomeInst + "_ACO.txt";
    escreveSolucao(solucao, instancia, arquivoSol);

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> tempoAux = end - start;
    double tempo = tempoAux.count();

    string fileResultado = "resultadoACO.csv";

    bool exists = std::filesystem::exists(fileResultado);
    std::ofstream outfile;
    outfile.open(fileResultado, std::ios_base::app);
    if(!exists)
    {

        std::time_t result = std::time(nullptr);
        auto data = std::asctime(std::localtime(&result));
        string dataStr;
        string temp(data);

        for(auto ch:temp)
        {
            if(ch != '\n')
                dataStr +=  ch;
        }

        outfile<<dataStr<<";\t;\t;\t;\t;\t;\t;\n";
        //outfile<<"nomeInst;\tmedia; \t\tbest; \t\tnumSol;\ttempo;\t1° nivel;\t2° nivel;\tultimaA;\ttempoViab;\tnumEVs\n";
        //outfile<<"nomeInst;\tbestAntDist;\ttempo\n";

        outfile<<"inst \t\t antsMedia \t antBest \t distSol \t antsViaveis \t numAntsTotal \t numAntsPorIt \t numIt \t numMedioAntsViaveisP_it \t tempo\n";
    }

    outfile<<nomeInst<<";\t";

    if(solucao.viavel)
    {
        //outfile<<solucao.distancia<<";\t";

        outfile<<" "<<acoEst.sumDistAntsViaveis<<"; \t "<<acoEst.distBestAnt<<"; \t "<<solucao.distancia<<"; \t "<<acoEst.nAntViaveis<<"; \t\t "<<acoEst.nAntGeradas<<"; \t\t "
            <<acoParm.numAnts<<"; \t\t "<<acoParm.numIteracoes<<"; \t "<<acoEst.mediaAntsViaveisPorIt<<"; \t\t\t\t "<<tempo<<"\n";
    }
    else
    {
        outfile<<";*;*;*;*;*;*;*;*\n";
    }

    //outfile<<tempo<<"\n";
    outfile.close();
}

#endif

#if MAIN == MAIN_SOMA_CARGA

int main(int argc, char* argv[])
{
    if(argc > 3)
    {
        cout<<"Utilizacao: \n./run instancia.txt solucao.txt\n";
        return 0;
    }

    std::string file(argv[1]);
    const string nomeInst = getNomeInstancia(file);
    Instance instance(file, nomeInst);

    std::string solucaoFile(argv[2]);
    Solucao solucao(instance);


}

#endif

#if MAIN == MAIN_METODO
int main(int argc, char* argv[])
{

    if(argc != 2 && argc != 3)
    {
        std::cerr<<"FORMATO: a.out file.txt\n";
        return -1;
    }

    long semente = 0;

    if(argc == 3)
    {
        semente = atol(argv[2]);
        cout<<"SEMENTE: \t"<<semente<<"\n";
        seed(semente);
    }




    std::string file(argv[1]);
    const string nomeInst = getNomeInstancia(file);
    cout<<"INSTANCIA: \t"<<nomeInst<<"\n";

    Instance *instance = getInstanceFromFile(file);



    try
    {
        Solucao bestB(*instance);
        float globalBest = FLOAT_MAX;

        float tempo = 0.0;
        float mediaRvnd = 0.0;
        float mediaConst = 0.0;
        float bestConst = FLOAT_MAX;
        const int N = 2000;
        int nReal = 0;

        for(int n = 0; n < 1; n++)
        {

            if(argc == 2)
            {
                semente = time(nullptr);
                seed(semente);
                cout<<"SEMENTE: \t"<<semente<<"\n\n";

            }

            std::string str;

            auto start = std::chrono::high_resolution_clock::now();
            for (int i = 0; i < N; ++i)
            {

                //cout<<"i: "<<i<<"\n";

                Solucao solution(*instance);
                greedy(solution, *instance, 0.4, 0.4);
                if (solution.viavel)
                {

                    float improv;

                    if (!solution.checkSolution(str, *instance)) {
                        cout << str << "\n\nERRO*********\nCONSTRUTIVO";
                        exit(-1);
                    }

                   //solution.print(*instance);

                    float distanciaAux = solution.calcCost(*instance);

                    //cout<<"\nANTES MOVIMENTO CORSS\n\n";
                    string erro = "";
                    //solution.print(erro);
                    //cout<<erro<<"\n\n";

                    //rvnd(solution, *instance);
                    bool mv = NS_LocalSearch::mvShiftInterRotasIntraSatellite(solution, *instance);

                    //cout<<"\n\n###################################################################################\n###################################################################################\n\n";

                    if(mv)
                    {
                        do
                        {

                            erro = "";
                            if(!solution.checkSolution(erro, *instance))
                            {
                                cout << erro << "\n\nERRO*********\nMV CROSS\n";
                                //solution.print(erro);
                                //cout << "\n\n\n"<<erro << "\n\nERRO*********\n";
                                exit(-1);
                            }

                            mv = NS_LocalSearch::mvShiftInterRotasIntraSatellite(solution, *instance);

                        }while(mv);


                        erro = "";
                        if(!solution.checkSolution(erro, *instance))
                        {
                            cout << erro << "\n\nERRO*********\nMV CROSS\n\a";
                            //solution.print(erro);
                            //cout << "\n\n\n"<<erro << "\n\nERRO*********\n";
                            exit(-1);
                        }
                    }


                    //float distLs = solution.getDistanciaTotal(); // TODO: resolver problema que os valores de distancia nas rotas nao estao sendo atualizados corretamente durante busca local swap
                    float distLs = solution.calcCost(*instance);

                    //cout<<"mv cross: "<<solution.mvCrossIntraSatellite;

                    if (distLs < globalBest) {
                        bestB = solution;
                        globalBest = distLs;
                    }

                    mediaRvnd += distLs;
                    mediaConst += distanciaAux;
                    nReal += 1;

                    if(distanciaAux < bestConst)
                        bestConst = distanciaAux;


                }
                else
                {
                    //cout << "SOL INVIAVEL\n";
                    //solution.print(*instance);

                }


            }

            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> tempoAux = end - start;

            tempo = tempoAux.count();

        }

        //saveSolutionToFile(bestB, "file.txt");
        //cout<<"CROSS: "<<bestB.mvCrossIntraSatellite<<"\n";



        cout <<"Melhor RVND: " << globalBest<<"; \tMedia RVND: " << mediaRvnd/float(nReal)<<"; \tMelhor Const: "<<bestConst<<"\tMedia Const: "<<mediaConst/nReal;

        cout<<boost::format(" \tTempo: %.2f S") % (tempo)<<" n: "<<nReal<<"\n\n";

        //bestB.print(*instance);

        //std::string str;
        //bestB.print(str);
        //cout<<str<<"\n";
    }
    catch(std::out_of_range &e)
    {
        std::cerr<<"CATCH out_of_range ERRO\n";
        std::cerr<<e.what()<<"\n\n";
        exit(-1);
    }
    catch(const char *erro)
    {
        std::cerr<<"CATCH ERRO\n";
        std::cerr<<erro<<"\n\n";
        std::cerr<<"Semente: "<<semente<<"\n";
        exit(-1);
    }
    return 0;
}
#endif

#if MAIN == MAIN_DIST

int main(int argc, char* argv[])
{

    if(argc != 2)
    {
        std::cerr<<"FORMATO: ./a.out inst.txt\n";
        return -1;
    }
    string file(argv[1]);
    string strInst = getNomeInstancia2(file);
    cout<<"INSTANCIA: "<<strInst<<"\n\n";

    Instancia instance(file, strInst);
    EvRoute evRoute(1, instance.getFirstEvIndex(), instance.getEvRouteSizeMax(), instance);
    Route route(instance);

    int num0, num1;
    char c;
    bool somenteNo = false;
    bool ev = false;

    cout<<"EV(s/n): ";
    cin>>c;
    if(c == 's')
        ev = true;


    if(ev)
    {
        cout << "Somente NO(s/n): ";
        cin >> c;

        if(c == 's')
            somenteNo = true;
        else
            somenteNo = false;
    }


    double saida = 0.0;

    do
    {

        if(ev)
        {
            cout << "saida: ";
            cin >> saida;
        }
        int i = 1;
        cin>>num0;

        if(ev)
            evRoute[0].cliente = num0;
        else
            route.rota[0].satellite = num0;

        if(ev)
        {
            do
            {

                cin >> num0;
                evRoute[i].cliente = num0;
                //cout<<"i: "<<i<<"; num: "<<num0<<"\n";
                i += 1;
                //cout<<instance.isSatelite(num0)<<"\n";

            } while(!instance.isSatelite(num0));

            evRoute.satelite = num0;
            evRoute.routeSize = i;
            if(saida < 0.0)
                evRoute[0].tempoSaida = instance.vetTempoSaida[num0];
            else
                evRoute[0].tempoSaida = saida;

            //evRoute.print(instance, true);

            double dist = NameViabRotaEv::testaRota(evRoute, i, instance, true, evRoute[0].tempoSaida, 0, nullptr);

            cout << "Dist: " << dist << "\n\n";
            string rotaStr;
            evRoute.print(rotaStr, instance, somenteNo);
            cout << "Rota: " << rotaStr << "\n\n";

            for(int j = 1; j < (evRoute.routeSize - 1); ++j)
            {
                int cliente = evRoute[j].cliente;
                cout << "\t" << cliente << ": " << (instance.vectCliente[cliente].fimJanelaTempo - evRoute[j].tempoCheg)
                     << "\n";
            }
        }
        else
        {
            do
            {

                cin >> num0;
                route.rota[i].satellite = num0;
                i += 1;

            }while(!instance.isDepot(num0));

            route.routeSize = i;

            double dist = 0.0;

            cout<<"\n";

            for(int i=0; i < (route.routeSize-1); ++i)
            {
                dist += instance.getDistance(route.rota[i].satellite, route.rota[i+1].satellite);
                cout<<"Tempo Chegada em "<<route.rota[i+1].satellite<<": "<<dist<<"\n";

                if(!(instance.isDepot(route.rota[i].satellite) || instance.isSatelite(route.rota[i].satellite)))
                {
                    cout<<route.rota[i].satellite<<" eh cliente!\n";
                    break;
                }

            }

            cout<<"Dist: "<<dist<<"\n";
        }

        cout<<"\n*********************************************************\n\n";

        cout<<"cont(s/n): ";
        cin>>c;

        if(c == 'n')
            break;

    } while(c == 's');

    cout<<"Arcos(s/n): ";
    cin>>c;

    while(c == 's')
    {

        cin>>num0>>num1;
        cout<<"Dist: "<<instance.getDistance(num0, num1);

        cout<<"\nArcos(s/n): ";
        cin>>c;

    }

}

#endif

#if MAIN == MAIN_TESTE


int main(int argc, char* argv[])
{

    if(argc != 3)
    {
        std::cerr<<"FORMATO: a.out file.txt k\n";
        return -1;
    }

    long semente = time(nullptr);

    int k = atoi(argv[2]);
    if(k < 0 || k >= T_TAM)
        exit(-1);


    std::string file(argv[1]);
    Instance *instance = getInstanceFromFile(file);

    string saida;
    testeMovimentos(saida, *instance, semente, k);
    cout<<saida<<"\n";

}

#endif

void leSolucao(Solucao &solucao, Instancia &instancia, string &file)
{
    std::ifstream inFile;
    inFile.open(file, std::ios_base::in);

    if(!inFile.is_open())
    {
        cout<<"NAO FOI POSSIVEL ABRIR O ARQUIVO: "<<file<<"\n";
        throw "ERRO";
    }


}

void escreveSolucao(Solucao &solution, Instancia &instance, string &file)
{

    //solution.print(instance);

    std::ofstream outfile;
    outfile.open(file, std::ios_base::out);

    if(outfile.is_open())
    {

        for(int s=instance.getFirstSatIndex(); s <= instance.getEndSatIndex(); ++s)
        {

            for(int e=0; e < solution.satelites[s].vetEvRoute.size(); ++e)
            {
                EvRoute &evRoute = solution.satelites[s].vetEvRoute[e];

                if(evRoute.routeSize > 2)
                {
                    string rota;

                    for(int i=0; i < evRoute.routeSize; ++i)
                    {
                        rota += to_string(evRoute[i].cliente) + " ";
                    }

                    outfile<<rota<<"\n";


                }
            }
        }

        for(int i = 0; i < solution.primeiroNivel.size(); ++i)
        {
            Route &veic = solution.primeiroNivel[i];
            string rota;

            if(veic.routeSize > 2)
            {
                for(int t=0; t < veic.routeSize; ++t)
                    rota += to_string(veic.rota[t].satellite) + " ";

                outfile<<rota<<"\n";
            }

        }

        outfile.close();

    }
    else
    {
        cout<<"Nao foi possivel abrir o arquivo: "<<file<<"\n";
        throw "ERRO FILE";
    }
}

void escreveInstancia(const Instancia &instance, string file)
{
    std::ofstream outfile;
    outfile.open(file, std::ios_base::out);

    if(outfile.is_open())
    {
        /*  Code:
         *  D	depot
         *  S	satellites
         *  C	customers
         *  F	recharging stations
         */

        // id   Code     x       y       demanda

        outfile<<"id code x y demanda\n";
        outfile<<"0 D "<<instance.vectCliente[0].coordX<<" "<<instance.vectCliente[0].coordY<<" 0\n";

        for(int s=instance.getFirstSatIndex(); s <= instance.getEndSatIndex(); ++s)
            outfile<<s<<" S "<<instance.vectCliente[s].coordX<<" "<<instance.vectCliente[s].coordY<<" 0\n";


        for(int f= instance.getFirstRS_index(); f <= instance.getEndRS_index(); ++f)
            outfile<<f<<" F "<<instance.vectCliente[f].coordX<<" "<<instance.vectCliente[f].coordY<<" 0\n";


        for(int c=instance.getFirstClientIndex(); c <= instance.getEndClientIndex(); ++c)
            outfile<<c<<" C "<<instance.vectCliente[c].coordX<<" "<<instance.vectCliente[c].coordY<<" "<<instance.vectCliente[c].demanda<<"\n";

        outfile.close();

    }

}

void routine(char** filenames, int nFileNames)
{

}
float distance(std::pair<float, float> p1, std::pair<float,float> p2)
{
    float x1 = p1.first, x2 = p2.first, y1 = p1.second, y2 = p2.second;
    return (float)sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2) * 1.0);
}
/*
Instance* getInstanceFromFile(std::string &fileName){
    std::ifstream file(fileName);

    if(!file.is_open())
    {
        cerr<<"ERRO AO ABRIR O ARQUIVO: "<<fileName<<"\n";
        exit(-1);
    }

    std::string line;
    stringstream ss, ssaux, ssaux2;
    int nSats=0, nClients=0, nRS=0;
    Vector<float> demands;
    Vector<std::pair<float, float>> coordinates;
    while(getline(file, line)){
        // std::cout << line << std::endl;
        // std::cout << "first char: " << line[0] << std::endl;
        char firstChar = line[0];
        ss = stringstream(line);
        string tag;
        float x, y, demand;
        if(firstChar == 'D' || firstChar == 'S' || firstChar == 'C'|| firstChar == 'F' ){
            if(firstChar=='S') nSats++;
            else if(firstChar=='C') nClients++;
            else if(firstChar=='F') nRS++;
            ss >> tag;
            ss >> x >> y;
            ss >> demand;
            demands.push_back(demand);
            coordinates.emplace_back(x, y);
            // std::cout << x << ", " << y << " -> " << demand << endl;
        }
        else{
            break;
        }
    }
    string aux;
    float truckCap, evCap, evBattery;
    while(ss >> aux);
    ssaux2 = stringstream(aux);
    ssaux2 >> truckCap;
    // // cout << " ... ";
    getline(file, line);
    ssaux = stringstream(line);
    while(ssaux >> aux); //cout << aux << ", ";
    ssaux2 = stringstream(aux);
    ssaux2 >> evCap;
    getline(file, line);
    ssaux = stringstream(line);
    while(ssaux >> aux); // cout << aux << ", ";
    ssaux2 = stringstream(aux);
    ssaux2 >> evBattery;
    //cout << ".";
    file.close();
    Vector<Vector<double>> distMat(demands.size(), Vector<double>(demands.size(), -1));
    for(int i = 0; i < distMat.size(); i++){
        for(int j = 0; j < distMat.size(); j++){
            distMat[i][j] = distance(coordinates[i], coordinates[j]);
        }
    }
    auto* Inst = new Instance(distMat, truckCap, evCap, evBattery, nSats, nClients, nRS, coordinates, demands);
    return Inst;
}

void saveSolutionToFile(const Solucao& Sol, const std::string& fileName){
    std::ofstream file(fileName);
    if(!file.is_open()){
        cout << "failed to open file" << endl;
        return;
    }
    for(const auto& route : Sol.primeiroNivel){
        for(int node : route.rota){
            file << node << ",";
        }
        file << endl;
    }
    for(const auto& satellite : Sol.satelites){
        for(int i = 0; i < satellite->getNRoutes(); i++){
            const auto& evRoute = satellite->getRoute(i);
            for(int j = 0; j < evRoute.size(); j++){
                file << evRoute.getNodeAt(j) << ",";
            }
            file << endl;
        }
    }
    file.close();
}
*/
string getNomeInstancia2(string str)
{
    int posNome = -1;

    for(int i=0; i < str.size(); ++i)
    {
        if(str[i] == '/')
            posNome = i+1;
    }

    if(posNome < str.size())
    {
        string nome = str.substr(posNome);

        int posPonto = -1;

        for(int i=0; i < nome.size(); ++i)
        {
            if(nome[i] == '.')
            {
                posPonto = i - 1;
                break;
            }

            //delete bestSol;
        }

        if(posPonto > 0)
        {   //cout<<"posNome: "<<posNome<<"\n\n";
            return nome.substr(0, (posPonto+1));
        }
        else
            return nome;
    }
    else
        return "ERRO";
}
