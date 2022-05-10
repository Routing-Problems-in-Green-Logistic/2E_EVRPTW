/*
 *  ./run ../instancias/2e-vrp-tw/Customer_10/C101_C10x.txt 1651594136
 *
 *  ERRO escrita da rota
 *
 *  Esta processando 1 9 12 4 15 1 (ja esta na solucao) no lugar de 1 11 9 12 4 15 1 (nao esta na solucao e eh inviavel)
 */

#include <iostream>
#include "Instance.h"
#include "Solution.h"
#include <fstream>
#include <sstream>
#include <math.h>
#include <time.h>
#include "greedyAlgorithm.h"
#include "Auxiliary.h"
#include "LocalSearch.h"
#include <cfloat>
#include <memory>
#include "mersenne-twister.h"
#include "Vnd.h"
#include <chrono>
#include <iomanip>
#include <boost/format.hpp>
#include "Teste.h"

using namespace std;
using namespace GreedyAlgNS;
using namespace NS_vnd;
using namespace NameTeste;

void routine(char** filenames, int nFileNames);
float distance(std::pair<float, float> p1, std::pair<float,float> p2);
Instance* getInstanceFromFile(std::string &fileName);
void saveSolutionToFile(const Solution& Sol, const std::string& fileName="solution.txt");
string getNomeInstancia(string str);


#define NUM_EXEC 400

#define MAIN_METODO     0
#define MAIN_DIST       1
#define MAIN_TESTE      2
#define MAIN_METODO_2   3

#define MAIN MAIN_METODO_2

#if MAIN == MAIN_METODO_2

int main(int argc, char* argv[])
{



    if(argc != 2 && argc != 3)
    {
        std::cerr<<"FORMATO: a.out file.txt\n";
        return -1;
    }

    uint32_t semente = 0;


    if(argc == 3)
    {
        semente = atoll(argv[2]);
        //cout<<"SEMENTE: \t"<<semente<<"\n";
    }
    else
    {
        //auto aux = std::chrono::high_resolution_clock::now();
        semente = duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();

    }

    seed(semente);

    try
    {

        std::string file(argv[1]);
        const string nomeInst = getNomeInstancia(file);

        cout << "INSTANCIA: \t" << nomeInst << "\n";
        cout<<"SEMENTE: \t"<<semente<<"\n";
        double val = 0.0;
        double tempo = 0.0;
        double best = DOUBLE_MAX;
        int num = 0;


        auto start = std::chrono::high_resolution_clock::now();
        string erro;
        Instance instance(file);

        instance.print();

        for(int i=0; i < NUM_EXEC; ++i)
        {


            Solution sol(instance);
            greedy(sol, instance, 0.5, 0.5);
            //cout<<"i: "<<i<<"\n";

            if(sol.viavel)
            {
                erro = "";

                if(!sol.checkSolution(erro, instance))
                {

                    cout<<"\n\nSOLUCAO:\n\n";
                    sol.print(instance);

                    cout << erro<< "\n****************************************************************************************\n\n";
                    break;

                }
                else
                {

                    num += 1;
                    val += sol.distancia;

                    if(sol.distancia < best)
                        best = sol.distancia;
                }
            }


        }


        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> tempoAux = end - start;
        tempo = tempoAux.count();

        cout<<"\nINST\tDISTANCIA_MEDIA\tBEST\tNUM\tTEMPO\n";
        cout<<nomeInst<<";\t"<<val/num<<";\t"<<best<<";\t"<<num<<";\t"<<tempo<<"\n";

        std::ofstream outfile;
        outfile.open("resultado.csv", std::ios_base::app);
        outfile<<nomeInst<<";\t"<<val/num<<";\t"<<best<<";\t"<<num<<";\t"<<tempo<<"\n";
        outfile.close();

    }
    catch(std::exception &e)
    {
        cout<<"EXCEPTION:\n";
        cout<<e.what()<<"\n";

        cout<<"SEMENTE: \t"<<semente<<"\n";
    }
    catch(char const* exception)
    {
        cout<<"EXCEPTION:\n"<<exception<<"\n\n";
        cout<<"SEMENTE: \t"<<semente<<"\n";


    }





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
        Solution bestB(*instance);
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

                Solution solution(*instance);
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
        std::cerr<<"FORMATO: ./a.out file.txt\n";
        return -1;
    }
    string file(argv[1]);
    Instance *instance = getInstanceFromFile(file);

    int num0, num1;

    do
    {

        cin>>num0>>num1;

        if(num0 != -1 && num1!=-1)
            cout<<"Distancia ("<<num0<<", "<<num1<<"): "<<instance->getDistance(num0, num1)<<"\n";


    }while(num0!=-1 && num1!=-1);
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
    std::vector<float> demands;
    std::vector<std::pair<float, float>> coordinates;
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
    std::vector<std::vector<double>> distMat(demands.size(), std::vector<double>(demands.size(), -1));
    for(int i = 0; i < distMat.size(); i++){
        for(int j = 0; j < distMat.size(); j++){
            distMat[i][j] = distance(coordinates[i], coordinates[j]);
        }
    }
    auto* Inst = new Instance(distMat, truckCap, evCap, evBattery, nSats, nClients, nRS, coordinates, demands);
    return Inst;
}

void saveSolutionToFile(const Solution& Sol, const std::string& fileName){
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
string getNomeInstancia(string str)
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
