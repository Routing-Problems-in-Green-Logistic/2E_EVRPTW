#include "Solution.h"
#include "Auxiliary.h"

Solution::Solution(const Instance &Inst) {
    this->Dep = new Depot(Inst);
    this->satelites = {};

    satelites.reserve(Inst.getNSats());

    for(int i = 0; i < Inst.getNSats(); i++)
    {
        this->satelites.push_back(new Satelite(Inst.getFirstSatIndex() + i, Inst));
    }

    nTrucks = Inst.getN_Trucks();
    primeiroNivel.reserve(nTrucks);

    for(int i=0; i < nTrucks; ++i)
        primeiroNivel.emplace_back(Inst);

}


void Solution::copia(const Solution &solution)
{

}

int Solution::getNSatelites() const {
    return this->satelites.size();
}

Satelite* Solution::getSatelite(int index) {
    return this->satelites.at(index);
}

Depot* Solution::getDepot() {
    return this->Dep;
}

bool Solution::checkSolution(std::string &erro, const Instance &Inst)
{

    // Verifica segundo nivel

    for(Satelite *satelite:satelites)
    {
        if(!satelite->checkSatellite(erro, Inst))
            return false;
    }

    int *vetCliente = new int[Inst.getNClients()];

    for(int i=0; i < Inst.getNClients(); ++i)
        vetCliente[i] = 0;

    for(Satelite *satelite:satelites)
    {
        for(EvRoute &evRoute:satelite->vetEvRoute)
        {
            for(int i=0; i < evRoute.routeSize; ++i)
            {
                if(Inst.isClient(evRoute[i]))
                    vetCliente[evRoute[i]-Inst.getFirstClientIndex()] += 1;

            }
        }
    }

    bool erroB = false;

    for(int i=0; i < Inst.getNClients(); ++i)
    {
        if(vetCliente[i] == 0)
        {
            erro += "ERRO, CLIENTE: "+std::to_string(i+Inst.getFirstClientIndex())+" NAO EH VISITADO\n";
            erroB = false;
        }
        else if(vetCliente[i] > 1)
        {
            erro += "ERRO, CLIENTE: "+std::to_string(i+Inst.getFirstClientIndex())+" EH VISITADO MAIS DE UMA VEZ\n";
            erroB = true;
        }
    }

    delete []vetCliente;

    // Verifica primeiro nivel

    std::vector<float> satelliteDemand;
    satelliteDemand.reserve(Inst.getNSats()+1);

    for(int i=0; i < Inst.getNSats()+1; ++i)
        satelliteDemand.push_back(0.0);

    for(int t=0; t < nTrucks; ++t)
    {
        Route &route = this->primeiroNivel[t];

        if(route.totalDemand > Inst.getTruckCap())
        {
            erro += "ERRO, TRUCK "+ std::to_string(t)+": DEMANDA DO VEICULO EH MAIOR QUE A SUA CAPACIDADE: "+ std::to_string(route.totalDemand)+">"+
                    std::to_string(Inst.getTruckCap())+"\n";

            erroB = true;
        }

        float dist = 0.0;

        if(!route.checkDistence(Inst, &dist))
        {
            erro += "ERRO, TRUCK "+std::to_string(t)+":: DISTANCIA CALCULADA: "+ std::to_string(dist) + " != DISTANCIA R0TA: "+ std::to_string(route.totalDistence)+"\n";
            erroB = true;
        }

        for(int c=1; (c+1)<Inst.getNSats()+1; ++c)
            satelliteDemand[c] += route.satelliteDemand[c];
    }

    for(int c=1; (c+1)<Inst.getNSats()+1; ++c)
    {

        if(std::abs(satelliteDemand[c]- this->satelites[c-1]->demand) > DEMAND_TOLENCE)
        {
            erro += "SATELLITE: "+ to_string(c)+"NAO FOI TOTALMENTE ATENDIDO. DEMANDA: "+ to_string(satelites[c-1]->demand) +
                    "; ATENDIDO: "+to_string(satelliteDemand[c])+"\n";

            erroB = true;
        }
    }

    return !erroB;

}

void Solution::print(std::string &str,  const Instance &instance)
{

    str += "2º NIVEL:\n";

    for(Satelite *satelite:satelites)
    {
        satelite->print(str, instance);
    }
}

void Solution::print(const Instance& Inst)
{

    std::cout<<"2º NIVEL:\n";

    for(Satelite *satelite:satelites)
    {
        satelite->print(Inst);
    }

    std::cout<<"1° NIVEL:\n";
    for(Route &route:primeiroNivel)
    {
        route.print();
    }
    cout << "CUSTO TOTAL: " << this->calcCost(Inst);

}

int Solution::findSatellite(int id) const {
    return -1;

}

double Solution::calcCost(const Instance& Inst) {
    double cost  = 0.0;

    for(int t = 0; t < this->primeiroNivel.size(); t++)
    {
        const auto& truckRoute = this->primeiroNivel.at(t);
        for(int i = 1; i < truckRoute.rota.size(); i++)
        {
            int n0 = truckRoute.rota.at(i-1);
            int n1 = truckRoute.rota.at(i);
            cost += Inst.getDistance(n0, n1);

            if(Inst.isDepot(n1))
                break;
        }
    }

    for(int s = 0; s < this->getNSatelites(); s++)
    {
        const auto& sat = this->getSatelite(s);
        for(int e = 0; e < sat->getNRoutes(); e++)
        {
            auto& evRoute = sat->getRoute(e);
            for(int i = 1; i < evRoute.routeSize; i++)
            {
                int n0 = evRoute.getNodeAt(i-1);
                int n1 = evRoute.getNodeAt(i);
                cost += Inst.getDistance(n0, n1);
            }
        }
    }
    return cost;
}

/*float Solution::getDistanciaTotal() {
    float distancia = 0.0;

    for(auto sat:satelites)
    {
        for(auto route:sat->vetEvRoute)
            distancia += route.distance;
    }

    for(auto route:primeiroNivel)
        distancia += route.totalDistence;

    return distancia;
}*/
