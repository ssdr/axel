  /********************************************************************\
  * Axel -- A lighter download accelerator for Linux and other Unices. *
  *                                                                    *
  * Copyright 2001 Wilmer van der Gaast                                *
  \********************************************************************/

/* Text interface							*/

/*
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License with
  the Debian GNU/Linux distribution in file /usr/doc/copyright/GPL;
  if not, write to the Free Software Foundation, Inc., 59 Temple Place,
  Suite 330, Boston, MA  02111-1307  USA
*/
#include "axel.h"

static void stop( int signal );
static char *size_human( long long int value );
static char *time_human( int value );
static void print_alternate_output( axel_t *axel );
static void print_help();
static void print_version();
static void print_messages( axel_t *axel );

int run = 1;

#ifdef NOGETOPTLONG
#define getopt_long( a, b, c, d, e ) getopt( a, b, c )
#else
static struct option axel_options[] =
{
	/* name			has_arg	flag	val */
	{ "max-speed",		1,	NULL,	's' },
	{ "num-connections",	1,	NULL,	'n' },
	{ "output",		1,	NULL,	'o' },
	{ "search",		2,	NULL,	'S' },
	{ "no-proxy",		0,	NULL,	'N' },
	{ "quiet",		0,	NULL,	'q' },
	{ "verbose",		0,	NULL,	'v' },
	{ "help",		0,	NULL,	'h' },
	{ "version",		0,	NULL,	'V' },
	{ "alternate",		0,	NULL,	'a' },
	{ "max-time",	1,	NULL,	'm'	},	// add by liuyan
	{ "header",		1,	NULL,	'H' },
	{ "user-agent",		1,	NULL,	'U' },
	{ NULL,			0,	NULL,	0 }
};
#endif

/* For returning string values from functions				*/
static char string[MAX_STRING];

/*
 * main process:
 * axel_new()
 * axel_open()
 * axel_start() : setup_thread()
 * signal SIGINT/SIGTERM
 * while{ axel_do()}
 */
