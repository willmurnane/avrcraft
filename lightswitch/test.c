char g_send_toggle asm("r3");
//Copyright 2012 <>< Charles Lohr, under the MIT-x11 or NewBSD license.  You choose.

//Err this is a total spaghetti code file.  My interest is in the libraries, not this.


#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "avr_print.h"
#include <stdio.h>
#include "iparpetc.h"
#include "enc424j600.h"
#include <avr/pgmspace.h>
#include <http.h>
#include <string.h>
#include <basicfat.h>
#include <avr/eeprom.h> 
#include <util10.h>

#define NOOP asm volatile("nop" ::)

static void setup_clock( void )
{
	/*Examine Page 33*/

	CLKPR = 0x80;	/*Setup CLKPCE to be receptive*/
	CLKPR = 0x00;	/*No scalar*/

	OSCCAL=0xff;
}

unsigned short frameno;
unsigned char cc;
char resultbuffer[128];

#define RPORT 8001

#define POP enc424j600_pop8()
#define POP16 enc424j600_pop16()
#define PUSH(x) enc424j600_push8(x)
#define PUSH16(x) enc424j600_push16(x)

uint8_t  EEMEM my_server_name_eeprom[16]; 
char my_server_name[16];

void SetServerName( const char * stname )
{
	memcpy( my_server_name, stname, strlen( stname ) + 1 );
	eeprom_write_block( stname, my_server_name_eeprom, strlen( stname ) + 1 );
}

/////////////////////////////////////////UDP AREA//////////////////////////////////////////////

void HandleUDP( uint16_t len )
{
	POP16; //Checksum
	len -= 8; //remove header.

	//You could pop things, or check ports, etc. here.

	return;
}


//Must be power-of-two
#define CIRC_BUFFER_SIZE 4096
#if ( (FREE_ENC_END) - (FREE_ENC_START) ) <= (CIRC_BUFFER_SIZE)
#error Not enough free space on enc424j600
#endif
#define CIRC_END ((FREE_ENC_START) + (CIRC_BUFFER_SIZE))

uint16_t circ_buffer_at = 0;
uint8_t incirc = 0;
uint16_t GetCurrentCircHead()
{
	return circ_buffer_at;
}

uint8_t UnloadCircularBufferOnThisClient( uint16_t * whence )
{
	//TRICKY: we're in the middle of a transaction now.
	//We need to back out, copy the memory, then pretend nothing happened.
	uint16_t w = *whence;
	uint16_t togomaxB = (circ_buffer_at - w)&((CIRC_BUFFER_SIZE)-1);

	if( togomaxB == 0 ) return 0;


	ETCSPORT |= ETCS;
	uint16_t esat = enc424j600_read_ctrl_reg16( EEGPWRPTL );

	uint16_t togomaxA = 512 - ( esat - sendbaseaddress );
	uint16_t togo = (togomaxA>togomaxB)?togomaxB:togomaxA;

//printf( "w: %04x togo: %04x (%04x/%04x)  %04x %04x %04x\n", w, togo, togomaxA, togomaxB, FREE_ENC_START, CIRC_END, circ_buffer_at );
	enc424j600_copy_memory( esat, w + (FREE_ENC_START), togo, (FREE_ENC_START), CIRC_END-1 );

	enc424j600_write_ctrl_reg16( EEGPWRPTL, esat + togo );
	ETCSPORT &= ~ETCS;
	espiW( EWGPDATA );

	*whence = (w + togo)&((CIRC_BUFFER_SIZE)-1);

	return togo != togomaxB;

}


void StartupBroadcast()
{
	ETCSPORT |= ETCS;

	//is_in_outcirc = 1;
	enc424j600_write_ctrl_reg16( EEUDAWRPTL, circ_buffer_at + (FREE_ENC_START) );
	enc424j600_write_ctrl_reg16( EEUDASTL, FREE_ENC_START );
	enc424j600_write_ctrl_reg16( EEUDANDL, CIRC_END - 1);

	ETCSPORT &= ~ETCS;
	espiW( EWUDADATA );
	incirc = 1;
}

void DoneBroadcast()
{
	ETCSPORT |= ETCS;
	circ_buffer_at = enc424j600_read_ctrl_reg16( EEUDAWRPTL ) - (FREE_ENC_START);
	incirc = 0;
}


unsigned char MyIP[4] = { 1, 1, 1, 2 };
unsigned char MyMask[4] = { 255, 255, 255, 0 };
unsigned char MyGateway[4] = { 1, 1, 1, 1 };
unsigned char MyMAC[6];


void GotDHCPLease()
{
	int i;
	char st[5];

//	puts( "New Lease." );
//	printf( "IP: %d.%d.%d.%d\n", MyIP[0], MyIP[1], MyIP[2], MyIP[3] );
//	printf( "MS: %d.%d.%d.%d\n", MyMask[0], MyMask[1], MyMask[2], MyMask[3] );
//	printf( "GW: %d.%d.%d.%d\n", MyGateway[0], MyGateway[1], MyGateway[2], MyGateway[3] );
	sendstr( "Got DHCP Lease" );
	for( i = 0; i < 4; i++ )
	{
		Uint8To10Str( st, MyIP[i] );
		sendstr( st );
		sendchr( '.' );
	}
	sendchr( '\n' );
	
}

