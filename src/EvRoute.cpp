#include <cfloat>
#include "EvRoute.h"
#include "Auxiliary.h"

using namespace NS_Auxiliary;
using namespace std;

EvRoute::EvRoute(float evBattery, float evCapacity, int satelite, const int RouteSizeMax) {
    this->satelite = satelite;
    this->initialBattery = evBattery;
    this->initialCapacity = evCapacity;

    route.reserve(RouteSizeMax);
    route.push_back(satelite);
    route.push_back(satelite);

    for(int i=2; i < RouteSizeMax; ++i) {
        route.push_back(-1);
    }
    recharges.emplace_back(1, this->satelite, this->initialBattery);
    rechargingStationRoute.reserve(RouteSizeMax);
    vetRemainingBattery.reserve(RouteSizeMax);

    for(int i=0; i < RouteSizeMax; ++i)
    {
        rechargingStationRoute.push_back(false);
        vetRemainingBattery.push_back(initialBattery);
    }

    routeSize = 2;
    routeSizeMax = RouteSizeMax;
    distance = 0.0;
    totalDemand = 0.0;

}
int EvRoute::size() const{
    return routeSize;
}
float EvRoute::getDemand() const{
    return totalDemand;
}
float EvRoute::getCost(const Instance& Inst) const{
    return distance;
}
float EvRoute::getCurrentCapacity() const{
    return this->initialCapacity - this->totalDemand;
}

void EvRoute::print() const
{
    for(int i=0; i < routeSize; ++i)
    {
        if(rechargingStationRoute[i])
            std::cout<<"RS("<<route[i]<<")_"<<vetRemainingBattery[i]<<"  ";
        else
            std::cout<<"("<<route[i]<<")_"<<vetRemainingBattery[i]<<"  ";
    }
    std::cout <<"\nDistance: "<<distance<<"\n\n";
}
float EvRoute::getMinDemand() const {
    if(this->size() <= 2){
        return 0;
    }
    return minDemand;
}

float EvRoute::getMaxDemand() const {
    return maxDemand;
}

