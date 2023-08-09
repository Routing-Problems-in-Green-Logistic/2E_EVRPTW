//
// Created by ****** on 30/10/22.
//

/* ****************************************
 * ****************************************
 *  Nome: ***********************
 *  Data:    30/10/22
 *  Arquivo: Hash.h
 * ****************************************
 * ****************************************/

#ifndef INC_2E_EVRP_HASH_H
#define INC_2E_EVRP_HASH_H

#include <cmath>
#include "../Auxiliary.h"

#define inc        1.32
#define fatCargMax 0.75

namespace NS_Hash
{

// https://github.com/srmalins/primelists
    const uint64_t vetPrimos[] = {7, 17, 37, 79, 163, 331, 673, 1361, 2729, 5471, 10949, 21911, 43853, 87719, 175447,
                                  350899, 701819, 1403641, 2807303, 5614657, 11229331, 22458671, 44917381, 89834777};


/*
 *
 * @tparam T        Tipo de dados. Elementos inseriridos devem ser T*
 * @tparam delPtr   Indica se val deve ser deletado
 */

    template<typename T, bool delPtr>
    class HashNo
    {
    public:

        T *val = nullptr;

        HashNo()
        { val = nullptr; }

        HashNo(T *val_)
        { val = val_; }

        ~HashNo()
        {
            if(delPtr && val != nullptr)
                delete val;
        }
    };


/*
 *
 * @tparam T        Tipo de dados. Elementos inseriridos devem ser T*
 * @tparam delPtr   Indica se val deve ser deletado
 *
 * Implementa uma tabela hash com double hash para tratar colisao. Quando a tabela tem fator de carga >= $fatCargMax,
 * uma nova tabela eh criada, com aproximadamente o dobro do tamanho
 */

    template<typename T, bool delPtr>
    class Hash
    {
    public:

        HashNo<T, delPtr> *tabHash = nullptr;
        uint64_t indVetPri = 0;
        uint64_t quant = 0;

        Hash()
        {
            indVetPri = 0;
            tabHash = new HashNo<T, delPtr>[vetPrimos[indVetPri]];

        }

        Hash(uint64_t numElem)
        {
            if(numElem == 0)
                return;

            long double n = numElem;

            numElem = ceil(n * inc);
            indVetPri = sizeof(vetPrimos) - 1;

            for(int i = 0; i < sizeof(vetPrimos); ++i)
            {
                if(vetPrimos[i] <= numElem)
                {
                    indVetPri = i;
                    break;
                }
            }

            tabHash = new HashNo<T, delPtr>[vetPrimos[indVetPri]];
        }

