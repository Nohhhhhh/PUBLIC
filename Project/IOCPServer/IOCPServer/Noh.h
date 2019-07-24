#pragma once

namespace CLIENT
{
    class CNoh
    {
    public:
        CNoh();
        ~CNoh();

        bool Initialize( void );
        void Start( void );
        void Stop( void );
        void KeyDetection( void );
        void OutputServerDataToConsole( void );

    private:
        bool m_bService;
        bool m_bKeyLock;
        bool m_bRun;
    };

}