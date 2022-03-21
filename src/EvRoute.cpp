#include <cfloat>
#include "EvRoute.h"
#include "Auxiliary.h"
#include "ViabilizadorRotaEv.h"
#include "greedyAlgorithm.h"

using namespace NS_Auxiliary;
using namespace std;
using namespace NameViabRotaEv;
using namespace GreedyAlgNS;



EvRoute::EvRoute(const int _satellite, const int RouteSizeMax, const int _idRota)
{
    satelite = _satellite;

    route.reserve(RouteSizeMax);
    route.emplace_back(satelite, -1.0, -1.0, -1.0);
    route.emplace_back(satelite, -1.0, -1.0, -1.0);

    for(int i=2; i < RouteSizeMax; ++i)
    {
        route.emplace_back();
    }

    routeSize = 2;
    routeSizeMax = RouteSizeMax;
    distancia = 0.0;
    demanda = 0.0;
    idRota = _idRota;

}

int EvRoute::size() const{
    return routeSize;
}


void EvRoute::print(const Instance &instance) const
{
    for(int i=0; i < routeSize; ++i)
    {
        if(instance.isRechargingStation(route[i].cliente))
            std::cout<<"RS("<<route[i]<<";[";
        else
            std::cout<<route[i]<<"([";

        std::cout<<route[i].tempoCheg<<";"<<route[i].tempoSaida<<"] BT: "<<route[i].bateriaRestante<<"\t";
    }

    std::cout <<"\nDistance: "<<distancia<<"\n\n";
}

