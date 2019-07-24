#include "FileStream.h"

NOH::CFileStream::CFileStream()
    : m_hFile(INVALID_HANDLE_VALUE)
{
}

NOH::CFileStream::~CFileStream()
{
}

bool NOH::CFileStream::Open(LPCWSTR FileName, DWORD dwAccessMode, DWORD dwCreateOption, DWORD dwSharedMode)
{
    if (INVALID_HANDLE_VALUE != m_hFile)
        CloseHandle(m_hFile);

    m_hFile = CreateFile(FileName, dwAccessMode, dwSharedMode, NULL, dwCreateOption, FILE_ATTRIBUTE_NORMAL, NULL);

    if (INVALID_HANDLE_VALUE == m_hFile)
        return false;

    return true;
}

void NOH::CFileStream::Close(void)
{
    if (INVALID_HANDLE_VALUE != m_hFile)
    {
        CloseHandle(m_hFile);
        m_hFile = INVALID_HANDLE_VALUE;
    }
}

bool NOH::CFileStream::Write(LPVOID wBuffer, DWORD & dwBytesToWrite, DWORD & dwBytesWritten)
{
    if (WriteFile(m_hFile, wBuffer, dwBytesToWrite, &dwBytesWritten, NULL))
        return true;
    else
        return false;
}

bool NOH::CFileStream::Read(LPVOID wBuffer, DWORD & dwBytesToRead, DWORD & dwReadBytes)
{
    if (ReadFile(m_hFile, wBuffer, dwBytesToRead, &dwReadBytes, NULL))
        return true;
    else
        return false;
}

bool NOH::CFileStream::Seek(DWORD dwMoveBytes, DWORD dwMoveMethod)
{
    if (INVALID_SET_FILE_POINTER == SetFilePointer(m_hFile, dwMoveBytes, NULL, dwMoveMethod))
        return false;

    return true;
}

DWORD NOH::CFileStream::GetSize(void)
{
    if (INVALID_HANDLE_VALUE != m_hFile)
        return GetFileSize(m_hFile, NULL);
    else
        return 0;
}


