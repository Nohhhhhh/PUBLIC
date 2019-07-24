#include "Packet.h"
#include "MemoryPool_TLS.h"

//tls
NOH::CMemoryPool_TLS<NOH::CPacket>	*NOH::CPacket::m_pMemoryPool;
// lf
//NOH::CMemoryPool_LF<NOH::CPacket>	*NOH::CPacket::m_pMemoryPool;
// no_lf
//NOH::CMemoryPool<NOH::CPacket>	*NOH::CPacket::m_pMemoryPool;

BYTE NOH::CPacket::m_cCode;
BYTE NOH::CPacket::m_cXORCode1;
BYTE NOH::CPacket::m_cXORCode2;

NOH::CPacket::CPacket(const int iHeaderSize) : m_iBufferSize(static_cast<int>( PACKET::BUFF_PACKET_1024 )), m_iHeaderSize(iHeaderSize), m_iUsingSize(0), m_lRefCnt(0), m_bEncodeComeplete(false)
{
	Initial();
}

NOH::CPacket::CPacket(const int iBufferSize, const int iHeaderSize) : m_iBufferSize(iBufferSize), m_iHeaderSize(static_cast<int>( PACKET::HEADER_MAX_SIZE )), m_iUsingSize(0), m_lRefCnt(0), m_bEncodeComeplete(false)
{
	Initial();
}

NOH::CPacket::~CPacket()
{
	Release();
}

void NOH::CPacket::Initial(void)
{
	if ( m_iBufferSize != static_cast<int>( PACKET::BUFF_PACKET_1024 ) )
	{
		m_cpBuffer = new char[m_iBufferSize];
		memset(m_cpBuffer, 0, m_iBufferSize);
	}
	else
	{
		m_cpBuffer = m_cBufferDefault;
		memset(m_cpBuffer, 0, m_iBufferSize);
	}

	// 초기 값은 처음 5byte는 헤더 정보가 들어갈 크기 이므로 5byte 더해줘야 함.
	//m_cpFront = m_cpBuffer + static_cast<int>( PACKET::HEADER_MAX_SIZE );
	//m_cpRear = m_cpBuffer + static_cast<int>( PACKET::HEADER_MAX_SIZE );

 //   m_iHeaderSize = static_cast<int>( PACKET::HEADER_MAX_SIZE );

    m_cpFront = m_cpBuffer + m_iHeaderSize;
	m_cpRear = m_cpBuffer + m_iHeaderSize;
}

void NOH::CPacket::Release(void)
{
	if (m_cpBuffer == nullptr)
		return;

	if ( m_iBufferSize != static_cast<int>( PACKET::BUFF_PACKET_1024 ) )
	{
		// 동적 할당 한 경우
		delete[] m_cpBuffer;
		m_cpBuffer = nullptr;
	}
	else
	{
		// 동적 할당 하지 않은 경우
		m_cpBuffer = nullptr;
	}
}

NOH::CPacket * NOH::CPacket::Clear(void)
{
	m_cpFront = m_cpBuffer + static_cast<int>( PACKET::HEADER_MAX_SIZE );
	m_cpRear = m_cpBuffer + static_cast<int>( PACKET::HEADER_MAX_SIZE );

	m_iUsingSize = 0;
	m_lRefCnt = 0;
	m_bEncodeComeplete = false;
    m_iHeaderSize = static_cast<int>( PACKET::HEADER_MAX_SIZE );

    InterlockedIncrement(&m_lRefCnt);

    return this;
}

//-----------------------------------------------------------------------------------------
// Param: char * (Header 포인터)
// Return: int (Header 사이즈)
//
// Header 데이터 넣기
//-----------------------------------------------------------------------------------------
void NOH::CPacket::SetHeader(const char & cHeaderPtr)
{
	m_iHeaderSize = static_cast<int>( PACKET::HEADER_DEFAULT_SIZE );
    
	memcpy(m_cpBuffer + ( static_cast<int>( PACKET::HEADER_MAX_SIZE ) - m_iHeaderSize ), &cHeaderPtr, m_iHeaderSize);
}

