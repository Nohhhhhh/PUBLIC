#pragma once

namespace NOH
{
    template <class T>
    class CSingleton
    {
    protected:
        CSingleton() = default;
        virtual ~CSingleton() = default;

    public:
        static T *GetInstance(void) 
        { 
            static T m_t;
            return &m_t; 
        }
    }; 

}