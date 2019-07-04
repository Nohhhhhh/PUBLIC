/*
- ��� ���

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
		// ���� �α� & �ܼ� ���
		//wprintf(L"Open File Fail: DB_INFO.ini\n");
			return FALSE;
	}

	// ���� �α� & �ܼ� ���
	//wprintf(L"Open File Success: DB_INFO.ini\n\n");


	if (!TextParser.ProvideArea(L"DB_INFO"))
	{
		// ���� �α� & �ܼ� ���
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
		// ���� �α� & �ܼ� ���
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
        // __NOH.h �� �ű� 
		//enum class TEXT_PARSER
		//{
		//	// ������ ���� ����.
		//	BUFF_TEXTPARSER_1000000 = 1000000
		//};

		//-------------------------------------------------
		// ������, �ı���.
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
		// param: WCHAR * (�ؽ�Ʈ ���� �̸�)
		// return: TRUE, FALSE
		//
		// �ؽ�Ʈ ���� ����
		//-------------------------------------------------
		bool LoadFile(const WCHAR &wFileName);

		//-------------------------------------------------
		// param: WCHAR * (���� �̸�)
		// return: TRUE, FALSE
		//
		// INI ���Ͽ� �Էµ� ������ ã�� �̵�
		//-------------------------------------------------
		bool ProvideArea(const WCHAR &wAreaName);

		//-------------------------------------------------
		// param: WCHAR *(INI�� ���� �̸�), WCHAR *(����� ���ڿ�), int *(������ ���� in / ���ڿ� ���� out)  
		// param: WCHAR *(INI�� ���� �̸�), int *(����� ����)
		// param: WCHAR *(INI�� ���� �̸�), float *(����� ����)
		//
		// GetValue ���ϴ� ���� ��´�.
		//-------------------------------------------------
		bool GetValue(const WCHAR &wName, const int &iBufferSize, WCHAR *wpValue);
		bool GetValue(const WCHAR &wName, int *ipValue);
		bool GetValue(const WCHAR &wName, float *fpValue);

	protected:
		//-------------------------------------------------
		// param: none
		// return: TRUE, FALSE
		//
		// �����̽�, ��, �����ڵ�, �ּ� ó�� ��ŵ
		// m_chBuffer�� �ε�� ������ m_iBufferFocusPos���� �ּ�ó���ϰ�
		// ���� ��ġ�� ã�Ƴ���.
		//-------------------------------------------------
		bool SkipNoneCommand(void);

		//-------------------------------------------------
		// param: WCHAR **(���� �ܾ��� ���� ��ġ), int *(���� �ܾ��� ����)
		// return: TRUE, FALSE
		//
		// ���� �ܾ� ���.
		// ���� ������ m_iBufferFocusPos ���� ���� �ܾ���ġ�� ��´�.
		// m_iBufferFocusPos�� ���� �ܾ��� ���� ��ġ.
		// �����͸� �ޱ� ���� �����Ϳ� �����ͷ� ����.
		//-------------------------------------------------
		bool GetNextWord(WCHAR **wppBuffer, int *ipLength);

		//-------------------------------------------------
		// param: WCHAR **(���� �ܾ��� ���� ��ġ), int *(���� �ܾ��� ����)
		// return: TURE, FALSE
		//
		// "......." ���ڿ� �б�
		// m_iBufferFocusPoss�� ���� ��ȭ������ ���� ��ġ.
		//-------------------------------------------------
		bool GetStringWord(WCHAR **wppBuffer, int *ipLength);

	protected:
		//-------------------------------------------------
		// ����
		//-------------------------------------------------
		//WCHAR *m_wpBuffer;                                        // array version
        std::unique_ptr<WCHAR []> m_spBuffer;                        // smart pointer version

		//-------------------------------------------------
		// ���ۿ� �ε�� INI ũ��
		//-------------------------------------------------
		DWORD m_dwFileSize;

		//-------------------------------------------------
		// ������ ���� ��ġ
		//-------------------------------------------------
		DWORD m_dwBufferAreaStart;
		DWORD m_dwBufferAreaEnd;
		DWORD m_dwBufferFocusPos;

		//-------------------------------------------------
		// ���� ã�� ����
		//-------------------------------------------------
		bool m_bProvideAreaMode;
	};
}
