#include "TextParser.h"
#include "FileStream.h"

#include <wchar.h>

//-------------------------------------------------
// param: none
// return: none
//-------------------------------------------------
void NOH::CTextParser::Initialize(void)
{
    /*m_wpBuffer = new WCHAR[static_cast<int>( TEXT_PARSER::BUFF_SIZE )];
	wmemset(m_wpBuffer, 0, static_cast<int>( TEXT_PARSER::BUFF_SIZE ));*/

    wmemset(m_spBuffer.get(), 0, static_cast<int>( TEXT_PARSER::BUFF_TEXTPARSER_1000000 ));
	m_dwFileSize = 0;

	m_dwBufferAreaStart = -1;
	m_dwBufferAreaEnd = -1;

	m_dwBufferFocusPos = 0;

	m_bProvideAreaMode = false;
}

//-------------------------------------------------
// param: WCHAR * (�ؽ�Ʈ ���� �̸�)
// return: true, false
//
// �ؽ�Ʈ ���� ����
//-------------------------------------------------
bool NOH::CTextParser::LoadFile(const WCHAR &wFileName)
{
    CFileStream _filestream;
	DWORD _dwreadsize = 0;

    Initialize();

	//hFile = CreateFile(szFileName, GENERIC_READ, NULL, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL||FILE_FLAG_DELETE_ON_CLOSE, NULL);
	// ���� ���� ����
	if ( !_filestream.Open( reinterpret_cast<LPCWSTR>( &wFileName ), GENERIC_READ, OPEN_EXISTING ) )
		return false;

	// ���� ũ�� �˻�
	//m_iFileSize = GetFileSize(hFile, NULL);
    m_dwFileSize = _filestream.GetSize();

	// ���ۺ��� ũ�ٸ�
	if ( static_cast<int>( TEXT_PARSER::BUFF_TEXTPARSER_1000000 ) < m_dwFileSize )
	{
		//CloseHandle(hFile);
        _filestream.Close();
		Initialize();
		return false;
	}

	// BOM �ڵ� ����
	WCHAR _wbomcode[10] = { NULL };
	DWORD _dwbomdcodesize = 2;
	// BOM �ڵ� ����
	// 2byte�� ����.
	//ReadFile(hFile, wpBOMCODE, 2, &dwRead, NULL);
    _filestream.Read( reinterpret_cast<LPVOID>( _wbomcode ), _dwbomdcodesize, _dwreadsize );

	if ( 0xfeff != _wbomcode[0] )
	{
		//CloseHandle(hFile);
        _filestream.Close();
		Initialize();
		return false;
	}

	m_dwFileSize -= 2;

	// ���� �б�
	//ReadFile(hFile, m_wpBuffer, m_iFileSize, &dwRead, NULL);
    /*FileStream.Read( reinterpret_cast<LPVOID>( m_wpBuffer ), m_dwFileSize, dwReadSize );*/
    _filestream.Read( reinterpret_cast<LPVOID>( m_spBuffer.get() ), m_dwFileSize, _dwreadsize );

	if ( _dwreadsize != m_dwFileSize )
	{
		//CloseHandle(hFile);
        _filestream.Close();
		Initialize();
		return false;
	}

	_filestream.Close();

	return true;
}

