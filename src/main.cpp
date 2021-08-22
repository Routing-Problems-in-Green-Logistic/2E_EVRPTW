#include <iostream>
#include "Instance.h"
#include "algorithm.h"
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


    Instance* Inst = new Instance(distMat, truckCap, evCap, evBattery, nSats, nClients, nRS, coordinates, demands);
    return Inst;
}
int main(int argc, char** argv){
    Instance* getInstanceFromFile(std::string fileName);
    Instance* Inst = getInstanceFromFile(argv[1]);

    construtivo(*Inst); // TODO: see if passing the Inst like this uses the copy operator or not
    delete Inst;
}

