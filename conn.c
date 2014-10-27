  /********************************************************************\
  * Axel -- A lighter download accelerator for Linux and other Unices. *
  *                                                                    *
  * Copyright 2001 Wilmer van der Gaast                                *
  \********************************************************************/

/* Connection stuff							*/

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

static int host2addr(const char *host, struct in_addr *addr, char *buff);
static int dns_resolve(axel_t *axel, int count, void *url);

char string[MAX_STRING];

/* Convert an URL to a conn_t structure					*/
int conn_set( conn_t *conn, char *set_url )
{
	char url[MAX_STRING];
	char *i, *j;
	
	// http://www.ifeng.com/video/path/to/file.mp4?unlimit=1
	/* protocol://							*/
	// 如果不带协议，默认是FTP，改成HTTP
	if( ( i = strstr( set_url, "://" ) ) == NULL )
	{
		conn->proto = PROTO_DEFAULT;
		strncpy( url, set_url, MAX_STRING );
	}
	else
	{
		if( set_url[0] == 'f' )
			conn->proto = PROTO_FTP;
		else if( set_url[0] == 'h' )
			conn->proto = PROTO_HTTP; //proto : http
		else
		{
			return( 0 );// 不支持其他协议
		}
		strncpy( url, i + 3, MAX_STRING );
	}
	
	// www.ifeng.com/video/path/to/file.mp4?unlimit=1
	/* Split							*/
	if( ( i = strchr( url, '/' ) ) == NULL )
	{
		strcpy( conn->dir, "/" );
	}
	else
	{
		*i = 0;
		snprintf( conn->dir, MAX_STRING, "/%s", i + 1 ); //dir
		if( conn->proto == PROTO_HTTP )
			http_encode( conn->dir );
	}
	strncpy( conn->host, url, MAX_STRING ); //host : www.ifeng.com
	j = strchr( conn->dir, '?' );
	if( j != NULL )
		*j = 0;
	i = strrchr( conn->dir, '/' );
	*i = 0;
	if( j != NULL )
		*j = '?';
	if( i == NULL )
	{
		strncpy( conn->file, conn->dir, MAX_STRING );
		strcpy( conn->dir, "/" );
	}
	else
	{
		strncpy( conn->file, i + 1, MAX_STRING ); // file : file.mp4?unlimit=1
		strcat( conn->dir, "/" ); // dir : /video/path/to/
	}
	
	/* Check for username in host field				*/
	if( strrchr( conn->host, '@' ) != NULL )
	{
		strncpy( conn->user, conn->host, MAX_STRING );
		i = strrchr( conn->user, '@' );
		*i = 0;
		strncpy( conn->host, i + 1, MAX_STRING );
		*conn->pass = 0;
	}
	/* If not: Fill in defaults					*/
	else
	{
		if( conn->proto == PROTO_FTP )
		{
			/* Dash the password: Save traffic by trying
			   to avoid multi-line responses		*/
			strcpy( conn->user, "anonymous" );
			strcpy( conn->pass, "mailto:axel-devel@lists.alioth.debian.org" );
		}
		else
		{
			*conn->user = *conn->pass = 0;
		}
	}
	
	/* Password?							*/
	if( ( i = strchr( conn->user, ':' ) ) != NULL )
	{
		*i = 0;
		strncpy( conn->pass, i + 1, MAX_STRING );
	}
	/* Port number?							*/
	if( ( i = strchr( conn->host, ':' ) ) != NULL )
	{
		*i = 0;
		sscanf( i + 1, "%i", &conn->port );
	}
	/* Take default port numbers from /etc/services			*/
	else
	{
/*
#ifndef DARWIN
		struct servent *serv;
		
		if( conn->proto == PROTO_FTP )
			serv = getservbyname( "ftp", "tcp" );
		else
			serv = getservbyname( "www", "tcp" );
		
		if( serv )
			conn->port = ntohs( serv->s_port );
		else
#endif
*/
		if( conn->proto == PROTO_FTP )
			conn->port = 21;
		else
			conn->port = 80;
	}
	
	return( conn->port > 0 );
}

/* Generate a nice URL string.						*/
char *conn_url( conn_t *conn )
{
	if( conn->proto == PROTO_FTP )
		strcpy( string, "ftp://" );
	else
		strcpy( string, "http://" );
	
	if( *conn->user != 0 && strcmp( conn->user, "anonymous" ) != 0 )
		sprintf( string + strlen( string ), "%s:%s@",
			conn->user, conn->pass );

	sprintf( string + strlen( string ), "%s:%i%s%s",
		conn->host, conn->port, conn->dir, conn->file );
	
	return( string );
}

