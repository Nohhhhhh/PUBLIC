#pragma once

#include "Global.h"
#include "Random.h"
#include <string>
#include <utility>

namespace CLIENT
{
    typedef struct st_ITEM_IN_BOX
    {
        unsigned long long  ullItemCode;
        unsigned int        uiQuantity;
        double              dProbability;

        st_ITEM_IN_BOX( unsigned long long  _ullItemCode, unsigned int _uiQuantity, double _dProbabilty )
            : ullItemCode(_ullItemCode), uiQuantity(_uiQuantity), dProbability(_dProbabilty) {};

    } ITEM_IN_BOX;

    class CItemBox : public NOH::CRandom
    {
    public:
        CItemBox();
        ~CItemBox();

        bool Initialize( void );

        unsigned long long GetItemCodeInBox( ITEMBOX_TYPE Type );

    private:
        std::vector<std::unique_ptr<ITEM_IN_BOX>> m_vecBox1; 
        std::discrete_distribution<size_t>        m_distBox1;
    };
}

