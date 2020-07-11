#ifndef __PLAT_H__
#define __PLAT_H__


#if defined(_WIN32)

	#include <winsock2.h>
	#include <ws2tcpip.h>
	#include <windows.h>
	#include <io.h>
	#include <time.h>
	//#include <Winbase.h>

#elif defined(__unix)

	#include <unistd.h>
	#include <sys/socket.h>
	#include <sys/select.h>
	#include <sys/ioctl.h>
#if !defined(__APPLE__) && !defined(__ARMEL__)
	#include <sys/io.h>
#endif
	#include <sys/types.h>
	#include <netinet/in.h>
	#include <netinet/tcp.h>
	#include <arpa/inet.h>
	#include <netdb.h>

	#if defined(__linux)
		#include <fcntl.h>
		//#include <linux/fs.h>
	#else
		#include <stropts.h>		//Unix
	#endif

	#include <stdint.h>
	#include <poll.h>
	#include <errno.h>

#endif


#if defined(_WIN32)

	typedef __int8 int8_t;
	typedef __int16 int16_t;
	typedef __int32 int32_t;
	typedef __int64 int64_t;

	typedef unsigned __int8 uint8_t;
	typedef unsigned __int16 uint16_t;
	typedef unsigned __int32 uint32_t;
	typedef unsigned __int64 uint64_t;

	typedef int socklen_t;

	#define ERRNO		WSAGetLastError()
	#define NEEDBLOCK	WSAEWOULDBLOCK

#elif defined(__unix)

	#define INVALID_SOCKET	(-1)
	#define SOCKET_ERROR	(-1)
	#define ERRNO			errno
	#define closesocket		close

#endif


#endif