/* Simple...								*/
void conn_disconnect( conn_t *conn )
{
	if( conn->proto == PROTO_FTP && !conn->proxy )
		ftp_disconnect( conn->ftp );
	else
		http_disconnect( conn->http );
	conn->fd = -1;
}

int conn_init( conn_t *conn )
{
	char *proxy = conn->conf->http_proxy, *host = conn->conf->no_proxy;
	int i;
	
	if( *conn->conf->http_proxy == 0 )
	{
		proxy = NULL;
	}
	else if( *conn->conf->no_proxy != 0 )
	{
		for( i = 0; ; i ++ )
			if( conn->conf->no_proxy[i] == 0 )
			{
				if( strstr( conn->host, host ) != NULL )
					proxy = NULL;
				host = &conn->conf->no_proxy[i+1];
				if( conn->conf->no_proxy[i+1] == 0 )
					break;
			}
	}
	
	conn->proxy = proxy != NULL;
	
	if( conn->proto == PROTO_FTP && !conn->proxy )
	{
		conn->ftp->local_if = conn->local_if;
		conn->ftp->ftp_mode = FTP_PASSIVE;
		if( !ftp_connect( conn->ftp, conn->host, conn->port, conn->user, conn->pass ) )
		{
			conn->message = conn->ftp->message;
			conn_disconnect( conn );
			return( 0 );
		}
		conn->message = conn->ftp->message;
		if( !ftp_cwd( conn->ftp, conn->dir ) )
		{
			conn_disconnect( conn );
			return( 0 );
		}
	}
	else
	{
		conn->http->local_if = conn->local_if;
		if( !http_connect( conn->http, conn->proto, proxy, conn->host, conn->port, conn->user, conn->pass ) )
		{
			conn->message = conn->http->headers;
			conn_disconnect( conn );
			return( 0 );
		}
		conn->message = conn->http->headers;
		conn->fd = conn->http->fd;
	}
	return( 1 );
}

int conn_setup( conn_t *conn )
{
	if( conn->ftp->fd <= 0 && conn->http->fd <= 0 )
		if( !conn_init( conn ) )
			return( 0 );
	
	if( conn->proto == PROTO_FTP && !conn->proxy )
	{
		if( !ftp_data( conn->ftp ) )	/* Set up data connnection	*/
			return( 0 );
		conn->fd = conn->ftp->data_fd;
		
		if( conn->currentbyte )
		{
			ftp_command( conn->ftp, "REST %lld", conn->currentbyte );
			if( ftp_wait( conn->ftp ) / 100 != 3 &&
			    conn->ftp->status / 100 != 2 )
				return( 0 );
		}
	}
	else
	{
		char s[MAX_STRING];
		int i;

		snprintf( s, MAX_STRING, "%s%s", conn->dir, conn->file );
		conn->http->firstbyte = conn->currentbyte;
		conn->http->lastbyte = conn->lastbyte;
		http_get( conn->http, s );
		http_addheader( conn->http, "User-Agent: %s", conn->conf->user_agent );
		for( i = 0; i < conn->conf->add_header_count; i++)
			http_addheader( conn->http, "%s", conn->conf->add_header[i] );
	}
	return( 1 );
}

int conn_exec( conn_t *conn )
{
	if( conn->proto == PROTO_FTP && !conn->proxy )
	{
		if( !ftp_command( conn->ftp, "RETR %s", conn->file ) )
			return( 0 );
		return( ftp_wait( conn->ftp ) / 100 == 1 );
	}
	else
	{
		if( !http_exec( conn->http ) )
			return( 0 );
		return( conn->http->status / 100 == 2 );
	}
}

