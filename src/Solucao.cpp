#include "Solucao.h"
#include "Auxiliary.h"



Solucao::Solucao(Instancia &inst)
{
    satelites.reserve(inst.getNSats() + 1);

    for(int i = 0; i < inst.getNSats() + 1; i++)
        satelites.emplace_back(inst, i);


    numTrucksMax = inst.getN_Trucks();
    numEvMax = inst.getN_Evs();
    numSats  = inst.numSats;

    primeiroNivel.reserve(numTrucksMax);

    for(int i=0; i < numTrucksMax; ++i)
        primeiroNivel.emplace_back(inst);

    satTempoChegMax.reserve(inst.getNSats() + 1);
    for(int i = 0; i < inst.getNSats() + 1; i++)
        satTempoChegMax.emplace_back(-1.0);

    //vetClientesAtend.reserve(1+Inst.getNSats()+Inst.getN_RechargingS()+Inst.getNClients());
    vetClientesAtend = Vector<int8_t>(1 + inst.getNSats() + inst.getN_RechargingS() + inst.getNClients());

    // Clientes
    std::fill((vetClientesAtend.begin() + inst.getFirstClientIndex()), vetClientesAtend.end(), 0);

    // Satelites
    std::fill((vetClientesAtend.begin()+1), (vetClientesAtend.begin() + inst.getEndSatIndex() + 1), 0);

    // Dep
    vetClientesAtend[0] = -1;

    // Estacoes
    std::fill((vetClientesAtend.begin() + inst.getFirstRS_index()), (vetClientesAtend.begin() + inst.getEndRS_index() + 1), -1);

    inicializaVetClientesAtend(inst);

    vetMatSatEvMv = Vector<Matrix<int>>((inst.numSats+1), Matrix<int>(inst.numEv, NUM_MV, 0));

}


void Solucao::resetaPrimeiroNivel(Instancia &instancia)
{
    primeiroNivel = Vector<Route>(numTrucksMax, Route(instancia));
    distancia = 0.0;

    for(Satelite &sat:satelites)
        distancia += sat.distancia;

    for(int i = 0; i < instancia.getNSats()+1; i++)
        satTempoChegMax[i] = -1;



    //for(int i=instancia.getFirstSatIndex(); i <= instancia.getEndSatIndex(); ++i)
    //    primeiroNivel[i].resetaRoute();

}

void Solucao::recalculaDistSat(Instancia &instancia)
{
    for(Satelite &sat:satelites)
    {
        sat.recalculaDemanda();
        sat.recalculaDist();
    }
}

void Solucao::atualizaVetSatTempoChegMax(const Instancia &instance)
{

    // Inicializa vetor
    for(int i=0; i < instance.getNSats()+1; ++i)
        satTempoChegMax[i] = -1.0;

    // Percorre as rotas do 1° nivel
    for(const Route &route:primeiroNivel)
    {
        if(route.routeSize > 2)
        {
            // Percorre as rotas do 2° nivel
            for(int i=1; i < (route.routeSize-1); ++i)
            {
                const RouteNo &routeNo = route.rota[i];

                if(routeNo.satellite != Instancia::getDepotIndex())
                {
                    if(routeNo.tempoChegada > satTempoChegMax[routeNo.satellite])
                    {
                        satTempoChegMax[routeNo.satellite] = routeNo.tempoChegada;
                        //cout << "Atual. sat: " << routeNo.satellite << "; tempoChegada: "
                    }
                }
            }
        }
    }

}


//void Solucao::copia(const Solucao &solution)

int Solucao::getNSatelites() const {
    return satelites.size()-1;
}

Satelite* Solucao::getSatelite(int index) {
    return &satelites.at(index);
}

