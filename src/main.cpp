#include <iostream>
#include "Instance.h"
#include "Solution.h"
#include <fstream>
#include <sstream>
#include <math.h>
#include <time.h>
#include "greedyAlgorithm.h"
#include "Auxiliary.h"
#include <cfloat>

using namespace std;
using namespace GreedyAlgNS;

float distance(std::pair<float, float> p1, std::pair<float,float> p2)
{
    float x1 = p1.first, x2 = p2.first, y1 = p1.second, y2 = p2.second;
    return sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2) * 1.0);
}

void routine(char** filenames, int nFileNames);

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
            coordinates.push_back(std::pair<float,float>(x, y));
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
int main(int argc, char* argv[])
{

    //auto semente = long(1637813360);
    auto semente = time(nullptr);
    cout<<"SEMENTE: "<<semente<<"\n\n";
    srand(semente);

    if(argc != 2)
    {
        std::cerr<<"FORMATO: a.out file.txt\n";
        return -1;
    }

    std::string file(argv[1]);
    Instance *instance = getInstanceFromFile(file);

    Solution solution(*instance);


/*    int vet[9] = {1,12,24,18,23,14,11,9,1};

    float dist = 0.0;

    for(int i=0; (i+1) < 9; ++i)
        dist += instance->getDistance(vet[i], vet[i+1]);

    cout<<"DIST: "<<dist<<"\n";
    exit(-1);*/

    try
    {

        float best = FLOAT_MAX;
        std::string str;

        for(int i=0; i < 200; ++i)
        {

            greedy(solution, *instance, 0.3, 0.3);
            if(solution.viavel)
            {
                if(!solution.checkSolution(str, *instance))
                {
                    cout<<str<<"\n\n";
                    exit(-1);
                }

                float distanciaAux = solution.getDistanciaTotal();

                if(distanciaAux < best)
                    best = distanciaAux;
            }

            cout<<"Best: "<<best<<"\n";


        }
        cout<<"SOLUCAO VIAVEL: "<<solution.viavel<<"\n";
        solution.print();

        cout<<"\n\nTRUCK CAP: "<<instance->getTruckCap()<<"\n";
        cout<<"SOLUCAO VIAVEL: "<<solution.viavel<<"\n";
    }
    catch(std::out_of_range &e)
    {
        std::cerr<<e.what()<<"\n\n";



        exit(-1);
    }
    catch(const char *erro)
    {
        solution.print();

        std::cout<<"CATCH ERRO\n";
        std::cerr<<erro<<"\n\n";
        std::cerr<<"Semente: "<<semente<<"\n";
        exit(-1);
    }



    return 0;
}
void routine(char** filenames, int nFileNames)
{

}