//-------------------------------------------------
// param: WCHAR * (���� �̸�)
// return: true, false
//
// INI ���Ͽ� �Էµ� ������ ã�� �̵�
//-------------------------------------------------
bool NOH::CTextParser::ProvideArea(const WCHAR &wAreaName)
{
	WCHAR *_wpbuffer, _wbuffer[1024];
	int _ilegnth = 0;
	bool _bareaflag = false;

	m_dwBufferAreaStart = -1;
	m_dwBufferAreaEnd = -1;
	m_dwBufferFocusPos = 0;

	m_bProvideAreaMode = true;

	//-------------------------------------------------
	// : , { , } �� ������ ã�´�.
	//-------------------------------------------------
	while (GetNextWord(&_wpbuffer, &_ilegnth))
	{
		memset(_wbuffer, 0, sizeof(_wbuffer));
	
		// GetNextWord���� Length�� 1���� �� �� ���� buffer�� byte�� 2byte�� �̵���.
		// ����, memcpy�� ���� sizeof(WCHAR) * ipLength ����� ��.
		memcpy_s(_wbuffer, sizeof(_wbuffer), _wpbuffer, sizeof(WCHAR) * _ilegnth);

		// ù ���ڰ� : ��� ���� ����
		if (_wbuffer[0] == L':')
		{
			
			// ���� �̸� ��
			if (0 == wcscmp(_wbuffer + 1, &wAreaName))
			{
				// ���� ���ڴ� �ݵ�� { ���� �Ѵ�.
				if (GetNextWord(&_wpbuffer, &_ilegnth))
				{
					memset(_wbuffer, 0, 256);
					memcpy_s(_wbuffer, sizeof(_wbuffer), _wpbuffer, _ilegnth);

					// { Ȯ��
					if (_wpbuffer[0] == '{')
					{
						// ���� ���� �� ��ġ ����
						if (!SkipNoneCommand())
						{
							m_bProvideAreaMode = false;
							return false;
						}

						// ������ ���� ��ġ�� ����
						m_dwBufferAreaStart = m_dwBufferFocusPos;
						_bareaflag = true;
					}
				}
				else
				{
					m_bProvideAreaMode = false;
					return false;
				}
			}
		}
		else if (_bareaflag && _wbuffer[0] == '}')
		{
			// ������ �� ��ġ�� ����
			m_dwBufferAreaEnd = m_dwBufferFocusPos - 1;
			m_bProvideAreaMode = false;
			return true;
		}
	}

	m_bProvideAreaMode = false;
	return false;
}

//-------------------------------------------------
// param: WCHAR *(INI�� ���� �̸�), WCHAR *(����� ���ڿ�), int *(������ ���� in / ���ڿ� ���� out)  
//
// GetValue ���ϴ� ���� ��´�.
//-------------------------------------------------
bool NOH::CTextParser::GetValue(const WCHAR &wName, const int &iBufferSize, WCHAR * wpValue)
{
	WCHAR *_wpbuffer, _wbuffer[1024];
	int _ilength = 0;

	// ���� ������ ó������ �˻縦 ����
	m_dwBufferFocusPos = m_dwBufferAreaStart;

	// szName �� ���� ���ڿ��� �˻�
	while (GetNextWord(&_wpbuffer, &_ilength))
	{
		memset(_wbuffer, 0, sizeof(_wbuffer));
		memcpy_s(_wbuffer, sizeof(_wbuffer), _wpbuffer, sizeof(WCHAR) * _ilength);

		if (0 == wcscmp(&wName, _wbuffer))
		{
			// ���� ���ڿ� �ε�ȣ�� �˻�
			if (GetNextWord(&_wpbuffer, &_ilength))
			{
				memset(_wbuffer, 0, 512);
				memcpy_s(_wbuffer, sizeof(_wbuffer), _wpbuffer, sizeof(WCHAR) * _ilength);

				// = Ȯ��
				if (0 == wcscmp(_wbuffer, L"="))
				{
					// ���� ���ڿ� ����
					if (GetNextWord(&_wpbuffer, &_ilength))
					{
						// ���� ���̷� �Է¹ް�, ��� ���� ���̷� �ٽ� ������
						if (iBufferSize <= _ilength)
							return false;

						memset(wpValue, 0, iBufferSize);
						memcpy_s(wpValue, iBufferSize, _wpbuffer, sizeof(WCHAR) * _ilength);

						return true;
					}
					return false;
				}
			}

			return false;
		}
	}

	return false;
}

