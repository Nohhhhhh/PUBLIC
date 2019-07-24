/*---------------------------------------------------------------

��Ʈ��ũ ��Ŷ�� Ŭ����.
�����ϰ� ��Ŷ�� ������� ����Ÿ�� In, Out �Ѵ�.

- ����.

CPacket cPacket;

�ֱ�.
cPacket << 40030;	or	cPacket << iValue;	(int �ֱ�)
cPacket << 3;		or	cPacket << byValue;	(BYTE �ֱ�)
cPacket << 1.4;	    or	cPacket << fValue;	(float �ֱ�)

����.
cPacket >> iValue;	(int ����)
cPacket >> byValue;	(BYTE ����)
cPacket >> fValue;	(float ����)

!.	���ԵǴ� ����Ÿ FIFO ������ �����ȴ�.
ť�� �ƴϹǷ�, �ֱ�(<<).����(>>) �� ȥ���ؼ� ����ϸ� �ȵȴ�.

16.11.21
- TLS ����

- ���� ����
1. CPacket�� CMemoryPool_TLS<CPacket> ���� 
2. CMemoryPool_TLS�� CMemoryPool_LF<CChunkBlock<TLS_DATA>> ����
3. CMemoryPool_LF�� Alloc�� ���� CChunkBlock ������ ȣ��
4. CMemoryPool_TLS�� Constructor()�� ���� CPacket ������ ȣ��

- ���� ����
1. CMemoryPool_TLS<CPacket> Delete ó���ϸ�, ~CMemoryPool_TLS() ȣ��
2. CMemoryPool_TLS���� CMemoryPool_LF�� Delete ó���ϸ�, ~CMemoryPool_LF() ȣ��
3. CMemoryPool_LF���� ������ ��带 Delete ó���ϸ�, ~CChunkBlock() ȣ��
----------------------------------------------------------------*/
#pragma once

#include <WinSock2.h>

#include "__NOH.h"

// tls
//#include "MemoryPool_TLS.h"
// lf
//#include "MemoryPool_LF.h"
// no_lf
//#include "MemoryPool.h"

namespace NOH
{
    struct exception_PacketOut
    {
	    explicit exception_PacketOut(int iSize) : _RequestOutSize(iSize) {};
	    int _RequestOutSize;
    };

    struct exception_PacketIn
    {
	    explicit exception_PacketIn(int iSize) : _RequestInSize(iSize) {};
	    int _RequestInSize;
    };

    template<class DATA> class CMemoryPool_TLS;
	class CPacket
	{
	public:
        typedef struct st_NET_HEADER
        {
	        char			cCode;
	        unsigned short	sLen;
	        char			cRandCode;
	        unsigned char	ucCheckSum;

            st_NET_HEADER() : cCode(0), sLen(0), cRandCode(0), ucCheckSum(0) {}
        } NET_HEADER;

		//-----------------------------------------------------------------------------------------
		// ������, �ı���.
		//
		// ����ڰ� ����� ������ ��� ���Ǵ� ������
		//-----------------------------------------------------------------------------------------
        explicit CPacket(const int iHeaderSize = static_cast<int>( PACKET::HEADER_MAX_SIZE ));
		explicit CPacket(const int iBufferSize, const int iHeaderSize = static_cast<int>( PACKET::HEADER_MAX_SIZE ));
		virtual	~CPacket();

		//-----------------------------------------------------------------------------------------
		// Param : void
		// Return: void
		//
		// ��Ŷ �ʱ�ȭ.
		//
		// �޸� �Ҵ��� ���⼭ �ϹǷ�, �Ժη� ȣ���ϸ� �ȵȴ�. 
		//-----------------------------------------------------------------------------------------
		void				Initial(void);

		//-----------------------------------------------------------------------------------------
		// Param :  void
		// Return: void
		//
		// ��Ŷ �ı�
		//-----------------------------------------------------------------------------------------
		void				Release(void);

		//-----------------------------------------------------------------------------------------
		// Param : void
		// Return: CPacket *
		//
		// ��Ŷ û��
		//-----------------------------------------------------------------------------------------
		CPacket *			Clear(void);

		//-----------------------------------------------------------------------------------------
		// Param : void
		// Return: int (��Ŷ ���� ������)
		//
		// �� ���� ������ ���
		//-----------------------------------------------------------------------------------------
		int					GetBufferSize(void) { return m_iBufferSize; }

		//-----------------------------------------------------------------------------------------
		// Param : int (���� ������)
		// Return: int (������ ������)
		//
		// ������� ������ ������ ���
		//
		// Header ����� ������ ���� payload ������
		//-----------------------------------------------------------------------------------------
		int					GetPayloadSize(void) { return m_iUsingSize; }

		//-----------------------------------------------------------------------------------------
		// Param : void
		// Return: int (��Ŷ ������)
		//
		// ������� ��Ŷ ������ ���
		//
		// Header ����� ���Ե� ���� ��Ŷ ������
		//-----------------------------------------------------------------------------------------
		int					GetPacketSize(void) { return m_iUsingSize + m_iHeaderSize; }