bool EvRoute::checkRoute(std::string &erro, const Instance &instance) const
{

    if(routeSize < 2)
        return false;

    // Verifica se a rota eh vazia
    if(routeSize == 2)
    {
        if((route[0].cliente == satelite) && (route[1].cliente == satelite) && demanda == 0.0)
            return true;
        else
        {
            if(!((route[0].cliente == satelite) && (route[1].cliente == satelite)))
                erro += "ERRO SATELLITE " + std::to_string(satelite) + "\nINICIO E FIM DA ROTA EH DIFERENTE DE SATELLITE_ID.\nROTA: " +
                        std::to_string(route[0].cliente) + " " + std::to_string(route[1].cliente) + "\n\n";
            else
                erro += "ERRO SATELLITE " + std::to_string(satelite) + "\nDEMANDA (" + to_string(demanda) + ") DEVERIA SER ZERO, ROTA VAZIA\n\n";

            return false;
        }

    }

    // Verifica se o inicio e o final da rota sao iguais ao satellite
    if(!((route[0].cliente == satelite) && (route[routeSize - 1].cliente == satelite)))
    {
        erro += "ERRO EV_ROUTE, SATELLITE " + std::to_string(satelite) + "\nINICIO E FIM DA ROTA SAO DIFERENTES DE SATELLITE_ID.\nROTA: ";
        print(erro, instance);
        return false;
    }

    float battery = instance.getEvBattery(idRota);
    double distanceAux = 0.0;
    float demandaAux = 0.0;

    // Verifica se a bateria no  inicio da rota eh igual a quantidade de bateria
    if(route[0].bateriaRestante != battery)
    {
        erro += "ERRO EV_ROUTE, SATELLITE " + std::to_string(satelite) + "\nBATERIA RESTANTE INCORRETO. POSICAO: " + std::to_string(0) + "\nROTA: ";

        print(erro, instance);
        return false;
    }

    double tempo = route[0].tempoSaida;

    // Percorre todas as posicoes da rota
    for(int i=1; i < routeSize; ++i)
    {

        const int clienteI = route[i].cliente;

        float aux = instance.getDistance(route[i - 1].cliente, route[i].cliente);
        demandaAux += instance.getDemand(route[i].cliente);

        distanceAux += aux;
        battery -= aux;

        // Calcula o tempo de chegada
        tempo += aux;

        if(abs(tempo-route[i].tempoCheg) > TOLERANCIA_TEMPO)
        {
            erro += "ERRO EV_ROUTE, SATELLITE " + std::to_string(satelite) + "\nTEMPO DE CHEGADA AO CLIENTE " + to_string(route[i].cliente) +
                    " ESTA ERRADO. CALCULADO: " + to_string(tempo) + ", ARMAZENADO: " + to_string(route[i].tempoCheg) + "\nROTA:\n";
            print(erro, instance);
            return false;
        }

        // Verifica a janela de tempo
        if(abs(tempo - instance.vectCliente[clienteI].fimJanelaTempo) > TOLERANCIA_JANELA_TEMPO)
        {
            erro += "ERRO EV_ROUTE, SATELLITE " + std::to_string(satelite) + "\nJANELA DE TEMPO DO CLIENTE " + to_string(clienteI) +
                    " EH VIOLADA, FINAL DA TW: " + to_string(instance.vectCliente[clienteI].fimJanelaTempo) + "; TEMPO DE CHEGADA: " +
                    to_string(tempo);
        }

        // Verifica se o EV chega antes do inicio da tw
        if(tempo < instance.vectCliente[clienteI].inicioJanelaTempo)
            tempo = instance.vectCliente[clienteI].inicioJanelaTempo;

        if(instance.isRechargingStation(route[i].cliente))
        {

            // Adiciona o tempo para a recarga completa da bateria

            if(!instance.vectVeiculo[idRota].eletrico)
            {
                erro += "ERRO, ID ROTA: "+ to_string(idRota) + " NAO EH ELETRICO!\n";
                throw "ERRO";
            }

            float tempoRecarga = (instance.getEvBattery(idRota) - battery) * instance.vectVeiculo[idRota].taxaRecarga;
            tempo += tempoRecarga;

        }
        else
            tempo += instance.vectCliente[clienteI].tempoServico;



        // Verifica o tempo de saida
        if(abs(tempo - route[i].tempoSaida) > TOLERANCIA_TEMPO)
        {
            erro += "ERRO EV_ROUTE, SATELLITE " + std::to_string(satelite) + "\nTEMPO DE SAIDA DO CLIENTE " +
                    to_string(route[i].cliente) + " ESTA ERRADO. CALCULADO: " + to_string(tempo) +
                    ", ARMAZENADO: " + to_string(route[i].tempoSaida) + "\nROTA:\n";
            print(erro, instance);
            return false;
        }


        // Verifica a bateria
        if(battery < -TOLERANCIA_BATERIA)
        {
            erro += "ERRO EV_ROUTE, SATELLITE " + std::to_string(satelite) + "\nBATERIA!  " + " POSICAO: " +
                    std::to_string(i) + "\nROTA: ";

            print(erro, instance);

            erro += "BATERIA: "+ to_string(battery);

            return false;
        }

        if(instance.isRechargingStation(route[i].cliente))
            battery = instance.getEvBattery(idRota);

    }


    if(abs(demanda - demandaAux) > TOLERANCIA_DEMANDA)
    {
        string erroPrint;
        print(erroPrint, instance);

        erro += "ERRO, SATELLITE: " + to_string(satelite) + ", ROTA: " + erroPrint + "; DEMANDA ESTA ERRADA, DEMANDA ROTA: " +
                to_string(demanda) + ", DEMANDA CALCULADA: " + to_string(demandaAux) + "\n\n";

        for(int j = 0; j < routeSize; ++j)
        {
            cout << route[j] << ": " << instance.getDemand(route[j].cliente) << "\n";
        }
        cout<<"\n";

        return false;

    }

    if(abs(distanceAux-distancia) > TOLERANCIA_DISTANCIA)
    {

        string erroPrint;
        print(erroPrint, instance);

        erro += "ERRO, SATELLITE: " + to_string(satelite) + ", ROTA: " + erroPrint + "; DISTANCIA ESTA ERRADA, DISTANCIA ROTA: " +
                to_string(distancia) + ", DISTANCIA CALCULADA: " + to_string(distanceAux) + "\n\n";

        return false;
    }

    return true;
}

void EvRoute::print(std::string &str, const Instance &instance) const
{

    for(int i=0; i < routeSize; ++i)
    {
        if(instance.isRechargingStation(route[i].cliente))
            str += "RS("+ to_string(route[i].cliente)+";[";
        else
            str +=  to_string(route[i].cliente)+"([";

        str += to_string(route[i].tempoCheg) + ";" + to_string(route[i].tempoSaida) + "] BT: " + to_string(route[i].bateriaRestante) + "\t";
    }

    str += "\nDistance: " + to_string(distancia) + "\n\n";

}

void EvRoute::atualizaPosMenorFolga(const Instance &instance)
{

    if(routeSize > 2)
    {

        posMenorFolga = -1;
        double menorDiferenca = DOUBLE_MAX;

        for(int i=1; i < (routeSize-1); ++i)
        {
            const EvNo &evNo = route[i];
            double diferenca = evNo.tempoCheg - instance.vectCliente[evNo.cliente].fimJanelaTempo;
            if(diferenca < menorDiferenca)
            {
                menorDiferenca = diferenca;
                posMenorFolga = i;
            }
        }
    }
}