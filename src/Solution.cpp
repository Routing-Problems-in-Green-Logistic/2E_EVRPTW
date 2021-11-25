#include "Solution.h"

Solution::Solution(const Instance &Inst) {
    this->Dep = new Depot(Inst);
    this->satelites = {};

    satelites.reserve(Inst.getNSats());

    for(int i = 0; i < Inst.getNSats(); i++)
    {
        this->satelites.push_back(new Satelite(Inst.getFirstSatIndex() + i, Inst));
    }

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
            for(int cliente:evRoute.route)
            {
                if(Inst.isClient(cliente))
                    vetCliente[cliente-Inst.getFirstClientIndex()] += 1;

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

    if(erroB)
        return false;
    else
        return true;

}

void Solution::print(std::string &str)
{

    str += "2º NIVEL:\n";

    for(Satelite *satelite:satelites)
    {
        satelite->print(str);
    }
}

void Solution::print()
{

    std::cout<<"2º NIVEL:\n";

    for(Satelite *satelite:satelites)
    {
        satelite->print();
    }
}
