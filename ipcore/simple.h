#ifndef SIMPLE_H
#define SIMPLE_H
#include "enc424j600.h"
#include "ntp.h"
#include "iparpetc.h"
#include "avr_print.h"

void SendUdpTo(struct IpAddress address, uint16_t sport, uint16_t dport, void* data, uint16_t count);

#endif
