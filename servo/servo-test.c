#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>

// Compile with  gcc -Wall -o blink blink.c -lwiringPi


void turn(int deg, int pin) {
	pinMode(pin, OUTPUT);
	
	if(deg < 30 || deg >= 180) {
		print("wrong angle");
		return;
	}

	float tmp = ( (float) 2 / 180) * deg;
	tmp *= 1000;
	int tmp2 = (int) tmp;

	printf("%d\n",tmp2);
	int i;
	for(i = 0; i < 100; i++) {
		digitalWrite(pin, 1);
		delayMicroseconds(tmp2);
		digitalWrite(pin, 0);
		delayMicroseconds(20000 - tmp2);
	}		
}

int main(void) {
	printf("Raspberry Pi blink\n");
	if (wiringPiSetup() == -1) return 1;
	
	turn(30, 7);
	delay(1000);
	turn(29, 7);
	delay(1000);
	turn(28, 7);
	delay(1000);
	turn(27, 7);
	delay(1000);
	turn(26, 7);
	delay(1000);
	turn(25, 7);
}