int main( int argc, char *argv[] )
{
	char fn[MAX_STRING] = "";
	int do_search = 0;
	search_t *search;
	conf_t conf[1];
	axel_t *axel;
	int i, j, cur_head = 0;
	char *s;
	
#ifdef I18N
	setlocale( LC_ALL, "" );
	bindtextdomain( PACKAGE, LOCALE );
	textdomain( PACKAGE );
#endif
	
	if( !conf_init( conf ) )
	{
		return( 1 );
	}
	
	opterr = 0;
	
	j = -1;
	while( 1 )
	{
		int option;
		
		option = getopt_long( argc, argv, "s:n:o:S::NqvhVam:H:U:", axel_options, NULL );
		if( option == -1 )
			break;
		
		switch( option )
		{
		case 'U':
			strncpy( conf->user_agent, optarg, MAX_STRING);
			break;
		case 'H':
			strncpy( conf->add_header[cur_head++], optarg, MAX_STRING );
			break;
		case 'm': // add by liuyan
			if( !sscanf( optarg, "%i", &conf->max_time ) )
			{
				print_help();
				return( 1 );
			}
			break;
		case 's':
			if( !sscanf( optarg, "%i", &conf->max_speed ) )
			{
				print_help();
				return( 1 );
			}
			break;
		case 'n':
			if( !sscanf( optarg, "%i", &conf->num_connections ) )
			{
				print_help();
				return( 1 );
			}
			break;
		case 'o':
			strncpy( fn, optarg, MAX_STRING );
			break;
		case 'S':
			do_search = 1;
			if( optarg != NULL )
			if( !sscanf( optarg, "%i", &conf->search_top ) )
			{
				print_help();
				return( 1 );
			}
			break;
		case 'a':
			conf->alternate_output = 1;
			break;
		case 'N':
			*conf->http_proxy = 0;
			break;
		case 'h':
			print_help();
			return( 0 );
		case 'v':
			if( j == -1 )
				j = 1;
			else
				j ++;
			break;
		case 'V':
			print_version();
			return( 0 );
		case 'q':
			close( 1 );
			conf->verbose = -1;
			if( open( "/dev/null", O_WRONLY ) != 1 )
			{
				fprintf( stderr, _("Can't redirect stdout to /dev/null.\n") );
				return( 1 );
			}
			break;
		default:
			print_help();
			return( 1 );
		}
	}
	conf->add_header_count = cur_head;
	if( j > -1 )
		conf->verbose = j;
	
	if( argc - optind == 0 )
	{
		print_help();
		return( 1 );
	}
	else if( strcmp( argv[optind], "-" ) == 0 )
	{
		s = malloc( MAX_STRING );
		if (scanf( "%1024[^\n]s", s) != 1) {
			fprintf( stderr, _("Error when trying to read URL (Too long?).\n") );
			return( 1 );
		}
	}
	else
	{
		s = argv[optind];
		if( strlen( s ) > MAX_STRING )
		{
			fprintf( stderr, _("Can't handle URLs of length over %d\n" ), MAX_STRING );
			return( 1 );
		}
	}
	
	// 下载准备
	printf( _("Initializing download: %s\n"), s );
	if( do_search )
	{
		search = malloc( sizeof( search_t ) * ( conf->search_amount + 1 ) );
		memset( search, 0, sizeof( search_t ) * ( conf->search_amount + 1 ) );
		search[0].conf = conf;
		if( conf->verbose )
			printf( _("Doing search...\n") );
		// 查询filesearching.com，获取多个url
		i = search_makelist( search, s );
		if( i < 0 )
		{
			fprintf( stderr, _("File not found\n" ) );
			return( 1 );
		}
		if( conf->verbose )
			printf( _("Testing speeds, this can take a while...\n") );
		// 测速
		j = search_getspeeds( search, i );
		search_sortlist( search, i );
		if( conf->verbose )
		{
			printf( _("%i usable servers found, will use these URLs:\n"), j );
			j = min( j, conf->search_top );
			printf( "%-60s %15s\n", "URL", "Speed" );
			for( i = 0; i < j; i ++ )
				printf( "%-70.70s %5i\n", search[i].url, search[i].speed );
			printf( "\n" );
		}
		// 搜索下载模式
		axel = axel_new( conf, j, search );
		free( search );
		if( axel->ready == -1 )
		{
			print_messages( axel );
			axel_close( axel );
			return( 1 );
		}
	}
	else if( argc - optind == 1 )
	{
		// 单源直接下载
		axel = axel_new( conf, 0, s ); //s is url_t( i.e. char* )
		if( axel->ready == -1 )
		{
			print_messages( axel );
			axel_close( axel );
			return( 1 );
		}
	}
	else
	{
		// 多源直接下载
		search = malloc( sizeof( search_t ) * ( argc - optind ) );
		memset( search, 0, sizeof( search_t ) * ( argc - optind ) );
		for( i = 0; i < ( argc - optind ); i ++ )
			strncpy( search[i].url, argv[optind+i], MAX_STRING );
		axel = axel_new( conf, argc - optind, search ); // search is search_t
		free( search );
		if( axel->ready == -1 )
		{
			print_messages( axel );
			axel_close( axel );
			return( 1 );
		}
	}
	print_messages( axel );
	if( s != argv[optind] )
	{
		free( s );
	}
	
	if( *fn )
	{
		struct stat buf;
		
		if( stat( fn, &buf ) == 0 )
		{
			if( S_ISDIR( buf.st_mode ) )
			{
				size_t fnlen = strlen(fn);
				size_t axelfnlen = strlen(axel->filename);
				
				if (fnlen + 1 + axelfnlen + 1 > MAX_STRING) {
					fprintf( stderr, _("Filename too long!\n"));
					return ( 1 );
				}
				
				fn[fnlen] = '/';
				memcpy(fn+fnlen+1, axel->filename, axelfnlen);
				fn[fnlen + 1 + axelfnlen] = '\0';
			}
		}
		sprintf( string, "%s.st", fn );
		if( access( fn, F_OK ) == 0 ) if( access( string, F_OK ) != 0 )
		{
			fprintf( stderr, _("No state file, cannot resume!\n") );
			return( 1 );
		}
		if( access( string, F_OK ) == 0 ) if( access( fn, F_OK ) != 0 )
		{
			printf( _("State file found, but no downloaded data. Starting from scratch.\n" ) );
			unlink( string );
		}
		strcpy( axel->filename, fn );
	}
	else
	{
		/* Local file existence check					*/
		i = 0;
		s = axel->filename + strlen( axel->filename );
		while( 1 )
		{
			sprintf( string, "%s.st", axel->filename );
			if( access( axel->filename, F_OK ) == 0 )
			{
				if( axel->conn[0].supported )
				{
					if( access( string, F_OK ) == 0 )
						break;
				}
			}
			else
			{
				if( access( string, F_OK ) )
					break;
			}
			sprintf( s, ".%i", i );
			i ++;
		}
	}
		
	// 打开文件（状态文件、数据文件）
	if( !axel_open( axel ) )
	{
		print_messages( axel );
		return( 1 );
	}

	print_messages( axel );
	axel_start( axel );//
	print_messages( axel );

	if( conf->alternate_output )
	{
		putchar('\n');
	} 
	axel->start_byte = axel->bytes_done;
	
	/* Install save_state signal handler for resuming support	*/
	signal( SIGINT, stop );
	signal( SIGTERM, stop );
	
	while( !axel->ready && run )
	{
		// 判断超时
		if( gettime() - axel->start_time > conf->max_time )
		{
			printf(_("\nTime's up (max_time=%ds) ! Download Incomplete!\n"), conf->max_time);
			axel->ready = 0;
			run = 0;
			break;
		}

		long long int prev;
		prev = axel->bytes_done;

		axel_do( axel ); // 真正下载数据的地方
		
		if( conf->alternate_output )
		{			
			if( !axel->message && prev != axel->bytes_done )
				print_alternate_output( axel );
		}
		
		if( axel->message )
		{
			if(conf->alternate_output==1)
			{
				putchar( '\r' );
				for( i = 0; i < 79; i++ ) 
					putchar( ' ' );
				putchar( '\r' );
			}
			else
			{
				putchar( '\n' );
			}
			print_messages( axel );
		}
		else if( axel->ready )
		{
			putchar( '\n' );
		}
	}
	
	strcpy( string + MAX_STRING / 2,
		size_human( axel->bytes_done - axel->start_byte ) );
	if( axel->ready && run )
	{
		printf( _("\nSuccessfully Downloaded %s in %s. (%.2f KB/s)\n"),
			string + MAX_STRING / 2,
			time_human( gettime() - axel->start_time ),
			(double) axel->bytes_per_second / 1024 );
	}
	else
	{
		printf(_("\nDownload Failed.\n"));
	}
		
	i = axel->ready ? 0 : 2;
	
	axel_close( axel );
	
	return( i );
}

