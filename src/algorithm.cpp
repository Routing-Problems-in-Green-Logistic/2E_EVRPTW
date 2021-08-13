#include "algorithm.h"

bool isFeasibleSolution(Solution& Sol, Instance& Inst){
    float trucksCurrentCap;
    float evCurrentCap, evCurrentBattery;
    std::vector<bool> clientVisited;
    int firstClient = Inst.getFirstClientIndex();
    for(int i = 0; i < Sol.getNTrucks(); i++){ // for each truck route:
        const std::vector<int>& route = Sol.getRoutes()[i]; // TODO: test this const reference;
        if(route[0] != 0 || route[route.size()-1] != 0){ // if the route doesn't start at the depot ("0");
            return false;
        }
        trucksCurrentCap = Inst.getTruckCap();
        for(int s = 0; s < route.size(); s++){ // for each satelite in route
            trucksCurrentCap -= Inst.getDemand(s);
            if(trucksCurrentCap < 0){
                return false;
            }
        }
    }
    for(int i = Sol.getNTrucks(); i < Sol.getNEvs() + Sol.getNTrucks(); i++){ // for each EV route:
        const std::vector<int>& evRoute = Sol.getRoutes()[i];
        if(evRoute[0] != evRoute[evRoute.size()-1]){ // if the route doesn't start at the depot ("0"); // TODO: check if the route starts at a satelite. Currently it only checks if it ends in the same node it starts;
            return false;
        }
        evCurrentCap = Inst.getEvCap();
        evCurrentBattery = Inst.getEvBattery();
        for(int c = 1; c < evRoute.size(); c++){ // for each client in route // whereas c=0 is the satelite;
            clientVisited[evRoute[c] - firstClient] = true;
            evCurrentCap -= Inst.getDemand(evRoute[c]);
            evCurrentBattery -= Inst.getDistance(evRoute[c - 1], evRoute[c]); // TODO: what if the battery coeficient != 1
            if(evCurrentBattery < 0 || evCurrentCap < 0){
                return false;
            }
        }
    }
    for(auto client : clientVisited){ // checks if all clients were visited;
        if(!client){
            return false;
        }
    }
    return true;
}
