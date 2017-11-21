#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>  
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>  
#include <sys/ioctl.h>

#include <net/if.h>
#if __GLIBC__ >= 2 && __GLIBC_MINOR >= 1
#include <netpacket/packet.h>
#include <net/ethernet.h>     /* the L2 protocols */
#else
#include <asm/types.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>   /* The L2 protocols */
#endif

#include <linux/filter.h>
#include <netinet/in.h>
#include "packet.h"

static int get_iface_index(int fd, const int8_t *device);


int init_packet_capture(struct lib_cap* p)
{
	int sock;
	struct ifreq ethreq;
	struct sock_fprog Filter;

	// tcpdump -dd ether proto 0x8033
	struct sock_filter BPF_code[]= {
		{ 0x28, 0, 0, 0x0000000c },
		{ 0x15, 0, 1, 0x00008033 },
		{ 0x6, 0, 0, 0x00000060 },
		{ 0x6, 0, 0, 0x00000000 }
	};                            

	//init filter settings
	Filter.len = 4;
	Filter.filter = BPF_code;

	//set default value
	p->device = "eth0";
	p->ifindex = -1;
	p->fd = -1;
	p->buffer = NULL;
	p->buf_len= 0;

	if ( (sock=socket(PF_PACKET, SOCK_RAW, htons(0x8033)))<0) {
		return -1;
	}

	/* Set the network card in promiscuos mode */
	bzero(&ethreq,sizeof(struct ifreq));
	strncpy(ethreq.ifr_name,"eth0",IFNAMSIZ);
	if (ioctl(sock,SIOCGIFFLAGS,&ethreq)==-1) {
		close(sock);
		return -1;
	}
	
	ethreq.ifr_flags|=IFF_PROMISC;
	if (ioctl(sock,SIOCSIFFLAGS,&ethreq)==-1) {
		close(sock);
		return -1;
	}
	
	/* Attach the filter to the socket */
	if(setsockopt(sock, SOL_SOCKET, SO_ATTACH_FILTER, &Filter, sizeof(Filter))<0){
		close(sock);
		return -1;
	}

	p->ifindex = get_iface_index(sock,p->device);
	if ( p->ifindex == -1 ) {
		close(sock);
		return -1;
	}

	p->fd = sock;
	p->buffer = malloc(1024);
	p->buf_len= 1024;

	return 0;
}

#if 1   // TS_UPGRADE
int init_packet_capture2(struct lib_cap* p, char *ifName)
{
    int sock;
    struct ifreq ethreq;
    struct sock_fprog Filter;

    // tcpdump -dd ether proto 0x8033
    struct sock_filter BPF_code[]= {
        { 0x28, 0, 0, 0x0000000c },
        { 0x15, 0, 1, 0x00008033 },
        { 0x6, 0, 0, 0x00000060 },
        { 0x6, 0, 0, 0x00000000 }
    };

    //init filter settings
    Filter.len = 4;
    Filter.filter = BPF_code;

    //set default value
    p->device = ifName;
    p->ifindex = -1;
    p->fd = -1;
    p->buffer = NULL;
    p->buf_len= 0;

    if ( (sock=socket(PF_PACKET, SOCK_RAW, htons(0x8033)))<0) {
        return -1;
    }

    /* Set the network card in promiscuos mode */
    bzero(&ethreq,sizeof(struct ifreq));
    strncpy(ethreq.ifr_name, ifName, IFNAMSIZ);
    if (ioctl(sock,SIOCGIFFLAGS,&ethreq)==-1) {
        close(sock);
        return -1;
    }

    ethreq.ifr_flags|=IFF_PROMISC;
    if (ioctl(sock,SIOCSIFFLAGS,&ethreq)==-1) {
        close(sock);
        return -1;
    }

    /* Attach the filter to the socket */
    if(setsockopt(sock, SOL_SOCKET, SO_ATTACH_FILTER, &Filter, sizeof(Filter))<0){
        close(sock);
        return -1;
    }

    p->ifindex = get_iface_index(sock,p->device);
    if ( p->ifindex == -1 ) {
        close(sock);
        return -1;
    }

    p->fd = sock;
    p->buffer = malloc(1024);
    p->buf_len= 1024;

    return 0;
}
#endif

int fini_packet_capture(struct lib_cap* p)
{
	if ( p->fd != -1 ) {
		close(p->fd);
		p->fd = -1;
	}

	if ( p->buffer ) {
		free( p->buffer );
		p->buffer = NULL;
		p->buf_len = 0;
	}

	return 0;
}

int get_capture_packet(struct lib_cap* p,u_int8_t** packet)
{
	int n;
	n = recvfrom(p->fd,p->buffer,p->buf_len,0,NULL,NULL);
	if ( n < 0 )
		*packet = NULL;
	else
		*packet = p->buffer;

	return n;
} 


int send_ether_packet(struct lib_cap* p, u_int8_t* packet,u_int32_t len)
{
	int c;
	struct sockaddr_ll sa;

	memset(&sa, 0, sizeof(sa));
	sa.sll_family = AF_PACKET;
	sa.sll_ifindex = p->ifindex; 
	sa.sll_protocol = htons(ETH_P_ALL);

	c = sendto(p->fd, packet, len, 0, (struct sockaddr *)&sa, sizeof(sa));

	return c;
}

static int get_iface_index(int fd, const int8_t *device)
{
    struct ifreq ifr;
 
    /* memset(&ifr, 0, sizeof(ifr)); */
    strncpy (ifr.ifr_name, device, sizeof(ifr.ifr_name) - 1);
    ifr.ifr_name[sizeof(ifr.ifr_name)-1] = '\0';
 
    if (ioctl(fd, SIOCGIFINDEX, &ifr) == -1) {
        return (-1);
    }
 
    return ifr.ifr_ifindex;
}