void NOH::CPacket::SetHeader_SHORT(const unsigned short shHeader)
{
	m_iHeaderSize = static_cast<int>( PACKET::HEADER_DEFAULT_SIZE );

	*reinterpret_cast<unsigned short *>( m_cpBuffer + ( static_cast<int>( PACKET::HEADER_MAX_SIZE ) - 2 ) ) = shHeader;
}

//-----------------------------------------------------------------------------------------
// Param: char * (Header 포인터), int (Header 사이즈)
// Return: int (Header 사이즈)
//
// (커스텀) Header 데이터 넣기
//-----------------------------------------------------------------------------------------
void NOH::CPacket::SetHeader_CustomHeader(const char & cHeaderPtr, const int iCustomHeaderSize)
{
	m_iHeaderSize = iCustomHeaderSize;

	memcpy(m_cpBuffer + ( static_cast<int>( PACKET::HEADER_MAX_SIZE ) - iCustomHeaderSize ), &cHeaderPtr, iCustomHeaderSize);
}


//-----------------------------------------------------------------------------------------
// Param :  int (이동할 사이즈)
// Return: int (이동한 사이즈)
//
// Rear / Front 포지션 이동
//-----------------------------------------------------------------------------------------
int NOH::CPacket::MoveRear(const int iSize)
{
	if (m_cpBuffer == nullptr)
		return 0;

	m_cpRear += iSize;

	m_iUsingSize = (int)(m_cpRear - m_cpFront);

	return iSize;
}


int NOH::CPacket::MoveFront(const int iSize)
{
	if (m_cpBuffer == nullptr)
		return 0;

	m_cpFront += iSize;

	m_iUsingSize = static_cast<int>(m_cpRear - m_cpFront);

	return iSize;
}


//-----------------------------------------------------------------------------------------
// Param:  char * (목적지 버퍼), int (쓸 사이즈)
// Return: int (쓴 사이즈)
//
// 데이터 쓰기
//-----------------------------------------------------------------------------------------
int	NOH::CPacket::Enqueue(const char &chpSrc, const int iSize)
{
	if (iSize > m_iBufferSize)
	{
		throw exception_PacketIn(iSize);
		return 0;
	}

	memcpy(m_cpRear, &chpSrc, iSize);

	MoveRear(iSize);

	return iSize;
}



//-----------------------------------------------------------------------------------------
// Param :  void
// Return: void
//
// 패킷 암호화
//-----------------------------------------------------------------------------------------
void NOH::CPacket::Encode(void)
{
	if (m_bEncodeComeplete)
		return;
	
	unsigned int _uichecksum = 0;
	int _ipayloadsize = m_iUsingSize;
	NET_HEADER *_pheader = reinterpret_cast<NET_HEADER *>(m_cpBuffer);
	char *_cppayloadbuffer = m_cpBuffer + static_cast<int>( PACKET::PACKET_HEADER_SIZE );
	
	// 0. 고정 Code 초기화
	_pheader->cCode = m_cCode;

	// 1. Rand XOR Code 생성
	char _crandcode = rand() % 256;
	_pheader->cRandCode = _crandcode;

	// 2. Payload의 CheckSum 계산
	for (int iCnt = 0; iCnt < _ipayloadsize; ++iCnt)
		_uichecksum += _cppayloadbuffer[iCnt];

	unsigned char _uccehcksum = (unsigned char)(_uichecksum % 256);

	// 3-1. Rand XOR Code로 CheckSum XOR
	_pheader->ucCheckSum = _uccehcksum ^ _crandcode;
	// 3-2. Rand XOR Code로 Payload XOR
	for (int iCnt = 0; iCnt < _ipayloadsize; ++iCnt)
		_cppayloadbuffer[iCnt] ^= _crandcode;

	// 4-1. 고정 XOR Code1 로 Rand XOR Code를 XOR
	_pheader->cRandCode ^= m_cXORCode1;
	// 4-2. 고정 XOR Code1 로 CheckSum을 XOR
	_pheader->ucCheckSum ^= m_cXORCode1;
	// 4-3. 고정 XOR Code1 로 Payload를 XOR
	for (int iCnt = 0; iCnt < _ipayloadsize; ++iCnt)
		_cppayloadbuffer[iCnt] ^= m_cXORCode1;

	// 4-1. 고정 XOR Code2 로 Rand XOR Code를 XOR
	_pheader->cRandCode ^=m_cXORCode2;
	// 4-2. 고정 XOR Code2 로 CheckSum을 XOR
	_pheader->ucCheckSum ^= m_cXORCode2;
	// 4-3. 고정 XOR Code2 로 Payload를 XOR
	for (int iCnt = 0; iCnt < _ipayloadsize; ++iCnt)
		_cppayloadbuffer[iCnt] ^= m_cXORCode2;

	m_bEncodeComeplete = true;
}

