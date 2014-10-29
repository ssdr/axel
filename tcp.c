/********************************************************************\
 * Axel -- A lighter download accelerator for Linux and other Unices. *
 *                                                                    *
 * Copyright 2001 Wilmer van der Gaast                                *
 \********************************************************************/

/* TCP control file							*/

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

/* Get a TCP connection */
int tcp_connect( char *hostname, int port, char *local_if )
{
	struct sockaddr_in addr;
	struct sockaddr_in local;
	char *buff = NULL;
	int fd;

	if( host2addr( hostname, &(addr.sin_addr), buff ) )
	{
		printf( "Could not resove %s URL.\n", hostname);
		free(buff);
		return( -1 );
	}
	
#ifdef DEBUG
	socklen_t i = sizeof( local );
	fprintf( stderr, "tcp_connect( %s, %i ) = ", hostname, port );
#endif

	if( ( fd = socket( AF_INET, SOCK_STREAM, 0 ) ) == -1 )
	{		
		free(buff);
		return( -1 );
	}

	if( local_if && *local_if )
	{
		local.sin_family = AF_INET;
		local.sin_port = 0;
		local.sin_addr.s_addr = inet_addr( local_if );
		if( bind( fd, (struct sockaddr *) &local, sizeof( struct sockaddr_in ) ) == -1 )
		{
			close( fd );
			free(buff);
			return( -1 );
		}
	}

	addr.sin_family = AF_INET;
	addr.sin_port = htons( port );
	//inet_aton( hostname, &addr.sin_addr );// 不可重入
	//inet_pton( AF_INET, hostname, &addr.sin_addr );// 可重入版本

	if( connect( fd, (struct sockaddr *) &addr, sizeof( struct sockaddr_in ) ) == -1 )
	{
		close( fd );
		free(buff);
		return( -1 );
	}
	// free buffer
	free(buff);

#ifdef DEBUG
	getsockname( fd, &local, &i );
	fprintf( stderr, "%i\n", ntohs( local.sin_port ) );
#endif

	return( fd );
}

int get_if_ip( char *iface, char *ip )
{
	struct ifreq ifr;
	int fd = socket( PF_INET, SOCK_DGRAM, IPPROTO_IP );

	memset( &ifr, 0, sizeof( struct ifreq ) );

	strcpy( ifr.ifr_name, iface );
	ifr.ifr_addr.sa_family = AF_INET;
	if( ioctl( fd, SIOCGIFADDR, &ifr ) == 0 )
	{
		struct sockaddr_in *x = (struct sockaddr_in *) &ifr.ifr_addr;
		strcpy( ip, inet_ntoa( x->sin_addr ) );
		return( 1 );
	}
	else
	{
		return( 0 );
	}
}

// 用于域名解析
static int host2addr(const char *host, struct in_addr *addr, char *buff) 
{
	struct hostent he, *result;
	int herr, ret, bufsz = 512;
	// 清空下addr
	memset( addr, 0, sizeof(struct in_addr) );
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


