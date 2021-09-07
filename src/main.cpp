#include <iostream>
#include "Instance.h"
#include "algorithm.h"
#include "localsearches.h"
#include "localSearch.h"
#include <fstream>
#include <sstream>
#include <math.h>
using namespace std;

float distance(std::pair<float, float> p1, std::pair<float,float> p2)
{
    float x1 = p1.first, x2 = p2.first, y1 = p1.second, y2 = p2.second;
    return sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2) * 1.0);
}

Instance* getInstanceFromFile(std::string fileName){
    std::ifstream file(fileName);
    std::string line;
    stringstream ss, ssaux, ssaux2;
    int nSats=0, nClients=0, nRS=0;
    std::vector<float> demands;
    std::vector<std::pair<float, float>> coordinates;
    while(getline(file, line)){
        std::cout << line << std::endl;
        std::cout << "first char: " << line[0] << std::endl;
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
            std::cout << x << ", " << y << " -> " << demand << endl;
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
    cout << " ... ";

    getline(file, line);
    ssaux = stringstream(line);
    while(ssaux >> aux) cout << aux << ", ";
    ssaux2 = stringstream(aux);

    ssaux2 >> evCap;

    getline(file, line);
    ssaux = stringstream(line);
    while(ssaux >> aux) cout << aux << ", ";
    ssaux2 = stringstream(aux);
    ssaux2 >> evBattery;
    cout << ".";
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
    for(int i = 0; i < Sol.getRoutes().size(); i++){
        bool truckRoute = false;
        const auto& route = Sol.getRoutes().at(i);
        if(i < Sol.getNTrucks()){
            truckRoute = true;
        }
        std::ofstream file("route" + std::string(truckRoute? "T":"E") + std::to_string(i) + ".csv"); // if truck route: routeT2.csv, else routeE4.csv
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
    Instance* getInstanceFromFile(std::string fileName);
    std::cout << argc;
    // for(int i = 1; i < argc; i++){
        // cout << argv[i] << endl;
        // Instance* Inst = getInstanceFromFile(argv[i]);
        // char* debug = argv[1];

    Instance* Inst = getInstanceFromFile(argv[1]);
    Solution *Sol;
    std::vector<std::vector<int>> ser;
    Sol = construtivo(*Inst, ser); // TODO: see if passing the Inst like this uses the copy operator or not
    bool isFeasible = isFeasibleSolution(*Sol,*Inst);
    cout << "feasible: " << isFeasible << endl;
    cout << "Cost:" << (double)Sol->getCost() << endl;
    cout << "teste";

    std::vector<std::pair<float,float>> costs;
    auto cpyRoutes = Sol->getRoutes();
    //ls::intra2opt(cpyRoutes, *Inst, costs);
    Solution cpySol = *Sol; // hoping the copy operator will do the job just fine
    lsh::reinsertion(cpySol, *Inst);
    isFeasible = isFeasibleSolution(cpySol,*Inst);
    lsh::shift(cpySol, *Inst);
    isFeasible = isFeasibleSolution(cpySol,*Inst);
    lsh::swap(cpySol, *Inst);
    isFeasible = isFeasibleSolution(cpySol,*Inst);
    solutionToCsv(*Sol, *Inst);
    delete Inst;
    //}

}