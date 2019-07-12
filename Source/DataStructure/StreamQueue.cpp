#include "StreamQueue.h"

#include <tchar.h>

//------------------------------------------------------
// Func	  : CStreamQueue(int iBufferSize)
// return : none
//
// 버퍼 사이즈를 인자로 전달시 호출되는 생성자
// 쓰기 위치가 읽기 위치와 만나지 않도록 하기 위해서 원하는 버퍼 사이즈에서 블랭크 버퍼 사이즈만큼 뺌
//------------------------------------------------------
NOH::CStreamQueue::CStreamQueue(const int iBufferSize) :
    m_spBuffer(std::make_unique<char[]>(iBufferSize)), m_iBufferSize(iBufferSize - static_cast<int>(STREAMQUEUE::BUFF_STREAMQUEUE_BLANK_8)), m_iFront(0), m_iRear(0)
{
    Initial(m_iBufferSize);
}

//------------------------------------------------------
// Func	  : ~CStreamQueue()
// return : none
//
// 소멸자
//------------------------------------------------------
NOH::CStreamQueue::~CStreamQueue()
{
    m_spBuffer.release();
}

//------------------------------------------------------
// Func	  : Initial(int iBufferSize)
// return : void
//
// 버퍼 및 버퍼 사이즈 초기화
//------------------------------------------------------
void NOH::CStreamQueue::Initial(const int iBufferSize)
{
    memset(m_spBuffer.get(), 0, iBufferSize);
}

//------------------------------------------------------
// Func	  : GetBufferSize(void)
// return : int
//
// 버퍼의 최대 사이즈
//------------------------------------------------------
int NOH::CStreamQueue::GetBufferSize(void)
{
    if (m_spBuffer == nullptr)
        return 0;
    else
        return m_iBufferSize;
}

//------------------------------------------------------
// Func	  : GetFreeSize(void)
// return : int
//
// 버퍼에 데이터를 쓸 수 있는 사이즈
//
// buffersize에는 이미 blank에 대한 처리가 된 사이즈.
// rear가 front를 따라 잡을 때는 blank에 대한 처리가 필요.
//------------------------------------------------------
int NOH::CStreamQueue::GetFreeSize(void)
{
    if (m_spBuffer == nullptr)
        return 0;

    // 쓰기 위치 >= 읽기 위치
    if (m_iRear >= m_iFront)
        return (m_iBufferSize - m_iRear) + m_iFront;
    else
        return m_iFront - m_iRear - static_cast<int>(STREAMQUEUE::BUFF_STREAMQUEUE_BLANK_8);
}

//------------------------------------------------------
// Func	  : GetUsingSize(void)
// return : int
//
// 버퍼에 데이터가 차지하고 있는 사이즈
//
// front가 rear를 따라 잡을 때는 blank에 대한 처리가 불필요.
// 해당 구조에서는 rear와 front가 만날 일 자체가 없음.
// rear 위치에는 앞으로 데이터를 넣을 위치이므로, 
// rear 위치에는 유효한 데이터가 없음.
//------------------------------------------------------
int NOH::CStreamQueue::GetUsingSize(void)
{
    if (m_spBuffer == nullptr)
        return 0;

    // 쓰기 위치 >= 읽기 위치
    if (m_iRear >= m_iFront)
        return m_iRear - m_iFront;
    else
        return (m_iBufferSize + static_cast<int>(STREAMQUEUE::BUFF_STREAMQUEUE_BLANK_8) - m_iFront) + m_iRear;
}

//------------------------------------------------------
// Func	  :GetNotCirculatedFreeSize(void)
// return : int
//
// 끊기지 않고 한 번에 쓸 수 있는 사이즈
//
// blank 위치에 따른 상황.
// 1. blank 사이즈 전체가 버퍼 끝 부분에 있는 경우
// 2. blank 사이즈가 버퍼 끝과 시작 부분에 나눠져 있는 경우
// 3. blank 사이즈 전체가 버퍼 시작 부분에 있는 경우
//------------------------------------------------------
int NOH::CStreamQueue::GetNotCirculatedFreeSize(void)
{
    if (m_spBuffer == nullptr)
        return 0;

    int _iblanksize = 0;

    // blank 위치에 따른 로직 처리
    if (m_iFront < static_cast<int>(STREAMQUEUE::BUFF_STREAMQUEUE_BLANK_8))
        _iblanksize = static_cast<int>(STREAMQUEUE::BUFF_STREAMQUEUE_BLANK_8) - m_iFront;

    // 쓰기 위치 >= 읽기 위치
    if (m_iRear >= m_iFront)
        return m_iBufferSize + static_cast<int>(STREAMQUEUE::BUFF_STREAMQUEUE_BLANK_8) - m_iRear - _iblanksize;
    else
        return m_iFront - m_iRear - static_cast<int>(STREAMQUEUE::BUFF_STREAMQUEUE_BLANK_8);
}