		//-----------------------------------------------------------------------------------------
		// Param : void
		// Return: int (��Ŷ ������)
		//
		// (Ŀ����) ������� ��Ŷ ������ ���
		//
		// ���ڷ� �Ѿ�� Header ����� ���Ե� ���� ��Ŷ ������
		//-----------------------------------------------------------------------------------------
		int					GetPacketSize_CustomHeader(void) { return m_iUsingSize + m_iHeaderSize; }

		//-----------------------------------------------------------------------------------------
		// Param : void
		// Return: char * (Header ���� ������)
		//
		// ���� ���� ������ ���
		//-----------------------------------------------------------------------------------------
		char				*GetBufferPtr(void) { return m_cpBuffer; }

		//-----------------------------------------------------------------------------------------
		// Param : void
		// Return: char * (Header ���� ������)
		//
		// Header ���� ������ ���
		//-----------------------------------------------------------------------------------------
		char				*GetHeaderBufferPtr(void) { return m_cpBuffer + (static_cast<int>( PACKET::HEADER_MAX_SIZE ) - static_cast<int>( PACKET::HEADER_DEFAULT_SIZE ) ); }

		//-----------------------------------------------------------------------------------------
		// Param : void
		// Return: char * (���̷ε� ���� ������)
		//
		// (Ŀ����) Header ���� ������ ���
		//-----------------------------------------------------------------------------------------
		char				*GetHeaderBufferPtr_CustomHeader(void) { return m_cpBuffer + static_cast<int>( PACKET::HEADER_MAX_SIZE ) - m_iHeaderSize; }
	
		//-----------------------------------------------------------------------------------------
		// Param : void
		// Return: char * (Payload ���� ������)
		//
		// Payload ���� ������ ���
		//
		// Header ������ Payload ���� ������ ���
		//-----------------------------------------------------------------------------------------
		char				*GetPayloadPtr(void) { return m_cpBuffer + static_cast<int>( PACKET::HEADER_MAX_SIZE ); }

		//-----------------------------------------------------------------------------------------
		// Param : void
		// Return: (char *)���� ������
		//
		// Rear ���� ������ ���.
		//-----------------------------------------------------------------------------------------
		char				*GetRearPtr(void) { return m_cpRear; }

		//-----------------------------------------------------------------------------------------
		// Param : void
		// Return: (char *)���� ������
		//
		// Front ���� ������ ���.
		//-----------------------------------------------------------------------------------------
		char				*GetFrontPtr(void) { return m_cpFront; }

		//-----------------------------------------------------------------------------------------
		// Param : char * (Header ������)
		// Return: void
		//
		// Header ������ �ֱ�
		//-----------------------------------------------------------------------------------------
		void				SetHeader(const char & cHeaderPtr);

		//-----------------------------------------------------------------------------------------
		// Param : char * (Header ������)
		// Return: void
		//
		// Header ������ �ֱ�
		//
		// memcpy ��� ����.
		// ������ �̿��ؼ� ���� ����.
		//-----------------------------------------------------------------------------------------
		void				SetHeader_SHORT(const unsigned short shHeader);

		//-----------------------------------------------------------------------------------------
		// Param : char * (Header ������), int (Header ������)
		// Return: void
		//
		// (Ŀ����) Header ������ �ֱ�
		//-----------------------------------------------------------------------------------------
		void				SetHeader_CustomHeader(const char & cHeaderPtr, const int iCustomHeaderSize);

		//-----------------------------------------------------------------------------------------
		// Param :  int (�̵��� ������)
		// Return: int (�̵��� ������)
		//
		// Rear / Front ������ �̵�
		//-----------------------------------------------------------------------------------------
		int					MoveRear(const int iSize);
		int					MoveFront(const int iSize);

		//-----------------------------------------------------------------------------------------
		// Param :  char * (������ ����), int (�� ������)
		// Return: int (�� ������)
		//
		// ������ ����
		//-----------------------------------------------------------------------------------------
		int					Enqueue(const char &crSrc, int iSize);

		//-----------------------------------------------------------------------------------------
		// Param :  void
		// Return: void
		//
		// ��Ŷ ��ȣȭ
		//-----------------------------------------------------------------------------------------
		void				Encode(void);
		
		//-----------------------------------------------------------------------------------------
		// Param : void
		// Return: true (����), flase (����)
		//
		// ��Ŷ ��ȣȭ
		//-----------------------------------------------------------------------------------------
		bool				Decode(NET_HEADER *pHeader = nullptr);

		//-----------------------------------------------------------------------------------------
		// Param : char (��Ŷ ���� ���� �ڵ�), char (XOR ���� �ڵ�), char (XOR ���� �ڵ�)
		// Return: void
		//
		// �ڵ� �ʱ�ȭ
		//-----------------------------------------------------------------------------------------
		static void			SetCode(const char cCode, const char cXORCode1, const char cXORCode2);

