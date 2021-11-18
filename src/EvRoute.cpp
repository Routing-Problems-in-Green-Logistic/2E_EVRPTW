#include <cfloat>
#include "EvRoute.h"

EvRoute::EvRoute(float evBattery, float evCapacity, int satelite, const int RouteSizeMax) {
    this->satelite = satelite;
    this->initialBattery = evBattery;
    this->initialCapacity = evCapacity;
    this->remainingCapacity = evCapacity;

    route.reserve(RouteSizeMax);
    route.push_back(satelite);
    route.push_back(satelite);

    for(int i=2; i < RouteSizeMax; ++i)
        route.push_back(-1);

    rechargingStationRoute.reserve(RouteSizeMax);

    for(int i=0; i < RouteSizeMax; ++i)
        rechargingStationRoute.push_back(false);

    routeSize = 2;
    routeSizeMax = RouteSizeMax;

}
int EvRoute::size() const{
    return routeSize;
}
float EvRoute::getDemand(const Instance& Inst) const{
    return totalDemand;
}
float EvRoute::getCost(const Instance& Inst) const{
    return distance;
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
    // get the battery distance;
    // get the demand distance;
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
    for(int node : this->route){
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
bool EvRoute::canInsert(int node, const Instance &Inst, Insertion &insertion)
{
    float demand = Inst.getDemand(node);
    float bestInsertionCost = FLT_MAX;
    if(demand > this->getCurrentCapacity()){
        return false;
    }
    // for each position, find the best one to insert.
    // Viabilidade de bateria ??
//    for(int pos = 1; pos < this->size(); pos++)
    for(int pos = 1; pos < this->size()-1; pos++)
    {
        /*
        int prevNode = this->route.at(pos - 1);    // ??  Permite a rota 0-1-2... ir para 3-0-1-2...
        int nextNode = this->route.at(pos);
        */


        int prevNode = this->route.at(pos);
        int nextNode = this->route.at(pos+1);

        float distanceAux = Inst.getDistance(prevNode, node) + Inst.getDistance(node, nextNode) - Inst.getDistance(prevNode, nextNode);

        float batteryConsumptionAux = 0.0;

        bool batteryOk = true;

        if(vetRemainingBattery[pos] - distanceAux < 0.0)
            batteryOk = false;

        if(batteryOk)
        {
            for (int i = nextNode; i < routeSize; ++i)
            {

                if(vetRemainingBattery[i] - distanceAux < 0.0)
                {
                    batteryOk = false;
                    break;
                }


                if(rechargingStationRoute[i])
                    break;
            }
        }

        if(batteryOk)
        {
            float insertionCost = distanceAux;
            if (insertionCost < bestInsertionCost)
            {
                bestInsertionCost = insertionCost;
                insertion = Insertion(pos, node, insertionCost, demand, batteryConsumptionAux, insertion.routeId, this->satelite);
            }
        }
        else
        {
            // Realizar a insersao da estacao de recarga

            int rechargingS_Dir = pos;
            int rechargingS_Esq = pos-1;

            // Encontra estacoes de recarga a direita e esquerda. Se nao existir, encontra o satellite

            for(; rechargingS_Dir < routeSize; ++rechargingS_Dir)
            {
                if (rechargingStationRoute[rechargingS_Dir])
                    break;
            }

            for(; rechargingS_Esq >= 0; ++rechargingS_Esq)
            {

                if (rechargingStationRoute[rechargingS_Dir])
                    break;
            }

            // Encontrou estacoes de recarga ou o satellite

            // Encontra uma estacao que nao esta sendo utilizada
            int idRechargingS = -1;

            for(int i=Inst.getFirstRechargingSIndex(); i <= Inst.getEndRechargingSIndex(); ++i)
            {
                if(!rechargingS_inUse(i))
                {
                    idRechargingS = i;
                    break;
                }
            }

            // Realiza a insersao da recharging station entre as duas estacoes (ou satellite)
            if(idRechargingS != -1)
            {
                for(int i=rechargingS_Esq+1; i <= pos; ++i)
                {
                    // Verifica se eh possivel chegar a estacao
                    float remainingBattery = vetRemainingBattery[i] - Inst.getDistance(route[i],idRechargingS) - Inst.getDistance(idRechargingS,route[i+1]);

                    if(remainingBattery >= 0)
                    {
                        remainingBattery = Inst.getEvBattery();
                        bool batteryOk = true;

                        // Verifica a viabilidade da bateria da estacao de recarga ate a estacao a direita (ou satellite)
                        for(int j=i+2; j < rechargingS_Dir; ++j)
                        {
                            remainingBattery -= Inst.getDistance(route[j], route[j+1]);

                            if(remainingBattery < 0)
                            {
                                batteryOk = false;
                                break;
                            }

                        }

                        if(batteryOk)
                        {
                            distanceAux +=  Inst.getDistance(route[i],idRechargingS) + Inst.getDistance(idRechargingS,route[i+1]) - Inst.getDistance(route[i], route[i+1]);
                            insertion = Insertion(pos+1, node, distanceAux, demand, batteryConsumptionAux, insertion.routeId, this->satelite, i);
                            insertion.rechargingS_Id = idRechargingS;
                        }
                    }
                }
            }

        }
    }
    if(bestInsertionCost == 1e8){
        return false;
    }
    return true;

}

bool EvRoute::rechargingS_inUse(int id)
{
    for(auto it:rechargingStationsPos_Rs)
    {
        if(it.second == id)
            return true;
    }

    return false;
}

/*
bool EvRoute::canInsert(int clientId, const Instance &Inst, Insertion& insertion) {
    float demand = Inst.getDemand(clientId);
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
        int prevNode = this->vetEvRoute.at(pos -1);
        int nextNode = this->vetEvRoute.at(pos);
        float distance = Inst.getDistance(prevNode, clientId) + Inst.getDistance(clientId, nextNode) - Inst.getDistance(prevNode, nextNode);
        float insertionCost = distance*1;
        float batteryCost = distance*1;

        // if not enough battery, tries to find recharging station insert position;
        int rs, rsPos;
       if(remainingBattery < batteryCost){
           std::vector<int> betweenRs;
           for(int i = prevRs + 1; i < nextRs; i++){
               if(i == pos){
                   betweenRs.push_back(clientId);
               }
               betweenRs.push_back(this->vetEvRoute.at(i));
           }
           for(int r = Inst.getFirstRsIndex(); r < Inst.getFirstRechargingSIndex() + Inst.getN_RechargingS(); r++){
               float rsDistance = Inst.getDistance(prevNode, clientId) + Inst.getDistance(clientId, nextNode) - Inst.getDistance(prevNode, nextNode);
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
           insertion = Insertion(pos, clientId, insertionCost, rs, rsPos);
       }
    }

    // get the battery distance;
    // get the demand distance;
    // checar se consegue com a capacidade atual
    // checar se consegue com a bateria atual (a bateria do veiculo antes da proxima recarga.
    return true;
}
*/

// Insertion em vector esta errado. Tamanho do vector eh fixo.
bool EvRoute::insert(Insertion &insertion, const Instance &Inst) {
    int pos = insertion.pos;
    int node = insertion.clientId;
    if(pos <= 0 || pos >= this->size() - 1){
        return false;
    }
    if(node < 0){
        return false;
    }
    // get the battery distance;
    // get the demand distance;
    // checar se consegue com a capacidade atual
    // checar se consegue com a bateria atual (a bateria do veiculo antes da proxima recarga.
    //  inserir
    //  atualizar capacidade
    //  atualizar bateria
    //  atualizar custo

    this->remainingCapacity -= insertion.demand;
    this->totalDemand += insertion.demand;
    this->distance += insertion.cost;
    // ignoring battery distance for now;
    //this->rechargingStations;

    int k = pos-1;

    if(insertion.rechargingS_Pos < insertion.pos)
    {
        if(insertion.rechargingS_Pos >= 0)
        {
            this->route.insert(this->route.begin() + insertion.rechargingS_Pos + 1, insertion.rechargingS_Id);
            this->rechargingStationRoute.insert(rechargingStationRoute.begin() + insertion.rechargingS_Pos + 1, true);

            k = insertion.rechargingS_Pos;
        }

        this->route.insert(this->route.begin()+pos+1, node);
        rechargingStationRoute.insert(rechargingStationRoute.begin()+pos+1, false);
    }
    else
    {
        this->route.insert(this->route.begin()+pos+1, node);

        if(insertion.rechargingS_Pos >= 0)
            this->route.insert(this->route.begin()+insertion.rechargingS_Pos+1, insertion.rechargingS_Id);
    }

    routeSize += 1;

    // Atualizar vetRemainingBattery


    if(k == insertion.rechargingS_Pos)
        vetRemainingBattery[k] = Inst.getEvBattery();

    k += 1;

    for(; k < routeSize; ++k)
    {
        if(!Inst.isRechargingStation(route[k]))
            vetRemainingBattery[k] = vetRemainingBattery[k - 1] - Inst.getDistance(route[k - 1], route[k]);
        else
        {
            vetRemainingBattery[k] = Inst.getEvBattery();
            break;
        }
    }
    return true;
}
