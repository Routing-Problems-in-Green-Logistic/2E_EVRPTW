/* ****************************************************************
 * ****************************************************************
 *
 * NAO EXECUTAR MULTIPLAS INSTANCIAS NA MESMA EXECUCAO!!!
 * Variaveis estaticas!!
 *
 * VERIFICAR ALTERACAO DE TEMPO DO VEIC A COMBUSTAO
 * greedyAlgorithm Linha 653
 *
 * ****************************************************************
 * ****************************************************************
 */


#include <iostream>
#include "Instance.h"
#include "Solucao.h"
#include <fstream>
#include <sstream>
#include <cmath>
#include <ctime>
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
#include <filesystem>
#include "Teste.h"
#include "Grasp.h"
#include "PreProcessamento.h"

using namespace std;
using namespace GreedyAlgNS;
using namespace NS_vnd;
using namespace NameTeste;
using namespace NameS_Grasp;
using namespace N_PreProcessamento;

void routine(char** filenames, int nFileNames);
float distance(std::pair<float, float> p1, std::pair<float,float> p2);
Instance* getInstanceFromFile(std::string &fileName);
void saveSolutionToFile(const Solucao& Sol, const std::string& fileName="solution.txt");
string getNomeInstancia(string str);
void escreveInstancia(const Instance &instance, string file);
void escreveSolucao(Solucao &solution, Instance &instance, string file);

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
    std::string file(argv[1]);
    const string nomeInst = getNomeInstancia(file);

    Instance instance(file, nomeInst);

    //cout<<nomeInst<<" ; "<<instance.numRechargingS<<" ; "<<instance.numSats<<" ; "<<instance.numEv<<" ; "<<instance.numTruck<<"\n";

    //instance.print();

    //return 0;

    try
    {



        string arquivo = "/home/igor/Documentos/Projetos/2E-EVRP-TW/Código/utils/instanciasMod/" + nomeInst + ".txt";
        string arquivoSol = "/home/igor/Documentos/Projetos/2E-EVRP-TW/Código/utils/solucao/" + nomeInst + ".txt";
        std::time_t result = std::time(nullptr);
        auto data = std::asctime(std::localtime(&result));

        cout << "INSTANCIA: " << nomeInst << "\t";
        cout<<"SEMENTE: "<<semente<<"\t"<<data<<"\n\n";

        double tempo = 0.0;
        //instance.print();

        const std::vector<float> vetAlfa{0.1, 0.2, 0.3, 0.4, 0.6, 0.7, 0.9};

        int num = instance.getN_Evs()/2;
        if(num == 0)
            num = 1;

        Parametros parametros(NUM_EXEC, 110, vetAlfa, 100, num);
          //Parametros parametros(100, 20, vetAlfa, 10, num);

        auto start = std::chrono::high_resolution_clock::now();

            ShortestPathSatCli shortestPathSatCli(instance);
            dijkstraSatCli(instance, shortestPathSatCli);

            //PRINT_DEBUG("", "RETURN 0");
            //return 0;

            instance.shortestPath = &shortestPathSatCli;

            Estatisticas estat;
            Solucao *solBest = grasp(instance, parametros, estat);


        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> tempoAux = end - start;
        tempo = tempoAux.count();

/*        cout<<"\nINST \t\tDISTANCIA_MEDIA \tBEST \t\tNUM \tTEMPO\n";
        if(solBest->viavel)
            cout<<nomeInst<<";\t"<<estat.media()<<";\t\t"<<solBest->distancia<<";\t"<<estat.numSol<<";\t"<<tempo<<"\n";
        else
            cout<<nomeInst<<";\t*; \t\t\t*; \t\t"<<estat.numSol<<";\t"<<tempo<<"\n";

        cout<<"viavel: "<<solBest->viavel<<"\n";*/

        escreveSolucao(*solBest, instance, arquivoSol);

        bool exists = std::filesystem::exists("resultado.csv");
        std::ofstream outfile;
        outfile.open("resultado.csv", std::ios_base::app);
        if(!exists)
        {
            string dataStr;
            string temp(data);

            for(auto ch:temp)
            {
                if(ch != '\n')
                    dataStr +=  ch;

            }


            outfile<<dataStr<<";\t;\t;\t;\t;\t;\n";
            outfile<<"nomeInst;\tmedia; \t\t best; \t\tnumSol;\ttempo;\t1° nivel;\t2° nivel\n";
        }

        if(solBest->viavel)
        {
            double dist1Nivel = solBest->getDist1Nivel();
            double dist2Nivel = solBest->getDist2Nivel();

            string saida = str(boost::format("%.2f; \t%.2f;\t\t%d;\t%.2f;\t%.2f;\t\t%.2f") % float(estat.media()) % float(solBest->distancia) % estat.numSol % float(tempo) % float(dist1Nivel/solBest->distancia) % float(dist2Nivel/solBest->distancia));
            //string saida = str(boost::format("%.2f") % float(estat.media()));
            //outfile << nomeInst << ";\t" << estat.media() << ";\t " << solBest->distancia << ";\t" << estat.numSol<< ";\t" << tempo << "\n";
            outfile << nomeInst << ";\t" <<saida<<"\n";
            cout<< nomeInst << ";\t" <<saida<<"\n";

        }
        else
        {
            outfile << nomeInst << ";\t*;\t*;\t" << estat.numSol << ";\t" << tempo << "\n";
            cout << nomeInst << ";\t*;\t*;\t" << estat.numSol << ";\t" << tempo << "\n";
        }

        outfile.close();

#if TEMPO_FUNC_VIABILIZA_ROTA_EV
        cout<<"\nTEMPO TOTAL FUNC VIABILIZA ROTA EV: "<<NameViabRotaEv::global_tempo<<"  "<<100.0*(NameViabRotaEv::global_tempo/tempo)<<" % DO TEMPO TOTAL\n";
#endif



        delete solBest;
        return 0;


    }
    catch(std::exception &e)
    {

        instance.print();

        cout<<"EXCEPTION:\n";
        cout<<e.what()<<"\n";

        cout<<"SEMENTE: \t"<<semente<<"\n";
    }
    catch(char const* exception)
    {
        instance.print();

        cout<<"EXCEPTION:\n"<<exception<<"\n\n";
        cout<<"SEMENTE: \t"<<semente<<"\n";


    }


    return 0;


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
    string strInst = getNomeInstancia(file);
    cout<<"INSTANCIA: "<<strInst<<"\n\n";

    Instance instance(file, strInst);
    EvRoute evRoute(1, instance.getFirstEvIndex(), instance.getEvRouteSizeMax(), instance);

    int num0, num1;
    char c;
    bool somenteNo = false;

    cout<<"Somente NO(s/n): ";
    cin>>c;

    if(c == 's')
        somenteNo = true;
    else
        somenteNo = false;

    do
    {

        int i = 1;
        cin>>num0;
        evRoute[0].cliente = num0;

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
        evRoute[0].tempoSaida = instance.vetTempoSaida[num0];
        //evRoute.print(instance, true);

        double dist = NameViabRotaEv::testaRota(evRoute, i, instance, true, evRoute[0].tempoSaida, 0, nullptr);

        cout<<"Dist: "<<dist<<"\n\n";
        string rotaStr;
        evRoute.print(rotaStr, instance, somenteNo);
        cout<<"Rota: "<<rotaStr<<"\n\n";

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

void escreveSolucao(Solucao &solution, Instance &instance, string file)
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

void escreveInstancia(const Instance &instance, string file)
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

        // Code     x       y       demanda

        outfile<<"D "<<instance.vectCliente[0].coordX<<" "<<instance.vectCliente[0].coordY<<" 0\n";

        for(int s=instance.getFirstSatIndex(); s <= instance.getEndSatIndex(); ++s)
            outfile<<"S "<<instance.vectCliente[s].coordX<<" "<<instance.vectCliente[s].coordY<<" 0\n";


        for(int f=instance.getFirstRechargingSIndex(); f <= instance.getEndRechargingSIndex(); ++f)
            outfile<<"F "<<instance.vectCliente[f].coordX<<" "<<instance.vectCliente[f].coordY<<" 0\n";


        for(int c=instance.getFirstClientIndex(); c <= instance.getEndClientIndex(); ++c)
            outfile<<"C "<<instance.vectCliente[c].coordX<<" "<<instance.vectCliente[c].coordY<<" "<<instance.vectCliente[c].demanda<<"\n";

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
