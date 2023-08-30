//
// Created by ****** on 31/10/22.
//

/* ****************************************
 * ****************************************
 *  Nome: ***********************
 *  Data:    31/10/22
 *  Arquivo: VetorHash.h
 * ****************************************
 * ****************************************/

#ifndef INC_2E_EVRP_VETORHASH_H
#define INC_2E_EVRP_VETORHASH_H

#include "Auxiliary.h"
#include "Solucao.h"
#include "Instancia.h"


namespace NS_VetorHash
{
    class NoEvRoute
    {
    public:

        EvRoute *evRoute = nullptr;
        explicit NoEvRoute(EvRoute &route)
        {
            evRoute = &route;
        }

        bool operator < (const NoEvRoute &noEvRoute) const
        {
            if(noEvRoute.evRoute == nullptr && evRoute != nullptr)
                return true;

            else if(noEvRoute.evRoute == nullptr || evRoute == nullptr)
                return false;


            if((*evRoute)[0].cliente != (*noEvRoute.evRoute)[0].cliente)
                return (*evRoute)[0].cliente < (*noEvRoute.evRoute)[0].cliente;

            if((*evRoute).routeSize != (*noEvRoute.evRoute).routeSize)
                return (*evRoute).routeSize < (*noEvRoute.evRoute).routeSize;

            for(int i=1; i < (*evRoute).routeSize; ++i)
            {
                if((*evRoute)[i].cliente != (*noEvRoute.evRoute)[i].cliente)
                    return (*evRoute)[i].cliente < (*noEvRoute.evRoute)[i].cliente;
            }

            return false;
        }

    };

    class VetorHash
    {
    public:

        bool tipoRota = true;   // Se falso, entao eh uma solucao completa
        Vector<int> vet;   // Vetor guarda a rota ou a solucao
        std::size_t valHash = 0;

        explicit VetorHash(Solucao &solucao, Instancia &instancia)
        {
            tipoRota = false;

            Vector<NoEvRoute> vetNoEvRoute;
            vetNoEvRoute.reserve(instancia.numEv*instancia.numSats);

            for(Satelite &satelite:solucao.satelites)
            {
                for(EvRoute &evRoute:satelite.vetEvRoute)
                {
                    if(evRoute.routeSize > 2)
                        vetNoEvRoute.emplace_back(evRoute);
                }
            }

            std::sort(vetNoEvRoute.begin(), vetNoEvRoute.end());
            int n=0;
            for(NoEvRoute &noEvRoute:vetNoEvRoute)
            {
                for(int i=0; i < (*noEvRoute.evRoute).routeSize; ++i)
                    n += 1;
            }

            if(n > 0)
            {

                vet = Vector<int>();
                vet.reserve(n);

                for(NoEvRoute &noEvRoute: vetNoEvRoute)
                {
                    for(int i = 0; i < (*noEvRoute.evRoute).routeSize; ++i)
                        vet.push_back((*noEvRoute.evRoute).route[i].cliente);
                }

                calculaValHash();
            }

        }

        explicit VetorHash(const EvRoute &evRoute)
        {

            vet = Vector<int>(evRoute.routeSize);

            for(int i=0; i < evRoute.routeSize; ++i)
                vet[i] = evRoute.route[i].cliente;

            calculaValHash();
        }

        VetorHash()=default;

        void calculaValHash()
        {
            // Funcao retirada de: https://cseweb.ucsd.edu/~kube/cls/100/Lectures/lec16/lec16-16.html

            valHash = 0;
            for(int i=0; i < vet.size(); ++i)
            {
                valHash = (valHash<<4) + vet[i];
                uint64_t g = valHash & 0xF0000000L;
                if(g != 0)
                    valHash ^= g >> 24;
                valHash &= ~g;
            }
        }


        void print() const
        {
            if(valHash == 0)
            {
                cout<<"vet vazio\n";
                return;
            }

            cout<<"vetSize: "<<vet.size()<<"\n";
            cout<<"VET: ";
            for(int t:vet)
                cout<<t<<" ";

            cout<<"\nHASH: "<<valHash<<"\n\n";
        }

        bool operator ==(const VetorHash &vetorHash) const
        {
            if(tipoRota != vetorHash.tipoRota)
            {
                cout<<"ERRO, TIPO ROTA DIFERENTES\n";
                PRINT_DEBUG("","");
                throw "ERRO";
            }

            if(valHash != vetorHash.valHash)
                return false;

            if(vet.size() != vetorHash.vet.size())
                return false;

            for(int i=0; i < vet.size(); ++i)
            {
                if(vet[i] != vetorHash.vet[i])
                    return false;
            }

            return true;
        }

        class HashFunc
        {
        public:

            size_t operator()(const VetorHash &vetorHash) const
            {
                return vetorHash.valHash;
            }
        };

    };
}

#endif //INC_2E_EVRP_VETORHASH_H