//-------------------------------------------------
// param: WCHAR *(INI�� ���� �̸�), int *(����� ����)
//
// GetValue ���ϴ� ���� ��´�.
//-------------------------------------------------
bool NOH::CTextParser::GetValue(const WCHAR &wName, int * ipValue)
{
	WCHAR *_wpbuffer, _wbuffer[1024];
	int _ilegnth = 0;

	// ���� ������ ó������ �˻縦 ����
	m_dwBufferFocusPos = m_dwBufferAreaStart;

	// szName �� ���� ���ڿ��� �˻�
	while (GetNextWord(&_wpbuffer, &_ilegnth))
	{
		memset(_wbuffer, 0, sizeof(_wbuffer));
		memcpy_s(_wbuffer, sizeof(_wbuffer), _wpbuffer, sizeof(WCHAR) * _ilegnth);

		if (0 == wcscmp(&wName, _wbuffer))
		{
			// ���� ���ڿ� �ε�ȣ�� �˻�
			if (GetNextWord(&_wpbuffer, &_ilegnth))
			{
				memset(_wbuffer, 0, 512);
				memcpy_s(_wbuffer, sizeof(_wbuffer), _wpbuffer, sizeof(WCHAR) * _ilegnth);

				// = Ȯ��
				if (0 == wcscmp(_wbuffer, L"="))
				{
					// ���� ���ڿ� ����
					if (GetNextWord(&_wpbuffer, &_ilegnth))
					{
						memset(_wbuffer, 0, 512);
						memcpy_s(_wbuffer, sizeof(_wbuffer), _wpbuffer, sizeof(WCHAR) * _ilegnth);
						
						*ipValue = _wtoi(_wbuffer);

						return true;
					}
					return false;
				}
			}

			return false;
		}
	}

	return false;
}

//-------------------------------------------------
// param: WCHAR *(INI�� ���� �̸�), float *(����� ����)
//
// GetValue ���ϴ� ���� ��´�.
//-------------------------------------------------
bool NOH::CTextParser::GetValue(const WCHAR &wName, float * fpValue)
{
	WCHAR *_wpbuffer, _wbuffer[256];
	int _ilegnth = 0;
	double _dvalue = 0;

	// ���� ������ ó������ �˻縦 ����
	m_dwBufferFocusPos = m_dwBufferAreaStart;

	// szName �� ���� ���ڿ��� �˻�
	while (GetNextWord(&_wpbuffer, &_ilegnth))
	{
		memset(_wbuffer, 0, 512);
		memcpy_s(_wbuffer, sizeof(_wbuffer), _wpbuffer, sizeof(WCHAR) * _ilegnth);

		if (0 == wcscmp(&wName, _wbuffer))
		{
			// ���� ���ڿ� �ε�ȣ�� �˻�
			if (GetNextWord(&_wpbuffer, &_ilegnth))
			{
				memset(_wbuffer, 0, 512);
				memcpy_s(_wbuffer, sizeof(_wbuffer), _wpbuffer, sizeof(WCHAR) * _ilegnth);

				// = Ȯ��
				if (0 == wcscmp(_wbuffer, L"="))
				{
					// ���� ���ڿ� ����
					if (GetNextWord(&_wpbuffer, &_ilegnth))
					{
						memset(_wbuffer, 0, 512);
						memcpy_s(_wbuffer, sizeof(_wbuffer), _wpbuffer, sizeof(WCHAR) * _ilegnth);
						
						_dvalue = _wtof(_wbuffer);
						*fpValue = (float)_dvalue;

						return true;
					}

					return false;
				}
			}

			return false;
		}
	}

	return false;
}

