#ifndef TTP_SEFE_ARRAY_H
#define TTP_SEFE_ARRAY_H

#include <iostream>
#include <array>

template<typename T, std::size_t n>
class Array: std::array<T,n>
{
public:

    explicit Array(const T& value):std::array<T, n>()
    {
        for(size_t i=0; i < n; ++i)
            std::array<T, n>::operator[](i) = value;
    }

    explicit Array():std::array<T, n>()
    {
#if VAR_VECTOR_SANITY_CHECK
        for(size_t i=0; i < n; ++i)
            std::array<T, n>::operator[](i) = T();
#endif
    }


    template <class InputIterator> Array(InputIterator first, InputIterator last): std::array<T,n>(first, last){}


    inline __attribute__((always_inline)) T& operator [] (size_t index)
    {
#if VAR_VECTOR_SANITY_CHECK
        if(index >= std::array<T,n>::size())
        {
            std::cout<<"Erro indice "<<index<<" esta errado para array de tam "<<std::array<T,n>::size()<<"\n";
            throw std::out_of_range("");
        }
#endif
        return std::array<T, n>::operator[](index);
    }

    inline __attribute__((always_inline)) const T& operator [] (size_t index) const
    {
#if VAR_VECTOR_SANITY_CHECK
        if(index >= std::array<T,n>::size())
        {
            std::cout<<"Erro indice "<<index<<" esta errado para array de tam "<<std::array<T,n>::size()<<"\n";
            throw std::out_of_range("");
        }
#endif
        return std::array<T,n>::operator[](index);
    }
};


template <typename T, size_t n>
inline __attribute__((always_inline)) std::ostream& operator << (std::ostream& os, const Array<T, n>& v)
{
    for(unsigned i=0; i<n; ++i) {
        os << v[i];
        if(i+1 < n)
            os << " ";
    }

    return os;
}

#endif //TTP_SEFE_ARRAY_H