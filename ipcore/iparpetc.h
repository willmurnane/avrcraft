//Copyright 2012 Charles Lohr under the MIT/x11, newBSD, LGPL or GPL licenses.  You choose.

#ifndef _IPARPETC_H
#define _IPARPETC_H

#ifndef __ASSEMBLY__
#include <stdint.h>
#endif
#ifdef INCLUDE_TCP
#include "tcp.h"
#endif

#define IP_HEADER_LENGTH 20

//enc28j60 calls this.
void enc28j60_receivecallback( uint16_t packetlen );

struct MAC
{
	union
	{
		uint16_t BytePairs[3];
		uint8_t Address[6];
	};
};
struct IpAddress
{
	union
	{
		uint32_t Address;
		uint8_t Bytes[4];
	};
};


//You must define these.
extern struct IpAddress MyIP;
extern struct IpAddress MyMask;
extern struct MAC MyMAC;
extern struct IpAddress MyGateway;

extern struct MAC macfrom;
extern struct IpAddress ipsource;
extern unsigned short remoteport;
extern unsigned short localport;
#ifdef DO_DNS
extern struct IpAddress dnsserver;
#endif
#ifdef DO_NTP
extern struct IpAddress ntpserver;
#endif

extern unsigned long icmp_in;
extern unsigned long icmp_out;

//Utility, for getting a new scratchpad.
uint16_t NetGetScratch();

//Utility out
void SwitchToBroadcast();
void send_etherlink_header( unsigned short type );
void send_ip_header( unsigned short totallen, const struct IpAddress to, unsigned char proto );
void util_finish_udp_packet();

#define ipsource_uint ((uint32_t*)&ipsource)

#ifdef INCLUDE_TCP
void HandleTCP( uint16_t iptotallen );
#endif

void HandleUDP( uint16_t len );

#ifdef ARP_CLIENT_SUPPORT

//Returns -1 if arp not found yet.
//Otherwise returns entry into ARP table.

int8_t RequestARP( const uint8_t * ip );

struct ARPEntry
{
	uint8_t mac[6];
	uint8_t ip[4];
};

extern uint8_t ClientArpTablePointer;

#ifndef ARP_CLIENT_TABLE_SIZE
#error Define ARP_CLIENT_TABLE_SIZE if using ARP.
#endif
extern struct ARPEntry ClientArpTable[ARP_CLIENT_TABLE_SIZE];


#endif

#ifdef PING_CLIENT_SUPPORT

#ifndef ARP_CLIENT_SUPPORT
#error Client pinging requires ARP Client.
#endif

struct PINGEntries
{
	uint8_t ip[4];
	uint8_t id;  //If zero, not in use.
	uint16_t last_send_seqnum;
	uint16_t last_recv_seqnum;
};

extern struct PINGEntries ClientPingEntries[PING_RESPONSES_SIZE];

int8_t GetPingslot( uint8_t * ip );
void DoPing( uint8_t pingslot );


#endif

#ifdef ENABLE_DHCP_CLIENT

//NOTE: This cannot exceed 255
#ifndef DHCP_TICKS_PER_SECOND
#define DHCP_TICKS_PER_SECOND 10
#endif

extern uint8_t did_get_dhcp;
void SetupDHCPName( const char * name  );
void TickDHCP(); //Call this DHCP_TICKS_PER_SECOND times per second.

//If DHCP is enabled, you must write this function:
void GotDHCPLease();

#else
inline void SetupDHCPName( const char * name  ) { }
inline void TickDHCP() { }
#endif


#endif

