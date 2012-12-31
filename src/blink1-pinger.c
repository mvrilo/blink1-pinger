/* blink-pinger.c
 *
 * A simple ping implementation working with Blink1
 * v.0.0.1
 *
 * Author: Murilo Santana <mvrilo@gmail.com>
 * Copyright. All rights reserved.
 *
 *
 * Lot of the work here was heavily based on myping.c and iconping.
 * Licenses and links below. Thanks for the authors and contributors.
 *
 *
 *
 * myping.c
 * http://www.cs.utah.edu/~swalton/listings/sockets/programs/part4/chap18/myping.c
 *
 * Copyright (c) 2000 Sean Walton and Macmillan Publishers.  Use may be in
 * whole or in part in accordance to the General Public License (GPL).
 *
 *
 *
 * iconping
 * https://github.com/antirez/iconping/
 *
 * Created by Salvatore Sanfilippo on 25/07/11.
 * Copyright Salvatore Sanfilippo. All rights reserved.
 *
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/socket.h>
#include <resolv.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>

#include "lib/blink1-lib.h"

#define MS 40
#define PACKETSIZE 64
#define ICMP_TYPE_ECHO_REPLY 0
#define ICMP_TYPE_ECHO_REQUEST 8

struct icmp {
	uint8_t  type;
	uint8_t  code;
	uint16_t checksum;
	uint16_t id;
	uint16_t sequence;
	char msg[PACKETSIZE-sizeof(struct icmp *)];
};

int pid = -1, fpid = -1;
hid_device* blink;

void blink_off()
{
	blink1_fadeToRGB(blink, MS, 0, 0, 0);
	blink1_close(blink);
	exit(0);
}

unsigned short checksum(void *b, int len)
{	
	unsigned short *buf = b;
	unsigned int sum=0;
	unsigned short result;

	for ( sum = 0; len > 1; len -= 2 )
		sum += *buf++;

	if ( len == 1 )
		sum += *(unsigned char*)buf;

	sum = (sum >> 16) + (sum & 0xFFFF);
	sum += (sum >> 16);
	result = ~sum;
	return result;
}

void listener(void)
{
	int sd;
	struct sockaddr_in addr;
	unsigned char buf[1024];

	sd = socket(PF_INET, SOCK_RAW, IPPROTO_ICMP);
	if ( sd < 0 ) {
		perror("socket");
	}

	for (;;) {
		int bytes;
		socklen_t len = sizeof(addr);
		bzero(buf, sizeof(buf));
		bytes = recvfrom(sd, buf, sizeof(buf), 0, (struct sockaddr*)&addr, &len);
	}

	blink_off();
}

void ping(struct sockaddr_in *addr)
{
	int i, sd, cnt = 1;
	struct icmp icmp;

	sd = socket(PF_INET, SOCK_RAW, IPPROTO_ICMP);
	if ( sd < 0 ) {
		perror("socket");
		return;
	}

	if ( fcntl(sd, F_SETFL, O_NONBLOCK) != 0 )
		perror("Request nonblocking I/O");

	for (;;) {
		bzero(&icmp, sizeof(icmp));
		icmp.id = pid;
		icmp.code = 0;
		icmp.type = ICMP_TYPE_ECHO_REQUEST;
		icmp.sequence = cnt++;
		icmp.checksum = checksum(&icmp, sizeof(icmp));

		for ( i = 0; i < sizeof(icmp.msg)-1; i++ )
			icmp.msg[i] = i + '0';

		icmp.msg[i] = 0;

		if ( sendto(sd, &icmp, sizeof(icmp), 0, (struct sockaddr *)addr, sizeof(addr)) <= 0 ) {
			blink1_fadeToRGB(blink, MS, 200, 0, 0);
			puts("sendto: fail");
		}
		else {
			blink1_fadeToRGB(blink, MS, 0, 0, 0);
			puts("sendto: success");
		}

		sleep(1);
	}
}

void trap(int s)
{
	kill(fpid, SIGTERM);
	blink_off();
}

int main(void)
{
	blink = blink1_open();
	if (!blink) {
		puts("no blink(1) devices found");
		exit(0);
	}

	pid = getpid();
	struct hostent *hname = gethostbyname("4.2.2.2");
	struct sockaddr_in addr;

	signal(SIGINT, &trap);

	bzero(&addr, sizeof(addr));
	addr.sin_family = hname->h_addrtype;
	addr.sin_port = 0;
	addr.sin_addr.s_addr = *(long*)hname->h_addr;

	fpid = fork();
	if ( fpid == 0 )
		listener();
	else
		ping(&addr);

	wait(0);
	return 0;
}
