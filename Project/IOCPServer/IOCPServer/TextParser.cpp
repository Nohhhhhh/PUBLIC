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
// param: WCHAR * (텍스트 파일 이름)
// return: true, false
//
// 텍스트 파일 열기
//-------------------------------------------------
bool NOH::CTextParser::LoadFile(const WCHAR &wFileName)
{
    CFileStream _filestream;
	DWORD _dwreadsize = 0;

    Initialize();

	//hFile = CreateFile(szFileName, GENERIC_READ, NULL, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL||FILE_FLAG_DELETE_ON_CLOSE, NULL);
	// 파일 오픈 여부
	if ( !_filestream.Open( reinterpret_cast<LPCWSTR>( &wFileName ), GENERIC_READ, OPEN_EXISTING ) )
		return false;

	// 파일 크기 검사
	//m_iFileSize = GetFileSize(hFile, NULL);
    m_dwFileSize = _filestream.GetSize();

	// 버퍼보다 크다면
	if ( static_cast<int>( TEXT_PARSER::BUFF_TEXTPARSER_1000000 ) < m_dwFileSize )
	{
		//CloseHandle(hFile);
        _filestream.Close();
		Initialize();
		return false;
	}

	// BOM 코드 삭제
	WCHAR _wbomcode[10] = { NULL };
	DWORD _dwbomdcodesize = 2;
	// BOM 코드 삭제
	// 2byte만 읽자.
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

	// 전부 읽기
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
// param: WCHAR * (구역 이름)
// return: true, false
//
// INI 파일에 입력된 구역을 찾아 이동
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
	// : , { , } 로 구역을 찾는다.
	//-------------------------------------------------
	while (GetNextWord(&_wpbuffer, &_ilegnth))
	{
		memset(_wbuffer, 0, sizeof(_wbuffer));
	
		// GetNextWord에서 Length가 1증가 할 때 실제 buffer의 byte는 2byte씩 이동함.
		// 따라서, memcpy할 때는 sizeof(WCHAR) * ipLength 해줘야 함.
		memcpy_s(_wbuffer, sizeof(_wbuffer), _wpbuffer, sizeof(WCHAR) * _ilegnth);

		// 첫 문자가 : 라면 구역 시작
		if (_wbuffer[0] == L':')
		{
			
			// 구역 이름 비교
			if (0 == wcscmp(_wbuffer + 1, &wAreaName))
			{
				// 다음 문자는 반드시 { 여야 한다.
				if (GetNextWord(&_wpbuffer, &_ilegnth))
				{
					memset(_wbuffer, 0, 256);
					memcpy_s(_wbuffer, sizeof(_wbuffer), _wpbuffer, _ilegnth);

					// { 확인
					if (_wpbuffer[0] == '{')
					{
						// 공백 제거 후 위치 저장
						if (!SkipNoneCommand())
						{
							m_bProvideAreaMode = false;
							return false;
						}

						// 구역의 시작 위치를 저장
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
			// 구역의 끝 위치를 저장
			m_dwBufferAreaEnd = m_dwBufferFocusPos - 1;
			m_bProvideAreaMode = false;
			return true;
		}
	}

	m_bProvideAreaMode = false;
	return false;
}

//-------------------------------------------------
// param: WCHAR *(INI의 변수 이름), WCHAR *(저장될 문자열), int *(버퍼의 길이 in / 문자열 길이 out)  
//
// GetValue 원하는 값을 얻는다.
//-------------------------------------------------
bool NOH::CTextParser::GetValue(const WCHAR &wName, const int &iBufferSize, WCHAR * wpValue)
{
	WCHAR *_wpbuffer, _wbuffer[1024];
	int _ilength = 0;

	// 현재 구역의 처음부터 검사를 진행
	m_dwBufferFocusPos = m_dwBufferAreaStart;

	// szName 과 같은 문자열을 검색
	while (GetNextWord(&_wpbuffer, &_ilength))
	{
		memset(_wbuffer, 0, sizeof(_wbuffer));
		memcpy_s(_wbuffer, sizeof(_wbuffer), _wpbuffer, sizeof(WCHAR) * _ilength);

		if (0 == wcscmp(&wName, _wbuffer))
		{
			// 다음 문자열 부등호를 검색
			if (GetNextWord(&_wpbuffer, &_ilength))
			{
				memset(_wbuffer, 0, 512);
				memcpy_s(_wbuffer, sizeof(_wbuffer), _wpbuffer, sizeof(WCHAR) * _ilength);

				// = 확인
				if (0 == wcscmp(_wbuffer, L"="))
				{
					// 다음 문자열 얻음
					if (GetNextWord(&_wpbuffer, &_ilength))
					{
						// 버퍼 길이로 입력받고, 출력 버퍼 길이로 다시 보내줌
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
// param: WCHAR *(INI의 변수 이름), int *(저장될 변수)
//
// GetValue 원하는 값을 얻는다.
//-------------------------------------------------
bool NOH::CTextParser::GetValue(const WCHAR &wName, int * ipValue)
{
	WCHAR *_wpbuffer, _wbuffer[1024];
	int _ilegnth = 0;

	// 현재 구역의 처음부터 검사를 진행
	m_dwBufferFocusPos = m_dwBufferAreaStart;

	// szName 과 같은 문자열을 검색
	while (GetNextWord(&_wpbuffer, &_ilegnth))
	{
		memset(_wbuffer, 0, sizeof(_wbuffer));
		memcpy_s(_wbuffer, sizeof(_wbuffer), _wpbuffer, sizeof(WCHAR) * _ilegnth);

		if (0 == wcscmp(&wName, _wbuffer))
		{
			// 다음 문자열 부등호를 검색
			if (GetNextWord(&_wpbuffer, &_ilegnth))
			{
				memset(_wbuffer, 0, 512);
				memcpy_s(_wbuffer, sizeof(_wbuffer), _wpbuffer, sizeof(WCHAR) * _ilegnth);

				// = 확인
				if (0 == wcscmp(_wbuffer, L"="))
				{
					// 다음 문자열 얻음
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
// param: WCHAR *(INI의 변수 이름), float *(저장될 변수)
//
// GetValue 원하는 값을 얻는다.
//-------------------------------------------------
bool NOH::CTextParser::GetValue(const WCHAR &wName, float * fpValue)
{
	WCHAR *_wpbuffer, _wbuffer[256];
	int _ilegnth = 0;
	double _dvalue = 0;

	// 현재 구역의 처음부터 검사를 진행
	m_dwBufferFocusPos = m_dwBufferAreaStart;

	// szName 과 같은 문자열을 검색
	while (GetNextWord(&_wpbuffer, &_ilegnth))
	{
		memset(_wbuffer, 0, 512);
		memcpy_s(_wbuffer, sizeof(_wbuffer), _wpbuffer, sizeof(WCHAR) * _ilegnth);

		if (0 == wcscmp(&wName, _wbuffer))
		{
			// 다음 문자열 부등호를 검색
			if (GetNextWord(&_wpbuffer, &_ilegnth))
			{
				memset(_wbuffer, 0, 512);
				memcpy_s(_wbuffer, sizeof(_wbuffer), _wpbuffer, sizeof(WCHAR) * _ilegnth);

				// = 확인
				if (0 == wcscmp(_wbuffer, L"="))
				{
					// 다음 문자열 얻음
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
// 스페이스, 탭, 엔터코드, 주석 처리 스킵
// m_chBuffer에 로드된 정보를 m_iBufferFocusPos부터 주석처리하고
// 다음 위치를 찾아낸다.
//-------------------------------------------------
bool NOH::CTextParser::SkipNoneCommand(void)
{
    // array version
	//WCHAR *_wpbuffer = m_wpBuffer + m_dwBufferFocusPos;
    
    // smart pointer version
    WCHAR *_wpbuffer = m_spBuffer.get() + m_dwBufferFocusPos;


	while (1)
	{
		// 텍스트 컴파일 모드에는 파일 전체를 읽어서 사용하므로
		// 파일의 길이와 버퍼의 길이가 같다.
		// 즉, 더이상 읽을 데이터가 없음.
		// m_iLoadSize는 문서의 총 크기이고, m_iBufferFocusPos는 WCHAR인 배열의 위치이므로, 크기 비교를 위해서는 m_iLoadSize / 2를 해야 함.
		// m_bProvideAreaMode는 구역을 찾는 상태에서는 m_iBufferAreaEnd가 m_iBufferFocusPos보다 작을 수 있기 때문에 필요함.
		// m_bProvideAreaMode가 true 상태면 뒤의 조건이 불필요 함.
		// 하지만, m_bProvideAreaMode가 false 상태라는 것은 구역을 찾는 상태가 아니라는 뜻이므로 뒤의 조건이 필요함.
		// 그렇기 때문에 not을 붙혀서 사용하고 있음.
		if (m_dwBufferFocusPos > m_dwFileSize / 2 || (!m_bProvideAreaMode && m_dwBufferFocusPos > m_dwBufferAreaEnd))
			return false;

		//-----------------------------------------------
		// NonCode
		// 백스페이스		:0x08
		// 탭			:0x09
		// 라인 피드		:0x0a
		// 캐리지 리턴	:0x0d
		// 공백			:0x20
		//-----------------------------------------------
		if (*_wpbuffer == 0x20 || *_wpbuffer == 0x0d ||
			*_wpbuffer == 0x0a || *_wpbuffer == 0x09 ||
			*_wpbuffer == 0x08)
		{
			// 혹시 몰라서 지움
			//*wpBuffer = 0x20;

			++_wpbuffer;
			++m_dwBufferFocusPos;
		}
		// 주석 '//' 처리
		// 엔터코드가 나올때 까지 무시
		else if (*_wpbuffer == '/' && *(_wpbuffer + 1) == '/')
		{
			while (*_wpbuffer != 0x0d)
			{
				// 혹시 몰라서 지움
				//*wpBuffer = 0x20;

				++_wpbuffer;
				++m_dwBufferFocusPos;

				// 텍스트 컴파일 모드에는 파일 전체를 읽어서 사용하므로
				// 파일의 길이와 버퍼의 길이가 같다.
				// 즉, 더이상 읽을 데이터가 없음.
				// m_iLoadSize는 문서의 총 크기이고, m_iBufferFocusPos는 WCHAR인 배열의 위치이므로, 크기 비교를 위해서는 m_iLoadSize / 2를 해야 함.
				// m_bProvideAreaMode는 구역을 찾는 상태에서는 m_iBufferAreaEnd가 m_iBufferFocusPos보다 작을 수 있기 때문에 필요함.
				// m_bProvideAreaMode가 true 상태면 뒤의 조건이 불필요 함.
				// 하지만, m_bProvideAreaMode가 false 상태라는 것은 구역을 찾는 상태
				if (m_dwBufferFocusPos > m_dwFileSize / 2 || (!m_bProvideAreaMode && m_dwBufferFocusPos > m_dwBufferAreaEnd))
					return false;
			}
		}
		// 주석 '/*' 처리 
		// '*/' 가 나올 때 까지 무시
		else if (*_wpbuffer == '/' && *(_wpbuffer + 1) == '*')
		{
			while (!(*_wpbuffer == '*' && *(_wpbuffer + 1) == '/'))
			{
				// 혹시 몰라서 지움
				//*wpBuffer = 0x20;

				++_wpbuffer;
				++m_dwBufferFocusPos;

				// 텍스트 컴파일 모드에는 파일 전체를 읽어서 사용하므로
				// 파일의 길이와 버퍼의 길이가 같다.
				// 즉, 더이상 읽을 데이터가 없음.
				// m_bProvideAreaMode는 구역을 찾는 상태에서는 m_iBufferAreaEnd가 m_iBufferFocusPos보다 작을 수 있기 때문에 필요함.
				// m_bProvideAreaMode가 true 상태면 뒤의 조건이 불필요 함.
				// 하지만, m_bProvideAreaMode가 false 상태라는 것은 구역을 찾는 상태가 아니라는 뜻이므로 뒤의 조건이 필요함.
				// 그렇기 때문에 not을 붙혀서 사용하고 있음.
				if (m_dwBufferFocusPos > m_dwFileSize / 2 || (!m_bProvideAreaMode && m_dwBufferFocusPos > m_dwBufferAreaEnd))
					return false;
			}
			// 혹시 몰라서 지움
			//*wpBuffer = 0x20;
			++_wpbuffer;
			// 혹시 몰라서 지움
			//*wpBuffer = 0x20;
			++_wpbuffer;

			m_dwBufferFocusPos += 2;
		}
		// 스킵할 문자가 아님
		else
		{
			break;
		}
	}

	return true;
}

//-------------------------------------------------
// param: WCHAR **(다음 단어의 시작 위치), int *(다음 단어의 길이)
// return: true, false
//
// 다음 단어 얻기.
// 현재 버퍼의 m_iBufferFocusPos 부터 다음 단어위치를 얻는다.
// m_iBufferFocusPos는 얻은 단어의 끝에 위치.
// 포인터를 받기 위해 포인터에 포인터로 넣음.
//-------------------------------------------------
bool NOH::CTextParser::GetNextWord(WCHAR ** wppBuffer, int * ipLength)
{
	WCHAR *_wpbuffertemp = nullptr;

	// 미리 공백 제거
	// 더 이상 읽을 데이터 자체가 없는 경우 false가 return
	if (!SkipNoneCommand())
		return false;

	// 텍스트 컴파일 모드에는 파일 전체를 읽어서 사용하므로
	// 파일의 길이와 버퍼의 길이가 같다.
	// 즉, 더이상 읽을 데이터가 없음.
	// m_iLoadSize는 문서의 총 크기이고, m_iBufferFocusPos는 WCHAR인 배열의 위치이므로, 크기 비교를 위해서는 m_iLoadSize / 2를 해야 함.
	// m_bProvideAreaMode는 구역을 찾는 상태에서는 m_iBufferAreaEnd가 m_iBufferFocusPos보다 작을 수 있기 때문에 필요함.
	// m_bProvideAreaMode가 true 상태면 뒤의 조건이 불필요 함.
	// 하지만, m_bProvideAreaMode가 false 상태라는 것은 구역을 찾는 상태
	if (m_dwBufferFocusPos > m_dwFileSize / 2 || (!m_bProvideAreaMode && m_dwBufferFocusPos > m_dwBufferAreaEnd))
		return false;

    // array version
	//wpBufferTemp = *wppBuffer = m_wpBuffer + m_dwBufferFocusPos;

    // smart pointer version
    _wpbuffertemp = *wppBuffer = m_spBuffer.get() + m_dwBufferFocusPos;

	*ipLength = 0;

	// 첫 문자가 '"' 라면 문자열 읽기로 얻는다.
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
		// 단어의 기준
		// 컴마			','
		// 마침표		'.'
		// 따옴표		'"'
		// 스페이스		0x20
		// 백스페이스		0x08
		// 탭			0x09
		// 라인 피드		0x0a
		// 케리지 리턴	0x0d
		// 공백			0x20
		//-----------------------------------------------
		if (**wppBuffer == 0x0d || **wppBuffer == 0x0a ||
			**wppBuffer == 0x09 || **wppBuffer == 0x08 ||
			**wppBuffer == 0x20 || **wppBuffer == '"' ||
			**wppBuffer == '.' || **wppBuffer == ',')
			break;

		// 단어 길이 계산
		// 타입이 WCHAR이기 때문에 ++하면 2byte씩 이동
		// 따라서, ipLength 만큼 memcpy할 때는 sizeof(WCHAR) * ipLength 해줘야 함.
		++m_dwBufferFocusPos;
		++(*wppBuffer);
		++(*ipLength);

		// 텍스트 컴파일 모드에는 파일 전체를 읽어서 사용하므로
		// 파일의 길이와 버퍼의 길이가 같다.
		// 즉, 더이상 읽을 데이터가 없음.
		// m_iLoadSize는 문서의 총 크기이고, m_iBufferFocusPos는 WCHAR인 배열의 위치이므로, 크기 비교를 위해서는 m_iLoadSize / 2를 해야 함.
		// m_bProvideAreaMode는 구역을 찾는 상태에서는 m_iBufferAreaEnd가 m_iBufferFocusPos보다 작을 수 있기 때문에 필요함.
		// m_bProvideAreaMode가 true 상태면 뒤의 조건이 불필요 함.
		// 하지만, m_bProvideAreaMode가 false 상태라는 것은 구역을 찾는 상태
		if (m_dwBufferFocusPos > m_dwFileSize / 2 || (!m_bProvideAreaMode && m_dwBufferFocusPos > m_dwBufferAreaEnd))
			break;
	}

	// 위치 복구
	*wppBuffer = _wpbuffertemp;

	// 길이가 0이면 에러
	//if (0 == *ipLength)
	//	return false;

	return true;
}

//-------------------------------------------------
// param: WCHAR **(다음 단어의 시작 위치), int *(다음 단어의 길이)
// return: TURE, false
//
// "......." 문자열 읽기
// m_iBufferFocusPoss는 얻은 대화문장의 끝에 위치.
//-------------------------------------------------
bool NOH::CTextParser::GetStringWord(WCHAR ** wppBuffer, int * ipLength)
{
    // array version
	//WCHAR *wpbuffertemp = *wppBuffer = m_wpBuffer + m_dwBufferFocusPos;

    // smart pointer version
    WCHAR *wpbuffertemp = *wppBuffer = m_spBuffer.get() + m_dwBufferFocusPos;

	*ipLength = 0;

	// 첫 문자가 '"' 가 아니라면 return false
	if (**wppBuffer != '"')
		return false;

	// 첫 따옴표 넘김
	++m_dwBufferFocusPos;
	++(*wppBuffer);
	++wpbuffertemp;

	while (1)
	{
		//-----------------------------------------------
		// 단어의 기준
		// 따옴표		'"'
		// 라인 피드		0x0a
		// 케리지 리턴	0x0d
		//-----------------------------------------------
		if (**wppBuffer == 0x0d || **wppBuffer == 0x0a || **wppBuffer == '"')
		{
			// **wppBuffer가 따옴표인 경우,
			// 따옴표까지 읽은 것이므로 위치 이동
			++m_dwBufferFocusPos;
			break;
		}

		// 단어 길이 계산
		++m_dwBufferFocusPos;
		++(*wppBuffer);
		++(*ipLength);

		// 텍스트 컴파일 모드에는 파일 전체를 읽어서 사용하므로
		// 텍스트 컴파일 모드에는 파일 전체를 읽어서 사용하므로
		// 파일의 길이와 버퍼의 길이가 같다.
		// 즉, 더이상 읽을 데이터가 없음.
		// m_iLoadSize는 문서의 총 크기이고, m_iBufferFocusPos는 WCHAR인 배열의 위치이므로, 크기 비교를 위해서는 m_iLoadSize / 2를 해야 함.
		// m_bProvideAreaMode는 구역을 찾는 상태에서는 m_iBufferAreaEnd가 m_iBufferFocusPos보다 작을 수 있기 때문에 필요함.
		// m_bProvideAreaMode가 true 상태면 뒤의 조건이 불필요 함.
		// 하지만, m_bProvideAreaMode가 false 상태라는 것은 구역을 찾는 상태
		if (m_dwBufferFocusPos > m_dwFileSize / 2 || (!m_bProvideAreaMode && m_dwBufferFocusPos > m_dwBufferAreaEnd))
			break;
	}

	// 위치 복구
	*wppBuffer = wpbuffertemp;

	// 길이가 0이면 에러
	//if (0 == *ipLength)
	//	return false;

	return true;
}
