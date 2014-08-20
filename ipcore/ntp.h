#ifndef NTP_H
#define NTP_H

#ifndef __ASSEMBLY__
#include <stdint.h>
#endif

enum LeapIndicatorValues 
{
	LeapIndicatorValues_NoWarning = 0,
	LeapIndicatorValues_LastMinuteHas59 = 1,
	LeapIndicatorValues_LastMinuteHas61 = 2,
	LeapIndicatorValues_NotSynchronized = 3,
};
enum VersionNumber
{
	VersionNumber_Ipv4Only = 3,
	VersionNumber_Ipv4OrV6 = 4,
};
enum Modes
{
	Modes_SymmetricActive = 1,
	Modes_SymmetricPassive = 2,
	Modes_Client = 3,
	Modes_Server = 4,
	Modes_Broadcast = 5,
};
struct NtpMessage
{
	enum LeapIndicatorValues LI : 2;
	enum VersionNumber VN : 3;
	enum Modes Mode : 3;
	unsigned char Stratum : 8;
	unsigned char Poll : 8;
	signed char Precision : 8;
	uint32_t RootDelay,
		RootDispersion,
		ReferenceIdentifier;
	uint64_t ReferenceTimestamp,
		OriginTimestamp,
		ReceiveTimestamp,
		TransmitTimestamp;
};

extern uint64_t NtpTimestamp;

#endif
