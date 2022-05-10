#include "Solution.h"
#include "Auxiliary.h"



Solution::Solution(const Instance &Inst)
{
    satelites.reserve(Inst.getNSats()+1);

    for(int i = 0; i < Inst.getNSats()+1; i++)
        satelites.emplace_back(Inst, i);


    numTrucks = Inst.getN_Trucks();
    numEvs = Inst.getN_Evs();

    primeiroNivel.reserve(numTrucks);

    for(int i=0; i < numTrucks; ++i)
        primeiroNivel.emplace_back(Inst);

    satTempoChegMax.reserve(Inst.getNSats()+1);
    for(int i = 0; i < Inst.getNSats()+1; i++)
        satTempoChegMax.emplace_back(-1.0);


}

void Solution::atualizaVetSatTempoChegMax(const Instance &instance)
{

    // Inicializa vetor
    for(int i=0; i < instance.getNSats()+1; ++i)
        satTempoChegMax[i] = -1.0;

    // Percorre as rotas do 1° nivel
    for(const Route &route:primeiroNivel)
    {
        // Percorre as rotas do 2° nivel
        for(const RouteNo &routeNo:route.rota)
        {
            if(routeNo.satellite != Instance::getDepotIndex())
            {
                if(routeNo.tempoChegada > satTempoChegMax[routeNo.satellite])
                    satTempoChegMax[routeNo.satellite] = routeNo.tempoChegada;
            }
        }
    }

}


//void Solution::copia(const Solution &solution)

int Solution::getNSatelites() const {
    return satelites.size()-1;
}

Satelite* Solution::getSatelite(int index) {
    return &satelites.at(index);
}