//-------------------------------------------------
// param: none
// return: true, false
//
// �����̽�, ��, �����ڵ�, �ּ� ó�� ��ŵ
// m_chBuffer�� �ε�� ������ m_iBufferFocusPos���� �ּ�ó���ϰ�
// ���� ��ġ�� ã�Ƴ���.
//-------------------------------------------------
bool NOH::CTextParser::SkipNoneCommand(void)
{
    // array version
	//WCHAR *_wpbuffer = m_wpBuffer + m_dwBufferFocusPos;
    
    // smart pointer version
    WCHAR *_wpbuffer = m_spBuffer.get() + m_dwBufferFocusPos;


	while (1)
	{
		// �ؽ�Ʈ ������ ��忡�� ���� ��ü�� �о ����ϹǷ�
		// ������ ���̿� ������ ���̰� ����.
		// ��, ���̻� ���� �����Ͱ� ����.
		// m_iLoadSize�� ������ �� ũ���̰�, m_iBufferFocusPos�� WCHAR�� �迭�� ��ġ�̹Ƿ�, ũ�� �񱳸� ���ؼ��� m_iLoadSize / 2�� �ؾ� ��.
		// m_bProvideAreaMode�� ������ ã�� ���¿����� m_iBufferAreaEnd�� m_iBufferFocusPos���� ���� �� �ֱ� ������ �ʿ���.
		// m_bProvideAreaMode�� true ���¸� ���� ������ ���ʿ� ��.
		// ������, m_bProvideAreaMode�� false ���¶�� ���� ������ ã�� ���°� �ƴ϶�� ���̹Ƿ� ���� ������ �ʿ���.
		// �׷��� ������ not�� ������ ����ϰ� ����.
		if (m_dwBufferFocusPos > m_dwFileSize / 2 || (!m_bProvideAreaMode && m_dwBufferFocusPos > m_dwBufferAreaEnd))
			return false;

		//-----------------------------------------------
		// NonCode
		// �齺���̽�		:0x08
		// ��			:0x09
		// ���� �ǵ�		:0x0a
		// ĳ���� ����	:0x0d
		// ����			:0x20
		//-----------------------------------------------
		if (*_wpbuffer == 0x20 || *_wpbuffer == 0x0d ||
			*_wpbuffer == 0x0a || *_wpbuffer == 0x09 ||
			*_wpbuffer == 0x08)
		{
			// Ȥ�� ���� ����
			//*wpBuffer = 0x20;

			++_wpbuffer;
			++m_dwBufferFocusPos;
		}
		// �ּ� '//' ó��
		// �����ڵ尡 ���ö� ���� ����
		else if (*_wpbuffer == '/' && *(_wpbuffer + 1) == '/')
		{
			while (*_wpbuffer != 0x0d)
			{
				// Ȥ�� ���� ����
				//*wpBuffer = 0x20;

				++_wpbuffer;
				++m_dwBufferFocusPos;

				// �ؽ�Ʈ ������ ��忡�� ���� ��ü�� �о ����ϹǷ�
				// ������ ���̿� ������ ���̰� ����.
				// ��, ���̻� ���� �����Ͱ� ����.
				// m_iLoadSize�� ������ �� ũ���̰�, m_iBufferFocusPos�� WCHAR�� �迭�� ��ġ�̹Ƿ�, ũ�� �񱳸� ���ؼ��� m_iLoadSize / 2�� �ؾ� ��.
				// m_bProvideAreaMode�� ������ ã�� ���¿����� m_iBufferAreaEnd�� m_iBufferFocusPos���� ���� �� �ֱ� ������ �ʿ���.
				// m_bProvideAreaMode�� true ���¸� ���� ������ ���ʿ� ��.
				// ������, m_bProvideAreaMode�� false ���¶�� ���� ������ ã�� ����
				if (m_dwBufferFocusPos > m_dwFileSize / 2 || (!m_bProvideAreaMode && m_dwBufferFocusPos > m_dwBufferAreaEnd))
					return false;
			}
		}
		// �ּ� '/*' ó�� 
		// '*/' �� ���� �� ���� ����
		else if (*_wpbuffer == '/' && *(_wpbuffer + 1) == '*')
		{
			while (!(*_wpbuffer == '*' && *(_wpbuffer + 1) == '/'))
			{
				// Ȥ�� ���� ����
				//*wpBuffer = 0x20;

				++_wpbuffer;
				++m_dwBufferFocusPos;

				// �ؽ�Ʈ ������ ��忡�� ���� ��ü�� �о ����ϹǷ�
				// ������ ���̿� ������ ���̰� ����.
				// ��, ���̻� ���� �����Ͱ� ����.
				// m_bProvideAreaMode�� ������ ã�� ���¿����� m_iBufferAreaEnd�� m_iBufferFocusPos���� ���� �� �ֱ� ������ �ʿ���.
				// m_bProvideAreaMode�� true ���¸� ���� ������ ���ʿ� ��.
				// ������, m_bProvideAreaMode�� false ���¶�� ���� ������ ã�� ���°� �ƴ϶�� ���̹Ƿ� ���� ������ �ʿ���.
				// �׷��� ������ not�� ������ ����ϰ� ����.
				if (m_dwBufferFocusPos > m_dwFileSize / 2 || (!m_bProvideAreaMode && m_dwBufferFocusPos > m_dwBufferAreaEnd))
					return false;
			}
			// Ȥ�� ���� ����
			//*wpBuffer = 0x20;
			++_wpbuffer;
			// Ȥ�� ���� ����
			//*wpBuffer = 0x20;
			++_wpbuffer;

			m_dwBufferFocusPos += 2;
		}
		// ��ŵ�� ���ڰ� �ƴ�
		else
		{
			break;
		}
	}

	return true;
}

