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

using namespace std;
using namespace GreedyAlgNS;


void routine(char** filenames, int nFileNames);
float distance(std::pair<float, float> p1, std::pair<float,float> p2);
Instance* getInstanceFromFile(std::string &fileName);
void saveSolutionToFile(const Solution& Sol, const std::string& fileName="solution.txt");

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
        cout<<"SEMENTE: "<<semente<<"\n\n";
        seed(semente);
    }


    std::string file(argv[1]);
    Instance *instance = getInstanceFromFile(file); // TODO(samuel): use smart pointer instead, maybe dont even use pointers.
    try
    {
        Solution best_(*instance);
        Solution bestB(*instance);
        float globalBest = FLOAT_MAX;
        float mean = 0;
        for(int n = 0; n < 10; n++)
        {
            if(argc == 2)
            {
                semente = time(nullptr);
                seed(semente);
                cout<<"SEMENTE: "<<semente<<"\n\n";

            }

            float best = FLOAT_MAX;
            std::string str;
            for (int i = 0; i < 1000; ++i) {

                Solution solution(*instance);
                greedy(solution, *instance, 0.3, 0.3);
                if (solution.viavel)
                {
                    float improv;

                    if (!solution.checkSolution(str, *instance)) {
                        cout << str << "\n\n";
                        exit(-1);
                        continue;
                    }
                    float distanciaAux = solution.getDistanciaTotal();

                    //cout << distanciaAux;
                    //NS_LocalSearch::interSatelliteSwap(solution, *instance, improv);


                    float melhora = 10;


                    while(true)
                    {

                        if(!NS_LocalSearch::mvShifitIntraRota(solution, *instance))
                            break;

                        cout<<"LOCAL SEARCH TRUE "<<solution.mvShiftIntraRota<<"\n\n";

                        string erro="";
                        if(!solution.checkSolution(erro, *instance))
                        {
                            cout << erro << "\n\nERRO*********\n";
                            exit(-1);
                        }


                    }

                    //float distLs = solution.getDistanciaTotal(); // TODO: resolver problema que os valores de distancia nas rotas nao estao sendo atualizados corretamente durante busca local swap
                    float distLs = solution.calcCost(*instance);

                    if (distLs < best) {
                        best_ = solution;
                        best = distLs;
                    }
                }


            }
            if(best < globalBest){
                globalBest = best;
                bestB = best_;
            }
            cout << "BEST: " << best <<"; MV SHIFIT INTRA ROTA: "<<(best_.mvShiftIntraRota?"True":"False")<<endl;
            mean += best;



            if(argc == 3)
                break;
        }

        saveSolutionToFile(bestB, "file.txt");

        cout << "globalBEST: " << globalBest << endl;
        cout << "mean: " << mean/10<<"\n\n";
        std::string str;
        bestB.print(str);
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
void routine(char** filenames, int nFileNames)
{

}
float distance(std::pair<float, float> p1, std::pair<float,float> p2)
{
    float x1 = p1.first, x2 = p2.first, y1 = p1.second, y2 = p2.second;
    return (float)sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2) * 1.0);
}
Instance* getInstanceFromFile(std::string &fileName){
    std::ifstream file(fileName);
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
