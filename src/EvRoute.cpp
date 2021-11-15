#include "EvRoute.h"

EvRoute::EvRoute(float evBattery, float evCapacity, int satelite) {
    this->satelite = satelite;
    this->initialBattery = evBattery;
    this->initialCapacity = evCapacity;
    this->remainingCapacity = evCapacity;
    this->nodes = {satelite, satelite};
}
int EvRoute::size() const{
    return (int)this->nodes.size();
}
float EvRoute::getDemand(const Instance& Inst) const{
    return totalDemand;
}
float EvRoute::getCost(const Instance& Inst) const{
    return cost;
}
float EvRoute::getCurrentCapacity() const{
    return this->remainingCapacity;
}
bool EvRoute::insert(int node, int pos, const Instance& Inst){
    if(pos <= 0 || pos >= this->size() - 1){
        return false;
    }
    else if(node < 0){
        return false;
    }
    // get the battery cost;
    // get the demand cost;
    // checar se consegue com a capacidade atual
    // checar se consegue com a bateria atual (a bateria do veiculo antes da proxima recarga.
    //  inserir
    //  atualizar capacidade
    //  atualizar bateria
    //  atualizar custo
    //
    return true;
}
void EvRoute::print() const {
    for(int node : this->nodes){
        std::cout << node << ", ";
    }
    std::cout << std::endl;
}
float EvRoute::getMinDemand() const {
    return -1;
}
float EvRoute::getBatteryAt(int pos, const Instance &Inst) const {
    return 0;
}
bool EvRoute::canInsert(int node, const Instance &Inst, Insertion& Ins) {
    float demand = Inst.getDemand(node);
    float bestInsertionCost = 1e8;
    if(demand > this->getCurrentCapacity()){
        return false;
    }
    // for each position, find the best one to insert.
    for(int pos = 1; pos < this->size(); pos++) {
        int prevNode = this->nodes.at(pos - 1);
        int nextNode = this->nodes.at(pos);
        float distance = Inst.getDistance(prevNode, node) + Inst.getDistance(node, nextNode) -
                         Inst.getDistance(prevNode, nextNode);
        float insertionCost = distance * 1;
        if (insertionCost < bestInsertionCost) {
            bestInsertionCost = insertionCost;
            Ins = Insertion(pos, node, insertionCost, demand, 0, -1, -1);
        }
    }
    if(bestInsertionCost == 1e8){
        return false;
    }
    return true;
}
/*
bool EvRoute::canInsert(int node, const Instance &Inst, Insertion& Ins) {
    float demand = Inst.getDemand(node);
    float bestInsertionCost;
    if(demand > this->getCurrentCapacity()){
        return false;
    }
    // for each position, find the best one to insert.
    for(int pos = 1; pos < this->size(); pos++){
        float remainingBattery = -1;
        int prevRs = 0, nextRs; // INDEX of recharging station (or satellite) next to position;
        // get the battery avaliable at the position
        // and the prev and next Rs';
        for(const auto& rs : this->rechargingStations){
            if(pos < rs.first){
                remainingBattery = rs.second;
                nextRs = rs.first;
                break;
            }
            prevRs = rs.first;
        }
        if(remainingBattery == -1){ // shouldnt ever fall in this if
            return false;
        }
        int prevNode = this->nodes.at(pos -1);
        int nextNode = this->nodes.at(pos);
        float distance = Inst.getDistance(prevNode, node) + Inst.getDistance(node, nextNode) - Inst.getDistance(prevNode, nextNode);
        float insertionCost = distance*1;
        float batteryCost = distance*1;

        // if not enough battery, tries to find recharging station insert position;
        int rs, rsPos;
       if(remainingBattery < batteryCost){
           std::vector<int> betweenRs;
           for(int i = prevRs + 1; i < nextRs; i++){
               if(i == pos){
                   betweenRs.push_back(node);
               }
               betweenRs.push_back(this->nodes.at(i));
           }
           for(int r = Inst.getFirstRsIndex(); r < Inst.getFirstRsIndex() + Inst.getNrs(); r++){
               float rsDistance = Inst.getDistance(prevNode, node) + Inst.getDistance(node, nextNode) - Inst.getDistance(prevNode, nextNode);
               float rsInsertionCost = distance*1;
               float rsBatteryCost = distance*1;

           }
       }
       else {
           rs = -1;
           rsPos = -1;
       //}
       if(insertionCost < bestInsertionCost){
           bestInsertionCost = insertionCost;
           Ins = Insertion(pos, node, insertionCost, rs, rsPos);
       }
    }

    // get the battery cost;
    // get the demand cost;
    // checar se consegue com a capacidade atual
    // checar se consegue com a bateria atual (a bateria do veiculo antes da proxima recarga.
    return true;
}
*/

bool EvRoute::insert(const Insertion &Ins, const Instance &Inst) {
    int pos = Ins.pos;
    int node = Ins.node;
    if(pos <= 0 || pos >= this->size() - 1){
        return false;
    }
    if(node < 0){
        return false;
    }
    // get the battery cost;
    // get the demand cost;
    // checar se consegue com a capacidade atual
    // checar se consegue com a bateria atual (a bateria do veiculo antes da proxima recarga.
    //  inserir
    //  atualizar capacidade
    //  atualizar bateria
    //  atualizar custo
    //
    this->remainingCapacity -= Ins.demand;
    this->nodes.insert(this->nodes.begin() + pos, node);
    this->totalDemand += Ins.demand;
    this->cost += Ins.cost;
    // ignoring battery cost for now;
    //this->rechargingStations;
    return true;
}