//-------------------------------------------------
// param: WCHAR **(���� �ܾ��� ���� ��ġ), int *(���� �ܾ��� ����)
// return: true, false
//
// ���� �ܾ� ���.
// ���� ������ m_iBufferFocusPos ���� ���� �ܾ���ġ�� ��´�.
// m_iBufferFocusPos�� ���� �ܾ��� ���� ��ġ.
// �����͸� �ޱ� ���� �����Ϳ� �����ͷ� ����.
//-------------------------------------------------
bool NOH::CTextParser::GetNextWord(WCHAR ** wppBuffer, int * ipLength)
{
	WCHAR *_wpbuffertemp = nullptr;

	// �̸� ���� ����
	// �� �̻� ���� ������ ��ü�� ���� ��� false�� return
	if (!SkipNoneCommand())
		return false;

	// �ؽ�Ʈ ������ ��忡�� ���� ��ü�� �о ����ϹǷ�
	// ������ ���̿� ������ ���̰� ����.
	// ��, ���̻� ���� �����Ͱ� ����.
	// m_iLoadSize�� ������ �� ũ���̰�, m_iBufferFocusPos�� WCHAR�� �迭�� ��ġ�̹Ƿ�, ũ�� �񱳸� ���ؼ��� m_iLoadSize / 2�� �ؾ� ��.
	// m_bProvideAreaMode�� ������ ã�� ���¿����� m_iBufferAreaEnd�� m_iBufferFocusPos���� ���� �� �ֱ� ������ �ʿ���.
	// m_bProvideAreaMode�� true ���¸� ���� ������ ���ʿ� ��.
	// ������, m_bProvideAreaMode�� false ���¶�� ���� ������ ã�� ����
	if (m_dwBufferFocusPos > m_dwFileSize / 2 || (!m_bProvideAreaMode && m_dwBufferFocusPos > m_dwBufferAreaEnd))
		return false;

    // array version
	//wpBufferTemp = *wppBuffer = m_wpBuffer + m_dwBufferFocusPos;

    // smart pointer version
    _wpbuffertemp = *wppBuffer = m_spBuffer.get() + m_dwBufferFocusPos;

	*ipLength = 0;

	// ù ���ڰ� '"' ��� ���ڿ� �б�� ��´�.
	if (**wppBuffer == '"')
	{
		if (GetStringWord(wppBuffer, ipLength))
			return true;
		else
			return false;
	}

	while (1)
	{
		//-----------------------------------------------
		// �ܾ��� ����
		// �ĸ�			','
		// ��ħǥ		'.'
		// ����ǥ		'"'
		// �����̽�		0x20
		// �齺���̽�		0x08
		// ��			0x09
		// ���� �ǵ�		0x0a
		// �ɸ��� ����	0x0d
		// ����			0x20
		//-----------------------------------------------
		if (**wppBuffer == 0x0d || **wppBuffer == 0x0a ||
			**wppBuffer == 0x09 || **wppBuffer == 0x08 ||
			**wppBuffer == 0x20 || **wppBuffer == '"' ||
			**wppBuffer == '.' || **wppBuffer == ',')
			break;

		// �ܾ� ���� ���
		// Ÿ���� WCHAR�̱� ������ ++�ϸ� 2byte�� �̵�
		// ����, ipLength ��ŭ memcpy�� ���� sizeof(WCHAR) * ipLength ����� ��.
		++m_dwBufferFocusPos;
		++(*wppBuffer);
		++(*ipLength);

		// �ؽ�Ʈ ������ ��忡�� ���� ��ü�� �о ����ϹǷ�
		// ������ ���̿� ������ ���̰� ����.
		// ��, ���̻� ���� �����Ͱ� ����.
		// m_iLoadSize�� ������ �� ũ���̰�, m_iBufferFocusPos�� WCHAR�� �迭�� ��ġ�̹Ƿ�, ũ�� �񱳸� ���ؼ��� m_iLoadSize / 2�� �ؾ� ��.
		// m_bProvideAreaMode�� ������ ã�� ���¿����� m_iBufferAreaEnd�� m_iBufferFocusPos���� ���� �� �ֱ� ������ �ʿ���.
		// m_bProvideAreaMode�� true ���¸� ���� ������ ���ʿ� ��.
		// ������, m_bProvideAreaMode�� false ���¶�� ���� ������ ã�� ����
		if (m_dwBufferFocusPos > m_dwFileSize / 2 || (!m_bProvideAreaMode && m_dwBufferFocusPos > m_dwBufferAreaEnd))
			break;
	}

	// ��ġ ����
	*wppBuffer = _wpbuffertemp;

	// ���̰� 0�̸� ����
	//if (0 == *ipLength)
	//	return false;

	return true;
}