/* Get file size and other information					*/
int conn_info( conn_t *conn )
{
	/* It's all a bit messed up.. But it works.			*/
	if( conn->proto == PROTO_FTP && !conn->proxy )
	{
		ftp_command( conn->ftp, "REST %lld", 1 );
		if( ftp_wait( conn->ftp ) / 100 == 3 ||
		    conn->ftp->status / 100 == 2 )
		{
			conn->supported = 1;
			ftp_command( conn->ftp, "REST %lld", 0 );
			ftp_wait( conn->ftp );
		}
		else
		{
			conn->supported = 0;
		}
		
		if( !ftp_cwd( conn->ftp, conn->dir ) )
			return( 0 );
		conn->size = ftp_size( conn->ftp, conn->file, MAX_REDIR );
		if( conn->size < 0 )
			conn->supported = 0;
		if( conn->size == -1 )
			return( 0 );
		else if( conn->size == -2 )
			conn->size = INT_MAX;
	}
	else
	{
		char s[MAX_STRING], *t;
		long long int i = 0;
		
		// redirections
		do
		{
			conn->currentbyte = 1;
			if( !conn_setup( conn ) )
				return( 0 );
			conn_exec( conn );
			conn_disconnect( conn );
			/* Code 3xx == redirect				*/
			if( conn->http->status / 100 != 3 )
				break;
			if( ( t = http_header( conn->http, "location:" ) ) == NULL )
				return( 0 );
			sscanf( t, "%255s", s );
			if( strstr( s, "://" ) == NULL)
			{
				sprintf( conn->http->headers, "%s%s",
					conn_url( conn ), s );
				strncpy( s, conn->http->headers, MAX_STRING );
			}
			else if( s[0] == '/' )
			{
				sprintf( conn->http->headers, "http://%s:%i%s",
					conn->host, conn->port, s );
				strncpy( s, conn->http->headers, MAX_STRING );
			}

			conn_set( conn, s );
			i ++;
			printf("***************** redirect : %s\n", s);
		}
		while( conn->http->status / 100 == 3 && i < MAX_REDIR );
		
		if( i == MAX_REDIR )
		{
			sprintf( conn->message, _("Too many redirects.\n") );
			return( 0 );
		}
		
		conn->size = http_size( conn->http );
		if( conn->http->status == 206 && conn->size >= 0 )
		{
			conn->supported = 1;
			conn->size ++;
		}
		else if( conn->http->status == 200 || conn->http->status == 206 )
		{
			// 在限速的情况下，此处如果supported设为0的话只能起一个线程
			// 为了不限速(添加unlimit参数)，这里supported改为1
			// added by liuyan
			conn->supported = 1;
			//conn->supported = 0;
			//conn->size = INT_MAX;
		}
		else
		{
			t = strchr( conn->message, '\n' );
			if( t == NULL )
				sprintf( conn->message, _("Unknown HTTP error.\n") );
			else
				*t = 0;
			return( 0 );
		}
	}
	
	return( 1 );
}

// 用于域名解析
static int host2addr(const char *host, struct in_addr *addr, char *buff) 
{
	struct hostent he, *result;
	int herr, ret, bufsz = 512;
	// 清空下addr
	//memset( addr, 0, sizeof(struct in_addr) );
	do {
		char *new_buff = (char *)realloc(buff, bufsz);
		if (new_buff == NULL) {
			free(buff);
			return ENOMEM;
		}   
		buff = new_buff;
		ret = gethostbyname_r(host, &he, buff, bufsz, &result, &herr);
		bufsz *= 2;
	} while (ret == ERANGE);

	if (ret == 0 && result != NULL) 
		*addr = *(struct in_addr *)he.h_addr;
	else if (result != &he) 
		ret = herr;
	return ret;
}

// conns的作用是暂存host
// 1, 通过conn_set获取conn->host
// 2, 利用host做DNS解析得到ip
// 3, 将url中的conn->host换成ip
static int dns_resolve(axel_t *axel, int count, void *url)
{
	int i;
	search_t *search;
	conn_t *conns;
	char *buff = NULL;
	struct sockaddr_in addr;

	// 兼容单源下载的情况，因为单源下载时count为0
	// 单源下载：count:0, url:url_t
	// 多源下载：count:>0, url:search_t
	int cnt = count;
	if( cnt == 0 ) {
		cnt = 1;
	} else {
		search = ( search_t *) url;
	}

	conns = malloc( sizeof(conn_t) * cnt );
	for( i = 0; i < cnt; i ++ )
	{
		conn_t *pconn = &conns[i];
		char *myurl;
		if(cnt==1)
			myurl = (char*)url;
		else
			myurl = search[i].url;

		// 解析url
		if( !conn_set( pconn, myurl ) )
		{
			axel_message( axel, _("Could not parse URL[%d].\n"), i );
			free( conns );
			return( -1 );
		}
		// 解析DNS
		if( host2addr( pconn->host, &(addr.sin_addr), buff ) )
		{
			axel_message( axel, _("Could not resove %s URL[%d].\n"), pconn->host, i );
			free(buff);
			free( conns );
			return( -1 );
		}
		// redirect
		if( !conn_info( pconn ) )
		{
			axel_message( axel, pconn->message );
			free(buff);
			free( conns );
			return( -1 );
		}
		// ip替换host
		char *ip = inet_ntoa( addr.sin_addr );
		memset(myurl, 0, strlen(myurl));
		snprintf(myurl, MAX_STRING, "http://%s:%i%s%s", ip, pconn->port, pconn->dir, pconn->file);
		
		//axel_message( axel, _("************** New URL: %s\n"), myurl );
		printf( "************** New URL: %s\n", myurl );
	}
	free(buff);
	free( conns );
	// DNS解析完毕，hu~

	return 0;
}
