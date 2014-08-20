void HandleUDP( uint16_t len )
{
	POP16; //Checksum
	len -= 8; //remove header.

	//You could pop things, or check ports, etc. here.

	return;
}


void RegisterUdpCallback(uint16_t port, void(*) func)
{
	
}

void SendUdpTo(struct IpAddress address, uint16_t sport, uint16_t dport, void* data, uint16_t count)
{
	if (address.Address & MyMask.Address == 0)
	{
		int macIndex = RequestARP(address);
	}
	else
	{
		// This is going to a remote host, we need to send it to the gateway's MAC.
		macIndex = RequestARP(MyGateway);
	}
	if (macIndex != -1)
	{
		memcpy(macfrom, ClientArpTable[macIndex].mac, 6);
	}
	
	enc424j600_startsend( NetGetScratch() );
	send_etherlink_header( 0x0800 );

	send_ip_header( 0, address, 17 );
	PUSH16(12345); // src port
	PUSH16(1248); // dst port
	PUSH16(0); // length goes here
	PUSH16(0); // checksum goes here
	PUSHB(data, count);
	util_finish_udp_packet();

}