//------------------------------------------------------
// Func	  : GetNotCirculatedUsingSize(void)
// return : int
//
// 끊기지 않고 한 번에 읽을 수 있는 사이즈
//------------------------------------------------------
int NOH::CStreamQueue::GetNotCirculatedUsingSize(void)
{
    if (m_spBuffer == nullptr)
        return 0;

    // 쓰기 위치 >= 읽기 위치
    if (m_iRear >= m_iFront)
        return m_iRear - m_iFront;
    else
        return (m_iBufferSize + static_cast<int>(STREAMQUEUE::BUFF_STREAMQUEUE_BLANK_8)) - m_iFront;
}

//------------------------------------------------------
// Func	  : Enqueue(char *chpData, int iSize)
// return : int
//
// 데이터 넣기
//------------------------------------------------------
int NOH::CStreamQueue::Enqueue(const char & cData, const int iSize)
{
    if (m_spBuffer == nullptr)
        return 0;

    int _ifreesize = GetFreeSize();
    int _inotcirculatedfreesize = GetNotCirculatedFreeSize();
    char *_cpstoredbuffer = m_spBuffer.get();

    if (iSize > _ifreesize)
        return -1;

    // 쓰기 위치 >= 읽기 위치.
    if (m_iRear >= m_iFront)
    {
        if (iSize > _inotcirculatedfreesize)
        {
            memcpy(_cpstoredbuffer + m_iRear, &cData, _inotcirculatedfreesize);
            memcpy(_cpstoredbuffer, &cData + _inotcirculatedfreesize, iSize - _inotcirculatedfreesize);

            m_iRear = iSize - _inotcirculatedfreesize;

            return iSize;
        }
        else
        {
            memcpy(_cpstoredbuffer + m_iRear, &cData, iSize);

            m_iRear += iSize;

            if (m_iRear == m_iBufferSize + static_cast<int>(STREAMQUEUE::BUFF_STREAMQUEUE_BLANK_8))
                m_iRear = 0;

            return iSize;
        }
    }
    else
    {
        memcpy(_cpstoredbuffer + m_iRear, &cData, iSize);
        m_iRear += iSize;

        return iSize;
    }
}

//------------------------------------------------------
// Func	  : Dequeue(char *chpData, int iSize)
// return : int
//
// 데이터 읽기
//------------------------------------------------------
int NOH::CStreamQueue::Dequeue(char *cpData, const int iSize)
{
    if (m_spBuffer == nullptr)
        return 0;

    int _iusingsize = GetUsingSize();
    int _inotcirculatedusingsize = GetNotCirculatedUsingSize();
    char *_cpstoredbuffer = m_spBuffer.get();

    if (iSize > _iusingsize)
        return -1;

    // 쓰기 위치 >= 읽기 위치
    if (m_iRear >= m_iFront)
    {
        memcpy(cpData, _cpstoredbuffer + m_iFront, iSize);
        m_iFront += iSize;

        return iSize;
    }
    else
    {
        if (iSize > _inotcirculatedusingsize)
        {
            memcpy(cpData, _cpstoredbuffer + m_iFront, _inotcirculatedusingsize);
            memcpy(cpData + _inotcirculatedusingsize, _cpstoredbuffer, iSize - _inotcirculatedusingsize);

            m_iFront = iSize - _inotcirculatedusingsize;

            return iSize;
        }
        else
        {
            memcpy(cpData, _cpstoredbuffer + m_iFront, iSize);

            m_iFront += iSize;

            if (m_iFront == m_iBufferSize + static_cast<int>(STREAMQUEUE::BUFF_STREAMQUEUE_BLANK_8))
                m_iFront = 0;

            return iSize;
        }
    }
}