#define sbi(p, n) p |= _BV(n)
#define cbi(p, n) p &= ~_BV(n)

void InitializeLightSwitch()
{
	g_send_toggle = 0;
	// Set pull-up resistor on PD0 and PD1
	PORTD |= _BV(0) | _BV(1);
	// Set pull-up resistor on PB1
	PORTB |= _BV(1);
	// Set direction to input on PD0 and PD1
	DDRD &= ~(_BV(1) | _BV(0));
	// Set direction to input on PB1
	DDRB &= ~(_BV(1));
	
	// Enable PCINT16 and PCINT17, the A and B phases for the rotary encoder
	PCICR |= _BV(2); // for the 23..16 PCINT to be triggered
	PCMSK2 |= _BV(1) | _BV(0); // for bits 17 and 16
	
	// Enable PCINT1, the momentary toggle switch
	PCICR |= _BV(0); // for the 7..0 PCINT to be triggered
	PCMSK0 |= _BV(1); // for bit 1
	
}

volatile int8_t g_tickCount = 0;
volatile uint8_t g_oldEncoderValue[2] = {0};
volatile uint8_t g_careAbout = 0;
int8_t f[4][4] = { {0, -1, 0, 1},
                   {1, 0, -1, 0},
                   {0, 1, 0, -1},
                   {-1, 0, 1, 0} };

ISR (PCINT0_vect)
{
	if (PINB & _BV(1))
	{
		g_send_toggle = 1;
	}
}
ISR(PCINT2_vect)
{
	uint8_t EncoderValue = PIND & 0x3; // PD1 and PD0
	if (EncoderValue & 0x2)
		EncoderValue ^= 0x1;
	if (EncoderValue == g_oldEncoderValue[1])
		return;
	int8_t delta = f[EncoderValue][g_oldEncoderValue[1]];
	g_tickCount += delta;

	g_oldEncoderValue[0] = g_oldEncoderValue[1];
	g_oldEncoderValue[1] = EncoderValue;
	PCIFR |= _BV(2);
}

const uint8_t lightDest[] = { 192, 168, 0, 245 };

int main( void )
{
	uint8_t delayctr;
	uint8_t marker;

	//Input the interrupt.
	DDRD &= ~_BV(2);
	cli();
	setup_spi();
	sendstr( "HELLO\n" );
	setup_clock();
	DIDR0 = 0;
	
	eeprom_read_block( &my_server_name[0], &my_server_name_eeprom[0], 16 );
	if( my_server_name[0] == 0 || my_server_name[0] == (char)0xff )
		SetServerName( "AVRCraft" );

	//Configure T2 to "overflow" at 100 Hz, this lets us run the TCP clock
	TCCR2A = _BV(WGM21) | _BV(WGM20);
	TCCR2B = _BV(WGM22) | _BV(CS22) | _BV(CS21) | _BV(CS20);
	//T2 operates on clkIO, fast PWM.  Fast PWM's TOP is OCR2A
	#define T2CNT  ((F_CPU/1024)/100)
	#if( T2CNT > 254 )
	#undef T2CNT
	#define T2CNT 254
	#endif
	OCR2A = T2CNT;
	
		
	sei();

	//unsigned short phys[32];

	DDRC &= 0;
	if( enc424j600_init( MyMAC ) )
	{
		sendstr( "Failure.\n" );
		while(1);
	}
	sendstr( "OK.\n" );

	SetupDHCPName( my_server_name );

	InitializeLightSwitch();
	
	while(1)
	{
		unsigned short r;

		r = enc424j600_recvpack( );

		if( r ) continue;

		if( TIFR2 & _BV(TOV2) )
		{
			TIFR2 |= _BV(TOV2);
			sendchr( 0 );

			int8_t diff = 0;
			uint8_t toggle = 0;
			cli();
			diff = g_tickCount;
			g_tickCount = 0;
			toggle = g_send_toggle;
			g_send_toggle = 0;
			sei();
			if (toggle != 0 || diff != 0)
			{
				int macIndex = RequestARP(lightDest);
				if (macIndex != -1)
				{
					memcpy(macfrom, ClientArpTable[macIndex].mac, 6);
				}
				
				enc424j600_startsend( NetGetScratch() );
				send_etherlink_header( 0x0800 );

				send_ip_header( 0, lightDest, 17 );
				PUSH16(12345); // src port
				PUSH16(1248); // dst port
				PUSH16(0); // length goes here
				PUSH16(0); // checksum goes here
				PUSH16(0x0010);
				PUSH(toggle);
				PUSH16(diff);
				util_finish_udp_packet();
			}
			delayctr++;
			if( delayctr==10 )
			{
				TickDHCP();
				delayctr = 0;
			}
		}
	}

	return 0;
} 




