//-------------------------------------------------
// param: WCHAR **(���� �ܾ��� ���� ��ġ), int *(���� �ܾ��� ����)
// return: TURE, false
//
// "......." ���ڿ� �б�
// m_iBufferFocusPoss�� ���� ��ȭ������ ���� ��ġ.
//-------------------------------------------------
bool NOH::CTextParser::GetStringWord(WCHAR ** wppBuffer, int * ipLength)
{
    // array version
	//WCHAR *wpbuffertemp = *wppBuffer = m_wpBuffer + m_dwBufferFocusPos;

    // smart pointer version
    WCHAR *wpbuffertemp = *wppBuffer = m_spBuffer.get() + m_dwBufferFocusPos;

	*ipLength = 0;

	// ù ���ڰ� '"' �� �ƴ϶�� return false
	if (**wppBuffer != '"')
		return false;

	// ù ����ǥ �ѱ�
	++m_dwBufferFocusPos;
	++(*wppBuffer);
	++wpbuffertemp;

	while (1)
	{
		//-----------------------------------------------
		// �ܾ��� ����
		// ����ǥ		'"'
		// ���� �ǵ�		0x0a
		// �ɸ��� ����	0x0d
		//-----------------------------------------------
		if (**wppBuffer == 0x0d || **wppBuffer == 0x0a || **wppBuffer == '"')
		{
			// **wppBuffer�� ����ǥ�� ���,
			// ����ǥ���� ���� ���̹Ƿ� ��ġ �̵�
			++m_dwBufferFocusPos;
			break;
		}

		// �ܾ� ���� ���
		++m_dwBufferFocusPos;
		++(*wppBuffer);
		++(*ipLength);

		// �ؽ�Ʈ ������ ��忡�� ���� ��ü�� �о ����ϹǷ�
		// �ؽ�Ʈ ������ ��忡�� ���� ��ü�� �о ����ϹǷ�
		// ������ ���̿� ������ ���̰� ����.
		// ��, ���̻� ���� �����Ͱ� ����.
		// m_iLoadSize�� ������ �� ũ���̰�, m_iBufferFocusPos�� WCHAR�� �迭�� ��ġ�̹Ƿ�, ũ�� �񱳸� ���ؼ��� m_iLoadSize / 2�� �ؾ� ��.
		// m_bProvideAreaMode�� ������ ã�� ���¿����� m_iBufferAreaEnd�� m_iBufferFocusPos���� ���� �� �ֱ� ������ �ʿ���.
		// m_bProvideAreaMode�� true ���¸� ���� ������ ���ʿ� ��.
		// ������, m_bProvideAreaMode�� false ���¶�� ���� ������ ã�� ����
		if (m_dwBufferFocusPos > m_dwFileSize / 2 || (!m_bProvideAreaMode && m_dwBufferFocusPos > m_dwBufferAreaEnd))
			break;
	}

	// ��ġ ����
	*wppBuffer = wpbuffertemp;

	// ���̰� 0�̸� ����
	//if (0 == *ipLength)
	//	return false;

	return true;
}
