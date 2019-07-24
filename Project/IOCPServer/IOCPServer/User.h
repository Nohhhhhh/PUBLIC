#pragma once

#include "Session.h"

namespace CLIENT
{
    class CUser : public NOH::CSession
    {
    public:
        CUser();
        ~CUser();

    protected:
        virtual bool SendPacket(NOH::CPacket & SendPacket, bool *bpSendPacket, const NOH::PQCS_TYPE PQCSType) override;
        virtual void Release(void) override;
        virtual bool PacketParsing(NOH::CPacket & RecvPacket, const NOH::PROTOCOL Protocol) override;
        virtual bool ClientJoin(const NOH::PQCS_TYPE PQCSType) override;

    private:
        bool Echo(NOH::CPacket & RecvPacket);
    };
}