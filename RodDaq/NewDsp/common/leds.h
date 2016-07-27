/* Common Header */

#ifndef _LED_
#define _LED_
/* leds.h */

typedef struct {
	void (*on)(void);
	void (*off)(void);
	void (*toggle)(void);
} Led;

void connectLeds(int *nLed, Led ***led);
void initLeds();
#endif