//------------------------------------------------------
// Func	  : Peek(char *chpDest, int iSize)
// return : int
//
// 데이터 읽기 (읽기 위치 변경 X)
//------------------------------------------------------
int NOH::CStreamQueue::Peek(char *chpDest, int iSize, const int iFrontSize)
{
    if (m_spBuffer == nullptr)
        return 0;

    int _itempfront = m_iFront;
    int _itemprear = m_iRear;
    int iUsingSize = 0;
    int iNotCirculatedUsingSize = 0;
    char *_cpstoredbuffer = m_spBuffer.get();

    if (0 != iFrontSize)
    {
        _itempfront += iFrontSize;

        if (_itempfront >= m_iBufferSize + static_cast<int>(STREAMQUEUE::BUFF_STREAMQUEUE_BLANK_8))
            _itempfront = _itempfront - (m_iBufferSize + static_cast<int>(STREAMQUEUE::BUFF_STREAMQUEUE_BLANK_8));
    }

    // 쓰기 위치 >= 읽기 위치
    if (_itemprear >= _itempfront)
    {
        iUsingSize = iNotCirculatedUsingSize = _itemprear - _itempfront;
    }
    else
    {
        iUsingSize = (m_iBufferSize + static_cast<int>(STREAMQUEUE::BUFF_STREAMQUEUE_BLANK_8) - _itempfront) + _itemprear;
        iNotCirculatedUsingSize = (m_iBufferSize + static_cast<int>(STREAMQUEUE::BUFF_STREAMQUEUE_BLANK_8)) - _itempfront;
    }

    if (iSize > iUsingSize)
        iSize = iUsingSize;

    // 쓰기 위치 >= 읽기 위치
    if (_itemprear >= _itempfront)
    {
        memcpy(chpDest, _cpstoredbuffer + _itempfront, iSize);

        return iSize;
    }
    else
    {
        if (iSize > iNotCirculatedUsingSize)
        {
            memcpy(chpDest, _cpstoredbuffer + _itempfront, iNotCirculatedUsingSize);
            memcpy(chpDest + iNotCirculatedUsingSize, _cpstoredbuffer, iSize - iNotCirculatedUsingSize);

            return iSize;
        }
        else
        {
            memcpy(chpDest, _cpstoredbuffer + _itempfront, iSize);

            return iSize;
        }
    }
}
//------------------------------------------------------
// Func	  : RemoveData(int iSize)
// return : int
//
// 읽기 위치 변경
//
// 해당 함수를 호출 하기 전에 이미 데이터는 들어간 상태임.
// 따라서, 인자로 주는 iSize만큼은 이동이 가능.
// m_iFornt가 m_iRear를 따라 잡는 경우는 큐에 데이터가 전부 비었다는 의미.
// 즉, 둘이 만나도 상관 없음.
//------------------------------------------------------
int NOH::CStreamQueue::RemoveData(const int iSize)
{
    if (m_spBuffer == nullptr)
        return 0;

    int _iusingsize = GetUsingSize();
    int _inotcircualtedusingsize = GetNotCirculatedUsingSize();

    if (iSize > _iusingsize)
        return -1;

    // 쓰기 위치 >= 읽기 위치
    if (m_iRear >= m_iFront)
    {
        m_iFront += iSize;
    }
    else
    {
        if (iSize > _inotcircualtedusingsize)
        {
            m_iFront = iSize - _inotcircualtedusingsize;
        }
        else
        {
            m_iFront += iSize;

            if (m_iFront == m_iBufferSize + static_cast<int>(STREAMQUEUE::BUFF_STREAMQUEUE_BLANK_8))
                m_iFront = 0;
        }
    }

    return iSize;
}

//------------------------------------------------------
// Func	  : MoveRear(int iSize)
// return : int
//
// 쓰기 위치 변경
//
// 해당 함수를 호출 하기 전에 이미 데이터는 들어간 상태임.
// 따라서, 인자로 주는 iSize만큼은 이동이 가능.
//------------------------------------------------------
int NOH::CStreamQueue::MoveRear(const int & iSize)
{
    if (m_spBuffer == nullptr)
        return 0;

    int _ifreesize = GetFreeSize();
    int _inotcirculatedfreesize = GetNotCirculatedFreeSize();

    if (iSize > _ifreesize)
        return -1;

    // 쓰기 위치 >= 읽기 위치.
    if (m_iRear >= m_iFront)
    {
        if (iSize > _inotcirculatedfreesize)
        {
            m_iRear = iSize - _inotcirculatedfreesize;
        }
        else
        {
            m_iRear += iSize;

            if (m_iRear == m_iBufferSize + static_cast<int>(STREAMQUEUE::BUFF_STREAMQUEUE_BLANK_8))
                m_iRear = 0;
        }
    }
    else
    {
        m_iRear += iSize;
    }

    return iSize;
}

//------------------------------------------------------
// Func	  : ClearBuffer(void)
// return : void
//
// 버퍼 데이터 초기화(읽기, 쓰기 위치 0으로 초기화)
//------------------------------------------------------
void NOH::CStreamQueue::ClearBuffer(void)
{
    if (m_spBuffer == nullptr)
        return;

    m_iFront = 0;
    m_iRear = 0;
}

//------------------------------------------------------
// Func	  : GetBufferPtr(void)
// return : char *
//
// 버퍼 시작 위치 포인터 얻기
//------------------------------------------------------
char* NOH::CStreamQueue::GetBufferPtr(void)
{
    if (m_spBuffer == nullptr)
        return nullptr;

    return m_spBuffer.get();
}

//------------------------------------------------------
// Func	  : GetFrontBufferPtr(void)
// return : char *
//
// 읽기 위치 포인터 얻기
//------------------------------------------------------
char* NOH::CStreamQueue::GetFrontBufferPtr(void)
{
    if (m_spBuffer == nullptr)
        return nullptr;

    return m_spBuffer.get() + m_iFront;
}

//------------------------------------------------------
// Func	  : GetRearBufferPtr(void)
// return : char *
//
// 쓰기 위치 포인터 얻기
//------------------------------------------------------
char* NOH::CStreamQueue::GetRearBufferPtr(void)
{
    if (m_spBuffer == nullptr)
        return nullptr;

    return m_spBuffer.get() + m_iRear;
}