//-----------------------------------------------------------------------------------------
// Param : void
// Return: true (성공), flase (실패)
//
// 패킷 복호화
//-----------------------------------------------------------------------------------------
bool NOH::CPacket::Decode(NET_HEADER *pHeader)
{
	int _ipayloadsize = m_iUsingSize;
	m_iHeaderSize = static_cast<int>( PACKET::PACKET_HEADER_SIZE );
	char *_cppayloadbuffer = m_cpBuffer + m_iHeaderSize;

	if(nullptr == pHeader)
		pHeader = reinterpret_cast<NET_HEADER *>( m_cpBuffer - m_iHeaderSize );
	
	// 1-1. 고정 XOR Code2 로 Rand XOR Code를 XOR
	pHeader->cRandCode ^= m_cXORCode2;
	// 1-2. 고정 XOR Code2 로 CheckSum을 XOR
	pHeader->ucCheckSum ^= m_cXORCode2;
	// 1-3. 고정 XOR Code2 로 Payload를 XOR
	for (int iCnt = 0; iCnt < _ipayloadsize; ++iCnt)
		_cppayloadbuffer[iCnt] ^= m_cXORCode2;

	// 2-1. 고정 XOR Code1 로 Rand XOR Code를 XOR
	pHeader->cRandCode ^= m_cXORCode1;
	// 2-2. 고정 XOR Code1 로 CheckSum을 XOR
	pHeader->ucCheckSum ^= m_cXORCode1;
	// 2-3. 고정 XOR Code1 로 Payload를 XOR
	for (int iCnt = 0; iCnt < _ipayloadsize; ++iCnt)
		_cppayloadbuffer[iCnt] ^= m_cXORCode1;

	// 3-1. Rand XOR Code로 CheckSum XOR
	pHeader->ucCheckSum ^= pHeader->cRandCode;
	// 3-2. Rand XOR Code로 Payload XOR
	for (int iCnt = 0; iCnt < _ipayloadsize; ++iCnt)
		_cppayloadbuffer[iCnt] ^= pHeader->cRandCode;

	// 4. 실제 Payload의 CheckSum과 헤더의 CheckSum이 동일한지 확인
	unsigned int uiCheckSum = 0;
	for (int iCnt = 0; iCnt < _ipayloadsize; ++iCnt)
		uiCheckSum += _cppayloadbuffer[iCnt];

	unsigned char _ucchecksum = (unsigned char)(uiCheckSum % 256);

	if (pHeader->ucCheckSum != _ucchecksum)
		return false;

	return true;
}

//-----------------------------------------------------------------------------------------
// Param : char (패킷 내부 고정 코드), char (XOR 고정 코드), char (XOR 고정 코드)
// Return: void
//
// 코드 초기화
//-----------------------------------------------------------------------------------------
void NOH::CPacket::SetCode(const char cCode, const char cXORCode1, const char cXORCode2)
{
	m_cCode = cCode;
	m_cXORCode1 = cXORCode1;
	m_cXORCode2 = cXORCode2;
}