/* SIGINT/SIGTERM handler						*/
void stop( int signal )
{
	printf(_("catch a signal!\n"));
	run = 0;
}

/* Convert a number of bytes to a human-readable form			*/
char *size_human( long long int value )
{
	if( value == 1 )
		sprintf( string, _("%lld byte"), value );
	else if( value < 1024 )
		sprintf( string, _("%lld bytes"), value );
	else if( value < 10485760 )
		sprintf( string, _("%.1f kilobytes"), (float) value / 1024 );
	else
		sprintf( string, _("%.1f megabytes"), (float) value / 1048576 );
	
	return( string );
}

/* Convert a number of seconds to a human-readable form			*/
char *time_human( int value )
{
	if( value == 1 )
		sprintf( string, _("%i second"), value );
	else if( value < 60 )
		sprintf( string, _("%i seconds"), value );
	else if( value < 3600 )
		sprintf( string, _("%i:%02i seconds"), value / 60, value % 60 );
	else
		sprintf( string, _("%i:%02i:%02i seconds"), value / 3600, ( value / 60 ) % 60, value % 60 );
	
	return( string );
}

static void print_alternate_output(axel_t *axel) 
{
	long long int done=axel->bytes_done;
	long long int total=axel->size;
	int i,j=0;
	double now = gettime();
	
	printf("\r[%3ld%%] [", min(100,(long)(done*100./total+.5) ) );
		
	for(i=0;i<axel->conf->num_connections;i++)
	{
		for(;j<((double)axel->conn[i].currentbyte/(total+1)*50)-1;j++)
			putchar('.');

		if(axel->conn[i].currentbyte<axel->conn[i].lastbyte)
		{
			if(now <= axel->conn[i].last_transfer + axel->conf->connection_timeout/2 )
			{
				// 0-9;A-Z;a-z
				if(i<10)
					putchar(i+'0');//0-9
				else if(i<36)
					putchar(i+7+'0');//A-Z
				else
					putchar(i+13+'0');//a-z...
			}
			else
				putchar('#');
		} else 
			putchar('.');

		j++;
		
		for(;j<((double)axel->conn[i].lastbyte/(total+1)*50);j++)
			putchar(' ');
	}
	
	if(axel->bytes_per_second > 1048576)
		printf( "] [%6.1fMB/s]", (double) axel->bytes_per_second / (1024*1024) );
	else if(axel->bytes_per_second > 1024)
		printf( "] [%6.1fKB/s]", (double) axel->bytes_per_second / 1024 );
	else
		printf( "] [%6.1fB/s]", (double) axel->bytes_per_second );
	
	if(done<total)
	{
		int seconds,minutes,hours,days;
		seconds=axel->finish_time - now;
		minutes=seconds/60;seconds-=minutes*60;
		hours=minutes/60;minutes-=hours*60;
		days=hours/24;hours-=days*24;
		if(days)
			printf(" [%2dd%2d]",days,hours);
		else if(hours)
			printf(" [%2dh%02d]",hours,minutes);
		else
			printf(" [%02d:%02d]",minutes,seconds);
	}
	
	fflush( stdout );
}

