#include <stdio.h>
#include <stdint.h>

volatile uint8_t PIND;
volatile uint8_t g_oldEncoderValue[2] = {0};
volatile int16_t g_tickCount = 0;


void isr_test()
{
	uint8_t EncoderValue = PIND & 0x3; // PD1 and PD0
	if (EncoderValue & 0x2)
		EncoderValue ^= 0x1;
	if (EncoderValue == g_oldEncoderValue[1])
		return;

	g_tickCount += (EncoderValue - g_oldEncoderValue[1]) % 2;

	printf("EV: %o %o %o ticks: %d\n", g_oldEncoderValue[0], g_oldEncoderValue[1], EncoderValue, g_tickCount);
	g_oldEncoderValue[0] = g_oldEncoderValue[1];
	g_oldEncoderValue[1] = EncoderValue;
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