#pragma once

namespace NOH
{
    typedef unsigned __int64    SESSION_CODE;
    typedef unsigned short      PACKET_PROTOCOL;

    enum class COMMON
    {
        BUF_16 = 16,
        BUF_32 = 32,
        BUF_64 = 64,
        BUF_128 = 128,
        BUF_256 = 256,
        BUF_512 = 512,
        BUF_1024 = 1024,
        BUF_2048 = 2048,
        BUF_4056 = 4056,
        BUF_8112 = 8112,
        BUF_16224 = 16224
    };

    enum class THREAD_TYPE
    {
        THREAD_MAIN = 0,
        THREAD_LAN = 1,
        THREAD_NET = 2,
        THREAD_ACCEPT = 3,
        THREAD_TIMER = 4,

        THREAD_TYPE_TOTAL
    };

    enum class PQCS_TYPE
    {
        CLOSE_THREAD    = 100001,
        RECVPOST        = 100002,
        SENDPOST        = 100003,
        JOIN            = 100004
    };

    enum class LOCK_TYPE
    {
        CRITICALSECTION = 0,
        SRWLOCK_SHARED = 1,
        SRWLOCK_EXCLUSIVE = 2,
        MUTEX = 3
    };

    enum class DISCONNECT_TYPE 
    { 
        GRACEFUL = 0, 
        FORCED = 1, 
        REUSESOCKET = 2
    };

    enum class LOG_LEVEL
	{
		LEVEL_DEBUG     =	0x00001111,
		LEVEL_WARNING   =	0x00000111,
		LEVEL_ERROR     =	0x00000011,
		LEVEL_SYSTEM    =   0x00000001
	};

	enum class LOG_WRITE_MODE
	{
		CONSOLE     =	0x00000001,
		WRITE_FILE  =	0x00000010,
		WRITE_DB    =	0x00000100
	};

    enum class PACKET
	{
		BUFF_PACKET_1024 = 1024,		// 패킷의 기본 버퍼 사이즈.
		HEADER_DEFAULT_SIZE = 2,
		HEADER_MAX_SIZE = 5,
		PACKET_HEADER_SIZE = 5
	};

    enum class SESSION
	{
		BUFF_SOCKET_64 = 64,
        BUFF_IP_32 = 32
	};

    enum class SOCKET_TYPE
    {
        DEFAULT = -1,
        USER = 0,
    };

    enum class PROFILE
    {
        THREAD_TYPE_COUNT   = 10,       // 스레드 종류 개수 예) Accept, iocp 만 있으면 종류는 2개
        THREAD_COUNT        = 32,       // 스레드 종류 별 스레드 개수
        DATA_SIZE           = 30,
        MINMAX_SIZE         = 3,
    };

    enum class TEXT_PARSER
	{
		BUFF_TEXTPARSER_1000000 = 1000000   // 구역을 읽을 버퍼.
	};

    enum class TIMER_TYPE
	{
        DEFAULT         = -1,

		FLAT_TIMER		= 1,
		INSTANT_TIMER   = 2
	};

    enum class FLAT_TIME
	{
		// 등록 후 아래 정각에 따라 발동 ( 밀리초 )
        DEFAULT         = -1,

        INSTANCE        = 0,
		EVERY_1_SECOND  = 1000,
		EVERY_5_SECOND  = EVERY_1_SECOND * 5,
		EVERY_10_SECOND = EVERY_1_SECOND * 10,
		EVERY_15_SECOND = EVERY_1_SECOND * 15,                   
		EVERY_20_SECOND = EVERY_1_SECOND * 20,
		EVERY_30_SECOND = EVERY_1_SECOND * 30,
                                                   
		EVERY_1_MINUTE  = EVERY_1_SECOND * 60,
		EVERY_5_MINUTE  = EVERY_1_MINUTE * 5,
		EVERY_10_MINUTE = EVERY_1_MINUTE * 10,
		EVERY_15_MINUTE = EVERY_1_MINUTE * 15,
		EVERY_20_MINUTE = EVERY_1_MINUTE * 20,
		EVERY_30_MINUTE = EVERY_1_MINUTE * 30,

		EVERY_1_HOUR    = EVERY_1_MINUTE * 60,
		EVERY_2_HOUR    = EVERY_1_HOUR   * 2,
		EVERY_3_HOUR    = EVERY_1_HOUR   * 3,
		EVERY_4_HOUR    = EVERY_1_HOUR   * 4,
		EVERY_6_HOUR    = EVERY_1_HOUR   * 6,
		EVERY_8_HOUR    = EVERY_1_HOUR   * 8,
		EVERY_12_HOUR   = EVERY_1_HOUR   * 12,
		EVERY_24_HOUR   = EVERY_1_HOUR   * 24,
	};

    enum class RANDOM_DISTRIBUTION_TYPE
    {
        DISCRETE = 0,
        BERNOULLI = 1
    };

    enum class PROTOCOL
    {
        DEFAULT = 0,
    };
}