bool Solution::checkSolution(std::string &erro, const Instance &inst)
{
    double distanciaAux = 0.0;

    // Verifica segundo nivel

    for(Satelite &satelite:satelites)
    {
        if(satelite.sateliteId == 0)
            continue;

        if(!satelite.checkSatellite(erro, inst))
            return false;

        distanciaAux += satelite.distancia;
    }

    int *vetCliente = new int[inst.getNClients()];

    for(int i=0; i < inst.getNClients(); ++i)
        vetCliente[i] = 0;

    for(Satelite &satelite:satelites)
    {

        if(satelite.sateliteId == 0)
            continue;

        for(EvRoute &evRoute:satelite.vetEvRoute)
        {
            for(int i=0; i < evRoute.routeSize; ++i)
            {
                if(inst.isClient(evRoute[i].cliente))
                    vetCliente[evRoute[i].cliente - inst.getFirstClientIndex()] += 1;

            }
        }
    }

    bool erroB = false;

    for(int i=0; i < inst.getNClients(); ++i)
    {
        if(vetCliente[i] == 0)
        {
            erro += "ERRO, CLIENTE: " + std::to_string(i + inst.getFirstClientIndex()) + " NAO EH VISITADO\n";
            erroB = false;
        }
        else if(vetCliente[i] > 1)
        {
            erro += "ERRO, CLIENTE: " + std::to_string(i + inst.getFirstClientIndex()) + " EH VISITADO MAIS DE UMA VEZ\n";
            erroB = true;
        }
    }

    delete []vetCliente;

    // Verifica primeiro nivel

    // Verifica se as demandas dos satelites sao atendidas

    std::vector<double> satelliteDemand;
    satelliteDemand.reserve(inst.getNSats() + 1);

    for(int i=0; i < inst.getNSats() + 1; ++i)
        satelliteDemand.push_back(0.0);


    string aux;

    for(int t=0; t < numTrucks; ++t)
    {
        Route &route = primeiroNivel[t];

        if(route.totalDemand > inst.getTruckCap(t))
        {
            erro += "ERRO, TRUCK " + std::to_string(t) + ": DEMANDA DO VEICULO EH MAIOR QUE A SUA CAPACIDADE: " + std::to_string(route.totalDemand) + ">" +
                    std::to_string(inst.getTruckCap(t)) + "\n";

            erroB = true;
        }

        double dist = 0.0;

        if(!route.checkDistence(inst, &dist, aux))
        {
            erro += "ERRO, TRUCK "+std::to_string(t)+":: " + aux;
            erroB = true;
            break;
        }

        distanciaAux += dist;

        for(int c=1; (c+1) < inst.getNSats() + 1; ++c)
            satelliteDemand[c] += route.satelliteDemand[c];
    }

    // Verifica satelite

    for(int c=1; (c+1) < inst.getNSats() + 1; ++c)
    {


        if(!satelites[c].checkSatellite(erro, inst))
        {
            erro += "SATELITE "+ to_string(c)+" ERRO: "+erro+"\n";
            erroB = true;
            break;
        }

        if(std::abs(satelliteDemand[c]- satelites[c].demanda) > TOLERANCIA_DEMANDA)
        {
            erro += "SATELLITE: "+ to_string(c)+"NAO FOI TOTALMENTE ATENDIDO. DEMANDA: "+ to_string(satelites[c].demanda) +
                    "; ATENDIDO: "+to_string(satelliteDemand[c])+"\n";

            erroB = true;
            break;
        }
    }

    if(!erroB)
    {
        // Verifica a sincronização entre o primeiro e o segundo nivel

        atualizaVetSatTempoChegMax(inst);

        // vet satTempo contem o tempo de chegada mais tarde de um veiculo do primeiro nivel para o i° satelite
        // Todos os tempos de saida do i° satelite tem que ser após o tempo em satTempo

        for(int sat = 1; sat < inst.getNSats()+1; ++sat)
        {
            const double tempo = satTempoChegMax[sat];
            Satelite &satelite = satelites[sat];

            // Verifica as rotas
            for(EvRoute &evRoute: satelite.vetEvRoute)
            {
                if(evRoute.routeSize > 2)
                {
                    if(evRoute[0].tempoSaida < tempo)
                    {
                        string strRota;
                        evRoute.print(strRota, inst, false);
                        erro += "ERRO SATELITE: " + to_string(sat) + " O TEMPO DE SAIDA DA ROTA: " + strRota +
                                " EH MENOR DO QUE O TEMPO DE CHEGADA DA ULTIMA ROTA DO 1° NIVEL: " + to_string(tempo) +
                                "\n";
                        erroB = true;
                        break;
                    }

                    distanciaAux += evRoute.distancia;
                }
            }

            if(erroB)
                break;
        }
    }

    if(!erroB)
    {

        if(abs(distanciaAux - distancia) > TOLERANCIA_DIST_SOLUCAO)
        {
            erro += "ERRO NA DISTANCIA DA SOLUCAO, DIST: " + to_string(distancia) + "; CALCULADO: " +
                    to_string(distanciaAux) + "; TOLERANCIA_DIST_SOLUCAO: " + to_string(TOLERANCIA_DIST_SOLUCAO) + "\n";
            erroB = true;
        }
    }
    return !erroB;

}

void Solution::print(std::string &str,  const Instance &instance)
{

    str += "2º NIVEL:\n";

    for(int sat = instance.getFirstSatIndex(); sat <= instance.getEndSatIndex(); ++sat)
    {
        satelites[sat].print(str, instance);
    }


    str += "1° NIVEL:\n";
    for(Route &route:primeiroNivel)
    {
        route.print(str);
    }

    str+="DISTANCIA TOTAL: "+ to_string(distancia)+"\n";

}

void Solution::print(const Instance& Inst)
{

    std::cout<<"2º NIVEL:\n";

    for(int sat = Inst.getFirstSatIndex(); sat <= Inst.getEndSatIndex(); ++sat)
    {
        satelites[sat].print(Inst);
    }

    std::cout<<"1° NIVEL:\n";
    for(Route &route:primeiroNivel)
    {
        route.print();
    }
    cout << "DISTANCIA TOTAL: " << distancia << "\n";

}

/*int Solution::findSatellite(int id) const {
    return -1;

}*/

double Solution::calcCost(const Instance& Inst) {
    double cost  = 0.0;

    for(int t = 0; t < this->primeiroNivel.size(); t++)
    {
        const auto& truckRoute = this->primeiroNivel.at(t);
        for(int i = 1; i < truckRoute.rota.size(); i++)
        {
            int n0 = truckRoute.rota[i-1].satellite;
            int n1 = truckRoute.rota[i].satellite;
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
                int n0 = evRoute[i-1].cliente;
                int n1 = evRoute[i].cliente;
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
