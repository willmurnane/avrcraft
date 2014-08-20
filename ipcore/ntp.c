#ifdef DO_NTP

#include "ntp.h"

uint64_t NtpTimestamp = 0;
void SendNtpRequest()
{
	struct NtpMessage message;
	message.LI = LeapIndicatorValues_NotSynchronized;
	message.VN = VersionNumber_Ipv4Only;
	message.Mode = Modes_Client;
	message.Stratum = 0;
	message.Poll = 3; // FIXME: this may be invalid? ntpdate does this. valid range 4-14.
	message.Precision = -6; // 2 ** -6 = 0.015625 seconds
	message.RootDelay = 0x00010000; // 1 second, properly justified
	message.RootDispersion = 0x00010000; // 1 second, properly justified
	message.ReferenceIdentifier = 0;
	message.ReferenceTimestamp = message.OriginTimestamp = message.ReceiveTimestamp = 0;
	message.TransmitTimestamp = NtpTimestamp;
	SendUdpTo(ntpserver, 123, 123, (void*)&message, sizeof(message));
}
void ReceiveNtpResponse()
{
	
}

#endif