bool EvRoute::canInsert(int node, const Instance &Inst, Insertion &insertion) const
{
    float demand = Inst.getDemand(node);
    float bestInsertionCost = FLT_MAX;
    bool viavel = false;

    if(demand > this->getCurrentCapacity()){
        return false;
    }


    for(int pos = 0; pos < routeSize-1; pos++)
    {

        int prevNode = this->route.at(pos);
        int nextNode = this->route.at(pos+1);


        float distanceAux = Inst.getDistance(prevNode, node) + Inst.getDistance(node, nextNode) - Inst.getDistance(prevNode, nextNode);

        float batteryConsumptionAux = 0.0;

        // Ate nextNode(pos+1)
        float remaingBattery = vetRemainingBattery[pos] - Inst.getDistance(prevNode, node) - Inst.getDistance(node, nextNode);




        bool batteryOk = true;

        if(rechargingStationRoute[pos+1])
        {

            float remainingBatteryAtR_S = vetRemainingBattery[pos] - Inst.getDistance(route[pos], node) - Inst.getDistance(node, route[pos+1]);



            if(remainingBatteryAtR_S < -BATTERY_TOLENCE)
                batteryOk = false;

        }
        else if(remaingBattery < -BATTERY_TOLENCE)
            batteryOk = false;

        if(batteryOk)
        {


            for(int i = pos+1; (i+1) < routeSize; ++i)
            {
                remaingBattery -= Inst.getDistance(route[i], route[i+1]);

                if(remaingBattery < -BATTERY_TOLENCE)
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
                insertion = Insertion(pos, node, insertionCost, demand, batteryConsumptionAux, insertion.routeId, insertion.satId, -1, -1);
                viavel = true;

            }
        }
       else
       {


            // Realizar a insersao da estacao de recarga

            int rechargingS_Dir = pos;
            int rechargingS_Esq = pos-1;


            // Encontra estacoes de recarga a direita e esquerda. Se nao existir, encontra o satellite


            if(pos == 0)
            {
                rechargingS_Dir=1;
                rechargingS_Esq = 0;
            }


            for(; rechargingS_Esq > 0; --rechargingS_Esq)
            {

                if (rechargingStationRoute[rechargingS_Esq] || this->satelite == route[rechargingS_Esq])
                    break;
            }

            if(pos == 0)
            {
                rechargingS_Dir=1;
                rechargingS_Esq = 0;
            }



            for(; rechargingS_Dir < routeSize; ++rechargingS_Dir)
            {
                if (rechargingStationRoute[rechargingS_Dir] || this->satelite == route[rechargingS_Dir])
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
            if(idRechargingS != -1 && !rechargingStationRoute[pos])
            {
                for(int i=rechargingS_Esq; i <= pos; ++i)
                {



                    // ******************************************************************************************



                    // ******************************************************************************************


                    // Verifica se eh possivel chegar a estacao
                    float remainingBattery = vetRemainingBattery[i] - Inst.getDistance(route[i],idRechargingS);//- Inst.getDistance(idRechargingS,route[i+1]);



                    float distAux = 0.0;
                    for(int l=0; (l +1)<= i; ++l)
                        distAux += Inst.getDistance(route[l],route[l+1]);

                    distanceAux = Inst.getDistance(route[i],idRechargingS) - Inst.getDistance(route[i], route[i+1]);
                    distAux += Inst.getDistance(route[i],idRechargingS);

                    if(remainingBattery >= -BATTERY_TOLENCE)
                    {
                        remainingBattery = Inst.getEvBattery();
                        bool batteryOk = true;


                        if(pos==i)
                        {


                            remainingBattery = remainingBattery - Inst.getDistance(idRechargingS, node) - Inst.getDistance(node, route[i+1]);
                            distanceAux += +Inst.getDistance(idRechargingS, node) + Inst.getDistance(node, route[i+1]);
                            distAux += Inst.getDistance(idRechargingS, node) + Inst.getDistance(node, route[i+1]);


                        }
                        else
                        {
                            distanceAux += Inst.getDistance(idRechargingS, route[i+1]);
                            distAux += Inst.getDistance(idRechargingS, route[i+1]);

                            remainingBattery += -Inst.getDistance(idRechargingS, route[i+1]);

                        }





                        bool passouPos = false;

                        // Verifica a viabilidade da bateria da estacao de recarga ate a estacao a direita (ou satellite)
                        for(int j=i+1; (j+1)<= rechargingS_Dir; ++j)
                        {


                            if(j == pos)
                            {
                                remainingBattery += -Inst.getDistance(route[j], node)-Inst.getDistance(node, route[j + 1]);

                                distanceAux += Inst.getDistance(route[j], node)+Inst.getDistance(node, route[j + 1]) -
                                        Inst.getDistance(route[i], route[i+1]);

                                distAux += Inst.getDistance(route[j], node)+Inst.getDistance(node, route[j + 1]);



                            }
                            else
                            {
                                remainingBattery -= Inst.getDistance(route[j], route[j + 1]);


                                distAux += Inst.getDistance(route[j], route[j + 1]);
                            }

                            if(remainingBattery < -BATTERY_TOLENCE)
                            {
                                batteryOk = false;
                                break;
                            }

                        }

                        for(int k=rechargingS_Dir; (k+1) < routeSize; ++k)
                            distAux += Inst.getDistance(route[k], route[k+1]);



                        if(remainingBattery >= -BATTERY_TOLENCE)
                        {

                            distanceAux = distAux - this->distance;

                            if(distanceAux < bestInsertionCost)
                            {
                                insertion = Insertion(pos+1, node, distanceAux, demand, batteryConsumptionAux, insertion.routeId, insertion.satId, i, idRechargingS);
                                bestInsertionCost = distanceAux;
                            }


                            viavel = true;
                            break;
                        }
                    }
                }
            }

       }



        }





    return viavel;

}

bool EvRoute::rechargingS_inUse(int id) const
{
    for(auto it:rechargingStationsPos_Rs)
    {
        if(it.second == id)
            return true;
    }

    return false;
}

bool EvRoute::insert(Insertion &insertion, const Instance &Inst)
{
    const int pos = insertion.pos;
    const int node = insertion.clientId;
    if(pos < 0 || pos >this->size() - 1){
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

    // atualiza estruturas auxiliares:
    this->totalDemand += insertion.demand;
    this->distance += insertion.cost;
    this->maxDemand = insertion.demand > this->maxDemand ? insertion.demand : this->maxDemand;
    this->minDemand = insertion.demand < this->maxDemand ? insertion.demand : this->maxDemand;

    // ignoring battery distance for now;
    //this->rechargingStations;
    int k = pos;
    if(insertion.rechargingS_Pos < insertion.pos)
    {
        // Inclui recharging Station
        if(insertion.rechargingS_Pos >= 0)
        {
            //this->route.insert(this->route.begin() + insertion.rechargingS_Pos + 1, insertion.rechargingS_Id);
            shiftVectorDir(route, insertion.rechargingS_Pos+1, 1, routeSize);
            shiftVectorDir(rechargingStationRoute, insertion.rechargingS_Pos+1, 1, routeSize);
            shiftVectorDir(vetRemainingBattery, insertion.rechargingS_Pos+1, 1, routeSize);

            route[insertion.rechargingS_Pos+1] = insertion.rechargingS_Id;
            rechargingStationRoute[insertion.rechargingS_Pos+1] = true;
            vetRemainingBattery[insertion.rechargingS_Pos+1] = Inst.getEvBattery();

            routeSize += 1;
            //this->rechargingStationRoute.insert(rechargingStationRoute.begin() + insertion.rechargingS_Pos + 1, true);
            k = insertion.rechargingS_Pos;
        }
        shiftVectorDir(route, pos+1, 1, routeSize);
        route[pos+1] = node;
        //this->route.insert(this->route.begin()+pos+1, node);
        shiftVectorDir(rechargingStationRoute, pos+1, 1, routeSize);
        rechargingStationRoute[pos+1] = false;
        shiftVectorDir(vetRemainingBattery, insertion.pos+1, 1, routeSize);
        routeSize += 1;
        // rechargingStationRoute.insert(rechargingStationRoute.begin()+pos+1, false);
    }
    else
    {
        //this->route.insert(this->route.begin()+pos+1, node);
        shiftVectorDir(route, pos+1, 1, routeSize);
        route[pos+1] = node;

        shiftVectorDir(rechargingStationRoute, pos+1, 1, routeSize);
        rechargingStationRoute[pos+1] = false;

        shiftVectorDir(vetRemainingBattery, insertion.pos+1, 1, routeSize);

        routeSize += 1;
        if(insertion.rechargingS_Pos >= 0)
        {
           // this->route.insert(this->route.begin() + insertion.rechargingS_Pos + 1, insertion.rechargingS_Id);
            shiftVectorDir(route, insertion.rechargingS_Pos+1, 1, routeSize);
            route[insertion.rechargingS_Pos+1] = insertion.rechargingS_Id;

            shiftVectorDir(rechargingStationRoute, insertion.rechargingS_Pos+1, 1, routeSize);
            rechargingStationRoute[insertion.rechargingS_Pos+1] = true;

            shiftVectorDir(vetRemainingBattery, insertion.rechargingS_Pos+1, 1, routeSize);

            routeSize += 1;

            // Atualizando estruturas auxiliares (demanda maxima e minima da rota)
            if(insertion.demand < this->getMinDemand()){
                this->minDemand = insertion.demand;
            }
            if(insertion.demand > this->getMaxDemand()){
                this->maxDemand = insertion.demand;
            }
        }
    }
    if(insertion.rechargingS_Pos >= 0)
        rechargingStationsPos_Rs.emplace_back(insertion.rechargingS_Pos, insertion.rechargingS_Id);
    // Atualizar vetRemainingBattery
    if(k == insertion.rechargingS_Pos)
        vetRemainingBattery[k+1] = Inst.getEvBattery();
    if(k == 0)
        vetRemainingBattery[0] = Inst.getEvBattery();
    k += 1;
    for(; k < routeSize; ++k)
    {
        if(!Inst.isRechargingStation(route[k]))
            vetRemainingBattery[k] = vetRemainingBattery[k - 1] - Inst.getDistance(route[k - 1], route[k]);
        else
        {
            vetRemainingBattery[k] = Inst.getEvBattery();
        }
    }
    /*
    updateRecharge(insertion.pos, insertion.batteryCost, true);
    if(insertion.rechargingS_Pos >= 0){
        addRechargeToList(insertion.rechargingS_Pos, insertion.rechargingS_Id, Inst);
    }
    */
    return true;
}

bool EvRoute::checkRoute(std::string &erro, const Instance &Inst) const
{

    if(routeSize < 2)
        return false;

    // Verifica se a rota eh vazia
    if(routeSize == 2)
    {
        if((route[0]==satelite) && (route[1]==satelite))
            return true;
        else
        {
            erro += "ERRO SATELLITE "+ std::to_string(satelite) + "\nINICIO E FIM DA ROTA EH DIFERENTE DE SATELLITE_ID.\nROTA: "+
                                                                  std::to_string(route[0]) + " "+ std::to_string(route[1])+"\n\n";

            return false;
        }

    }

    // Verifica se o inicio e o final da rota sao iguais ao satellite
    if(!((route[0]==satelite) && (route[routeSize-1]==satelite)))
    {
        erro += "ERRO EV_ROUTE, SATELLITE "+ std::to_string(satelite) + "\nINICIO E FIM DA ROTA EH DIFERENTE DE SATELLITE_ID.\nROTA: ";
        print(erro);
        return false;
    }

    float battery = Inst.getEvBattery();
    float distanceAux = 0.0;

    // Verifica se a bateria no  inicio da rota eh igual a quantidade de bateria
    if(vetRemainingBattery[0] != battery)
    {
        erro += "ERRO EV_ROUTE, SATELLITE "+ std::to_string(satelite) + "\nBATERIA RESTANTE INCORRETO. POSICAO: "+std::to_string(0)+"\nROTA: ";

        print(erro);
        return false;
    }

    // Percorre todas as posicoes da rota
    for(int i=1; i < routeSize; ++i)
    {

        // Verifica se a posicao eh uma estacao de recarga
        if(rechargingStationRoute[i] != Inst.isRechargingStation(route[i]))
        {

            erro += "ERRO EV_ROUTE, SATELLITE "+ std::to_string(satelite) + "\nINDICACAO DE ESTACAO DE RECARGA ESTA ERRADO. POSICAO: "+ std::to_string(i)+"\nROTA: ";
        }

        float aux = Inst.getDistance(route[i-1], route[i]);

        distanceAux += aux;
        battery -= aux;

        if(rechargingStationRoute[i])
        {
            // Verifica se eh possivel chegar a estacao de recarga
            if(battery < BATTERY_TOLENCE)
            {
                erro += "ERRO EV_ROUTE, SATELLITE "+ std::to_string(satelite) + "\nBATERIA EH INSUFICIENTE PARA CHEGAR NA ESTACAO DE RECARGA NA "+" POSICAO: "+
                        std::to_string(i)+"\nROTA: ";

                print(erro);

                erro += "BATERIA: "+ to_string(battery);

                return false;
            }

            battery = Inst.getEvBattery();
        }

        // Verifica se battery eh igual a vet vetRemainingBattery
        /* // TODO(samuel): descomentar isso depois de testar;
        if(std::abs(battery - vetRemainingBattery[i]) > BATTERY_TOLENCE || (vetRemainingBattery[i] < -BATTERY_TOLENCE))
        {

            erro += "ERRO EV_ROUTE, SATELLITE "+ std::to_string(satelite) + "\nBATERIA CALCULADA EH DIFERENTE DE EV_ROUTE, "+
                     std::to_string(battery)+" != "+  std::to_string(vetRemainingBattery[i]) + " POSICAO: "+ std::to_string(i)+"\nROTA: ";
            print(erro);

            return false;
        }
         */


    }

    // Verifica se a distancia calculada eh igual a distancia do EV
    /* TODO(samuel) descomentar isso dps de testar
    if(std::abs(distance-distanceAux) > DISTANCE_TOLENCE)
    {

        erro += "ERRO EV_ROUTE, SATELLITE "+ std::to_string(satelite) + "\nDISTANCIA CALCULADA EH DIFERENTE DE EV_ROUTE, CALCULADO: "+
                std::to_string(distanceAux)+" != EV ROUTE: "+  std::to_string(distance) + "\nROTA: ";
        print(erro);

        return false;
    }
     */

    return true;
}

void EvRoute::print(std::string &str) const
{
    if(routeSize > 2)
    {
        for(int i = 0; i < routeSize; ++i)
        {
            if(rechargingStationRoute[i])
                str += "RS(" + std::to_string(route[i]) + ")_" + std::to_string(vetRemainingBattery[i]) + "  ";
            else
                str += "("+std::to_string(route[i]) + ")_" + std::to_string(vetRemainingBattery[i]) + "  ";
        }

        str += "\n";
    }
}

float EvRoute::getInitialCapacity() const {
    return initialCapacity;
}

float EvRoute::getInitialBattery() const {
    return initialBattery;
}

float EvRoute::getDemandOf(int i, const Instance &Inst) const {
    return Inst.getDemand(this->route.at(i));
}

bool EvRoute::isRechargingS(int pos, const Instance& Inst) const {
    return Inst.isRechargingStation(this->route.at(pos));
}

int EvRoute::getNodeAt(int pos) const {
    return this->route.at(pos);
}

float EvRoute::getRemainingBatteryBefore(int pos) const {
    for(auto& recharge : this->recharges) { // O(n) but the size will probably be quite small;
        if (pos < recharge.pos) {
            return recharge.remainingBattery;
        }
    }
    std::cerr << "unexpected to pass the for loop @ EvRoute::getRemainingBatteryBefore";
    exit(13);
    return 0;
}

void EvRoute::replace(int pos, int node, float distance, const Instance &Inst) {
    // partindo do pressuposto que vao ser trocados 2 clientes, nao estacoes de recarga
    float demand = Inst.getDemand(node);
    if(demand < this->getMinDemand()){
        this->minDemand = demand;
    } else if(demand > this->getMaxDemand()){
        this->maxDemand = demand;
    }
    // if maxdemand or mindemand == old_node_demand?????
    this->totalDemand += demand - this->getDemandOf(pos, Inst);
    //this->
    this->route.at(pos) = node;
    // atualizar vetRemainingBattery, tamanho da rota Não,
    vetRemainingBattery.at(pos) -= distance;
    for(int k = pos+1; k < this->vetRemainingBattery.size(); k++){
        if(this->isRechargingS(k, Inst)) {
            break;
        }
        vetRemainingBattery.at(pos) -= distance;
    }
    this->distance += distance; // update distance; Note that if a Recharging station insertion is involved ( which iis not), battery_lost != distance
    updateRecharge(pos, distance, false);
}

void EvRoute::updateRecharge(int pos, float _distance, bool isInsertion) {
    bool hasPassed = false;
    for(auto& recharge : this->recharges){
        if(pos < recharge.pos){
            hasPassed = true;
            recharge.remainingBattery -= _distance;
        }
        if(hasPassed && isInsertion) {
            recharge.pos++;
        }
    }
}

void EvRoute::addRechargeToList(int pos, int rsId, const Instance& Inst) {
    if(checkOutOfBounds(pos, true)){
        std::cerr << "Out of Bounds @ EvRoute::addRechargeToList" << std::endl;
        exit(11);
    }
    int lastPos = 0;
    int start = -1; // first position that goes to the inserted recharging station. (R: rs, nR: new rs, C: client, C*: start) ...--R--C--C--R--C*--C--C--nR--C--...
    for(auto it = recharges.begin(); it != recharges.end(); ++it) {
        if (pos < it->pos) {
            recharges.insert(it, Recharge(pos, rsId, this->initialBattery));
            start = lastPos+1;
        }
        lastPos = it->pos;
    } // using 2 for loops to avoid errors
    if(start == -1){
        std::cerr << "No recharging station found" << std::endl;
        exit(12);
    }
    int end = this->size();
    for(auto it = recharges.begin(); it != recharges.end(); ++it) {
        if (start < it->pos) {
            for(int i = start; i < it->pos; i++){
                it->remainingBattery -= Inst.getDistance(route.at(i-1), route.at(i)); // tira da bateria restante a distancia do ponto i pro ponto anterior.
            }
            end = it->pos;
            ++it; // next recharging station;
            for(int i = end+1; i < it->pos; i++){
                it->remainingBattery -= Inst.getDistance(route.at(i-1), route.at(i)); // tira da bateria restante a distancia do ponto i pro ponto anterior.
            }
            return;
        }
    }
}

void EvRoute::addRechargeToList(int pos, int rsId, float remainingBattery) {
    recharges.emplace_back(pos, rsId, remainingBattery);
}
    bool EvRoute::checkOutOfBounds(int pos, bool notSat) const {
    if(notSat)
        return pos <= 0 || pos >= this->size()-1;
    return pos < 0 || pos >= this->size();
}

void EvRoute::setAuxStructures(const Instance& Inst) {
    float remainingBattery = this->initialBattery;
    this->recharges.clear();
    this->maxDemand = 0;
    this->minDemand = FLOAT_MAX;
    if(this->size() < 2){
        std::cout <<"tamanho < 2" << std::endl;
    }
    for(int i = 1; i < this->size()-1; i++){
        remainingBattery -= getDistance(i-1, i, Inst);
        if(isRechargingS(i, Inst)){
            addRechargeToList(i, route.at(i), remainingBattery);
            remainingBattery = initialBattery;
        }
        else{
            float currentDemand = getDemandOf(i, Inst);
            if(currentDemand > maxDemand){
                maxDemand = currentDemand;
            }
            if(currentDemand < minDemand){
                minDemand = currentDemand;
            }
        }
    }
    int i = size()-1;
    remainingBattery -= getDistance(i-1, i, Inst);
    // add satelite at the end of list of recharges;
    addRechargeToList(i, satelite, remainingBattery);
}

float EvRoute::getDistance(int pos0, int pos1, const Instance& Inst) {
    return Inst.getDistance(route.at(pos0), route.at(pos1));
}