// Retorna true se a solucao eh vialvel
bool Solucao::checkSolution(std::string &erro, const Instancia &inst)
{
    double distanciaAux = 0.0;

    // Verifica segundo nivel

    for(Satelite &satelite:satelites)
    {
        if(satelite.sateliteId == 0)
            continue;

        if(!satelite.checkSatellite(erro, inst))
            return false;

        //distanciaAux += satelite.distancia;
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

    Vector<double> satelliteDemand;
    satelliteDemand.reserve(inst.getNSats() + 1);

    for(int i=0; i < inst.getNSats() + 1; ++i)
        satelliteDemand.push_back(0.0);


    string aux;

    for(int t=0; t < numTrucksMax; ++t)
    {
        Route &route = primeiroNivel[t];

        if(route.totalDemand > inst.getTruckCap(t))
        {
            erro += "ERRO, TRUCK " + std::to_string(t) + ": DEMANDA DO VEICULO EH MAIOR QUE A SUA CAPACIDADE: " + std::to_string(route.totalDemand) + ">" +
                    std::to_string(inst.getTruckCap(t)) + "\n";

            erroB = true;
            break;
        }

        double dist = 0.0;
        double demandaRoute = 0.0;

        if(!route.checkDistence(inst, &dist, aux))
        {
            erro += "ERRO, TRUCK "+std::to_string(t)+":: \n" + aux;
            erroB = true;
            break;
        }

        if(route.routeSize > 2)
        {
            if(route.rota[0].tempoChegada != 0.0)
            {
                erro += "ERRO, TRUCK NAO VAZIO "+ to_string(t)+" SAI NO TEMPO(" + to_string(route.rota[0].tempoChegada)+
                        ") != 0";
                erroB = true;
                break;
            }

            for(int i=1; i < route.routeSize; ++i)
            {
                double tempo = route.rota[i-1].tempoChegada + inst.getDistance(route.rota[i-1].satellite, route.rota[i].satellite);

                //if(tempo != route.rota[i].tempoChegada)
                if(!NS_Auxiliary::numerosIguais(tempo, route.rota[i].tempoChegada))
                {
                    erro += "ERRO, TRUCK("+to_string(t)+" TEMPO CHEGADA DIF, CALCULADO("+ to_string(tempo)+"), ROTA("+
                            to_string(route.rota[i].tempoChegada)+")";
                    erroB = true;
                    break;
                }
            }

        }

        if(erroB)
            break;

        distanciaAux += dist;

        for(int c=1; (c+1) < inst.getNSats() + 1; ++c)
            satelliteDemand[c] += route.satelliteDemand[c];
    }



    if(!erroB)
    {


        // Verifica satelite

        for(int c = 1; (c + 1) < inst.getNSats() + 1; ++c)
        {


            if(!satelites[c].checkSatellite(erro, inst))
            {
                erro += "SATELITE " + to_string(c) + " ERRO: " + erro + "\n";
                erroB = true;
                break;
            }

            if(std::abs(satelliteDemand[c] - satelites[c].demanda) > TOLERANCIA_DEMANDA)
            {
                erro += "!SATELLITE: " + to_string(c) + "NAO FOI TOTALMENTE ATENDIDO. DEMANDA: " +
                        to_string(satelites[c].demanda) + "; ATENDIDO: " + to_string(satelliteDemand[c]) + "\n";

                erroB = true;
                break;
            }
        }

        // Verifica a sincronização entre o primeiro e o segundo nivel

        atualizaVetSatTempoChegMax(inst);

        // vet satTempo contem o tempo de chegada mais tarde de um veiculo do primeiro nivel para o i° satelite
        // Todos os tempos de saida do i° satelite tem que ser após o tempo em satTempo

        for(int sat = 1; sat < inst.getNSats() + 1; ++sat)
        {
            const double tempo = satTempoChegMax[sat];
            Satelite &satelite = satelites[sat];

            // Verifica as rotas
            for(EvRoute &evRoute: satelite.vetEvRoute)
            {
                if(evRoute.routeSize > 2)
                {
                    if(evRoute[0].tempoSaida < tempo && !NS_Auxiliary::numerosIguais(evRoute[0].tempoSaida, tempo))
                    {
                        string strRota;
                        evRoute.print(strRota, inst, true);
                        erro += "ERRO SATELITE: " + to_string(sat) + " O TEMPO DE SAIDA DA ROTA: " + strRota +
                                " EH MENOR DO QUE O TEMPO DE CHEGADA DA ULTIMA ROTA DO 1° NIVEL: " + to_string(tempo) +
                                "\nsaida: " + to_string(evRoute[0].tempoSaida) + "; tempo max ~EV: " + to_string(tempo);
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

    if(erroB)
        PRINT_DEBUG("", "");

    return !erroB;

}

void Solucao::print(std::string &saida, const Instancia &instance, const bool somenteNo)
{
/*    saida += "satTempoChegMax: ";

    for(int i=0; i < satTempoChegMax.size(); ++i)
        saida += to_string(i) + "\t\t";

    saida += "\n\t\t\t\t";

    for(int i=0; i < satTempoChegMax.size(); ++i)
        saida += str(boost::format("%.1f\t") % satTempoChegMax[i]);*/

    saida += "!2º NIVEL:\n";

    for(int sat = instance.getFirstSatIndex(); sat <= instance.getEndSatIndex(); ++sat)
    {
        satelites[sat].print(saida, instance, somenteNo);
    }


    saida += "1° NIVEL:\n";
    for(Route &route:primeiroNivel)
    {
        route.print(saida);
    }

    saida+= "DISTANCIA TOTAL SOLUCAO: " + to_string(distancia) + "\n";

}

void Solucao::print(const Instancia& Inst)
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
    cout << "DISTANCIA TOTAL SOLUCAO: " << distancia << "\n";

}

void Solucao::copia(Solucao &solution)
{

    distancia = solution.distancia;
    viavel = solution.viavel;
    numEv = solution.numEv;
    numSats = solution.numSats;
    ultimaA = solution.ultimaA;

    for(int s=0; s <= solution.getNSatelites(); ++s)
        satelites.at(s).copia(solution.satelites[s]);


    for(int i=0; i < primeiroNivel.size(); ++i)
        primeiroNivel.at(i).copia(solution.primeiroNivel[i]);

    //satTempoChegMax = solution.satTempoChegMax;
    satTempoChegMax = solution.satTempoChegMax;

    vetClientesAtend = solution.vetClientesAtend;

    //cout<<"Tempo de chegada 4º arg: "<<solution.satTempoChegMax[4]<<"; Tempo de chegada 4º copia: "<<satTempoChegMax[4]<<"\n\n";

    //vetMatSatEvMv = solution.vetMatSatEvMv;

#if UTILIZA_MAT_MV
    todasRotasEvAtualizadas();
#endif
}


int Solucao::numSatVazios()
{
    int num = 0;

    for(int i=1; i < satelites.size(); ++i)
    {
        Satelite &sat = satelites.at(i);

        num += int(sat.vazio());
    }

    return num;

}

int Solucao::getNumEvNaoVazios()
{

    int num = 0;

    for(Satelite &sat:satelites)
    {
        num += sat.numEv();
    }

    return num;
}


void Solucao::rotaEvAtualizada(const int sat, const int ev)
{


#if UTILIZA_MAT_MV
    for(int i=0; i < NUM_MV; ++i)
        vetMatSatEvMv[sat](ev, i) = 0;
#endif
}

void Solucao::inicializaVetClientesAtend(Instancia &instance)
{

    vetClientesAtend = Vector<int8_t>(1 + instance.getNSats() + instance.getN_RechargingS() + instance.getNClients());

    for(int i = 0; i < instance.getFirstClientIndex(); ++i)
        vetClientesAtend[i] = -1;

    for(int i = instance.getFirstClientIndex(); i < vetClientesAtend.size(); ++i)
        vetClientesAtend[i] = 0;
}

double Solucao::getDist1Nivel()
{
    double dist = 0.0;


    for(const auto& rota:primeiroNivel)
    {
        if(rota.routeSize > 2)
            dist += rota.totalDistence;
    }

    return dist;



}

double Solucao::getDist2Nivel()
{
    double dist = 0.0;

    for(auto &sat:satelites)
    {
        dist += sat.distancia;
    }

    return dist;

}

void Solucao::recalculaDist()
{
    distancia = 0.0;
    for(auto &sat:satelites)
    {
        sat.recalculaDist();
        distancia += sat.distancia;
    }

    for(auto &rota:primeiroNivel)
    {
        if(rota.routeSize > 2)
            distancia += rota.totalDistence;
    }
}


void Solucao::resetaSol()
{
    distancia = 0.0;

    for(int i=1; i < satelites.size(); ++i)
    {
        Satelite &satelite = satelites[i];
        satelite.resetaSat();
    }

    for(Route &route:primeiroNivel)
    {
        route.resetaRoute();
    }


#if UTILIZA_MAT_MV
    todasRotasEvAtualizadas();
#endif
}

void Solucao::todasRotasEvAtualizadas()
{

#if UTILIZA_MAT_MV
    vetMatSatEvMv = Vector<Matrix<int>>((numSats+1), Matrix<int>(numEvMax, NUM_MV, 0));

    for(auto it:vetMatSatEvMv)
    {
        it.clear();
    }
#endif
}

void Solucao::resetaSat(int satId, Instancia &instancia, Vector<int> &vetClienteDel)
{

    if(satId >= 1 && satId <= instancia.getEndSatIndex())
    {
        distancia -= satelites[satId].distancia;
        satelites[satId].distancia = 0.0;
        satelites[satId].demanda   = 0.0;

        vetClienteDel = Vector<int>(instancia.getEndClientIndex()+1, 0);

        Satelite &satelite = satelites[satId];

        for(EvRoute &evRoute:satelite.vetEvRoute)
        {
            if(evRoute.routeSize > 2)
            {
                for(int i=1; i < (evRoute.routeSize-1); ++i)
                    vetClienteDel[evRoute.route[i].cliente] = 1;

                evRoute.distancia = 0.0;
                evRoute.demanda   = 0.0;
                evRoute[1].cliente = satId;
                evRoute.routeSize = 2;
            }
        }

        for(int i=instancia.getFirstClientIndex(); i <= instancia.getEndClientIndex(); ++i)
        {
            if(vetClienteDel[i] == 1)
            {
                //cout<<"\t\t"<<i<<"<- 0\n";
                vetClientesAtend[i] = int8_t(0);
            }
        }

        satTempoChegMax[satId] = -1.0;

    }
    else
    {
        PRINT_DEBUG("", "");
        cout<<"ERRO, SAT ID: "<<satId<<" EH INVIAVEL\n";
        throw "ERRO";
    }

}

void Solucao::reseta1Nivel(Instancia &instancia)
{

    for(Route &route:primeiroNivel)
    {
        if(route.routeSize > 2)
        {
            distancia -= route.totalDistence;
            route.resetaRoute();
        }
    }

}

void Solucao::resetaIndiceEv(Instancia &instancia)
{

    const int evId1 = instancia.getFirstEvIndex();
    for(int sat=1; sat <= instancia.numSats; ++sat)
    {
        Satelite &satelite = satelites[sat];
        for(int ev=0; ev < instancia.numEv; ++ev)
            satelite.vetEvRoute[ev].idRota = ev+evId1;
    }

}

void Solucao::printPlot(string &saida, const Instancia &instance)
{
    for(int s=instance.getFirstSatIndex(); s <= instance.getEndSatIndex(); ++s)
    {

        for(int e=0; e < satelites[s].vetEvRoute.size(); ++e)
        {
            EvRoute &evRoute = satelites[s].vetEvRoute[e];

            if(evRoute.routeSize > 2)
            {
                string rota;

                for(int i=0; i < evRoute.routeSize; ++i)
                {
                    rota += to_string(evRoute[i].cliente) + " ";
                }

                saida += rota+"\n";


            }
        }
    }

    for(int i = 0; i < primeiroNivel.size(); ++i)
    {
        Route &veic = primeiroNivel[i];
        string rota;

        if(veic.routeSize > 2)
        {
            for(int t=0; t < veic.routeSize; ++t)
                rota += to_string(veic.rota[t].satellite) + " ";

            saida+rota+"\n";
        }

    }
}

bool Solucao::ehSplit(const Instancia &instancia)const
{
    static Vector<int> vetSat(instancia.numSats+1);
    NS_Auxiliary::vetSetAll(vetSat, 0);
    int sat=0;

    for(const Route &route:primeiroNivel)
    {
        if(route.routeSize <= 2)
            continue;

        for(int i=1; i < (route.routeSize-1); ++i)
        {
            sat = route.rota[i].satellite;
            vetSat[sat] += 1;
            if(vetSat[sat] > 1)
                return true;
        }

    }

    return false;
}

void Solucao::atualizaDemandaRoute(const Instancia &instancia)
{
    for(Route &route:primeiroNivel)
        route.atualizaTotalDemand(instancia);
}