//-----------------------------------------------------------------------------------------
// Param:  char * (목적지 버퍼), int (읽을 사이즈)
// Return: int (읽은 사이즈)
//
// 데이터 읽기
//
// header 포함
//-----------------------------------------------------------------------------------------
int NOH::CPacket::Dequeue(char *cpDest, int iSize)
{
	if (iSize > m_iBufferSize)
	{
		throw exception_PacketOut(iSize);
		return 0;
	}

	memcpy(cpDest, m_cpFront, iSize);

	MoveFront(iSize);

	return iSize;
}

//------------------------------------------------------------------------
// 넣기.	각 변수 타입마다 모두 만듬.
//------------------------------------------------------------------------
NOH::CPacket	&NOH::CPacket::operator << (const BYTE &byValue)
{
	Enqueue( reinterpret_cast<const char &>( byValue ), sizeof(BYTE) );

	return *this;
}

NOH::CPacket	&NOH::CPacket::operator << (const char &chValue)
{
	Enqueue(chValue, sizeof(char));

	return *this;
}

NOH::CPacket	&NOH::CPacket::operator << (const short &shValue)
{
	Enqueue( reinterpret_cast<const char &>( shValue ), sizeof(short) );

	return *this;
}

NOH::CPacket	&NOH::CPacket::operator << (const WORD &wValue)
{
	Enqueue( reinterpret_cast<const char &>( wValue), sizeof(WORD) );

	return *this;
}

NOH::CPacket	&NOH::CPacket::operator << (const int &iValue)
{
	Enqueue( reinterpret_cast<const char &>( iValue ), sizeof(int) );

	return *this;
}

NOH::CPacket	&NOH::CPacket::operator << (const DWORD &dwValue)
{
	Enqueue( reinterpret_cast<const char &>( dwValue ), sizeof(DWORD) );

	return *this;
}

NOH::CPacket	&NOH::CPacket::operator << (const float &fValue)
{
	Enqueue( reinterpret_cast<const char &>( fValue ), sizeof(float) );

	return *this;
}

NOH::CPacket	&NOH::CPacket::operator << (const __int64 &iValue)
{
	Enqueue( reinterpret_cast<const char &>( iValue ), sizeof(__int64) );

	return *this;
}

NOH::CPacket	&NOH::CPacket::operator << (const double &dValue)
{
	Enqueue( reinterpret_cast<const char &>( dValue ), sizeof(double) );

	return *this;
}

NOH::CPacket	&NOH::CPacket::operator << (const UINT64 &ui64Value)
{
	Enqueue( reinterpret_cast<const char &>( ui64Value ), sizeof(UINT64) );

	return *this;
}

//------------------------------------------------------------------------
//// 빼기.	각 변수 타입마다 모두 만듬.
//------------------------------------------------------------------------
NOH::CPacket	&NOH::CPacket::operator >> (BYTE *byValue)
{
	Dequeue( reinterpret_cast<char *>( byValue ), sizeof(BYTE) );

	return *this;
}

NOH::CPacket	&NOH::CPacket::operator >> (char *chValue)
{
	Dequeue( reinterpret_cast<char *>( chValue ), sizeof(char) );

	return *this;
}

NOH::CPacket	&NOH::CPacket::operator >> (short *shValue)
{
	Dequeue( reinterpret_cast<char *>( shValue ), sizeof(short) );

	return *this;
}

NOH::CPacket	&NOH::CPacket::operator >> (WORD *wValue)
{
	Dequeue( reinterpret_cast<char *>( wValue ), sizeof(WORD) );

	return *this;
}

NOH::CPacket	&NOH::CPacket::operator >> (int *iValue)
{
	Dequeue( reinterpret_cast<char *>( iValue ), sizeof(int) );

	return *this;
}

