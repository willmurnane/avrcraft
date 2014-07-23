#include <stdio.h>
#include <stdint.h>

volatile uint8_t PIND;
volatile uint8_t g_oldEncoderValue = 2;
volatile uint8_t g_lastChange = 1; // change to 2 to alter direction?
volatile int8_t g_direction = 1;
volatile int16_t g_tickCount = 0;

void isr_test()
{
	uint8_t EncoderValue = PIND & 0x3; // PD1 and PD0
	if ((g_oldEncoderValue ^ EncoderValue) == g_lastChange)
	{
		g_direction *= -1;
	}
	g_tickCount += g_direction;
	printf("EV: %o oEV: %o lc: %o ticks: %d\n", EncoderValue, g_oldEncoderValue, g_lastChange, g_tickCount);
	
	g_lastChange = g_oldEncoderValue ^ EncoderValue;
	g_oldEncoderValue = EncoderValue;
}

void forward(int n)
{
	printf("Forwards %d revs\n", n);
	for (int i = 0; i < n; i++)
	{
		PIND = 0;
		isr_test();
		PIND = 1;
		isr_test();
		PIND = 3;
		isr_test();
		PIND = 2;
		isr_test();
	}
}
void backward(int n)
{
	printf("Backwards %d revs\n", n);
	for (int i = 0; i < n; i++)
	{
		PIND = 3;
		isr_test();
		PIND = 1;
		isr_test();
		PIND = 0;
		isr_test();
		PIND = 2;
		isr_test();
	}
}

int main()
{
	forward(3);
	backward(5);
	forward(3);
	return 0;
}