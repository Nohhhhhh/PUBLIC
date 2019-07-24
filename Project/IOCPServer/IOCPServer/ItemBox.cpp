#include "ItemBox.h"

CLIENT::CItemBox::CItemBox()
{
}

CLIENT::CItemBox::~CItemBox()
{
}

bool CLIENT::CItemBox::Initialize(void)
{
    m_vecBox1.emplace_back(std::move(std::make_unique<ITEM_IN_BOX>( 1,        1,       75000 )));
    m_vecBox1.emplace_back(std::move(std::make_unique<ITEM_IN_BOX>( 2,        1,       75000 )));
    m_vecBox1.emplace_back(std::move(std::make_unique<ITEM_IN_BOX>( 3,        1,       200000 )));
    m_vecBox1.emplace_back(std::move(std::make_unique<ITEM_IN_BOX>( 4,        1,       400000 )));
    m_vecBox1.emplace_back(std::move(std::make_unique<ITEM_IN_BOX>( 5,        1,       400000 )));
    m_vecBox1.emplace_back(std::move(std::make_unique<ITEM_IN_BOX>( 6,        1,       200000 )));
    m_vecBox1.emplace_back(std::move(std::make_unique<ITEM_IN_BOX>( 7,        1,       200000 )));
    m_vecBox1.emplace_back(std::move(std::make_unique<ITEM_IN_BOX>( 8,        1,       300000 )));
    m_vecBox1.emplace_back(std::move(std::make_unique<ITEM_IN_BOX>( 9,        1,       300000 )));
    m_vecBox1.emplace_back(std::move(std::make_unique<ITEM_IN_BOX>( 10,       1,       10000 )));
    m_vecBox1.emplace_back(std::move(std::make_unique<ITEM_IN_BOX>( 11,       1,       25000 )));
    m_vecBox1.emplace_back(std::move(std::make_unique<ITEM_IN_BOX>( 12,       1,       10000 )));
    m_vecBox1.emplace_back(std::move(std::make_unique<ITEM_IN_BOX>( 13,       1,       25000 )));
    m_vecBox1.emplace_back(std::move(std::make_unique<ITEM_IN_BOX>( 14,       1,       30000 )));
    m_vecBox1.emplace_back(std::move(std::make_unique<ITEM_IN_BOX>( 15,       1,       30000 )));
    m_vecBox1.emplace_back(std::move(std::make_unique<ITEM_IN_BOX>( 16,       1,       30000 )));
                                          
    m_vecBox1.emplace_back(std::move(std::make_unique<ITEM_IN_BOX>( 17,       1,       25000 )));
    m_vecBox1.emplace_back(std::move(std::make_unique<ITEM_IN_BOX>( 18,       1,       25000 )));
    m_vecBox1.emplace_back(std::move(std::make_unique<ITEM_IN_BOX>( 19,       1,       25000 )));
    m_vecBox1.emplace_back(std::move(std::make_unique<ITEM_IN_BOX>( 20,       1,       25000 )));
    m_vecBox1.emplace_back(std::move(std::make_unique<ITEM_IN_BOX>( 21,       1,       25000 )));
    m_vecBox1.emplace_back(std::move(std::make_unique<ITEM_IN_BOX>( 22,       1,       30000 )));
    m_vecBox1.emplace_back(std::move(std::make_unique<ITEM_IN_BOX>( 23,       1,       30000 )));
    m_vecBox1.emplace_back(std::move(std::make_unique<ITEM_IN_BOX>( 24,       1,       30000 )));
    m_vecBox1.emplace_back(std::move(std::make_unique<ITEM_IN_BOX>( 25,       1,       30000 )));
    m_vecBox1.emplace_back(std::move(std::make_unique<ITEM_IN_BOX>( 26,       1,       25000 )));
    m_vecBox1.emplace_back(std::move(std::make_unique<ITEM_IN_BOX>( 27,       1,       25000 )));
                                          
    m_vecBox1.emplace_back(std::move(std::make_unique<ITEM_IN_BOX>( 28,       1,       6500 )));
    m_vecBox1.emplace_back(std::move(std::make_unique<ITEM_IN_BOX>( 29,       1,       6500 )));
    m_vecBox1.emplace_back(std::move(std::make_unique<ITEM_IN_BOX>( 30,       1,       6500 )));
    m_vecBox1.emplace_back(std::move(std::make_unique<ITEM_IN_BOX>( 31,       1,       5500 )));
    m_vecBox1.emplace_back(std::move(std::make_unique<ITEM_IN_BOX>( 32,       1,       5500 )));
    m_vecBox1.emplace_back(std::move(std::make_unique<ITEM_IN_BOX>( 33,       1,       5500 )));
    m_vecBox1.emplace_back(std::move(std::make_unique<ITEM_IN_BOX>( 34,       1,       4250 )));
    m_vecBox1.emplace_back(std::move(std::make_unique<ITEM_IN_BOX>( 36,       1,       4250 )));
    m_vecBox1.emplace_back(std::move(std::make_unique<ITEM_IN_BOX>( 37,       1,       4250 )));
    m_vecBox1.emplace_back(std::move(std::make_unique<ITEM_IN_BOX>( 38,       1,       4000 )));
    m_vecBox1.emplace_back(std::move(std::make_unique<ITEM_IN_BOX>( 39,       1,       4000 )));
    m_vecBox1.emplace_back(std::move(std::make_unique<ITEM_IN_BOX>( 35,       1,       4250 )));
                                           
    m_vecBox1.emplace_back(std::move(std::make_unique<ITEM_IN_BOX>( 40,       1,       250 )));
    m_vecBox1.emplace_back(std::move(std::make_unique<ITEM_IN_BOX>( 41,       1,       250 )));
    m_vecBox1.emplace_back(std::move(std::make_unique<ITEM_IN_BOX>( 42,       1,       250 )));
    m_vecBox1.emplace_back(std::move(std::make_unique<ITEM_IN_BOX>( 43,       1,       250 )));
    m_vecBox1.emplace_back(std::move(std::make_unique<ITEM_IN_BOX>( 44,       1,       275 )));
    m_vecBox1.emplace_back(std::move(std::make_unique<ITEM_IN_BOX>( 45,       1,       275 )));
    m_vecBox1.emplace_back(std::move(std::make_unique<ITEM_IN_BOX>( 46,       1,       250 )));
    m_vecBox1.emplace_back(std::move(std::make_unique<ITEM_IN_BOX>( 47,       1,       250 )));
    m_vecBox1.emplace_back(std::move(std::make_unique<ITEM_IN_BOX>( 48,       1,       250 )));
    m_vecBox1.emplace_back(std::move(std::make_unique<ITEM_IN_BOX>( 49,       1,       300 )));
    m_vecBox1.emplace_back(std::move(std::make_unique<ITEM_IN_BOX>( 50,       1,       300 )));
    m_vecBox1.emplace_back(std::move(std::make_unique<ITEM_IN_BOX>( 51,       1,       300 )));
    m_vecBox1.emplace_back(std::move(std::make_unique<ITEM_IN_BOX>( 52,       1,       300 )));
    m_vecBox1.emplace_back(std::move(std::make_unique<ITEM_IN_BOX>( 53,       1,       300 )));
    m_vecBox1.emplace_back(std::move(std::make_unique<ITEM_IN_BOX>( 54,       1,       300 )));
    m_vecBox1.emplace_back(std::move(std::make_unique<ITEM_IN_BOX>( 55,       1,       300 )));
    m_vecBox1.emplace_back(std::move(std::make_unique<ITEM_IN_BOX>( 56,       1,       300 )));
    m_vecBox1.emplace_back(std::move(std::make_unique<ITEM_IN_BOX>( 57,       1,       300 )));
                                                
    m_vecBox1.emplace_back(std::move(std::make_unique<ITEM_IN_BOX>( 58,       1,       7 )));
    m_vecBox1.emplace_back(std::move(std::make_unique<ITEM_IN_BOX>( 59,       1,       7 )));
    m_vecBox1.emplace_back(std::move(std::make_unique<ITEM_IN_BOX>( 60,       1,       7 )));
    m_vecBox1.emplace_back(std::move(std::make_unique<ITEM_IN_BOX>( 61,       1,       7 )));
    m_vecBox1.emplace_back(std::move(std::make_unique<ITEM_IN_BOX>( 62,       1,       7 )));
    m_vecBox1.emplace_back(std::move(std::make_unique<ITEM_IN_BOX>( 63,       1,       5 )));
    m_vecBox1.emplace_back(std::move(std::make_unique<ITEM_IN_BOX>( 64,       1,       5 )));
    m_vecBox1.emplace_back(std::move(std::make_unique<ITEM_IN_BOX>( 65,       1,       5 )));
    m_vecBox1.emplace_back(std::move(std::make_unique<ITEM_IN_BOX>( 66,       1,       5 )));
                                            
    m_vecBox1.emplace_back(std::move(std::make_unique<ITEM_IN_BOX>( 67,       1,      7328945 )));
    
    std::vector<double> _vectempbox1prob;

    for ( auto &ptr : m_vecBox1 )
        _vectempbox1prob.emplace_back( ptr.get()->dProbability );

    m_distBox1 = { _vectempbox1prob.cbegin(), _vectempbox1prob.cend() };
  
    return true;
}

unsigned long long CLIENT::CItemBox::GetItemCodeInBox(ITEMBOX_TYPE Type)
{
    switch( Type )
    {
    case ITEMBOX_TYPE::BOX1:
        return m_vecBox1[Execute(m_distBox1)].get()->ullItemCode;
    }

    return 0;
}