void print_help()
{
#ifdef NOGETOPTLONG
	printf(	_("Usage: axel [options] url1 [url2] [url...]\n"
		"\n"
		"-s x\tSpecify maximum speed (bytes per second)\n"
		"-n x\tSpecify maximum number of connections\n"
		"-o f\tSpecify local output file\n"
		"-S [x]\tSearch for mirrors and download from x servers\n"
		"-H x\tAdd header string\n"
		"-U x\tSet user agent\n"
		"-N\tJust don't use any proxy server\n"
		"-q\tLeave stdout alone\n"
		"-v\tMore status information\n"
		"-a\tAlternate progress indicator\n"
		"-m x\tMaximum time in seconds that allows the whole operation to take\n"
		"-h\tThis information\n"
		"-V\tVersion information\n"
		"\n"
		"Visit http://axel.alioth.debian.org/ to report bugs\n") );
#else
	printf(	_("Usage: axel [options] url1 [url2] [url...]\n"
		"\n"
		"--max-speed=x\t\t-s x\tSpecify maximum speed (bytes per second)\n"
		"--num-connections=x\t-n x\tSpecify maximum number of connections\n"
		"--output=f\t\t-o f\tSpecify local output file\n"
		"--search[=x]\t\t-S [x]\tSearch for mirrors and download from x servers\n"
		"--header=x\t\t-H x\tAdd header string\n"
		"--user-agent=x\t\t-U x\tSet user agent\n"
		"--no-proxy\t\t-N\tJust don't use any proxy server\n"
		"--quiet\t\t\t-q\tLeave stdout alone\n"
		"--verbose\t\t-v\tMore status information\n"
		"--alternate\t\t-a\tAlternate progress indicator\n"
		"--max-time=x\t\t-m x\tMaximum time in seconds that allows the whole operation to take\n"
		"--help\t\t\t-h\tThis information\n"
		"--version\t\t-V\tVersion information\n"
		"\n"
		"Visit http://axel.alioth.debian.org/ to report bugs\n") );
#endif
}

void print_version()
{
	printf( "\nAxel v1.1 support :\n"
			"支持多源下载，之前除第一个源之外下载都是失败的:(\n");
	printf( "\nAxel v1.0 support :\n"
		 "1, -m 最大下载时间;\n"
		 "2, 支持unlimit=1请求参数;\n" );

	printf( _("\nAxel version %s (%s)\n"), AXEL_VERSION_STRING, ARCH );
	printf( "Copyright 2014 edit by http://github.com/ssdr\n" );
}

/* Print any message in the axel structure				*/
void print_messages( axel_t *axel )
{
	message_t *m;
	
	while( axel->message )
	{
		printf( "%s\n", axel->message->text );
		m = axel->message;
		axel->message = axel->message->next;
		free( m );
	}
}
