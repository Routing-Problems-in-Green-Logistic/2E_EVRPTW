#include <iostream>
#include "Instance.h"
#include "algorithm.h"
#include "vns.h"
#include "localSearch.h"
#include <fstream>
#include <sstream>
#include <math.h>
#include <time.h>
#include <boost/numeric/ublas/matrix.hpp>

using namespace std;
using namespace boost;
using namespace numeric::ublas;

void func()
{
    matrix<int> m(5,5);


}

float distance(std::pair<float, float> p1, std::pair<float,float> p2)
{
    float x1 = p1.first, x2 = p2.first, y1 = p1.second, y2 = p2.second;
    return sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2) * 1.0);
}

Instance* getInstanceFromFile(std::string fileName){
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
void solutionToCsv(Solution& Sol, Instance& Inst){
    std::ofstream instanceFile("instance.csv");
    instanceFile <<  "x,y,node,type,demand" << endl;
    for(int i = 0; i < Inst.getNumRechargingS() + Inst.getNSats() + Inst.getNClients() + 1; i++){
        int node = i;
        std::string type;
        if(Inst.isClient(node)) { type = "client"; }
        else if(Inst.isRechargingStation(node)) { type = "rs"; }
        else if(Inst.isSatelite(node)) { type = "sat"; }
        else if(Inst.isDepot(node)) { type = "depot"; }
        else{
            return; }
        //instanceFile << Inst.getCoordinate(node).first << "," << Inst.getCoordinate(node).second << "," << node << "," << type << "," << Inst.getDemand(node) << endl;
        instanceFile << Inst.getCoordinate(node).first << "," << Inst.getCoordinate(node).second << "," << node << "," << type << "," << Inst.getDemand(node)<< std::endl;

    }
    instanceFile.close();

    for(int i = 0; i < Sol.getRoutes().size(); i++){
        bool truckRoute = false;
        const auto& route = Sol.getRoutes().at(i);
        if(i < Sol.getNTrucks()){
            truckRoute = true;
        }

        std::ofstream file("vet_route" + std::string(truckRoute? "T":"E") + std::to_string(i) + ".csv"); // if truck vet_route: routeT2.csv, else routeE4.csv
        file << "x,y,node,type,demand"<< std::endl;
        for(int j = 0; j < route.size(); j++){
            int node = route.at(j);
            std::string type;
            if(Inst.isClient(node)) { type = "client"; }
            else if(Inst.isRechargingStation(node)) {
                type = "rs"; }
            else if(Inst.isSatelite(node)) { type = "sat"; }
            else if(Inst.isDepot(node)) { type = "depot"; }
            else{ return; }
            file << Inst.getCoordinate(node).first << "," << Inst.getCoordinate(node).second << "," << node << "," << type << "," << Inst.getDemand(node)<< std::endl;
        }
        file.close();
    }
}

int main(int argc, char* argv[]){

    /*
     * seed(time(NULL));
     * rand_u32();
     */
    long seend = time(NULL);
    cout<<"Semente: "<<seend<<'\n';
    srand(seend);
    //srand(1);
    void routine(char** filenames, int nFileNames);
    routine(argv, argc);
    return 0;
    /*
    for(int i = 1; i < argc; i++) {
        cout << argv[i] << endl;
        Instance *Inst = getInstanceFromFile(argv[i]);
        if(Inst == nullptr){ return -1; }
        float solCost, localSearchCost;
        //Instance* Inst = getInstanceFromFile(argv[1]);
        Solution *Sol;
        std::vector<std::vector<int>> ser;
        Sol = construtivo(*Inst, ser); // TODO: see if passing the Inst like this uses the copy operator or not
        bool isFeasible = isFeasibleSolution(*Sol, *Inst);
        cout << "feasible: " << isFeasible << endl;
        cout << "Cost:" << (double) Sol->getCost() << endl;
        cout << "teste";
        delete Inst;
    }
    */
    float solCost, localSearchCost;
    Instance* Inst = getInstanceFromFile(argv[1]);
    Solution *Sol;
    std::vector<std::vector<int>> ser;
    Sol = construtivo(*Inst, ser); // TODO: see if passing the Inst like this uses the copy operator or not
    bool isFeasible = isFeasibleSolution(*Sol, *Inst);
    std::vector<std::pair<float,float>> costs;
    Solution cpySol = *Sol; // hoping the copy operator will do the job just fine
    solutionToCsv(cpySol, *Inst);

    float autoSolCost = getSolCost(cpySol, *Inst);
    solCost = getSolCost(cpySol, *Inst);
    //vns::rvnd(cpySol, *Inst);
    vns::gvns(cpySol, *Inst);
    localSearchCost = getSolCost(cpySol, *Inst);
    getchar();
    solutionToCsv(cpySol, *Inst);
    return -11;
}
void routine(char** filenames, int nFileNames){

    //Instance* getInstanceFromFile(std::string fileName);

    if(nFileNames <= 1)
    {
        cerr<<"NUMERO INCORRETO DE PARAMETROS\n";
        exit(-1);
    }

    float media = 0;
    float best = 1e8;
    for(int i = 1; i < nFileNames; i++){
        std::string fileName(filenames[i]);
        cout << "<< " << fileName << " >>" << endl;
        Instance* Inst = getInstanceFromFile(fileName);
        for(int j = 0; j < 10; j++){
        Solution *sol = nullptr;
        std::vector<std::vector<int>> ser;
        float bestConstrCost = 1e8;
        Solution *bestSol = nullptr;
        bool hasSolution = false;

        for(int c = 0; c < 50; c++) {
            sol = construtivo(*Inst, ser); // TODO: see if passing the Inst like this uses the copy operator or not
            //bool isFeasible = isFeasibleSolution(*Sol, *Inst);
            //bool isFeasible = true;
            float ccost = -1;
            //if(isFeasible) {
            ccost = getSolCost(*sol, *Inst);
            if(ccost < bestConstrCost){
                bestConstrCost = ccost;
                delete bestSol;
                bestSol = sol;
                hasSolution = true;
                sol = nullptr;

            }
            else
            {
                delete sol;
                sol = nullptr;
            }
            //}
            //cout << "Greedy Alg Cost: " << bestConstrCost << endl;
            // cout << "feasibility: " << hasSolution << endl;
        }
        cout << "greedy cost: " << bestConstrCost << endl;
        //cout << "feasibility: " << hasSolution << endl;
        bool foundFeasibleGVNSSolution = false;
        if(hasSolution){
                vns::gvns(*bestSol, *Inst);
                float localSearchCost = getSolCost(*bestSol, *Inst);
                cout << "gvns Cost(" << j << "): " << localSearchCost << endl;
                /*
                if(!isFeasibleSolution(cpySol, *Inst)) {
                    cout << "not feasible!" << endl;
                    continue;
                }
                */
                if(localSearchCost < best) {
                    best = localSearchCost;
                    foundFeasibleGVNSSolution = true;
                }
                media += localSearchCost;
            }

            delete bestSol;
        }
        media /= 10;
        cout << "best: " << best << endl;
        cout << "mean: " << media << endl;
        //cout << "feasible GVNS Sol:" <<  foundFeasibleGVNSSolution << endl;

        delete Inst;
    }
        cout << endl;
}
