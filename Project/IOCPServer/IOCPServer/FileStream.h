#pragma once

#include <WinSock2.h>

namespace NOH
{
    class CFileStream
    {
    public:
        CFileStream();
        ~CFileStream();

        bool Open(LPCWSTR FileName, DWORD dwAccessMode = GENERIC_READ, DWORD dwCreateOption = OPEN_EXISTING, DWORD dwSharedMode = FILE_SHARE_READ);
        void Close(void);
        bool Write(LPVOID wBuffer, DWORD &dwBytesToWrite, DWORD &dwBytesWritten);
        bool Read(LPVOID wBuffer, DWORD &dwBytesToRead, DWORD &dwReadBytes);
        bool Seek(DWORD dwMoveBytes, DWORD dwMoveMethod);
        DWORD GetSize(void);

    private:
        HANDLE m_hFile;
    };
}