NOH::CPacket	&NOH::CPacket::operator >> (DWORD *dwValue)
{
	Dequeue( reinterpret_cast<char *>( dwValue ), sizeof(DWORD) );

	return *this;
}

NOH::CPacket	&NOH::CPacket::operator >> (float *fValue)
{
	Dequeue( reinterpret_cast<char *>( fValue ), sizeof(float) );

	return *this;
}

NOH::CPacket	&NOH::CPacket::operator >> (__int64 *iValue)
{
	Dequeue( reinterpret_cast<char *>( iValue ), sizeof(__int64) );

	return *this;
}

NOH::CPacket	&NOH::CPacket::operator >> (double *dValue)
{
	Dequeue( reinterpret_cast<char *>( dValue ), sizeof(double) );

	return *this;
}

NOH::CPacket	&NOH::CPacket::operator >> (UINT64 *ui64Value)
{
	Dequeue( reinterpret_cast<char *>( ui64Value ), sizeof(UINT64) );

	return *this;
}

//-----------------------------------------------------------------------------------------
// param : int (청크 사이즈)
// return: void
//
// 메모리풀 생성
//
// 블록 개수 = 0, Alloc()으로 동적할당
// 블록 개수 != 0, 미리 동적할당
//-----------------------------------------------------------------------------------------
void NOH::CPacket::AllocMemoryPool(const long lChunkSize)
{
	// tls
	// TLS의 경우 생성자에서 한 번만 repalcement new 여부를 정함.
	m_pMemoryPool = new CMemoryPool_TLS<CPacket>(lChunkSize, false);
	// lf, 
	//m_pMemoryPool = new CMemoryPool_LF<CPacket>(iBlockNum);
}

//-----------------------------------------------------------------------------------------
// param : void
// return: void
//
// 메모리풀 삭제
//-----------------------------------------------------------------------------------------
void NOH::CPacket::DeleteMemoryPool(void)
{
	delete[] m_pMemoryPool;
}

//-----------------------------------------------------------------------------------------
// param : void
// return: CPacket *(패킷 구조체 포인터)
//
// 메모리 풀 할당
//-----------------------------------------------------------------------------------------
NOH::CPacket * NOH::CPacket::Alloc(void)
{
	////PRO_BEGIN(L"PacketAlloc");
	//// tls, lf
	//CPacket *cPacket = m_pMemoryPool->Alloc();
	//// new delete 
	////CPacket *cPacket = new CPacket;
	////PRO_END(L"PacketAlloc");

	//cPacket->Clear();

	//InterlockedIncrement(&cPacket->m_lRefCnt);

	//return cPacket;

    return m_pMemoryPool->Alloc()->Clear();
}

//-----------------------------------------------------------------------------------------
// param : CPacket *(free할 포인터)
// return: void
//
// m_lRefCnt 감소 == 0 이면 메모리 풀 반환
//-----------------------------------------------------------------------------------------
bool NOH::CPacket::Free(CPacket & Data)
{
    
	if (0 == InterlockedDecrement( &Data.m_lRefCnt ) )
	{
        //SYSLOG(g_LogGameServer, NOH::LOG_LEVEL::LEVEL_SYSTEM, L"CPakcet - refcnt(Free1): %d", pData->m_lRefCnt);
		//PRO_BEGIN(L"PacketFree");
		// tls
		m_pMemoryPool->Free( &Data );
		// lf
		//m_pMemoryPool->Free(pData, false);
		// new delete 
		//delete pData;
		//PRO_END(L"PacketFree");
		//SYSLOG(L"Free", en_LOG_LEVEL::SYS_LOG_DEBUG, L"Delete");

		return true;
	}

    return false;
}

//-----------------------------------------------------------------------------------------
// param : void
// return: void
//
// m_lRefCnt 1 증가
//-----------------------------------------------------------------------------------------
void NOH::CPacket::AddRef(void)
{
	InterlockedIncrement(&m_lRefCnt);
}

bool NOH::CPacket::EncodeStatus(void)
{
	return m_bEncodeComeplete;
}