		//-----------------------------------------------------------------------------------------
		// Param :  char * (������ ����), int (���� ������)
		// Return: int (���� ������)
		//
		// ������ �б�
		//-----------------------------------------------------------------------------------------
		int					Dequeue(char *cpDest, int iSize);

		//-----------------------------------------------------------------------------------------
		// param : long (ûũ ������)
		// return: void
		//
		// �޸�Ǯ ����
		//
		// ��� ���� = 0, Alloc()���� �����Ҵ�
		// ��� ���� != 0, �̸� �����Ҵ�
		//-----------------------------------------------------------------------------------------
		static void			AllocMemoryPool(const long lChunkSize = 0);

		//-----------------------------------------------------------------------------------------
		// param : void
		// return: void
		//
		// �޸�Ǯ ����
		//-----------------------------------------------------------------------------------------
		static void			DeleteMemoryPool(void);

		//-----------------------------------------------------------------------------------------
		// param : void
		// return: CPacket &(��Ŷ ����ü ���۷���)
		//
		// �޸� Ǯ �Ҵ�
		//-----------------------------------------------------------------------------------------
		static CPacket *	Alloc(void);

		//-----------------------------------------------------------------------------------------
		// param : CPacket &(free�� ���۷���)
		// return: true (��ȯ ����), false (m_iRefCnt 1����)
		//
		// m_lRefCnt ���� == 0 �̸� �޸� Ǯ ��ȯ
		//-----------------------------------------------------------------------------------------
		static bool			Free(CPacket & Data);

		//-----------------------------------------------------------------------------------------
		// param : void
		// return: void
		//
		// m_lRefCnt 1 ����
		//-----------------------------------------------------------------------------------------
		void				AddRef(void);

		bool				EncodeStatus(void);

		// �� ������� Chunk ����	
        static long			GetChunkCount(void) { return m_pMemoryPool->GetChunkCount(); }
		// �� ������� DATA ����
		static long			GetUsingBlockCount(void) { return m_pMemoryPool->GetUsingBlockCount(); };
		// �� ������� Chunk ����
		static long			GetUsingChunkCount(void) { return m_pMemoryPool->GetUsingChunkCount(); }

	public:
		//-----------------------------------------------------------------------------------------
		// �ֱ�.	�� ���� Ÿ�Ը��� ��� ����.
		//-----------------------------------------------------------------------------------------
		CPacket	&operator << (const BYTE &byValue);
		CPacket	&operator << (const char &chValue);

		CPacket	&operator << (const short &shValue);
		CPacket	&operator << (const WORD &wValue);

		CPacket	&operator << (const int &iValue);
		CPacket	&operator << (const DWORD &dwValue);
		CPacket	&operator << (const float &fValue);

		CPacket	&operator << (const __int64 &iValue);
		CPacket	&operator << (const double &dValue);

		CPacket	&operator << (const UINT64 &iValue);

		//-----------------------------------------------------------------------------------------
		// ����.	�� ���� Ÿ�Ը��� ��� ����.
		//-----------------------------------------------------------------------------------------
		CPacket	&operator >> (BYTE *byValue);
		CPacket	&operator >> (char *chValue);

		CPacket	&operator >> (short *shValue);
		CPacket	&operator >> (WORD *wValue);

		CPacket	&operator >> (int *iValue);
		CPacket	&operator >> (DWORD *dwValue);
		CPacket	&operator >> (float *fValue);

		CPacket	&operator >> (__int64 *iValue);
		CPacket	&operator >> (double *dValue);

		CPacket	&operator >> (UINT64 *ui64Value);

	private:

		//-----------------------------------------------------------------------------
		// ��Ŷ���� / ���� ������.
		//-----------------------------------------------------------------------------
		char	m_cBufferDefault[static_cast<int>( PACKET::BUFF_PACKET_1024 )];
		int		m_iBufferSize;
		int		m_iHeaderSize;

		//-----------------------------------------------------------------------------
		// ������ ���� ��ġ, ���� ��ġ, ���� ��ġ.
		//-----------------------------------------------------------------------------
		char	*m_cpBuffer;
		char	*m_cpRear;
		char	*m_cpFront;

		//-----------------------------------------------------------------------------
		// ���� ���ۿ� ������� ������.
		//-----------------------------------------------------------------------------
		int		m_iUsingSize;

		public:
		//-----------------------------------------------------------------------------
		// ��� �޸�Ǯ
		//-----------------------------------------------------------------------------
		// tls
		static CMemoryPool_TLS<CPacket>	*m_pMemoryPool;

		public:
		//-----------------------------------------------------------------------------
		// ���� ī����
		// 0 �̸� ��ȯ
		//-----------------------------------------------------------------------------
		long		m_lRefCnt;

		//-----------------------------------------------------------------------------
		// ��ȣȭ ����
		//-----------------------------------------------------------------------------
		bool		m_bEncodeComeplete;

		//-----------------------------------------------------------------------------
		// ��ȣȭ & ��ȣȭ ���� �ڵ�
		//-----------------------------------------------------------------------------
		static BYTE m_cCode;
		static BYTE m_cXORCode1;
		static BYTE m_cXORCode2;
	};
}