        ~Hash()
        {
            if(indVetPri >= 0)
            {
                if(delPtr)
                {

                    for(uint64_t i = 0; i < vetPrimos[indVetPri]; ++i)
                    {
                        if(tabHash[i].val != nullptr)
                        {
                            tabHash[i].val->print(<#initializer#>, <#initializer#>, false);
                            cout<<"dell "<<i<<"\n";
                            delete tabHash[i].val;
                            tabHash[i].val = nullptr;
                        }
                    }
                }

                delete[]tabHash;
            }
        }

        bool add(T *val)
        {
            quant += 1;
            bool valExiste = false;

            if(indVetPri == -1)
            {
                indVetPri = 0;
                tabHash = new HashNo<T, delPtr>[vetPrimos[indVetPri]];
            }

            uint64_t h0 = val->hash() % vetPrimos[indVetPri];

            if(tabHash[h0].val == nullptr)
            {
                tabHash[h0].val = val;
                valExiste = false;
            } else
            {

                if(!insereVal(vetPrimos[indVetPri], val, h0, valExiste))
                {

                    if(valExiste)
                    {
                        delete val;
                        return false;
                    }

                    // Aumentar tamanho da tabela
                    rehash();

                    if(insereVal(vetPrimos[indVetPri], val, h0, valExiste))
                        return true;
                    else
                    {

                        PRINT_DEBUG("", "ERRO CLASS Hash;  NAO CONSEGUIU INSERIR ELEMENTO APOS HEHASH ????");
                        throw "ERRO";
                    }
                }


            }

            // Verifica o fator de carga

            long double q = quant;
            long double fatorCarga = q / vetPrimos[indVetPri];
            //cout<<"Fator de carga: "<<fatorCarga<<"\n\n";

            if(fatorCarga > fatCargMax)
            {
                if(rehash())
                    return true;
                else
                {

                    PRINT_DEBUG("", "ERRO CLASS Hash;  NAO CONSEGUIU REALIZAR O HEHASH ????");
                    throw "ERRO";
                }
            }

            if(delPtr)
                delete val;

            return false;

        }


        T *getPrimeiro(T *val)
        {
            uint64_t tamHash = vetPrimos[indVetPri];
            uint64_t h0 = val->hash() % tamHash;

            if(tabHash[h0].val != nullptr)
            {
                if((*tabHash[h0].val) == (*val))
                    return tabHash[h0].val;
            }

            uint64_t h1 = 1 + ((val->hash() / tamHash) % (tamHash - 1));
            int i = 1;


            uint64_t next = (h0 + i * h1) % tamHash;

            while(next != h0)
            {
                if(tabHash[next].val != nullptr)
                {
                    if((*tabHash[next].val) == (*val))
                        return tabHash[next].val;
                }

                i += 1;
                next = (h0 + i * h1) % tamHash;
            }

            return nullptr;
        }

    private:

        bool insereVal(uint64_t tamHash, T *val, uint64_t h0, bool &valExiste)
        {

            uint64_t h1 = 1 + ((val->hash() / tamHash) % (tamHash - 1));
            int i = 1;
            uint64_t next = (h0 + i * h1) % tamHash;

            while(next != h0)
            {
                cout<<"ini\n";
                if(tabHash[next].val == nullptr)
                {
                    tabHash[next].val = val;
                    cout<<"fim\n";
                    return true;
                }
                else if((*tabHash[next].val) == (*val))
                {
                    valExiste = true;
                    cout<<"fim\n";
                    return false;
                }

                cout<<"fim\n";

                i += 1;
                next = (h0 + i * h1) % tamHash;
            }

            return false;
        }


        bool rehash()
        {
            //cout<<"rehash\n";

            if(indVetPri == -1)
            {
                indVetPri = 0;
                tabHash = new HashNo<T, delPtr>[vetPrimos[indVetPri]];
                return true;
            }

            // Verifica se eh possivel aumentar a tabela
            if((indVetPri + 1) < sizeof(vetPrimos))
            {

                HashNo<T, delPtr> *newTabHash = new HashNo<T, delPtr>[vetPrimos[indVetPri + 1]];

                for(uint64_t i = 0; i < vetPrimos[indVetPri]; ++i)
                {
                    T *val = tabHash[i].val;
                    tabHash[i].val = nullptr;

                    if(val != nullptr)
                    {

                        uint64_t h0 = val->hash() % vetPrimos[indVetPri + 1];

                        if(newTabHash[h0].val == nullptr)
                            newTabHash[h0].val = val;
                        else
                        {


                            uint64_t h1 =
                                    1 + ((val->hash() / vetPrimos[indVetPri + 1]) % (vetPrimos[indVetPri + 1] - 1));
                            uint64_t t = 1;
                            uint64_t next = (h0 + t * h1) % vetPrimos[indVetPri + 1];


                            while(next != h0)
                            {
                                if(newTabHash[next].val == nullptr)
                                {
                                    newTabHash[next].val = val;
                                    val = nullptr;
                                    break;
                                }

                                t += 1;
                                next = (h0 + t * h1) % vetPrimos[indVetPri + 1];
                            }

                            if(val != nullptr)
                            {

                                PRINT_DEBUG("", "ERRO CLASS Hash;  VOLTA NO PRIMEIRO INDICE DA HASH ??");
                                throw "ERRO";
                            }

                        }

                    }

                }

                delete[]tabHash;
                tabHash = newTabHash;
                indVetPri += 1;

                return true;

            } else
                return false;
        }


    };

/*
 * HashIterator permite percorrer a hash. Nao eh possivel alterar os valores
 */

    template<typename T, bool delPtr>
    class HashIterator
    {
    public:

        uint64_t i_next = 0;
        Hash<T, delPtr> *hash = nullptr;

        HashIterator() = default;

        explicit HashIterator(Hash<T, delPtr> *hash_)
        {
            hash = hash_;
        }

        ~HashIterator()
        {
            hash = nullptr;
        }

        bool hashNext()
        {
            if(hash == nullptr)
                return false;

            return i_next < vetPrimos[hash->indVetPri];
        }

        const T *next()
        {
            cout<<"next()\n";
            if(hash == nullptr)
                return nullptr;

            cout<<"ini next()\n";

            if(i_next < vetPrimos[hash->indVetPri])
            {
                HashNo<T, delPtr> *temp = &hash->tabHash[i_next];
                i_next += 1;

                for(; i_next < vetPrimos[hash->indVetPri]; ++i_next)
                {
                    if(hash->tabHash[i_next].val != nullptr)
                        break;
                }

                cout << "fim next()\n";
                cout << "==null: " << (hash->tabHash[i_next].val == nullptr) << "\n\n";

                return temp->val;
            }
            return nullptr;
        }

        void setHash(Hash<T, delPtr> *hash_)
        {
            hash = hash_;
        }


    };
}

#endif //INC_2E_EVRP_HASH_H
