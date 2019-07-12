#include "StreamQueue.h"

#include <tchar.h>

//------------------------------------------------------
// Func	  : CStreamQueue(int iBufferSize)
// return : none
//
// ���� ����� ���ڷ� ���޽� ȣ��Ǵ� ������
// ���� ��ġ�� �б� ��ġ�� ������ �ʵ��� �ϱ� ���ؼ� ���ϴ� ���� ������� ����ũ ���� �����ŭ ��
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
// �Ҹ���
//------------------------------------------------------
NOH::CStreamQueue::~CStreamQueue()
{
    m_spBuffer.release();
}

//------------------------------------------------------
// Func	  : Initial(int iBufferSize)
// return : void
//
// ���� �� ���� ������ �ʱ�ȭ
//------------------------------------------------------
void NOH::CStreamQueue::Initial(const int iBufferSize)
{
    memset(m_spBuffer.get(), 0, iBufferSize);
}

//------------------------------------------------------
// Func	  : GetBufferSize(void)
// return : int
//
// ������ �ִ� ������
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
// ���ۿ� �����͸� �� �� �ִ� ������
//
// buffersize���� �̹� blank�� ���� ó���� �� ������.
// rear�� front�� ���� ���� ���� blank�� ���� ó���� �ʿ�.
//------------------------------------------------------
int NOH::CStreamQueue::GetFreeSize(void)
{
    if (m_spBuffer == nullptr)
        return 0;

    // ���� ��ġ >= �б� ��ġ
    if (m_iRear >= m_iFront)
        return (m_iBufferSize - m_iRear) + m_iFront;
    else
        return m_iFront - m_iRear - static_cast<int>(STREAMQUEUE::BUFF_STREAMQUEUE_BLANK_8);
}

//------------------------------------------------------
// Func	  : GetUsingSize(void)
// return : int
//
// ���ۿ� �����Ͱ� �����ϰ� �ִ� ������
//
// front�� rear�� ���� ���� ���� blank�� ���� ó���� ���ʿ�.
// �ش� ���������� rear�� front�� ���� �� ��ü�� ����.
// rear ��ġ���� ������ �����͸� ���� ��ġ�̹Ƿ�, 
// rear ��ġ���� ��ȿ�� �����Ͱ� ����.
//------------------------------------------------------
int NOH::CStreamQueue::GetUsingSize(void)
{
    if (m_spBuffer == nullptr)
        return 0;

    // ���� ��ġ >= �б� ��ġ
    if (m_iRear >= m_iFront)
        return m_iRear - m_iFront;
    else
        return (m_iBufferSize + static_cast<int>(STREAMQUEUE::BUFF_STREAMQUEUE_BLANK_8) - m_iFront) + m_iRear;
}

//------------------------------------------------------
// Func	  :GetNotCirculatedFreeSize(void)
// return : int
//
// ������ �ʰ� �� ���� �� �� �ִ� ������
//
// blank ��ġ�� ���� ��Ȳ.
// 1. blank ������ ��ü�� ���� �� �κп� �ִ� ���
// 2. blank ����� ���� ���� ���� �κп� ������ �ִ� ���
// 3. blank ������ ��ü�� ���� ���� �κп� �ִ� ���
//------------------------------------------------------
int NOH::CStreamQueue::GetNotCirculatedFreeSize(void)
{
    if (m_spBuffer == nullptr)
        return 0;

    int _iblanksize = 0;

    // blank ��ġ�� ���� ���� ó��
    if (m_iFront < static_cast<int>(STREAMQUEUE::BUFF_STREAMQUEUE_BLANK_8))
        _iblanksize = static_cast<int>(STREAMQUEUE::BUFF_STREAMQUEUE_BLANK_8) - m_iFront;

    // ���� ��ġ >= �б� ��ġ
    if (m_iRear >= m_iFront)
        return m_iBufferSize + static_cast<int>(STREAMQUEUE::BUFF_STREAMQUEUE_BLANK_8) - m_iRear - _iblanksize;
    else
        return m_iFront - m_iRear - static_cast<int>(STREAMQUEUE::BUFF_STREAMQUEUE_BLANK_8);
}

//------------------------------------------------------
// Func	  : GetNotCirculatedUsingSize(void)
// return : int
//
// ������ �ʰ� �� ���� ���� �� �ִ� ������
//------------------------------------------------------
int NOH::CStreamQueue::GetNotCirculatedUsingSize(void)
{
    if (m_spBuffer == nullptr)
        return 0;

    // ���� ��ġ >= �б� ��ġ
    if (m_iRear >= m_iFront)
        return m_iRear - m_iFront;
    else
        return (m_iBufferSize + static_cast<int>(STREAMQUEUE::BUFF_STREAMQUEUE_BLANK_8)) - m_iFront;
}

//------------------------------------------------------
// Func	  : Enqueue(char *chpData, int iSize)
// return : int
//
// ������ �ֱ�
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

    // ���� ��ġ >= �б� ��ġ.
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
// ������ �б�
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

    // ���� ��ġ >= �б� ��ġ
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
// ������ �б� (�б� ��ġ ���� X)
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

    // ���� ��ġ >= �б� ��ġ
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

    // ���� ��ġ >= �б� ��ġ
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
// �б� ��ġ ����
//
// �ش� �Լ��� ȣ�� �ϱ� ���� �̹� �����ʹ� �� ������.
// ����, ���ڷ� �ִ� iSize��ŭ�� �̵��� ����.
// m_iFornt�� m_iRear�� ���� ��� ���� ť�� �����Ͱ� ���� ����ٴ� �ǹ�.
// ��, ���� ������ ��� ����.
//------------------------------------------------------
int NOH::CStreamQueue::RemoveData(const int iSize)
{
    if (m_spBuffer == nullptr)
        return 0;

    int _iusingsize = GetUsingSize();
    int _inotcircualtedusingsize = GetNotCirculatedUsingSize();

    if (iSize > _iusingsize)
        return -1;

    // ���� ��ġ >= �б� ��ġ
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
// ���� ��ġ ����
//
// �ش� �Լ��� ȣ�� �ϱ� ���� �̹� �����ʹ� �� ������.
// ����, ���ڷ� �ִ� iSize��ŭ�� �̵��� ����.
//------------------------------------------------------
int NOH::CStreamQueue::MoveRear(const int & iSize)
{
    if (m_spBuffer == nullptr)
        return 0;

    int _ifreesize = GetFreeSize();
    int _inotcirculatedfreesize = GetNotCirculatedFreeSize();

    if (iSize > _ifreesize)
        return -1;

    // ���� ��ġ >= �б� ��ġ.
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
// ���� ������ �ʱ�ȭ(�б�, ���� ��ġ 0���� �ʱ�ȭ)
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
// ���� ���� ��ġ ������ ���
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
// �б� ��ġ ������ ���
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
// ���� ��ġ ������ ���
//------------------------------------------------------
char* NOH::CStreamQueue::GetRearBufferPtr(void)
{
    if (m_spBuffer == nullptr)
        return nullptr;

    return m_spBuffer.get() + m_iRear;
}