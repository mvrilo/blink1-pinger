/* blink-pinger.c
 *
 * A simple ping implementation to work together with blink(1)
 * v.0.0.1
 *
 * Author: Murilo Santana <mvrilo@gmail.com>
 * Copyright. All rights reserved.
 *
 *
 * This is an implementation/fork of the iconping from @antirez,
 * using the C api of blink(1) and libev for the timer loop.
 * 
 *
 * iconping
 * https://github.com/antirez/iconping/
 *
 * Created by Salvatore Sanfilippo on 25/07/11.
 * Copyright Salvatore Sanfilippo. All rights reserved.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/time.h>
#include <ev.h>

struct ev_loop *loop;

uint16_t icmp_id;
uint16_t icmp_seq;
int64_t last_received_time;
int last_rtt;
int icmp_socket;
int connection_state;

struct ICMPHeader {
	uint8_t  type;
	uint8_t  code;
	uint16_t checksum;
	uint16_t identifier;
	uint16_t sequenceNumber;
	int64_t  sentTime;
};

#define ICMP_TYPE_ECHO_REPLY 0
#define ICMP_TYPE_ECHO_REQUEST 8

#define CONN_STATE_KO 0
#define CONN_STATE_SLOW 1
#define CONN_STATE_OK 2

/* This is the standard BSD checksum code, modified to use modern types. */
static uint16_t in_cksum(const void *buffer, size_t bufferLen)
{
	size_t              bytesLeft;
	int32_t             sum;
	const uint16_t *    cursor;
	union {
		uint16_t        us;
		uint8_t         uc[2];
	} last;
	uint16_t            answer;

	bytesLeft = bufferLen;
	sum = 0;
	cursor = buffer;

	/*
	 * Our algorithm is simple, using a 32 bit accumulator (sum), we add
	 * sequential 16 bit words to it, and at the end, fold back all the
	 * carry bits from the top 16 bits into the lower 16 bits.
	 */
	while (bytesLeft > 1) {
		sum += *cursor;
		cursor += 1;
		bytesLeft -= 2;
	}
	/* mop up an odd byte, if necessary */
	if (bytesLeft == 1) {
		last.uc[0] = * (const uint8_t *) cursor;
		last.uc[1] = 0;
		sum += last.us;
	}

	/* add back carry outs from top 16 bits to low 16 bits */
	sum = (sum >> 16) + (sum & 0xffff);     /* add hi 16 to low 16 */
	sum += (sum >> 16);                     /* add carry */
	answer = ~sum;                          /* truncate to 16 bits */

	return answer;
}

int setSocketNonBlocking(int fd)
{
	int flags;

	/* Set the socket nonblocking.
	 * Note that fcntl(2) for F_GETFL and F_SETFL can't be
	 * interrupted by a signal. */
	if ((flags = fcntl(fd, F_GETFL)) == -1) return -1;
	if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) return -1;
	return 0;
}

/* Return the UNIX time in microseconds */
int64_t ustime(void)
{
	struct timeval tv;
	long long ust;

	gettimeofday(&tv, NULL);
	ust = ((int64_t)tv.tv_sec)*1000000;
	ust += tv.tv_usec;
	return ust;
}

void changeConnectionState(int state)
{
	if (state == CONN_STATE_KO) {
		//printf("OFF\n");
	} else if (state == CONN_STATE_OK) {
		//printf("ON\n");
	} else if (state == CONN_STATE_SLOW) {
		//printf("SLOW\n");
	}

	connection_state = state;
}

void sendPingwithId()
{
	if (icmp_socket != -1) close(icmp_socket);

	int s = icmp_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_ICMP);

	struct sockaddr_in sa;
	struct ICMPHeader icmp;

	if (s == -1) return;
	inet_aton("4.2.2.2", &sa.sin_addr);
	setSocketNonBlocking(s);

	/* Note that we create always a new socket, with a different identifier
	 * and sequence number. This is to avoid to read old replies to our ICMP
	 * request, and to be sure that even in the case the user changes
	 * connection, routing, interfaces, everything will continue to work. */
	icmp.type = ICMP_TYPE_ECHO_REQUEST;
	icmp.code = 0;
	icmp.identifier = icmp_id;
	icmp.sequenceNumber = icmp_seq;
	icmp.sentTime = ustime();   
	icmp.checksum = in_cksum(&icmp,sizeof(icmp));

	sendto(icmp_socket, (char *)&icmp, sizeof(icmp), 0, (struct sockaddr*)&sa, sizeof(sa));
}

void receivePing()
{
	unsigned char packet[1024*16];
	struct ICMPHeader *reply;
	ssize_t nread = read(icmp_socket, packet, sizeof(packet));
	int icmpoff;

	if (nread <= 0) return;
	//printf("Received ICMP %d bytes\n", (int)nread);

	icmpoff = (packet[0]&0x0f)*4;
	//printf("ICMP offset: %d\n", icmpoff);

	/* Don't process malformed packets. */
	if (nread < (icmpoff + (signed)sizeof(struct ICMPHeader))) return;
	reply = (struct ICMPHeader*) (packet+icmpoff);

	/* Make sure that identifier and sequence match */
	if (reply->identifier != icmp_id || reply->sequenceNumber != icmp_seq) return;

	//printf("OK received an ICMP packet that matches!\n");
	if (reply->sentTime > last_received_time) {
		last_rtt = (int)(ustime()-reply->sentTime)/1000;
		last_received_time = reply->sentTime;
	}
}

void timerHandler(struct ev_loop *loop, ev_timer *w, int revents)
{
	static long clicks = -1;
	int64_t elapsed;
	int state;

	clicks++;
	if ((clicks % 10) == 0) {
		//printf("Sending ping #%lu\n", clicks / 10);
		sendPingwithId();
	}
	receivePing();

	/* Update the current state accordingly */
	elapsed = (ustime() - last_received_time)/1000; /* in milliseconds */

	if (elapsed > 3000) {
		state = CONN_STATE_KO;
	} else if (last_rtt < 300) {
		state = CONN_STATE_OK;
	} else {
		state = CONN_STATE_SLOW;
	}

	if (state == connection_state) return;
	changeConnectionState(state);
}

int main(int argc, char **argv)
{
	/* Run the program as daemon */
	if (argc > 0 && strcmp(argv[1], "--daemon") == 0 || strcmp(argv[1], "-d") == 0) {
		if (fork()) exit(0);

		if (setsid() < 0) {
			perror("setsid()");
			exit(1);
		}
	}

	loop = ev_default_loop(0);
	struct ev_timer mytimer;

	icmp_socket = -1;
	icmp_id = random()&0xffff;
	icmp_seq = random()&0xffff;

	ev_timer_init(&mytimer, timerHandler, 0., .1);

	ev_timer_start(loop, &mytimer);
	ev_loop(loop, 0);

	return 0;
}
