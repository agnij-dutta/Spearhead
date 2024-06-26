#ifndef __COMPAT_H
#define __COMPAT_H
 
// LINUX & SOLARIS include files
#if defined LINUX || defined SOLARIS
#include "function_strings.h"
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/types.h>
#include <inttypes.h>
#include <signal.h>
 
#define closesocket(x) close(x)
#define Sleep(x) sleep(x/1000)
#define sprintf_s(x, ...) snprintf(x, __VA_ARGS__)
#define SOCKET_ERROR -1
#define INVALID_SOCKET ~0
#define USHORT	unsigned short
 
// LINUX-only include files
#if defined LINUX
#include <getopt.h>
#endif
 
// WIN32 include files
#elif WIN32
 
#include <winsock2.h>
#include <windows.h>
#include <Pdh.h>
#include <IPHlpApi.h>
#include <process.h>
#include "stdint.h"
 
#define sleep(x) Sleep(x)
//#define _LITTLE_ENDIAN
#define unlink(x) _unlink(x)
#define fileno(x) _fileno(x)
 
#endif	// OS SPECIFIC
 
// COMMON include files
 
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
 
#define UPTIME_STR_LEN 256
#define DEFAULT_INITIAL_DELAY	3 * 60 * 1000	// 3 minutes
#define DEFAULT_BEACON_PORT		443				// TCP port 443 (HTTPS)
#define DEFAULT_BEACON_INTERVAL	0				// operators did not want a default beacon interval
#define DEFAULT_TRIGGER_DELAY	60 * 1000		// 60 seconds
#define DEFAULT_BEACON_JITTER	3			//integer for percentage of variance [0-30] range
 
#define SELF_DEL_TIMEOUT 60 * 60 * 24 * 60 	//60 secs * 60 mins * 24 hours * 60 days   ( No *1000 is used )
//#define SELF_DEL_TIMEOUT 60 * 3
 
#ifndef SUCCESS
#define SUCCESS 0
#endif
 
#ifndef SHUTDOWN
#define SHUTDOWN 2
#endif
 
#ifndef FAILURE
#define FAILURE -1
#endif
 
#define MAX(a,b)	( ((a)>(b)) ? (a) : (b) )
#define MIN(a,b)	( ((a)>(b)) ? (b) : (a) )
 
#if defined SOLARIS || defined WIN32
char exe_path[256];
#endif
///////////////////////////////
 
//#define _CRTDBG_MAP_ALLOC //IAN DELETE LATER IAN COMMENT OR DELETE
//#include <crtdbg.h> //IAN DELETE LATER IAN COMMENT OR DELETE
 
#endif //__COMPAT_H
 