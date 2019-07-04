/*
- 사용 방법

namespace DB_INFO
{
	char	g_szIP[64] = { NULL };
	char	g_szUser[128] = { NULL };
	char	g_szPassword[256] = { NULL };
	char	g_szDBName[256] = { NULL };
	int		g_iPort = 0;
};

bool GetDBInfo(void)
{
	WCHAR szIP[32] = { NULL };
	WCHAR szUser[64] = { NULL };
	WCHAR szPassword[128] = { NULL };
	WCHAR szDBName[128] = { NULL };
	int iPort;

	int iIPSize = sizeof(szIP);
	int iUserSize = sizeof(szUser);
	int iPassSize = sizeof(szPassword);
	int iDBNameSize = sizeof(szDBName);

	NOH::CTextParser TextParser;

	if (!TextParser.LoadFile(L"DB_INFO.ini"))
	{
		// 실패 로그 & 콘솔 출력
		//wprintf(L"Open File Fail: DB_INFO.ini\n");
			return FALSE;
	}

	// 실패 로그 & 콘솔 출력
	//wprintf(L"Open File Success: DB_INFO.ini\n\n");


	if (!TextParser.ProvideArea(L"DB_INFO"))
	{
		// 실패 로그 & 콘솔 출력
		//wprintf(L"DB_INFO AREA NOT FOUND!!!\n");
			return FALSE;
	}

	bool bFlag1 = TextParser.GetValue(L"IP", szIP, &iIPSize);
	bool bFlag2 = TextParser.GetValue(L"USER", szUser, &iUserSize);
	bool bFlag3 = TextParser.GetValue(L"PASSWORD", szPassword, &iPassSize);
	bool bFlag4 = TextParser.GetValue(L"DB_NAME", szDBName, &iDBNameSize);
	bool bFlag5 = TextParser.GetValue(L"PORT", &iPort);

	if (FALSE == bFlag1 || FALSE == bFlag2 || FALSE == bFlag3 || FALSE == bFlag4 || FALSE == bFlag5)
	{
		// 실패 로그 & 콘솔 출력
		return FALSE;
	}

	size_t i1, i2, i3, i4;

	wcstombs_s(&i1, DB_INFO::g_szIP, sizeof(DB_INFO::g_szIP), szIP, sizeof(szIP));
	wcstombs_s(&i2, DB_INFO::g_szUser, sizeof(DB_INFO::g_szUser), szUser, sizeof(szUser));
	wcstombs_s(&i3, DB_INFO::g_szPassword, sizeof(DB_INFO::g_szPassword), szPassword, sizeof(szPassword));
	wcstombs_s(&i4, DB_INFO::g_szDBName, sizeof(DB_INFO::g_szDBName), szDBName, sizeof(szDBName));
	DB_INFO::g_iPort = iPort;

	return TRUE;
}

*/
#pragma once

#include <WinSock2.h>
#include <memory>

#include "__NOH.h"

namespace NOH
{
	class CTextParser
	{
	public:
        // __NOH.h 로 옮김 
		//enum class TEXT_PARSER
		//{
		//	// 구역을 읽을 버퍼.
		//	BUFF_TEXTPARSER_1000000 = 1000000
		//};

		//-------------------------------------------------
		// 생성자, 파괴자.
		//-------------------------------------------------
        // array version
        /*CTextParser()
            : m_spBuffer(std::make_shared<WCHAR *>( new WCHAR[ static_cast<int>( TEXT_PARSER::BUFF_SIZE ) ] )), m_dwFileSize(0), m_dwBufferAreaStart(0), m_dwBufferAreaEnd(0), m_dwBufferFocusPos(0), m_bProvideAreaMode(false) {}*/
		// smart pointer version
        CTextParser()
            : m_spBuffer(std::make_unique<WCHAR []>( static_cast<int>( TEXT_PARSER::BUFF_TEXTPARSER_1000000 ) )), m_dwFileSize(0), m_dwBufferAreaStart(0), m_dwBufferAreaEnd(0), m_dwBufferFocusPos(0), m_bProvideAreaMode(false) {}
        ~CTextParser() {}

		//-------------------------------------------------
		// param: none
		// return: none
		//-------------------------------------------------
		void Initialize(void);

		//-------------------------------------------------
		// param: WCHAR * (텍스트 파일 이름)
		// return: TRUE, FALSE
		//
		// 텍스트 파일 열기
		//-------------------------------------------------
		bool LoadFile(const WCHAR &wFileName);

		//-------------------------------------------------
		// param: WCHAR * (구역 이름)
		// return: TRUE, FALSE
		//
		// INI 파일에 입력된 구역을 찾아 이동
		//-------------------------------------------------
		bool ProvideArea(const WCHAR &wAreaName);

		//-------------------------------------------------
		// param: WCHAR *(INI의 변수 이름), WCHAR *(저장될 문자열), int *(버퍼의 길이 in / 문자열 길이 out)  
		// param: WCHAR *(INI의 변수 이름), int *(저장될 변수)
		// param: WCHAR *(INI의 변수 이름), float *(저장될 변수)
		//
		// GetValue 원하는 값을 얻는다.
		//-------------------------------------------------
		bool GetValue(const WCHAR &wName, const int &iBufferSize, WCHAR *wpValue);
		bool GetValue(const WCHAR &wName, int *ipValue);
		bool GetValue(const WCHAR &wName, float *fpValue);

	protected:
		//-------------------------------------------------
		// param: none
		// return: TRUE, FALSE
		//
		// 스페이스, 탭, 엔터코드, 주석 처리 스킵
		// m_chBuffer에 로드된 정보를 m_iBufferFocusPos부터 주석처리하고
		// 다음 위치를 찾아낸다.
		//-------------------------------------------------
		bool SkipNoneCommand(void);

		//-------------------------------------------------
		// param: WCHAR **(다음 단어의 시작 위치), int *(다음 단어의 길이)
		// return: TRUE, FALSE
		//
		// 다음 단어 얻기.
		// 현재 버퍼의 m_iBufferFocusPos 부터 다음 단어위치를 얻는다.
		// m_iBufferFocusPos는 얻은 단어의 끝에 위치.
		// 포인터를 받기 위해 포인터에 포인터로 넣음.
		//-------------------------------------------------
		bool GetNextWord(WCHAR **wppBuffer, int *ipLength);

		//-------------------------------------------------
		// param: WCHAR **(다음 단어의 시작 위치), int *(다음 단어의 길이)
		// return: TURE, FALSE
		//
		// "......." 문자열 읽기
		// m_iBufferFocusPoss는 얻은 대화문장의 끝에 위치.
		//-------------------------------------------------
		bool GetStringWord(WCHAR **wppBuffer, int *ipLength);

	protected:
		//-------------------------------------------------
		// 버퍼
		//-------------------------------------------------
		//WCHAR *m_wpBuffer;                                        // array version
        std::unique_ptr<WCHAR []> m_spBuffer;                        // smart pointer version

		//-------------------------------------------------
		// 버퍼에 로드된 INI 크기
		//-------------------------------------------------
		DWORD m_dwFileSize;

		//-------------------------------------------------
		// 버퍼의 현재 위치
		//-------------------------------------------------
		DWORD m_dwBufferAreaStart;
		DWORD m_dwBufferAreaEnd;
		DWORD m_dwBufferFocusPos;

		//-------------------------------------------------
		// 구역 찾는 상태
		//-------------------------------------------------
		bool m_bProvideAreaMode;
	};
}
