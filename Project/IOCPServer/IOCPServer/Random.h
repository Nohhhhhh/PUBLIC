#pragma once

#include "Global.h"
#include <random>

namespace NOH
{
    class CRandom
    {
    public:
        CRandom() {};
        virtual ~CRandom()  {};

    protected:
        bool    Execute( double dProbability )
        {
            std::random_device _rd;
            std::mt19937_64 _rng{ _rd() };
            std::bernoulli_distribution _bernoulli( dProbability );

            return _bernoulli( _rng );   
        };

        size_t  Execute( std::discrete_distribution<size_t> &distObject)   
        {
            std::random_device _rd;
            std::mt19937_64 _rng{ _rd() };

            return distObject( _rng );
        };

    };

}