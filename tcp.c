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

int axel_gethostbyname(const char *hostname, struct hostent *hostinfo);

/* Get a TCP connection */
int tcp_connect( char *hostname, int port, char *local_if )
{
	struct hostent host[1];
	memset(host, 0, sizeof(struct hostent));
	//struct hostent *host = NULL;
	struct sockaddr_in addr;
	struct sockaddr_in local;
	int fd;

#ifdef DEBUG
	socklen_t i = sizeof( local );
	
	fprintf( stderr, "tcp_connect( %s, %i ) = ", hostname, port );
#endif
	
	/* Why this loop? Because the call might return an empty record.
	   At least it very rarely does, on my system...		*/
	for( fd = 0; fd < 5; fd ++ )
	{
		// gethostbyname is obsolete, we use gethostbyname_r instead
		if( axel_gethostbyname( hostname, host ) )
			return( -1 );
		if( *host->h_name ) break;
	}
	if( !host->h_name || !*host->h_name )
		return( -1 );
	
	if( ( fd = socket( AF_INET, SOCK_STREAM, 0 ) ) == -1 )
	{
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
			return( -1 );
		}
	}
	
	addr.sin_family = AF_INET;
	addr.sin_port = htons( port );
	addr.sin_addr = *( (struct in_addr *) host->h_addr );

	if( connect( fd, (struct sockaddr *) &addr, sizeof( struct sockaddr_in ) ) == -1 )
	{
		close( fd );
		return( -1 );
	}
	
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

int axel_gethostbyname(const char *hostname, struct hostent *hostinfo )
{
	char    buf[1024];
	struct  hostent  *phost;
	int     ret;

	if( !hostinfo )
		return( 1 );

	if( gethostbyname_r(hostname, hostinfo, buf, sizeof(buf), &phost, &ret) ) {
		fprintf( stderr, "ERROR : gethostbyname_r(%s) returns %d\n", hostname, ret);
		return( 1 );
	}
/*
	else {
		printf("SUCCESS : gethostbyname_r(%s) returns %d,", hostname , ret);
		if(phost) {
			printf("[hostent] name:%s,addrtype:%d(AF_INET:%d),len:%d\n",
					phost->h_name,phost->h_addrtype,AF_INET,phost->h_length);
		}
		int i;
		for(i = 0;hostinfo->h_aliases[i];i++) {
			printf("hostinfo alias%d is:%s\n", i, hostinfo->h_aliases[i]);
		}
		for(i = 0;hostinfo->h_addr_list[i];i++) {
			printf("host addr%d is:%s\n", i, inet_ntoa(*(struct in_addr*)hostinfo->h_addr_list[i]));
		}
	}
*/

	return( 0 );
}

