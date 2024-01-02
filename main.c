/*
 * main.c
 *
 *  Created on: 17 Jan 2018
 *      Author: sid
 */


#include <stdbool.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <util/delay.h>

#include "si4012.h"
#include "main.h"
#include "flexport.h"

/*
 *
 * serial -> eeprom:
 * order: high byte first
 * /usr/local/bin/avrdude -pt44 -cdragon_isp -Ueeprom:w:0x01,0x02:m
 *
 * overall:
 * /usr/local/bin/avrdude -pt44a -cdragon_isp -Uflash:w:button.hex:a -Ueeprom:w:0x01,0x02:m
 *
 */

volatile uint8_t btn_activity = 0;
volatile uint8_t btn_port_last = BTNpinMASK;
volatile uint8_t btn_port_current = 0;
volatile uint8_t btn_port_changes = 0;

/* high signal -> released */
ISR(BTN_ISR)
{
	btn_port_current = BTNpin & BTNpinMASK;					//store current state
	btn_port_changes = btn_port_current ^ btn_port_last;			//get changes
	if(btn_port_changes & BTN1bm)
		btn_activity |= 1<<((btn_port_current&BTN1bm) ? BTN1_RELEASED : BTN1_PRESSED);
	if(btn_port_changes & BTN2bm)
		btn_activity |= 1<<((btn_port_current&BTN2bm) ? BTN2_RELEASED : BTN2_PRESSED);

	btn_port_last = btn_port_current;
}

void val2buf(uint16_t val, uint8_t idcrc, uint8_t *buf)
{
	buf[0] = val & 0xff;
	buf[1] = val >> 8;
	buf[2] = buf[0] ^ buf[1] ^ idcrc;
}

int main(void)
{
	/* Sleep */
	set_sleep_mode(SLEEP_MODE_PWR_DOWN);

	/* Buttons */
	INPUT(BTN1p);
	CLR(BTN1p);
	INPUT(BTN2p);
	CLR(BTN2p);
	BTN_PC_REG = (1<<BTN1_PCINT)|(1<<BTN2_PCINT);
	GIMSK |= 1<<BTN_PC_EN;

	/* Radio */
	si4012_init();

	/* Interrupt */
	sei();

	/* packet */
	uint16_t serial = eeprom_read_word (0);
	uint8_t pkt[14];
	pkt[0]=0xAA;					//preamble
	pkt[1]=0xAA;
	pkt[2]=0xAA;
	pkt[3]=0xAA;
	pkt[4]=0x2D;					//sync word
	pkt[5]=0xD4;

	pkt[6]=0x07;					//pkt type 7 (no further serial)
	pkt[7]=(serial>>8);				//upper 8bits of serial
	uint8_t uIDCRC = pkt[6] ^ pkt[7];
	val2buf((serial&0xff)<<8, uIDCRC, &pkt[8]);	//extended type 0 (lower), lower 8bit serial in upper byte (8-10)

	uint16_t vbat = 0;
	uint8_t loop = 0;

	while(1)
	{

		cli();
		uint8_t btn_act_local = btn_activity;
		btn_activity = 0;
		sei();

		uint16_t payload = btn_act_local<<8;
		if(payload == 0)
		{
			sleep_mode();
			continue;
		}

		/* wake up hardware */
		si4012_configure();

		/* Battery measurement */
		if((payload & ((1<<(BTN1_PRESSED+8)) | (1<<(BTN2_PRESSED+8)))) == 0)
		{
			if(loop == 0)
				vbat = si4012_get_battery(false);
			if(++loop > 4)
				loop = 0;
		}

		payload |= vbat & 0x0FFF;

		/* generate packet*/
		val2buf(payload, uIDCRC, &pkt[11]);	//(11-13)

		/* transmit a packet */
		si4012_set_data(pkt, sizeof(pkt));
		si4012_tx();

		/* debounce 30ms later... */
		cli();

		/* btn 1 */
		/* step 1: full cycle -> reduce to single event */
		if((btn_activity & ((1<<BTN1_RELEASED) | (1<<BTN1_PRESSED))) == ((1<<BTN1_RELEASED) | (1<<BTN1_PRESSED)))
		{
			btn_activity &= ~((1<<BTN1_RELEASED) | (1<<BTN1_PRESSED));
			btn_activity |= 1<<(GET(BTN1p) ? BTN1_RELEASED : BTN1_PRESSED);

		}
		/* step 2: event included-> cancel */
		if((btn_activity & ((1<<BTN1_RELEASED) | (1<<BTN1_PRESSED))) & (btn_act_local & ((1<<BTN1_RELEASED) | (1<<BTN1_PRESSED))))
		{
			btn_activity &= ~((1<<BTN1_RELEASED) | (1<<BTN1_PRESSED));
		}

		/* btn 2 */
		/* step 1: full cycle -> reduce to single event */
		if((btn_activity & ((1<<BTN2_RELEASED) | (1<<BTN2_PRESSED))) == ((1<<BTN2_RELEASED) | (1<<BTN2_PRESSED)))
		{
			btn_activity &= ~((1<<BTN2_RELEASED) | (1<<BTN2_PRESSED));
			btn_activity |= 1<<(GET(BTN2p) ? BTN2_RELEASED : BTN2_PRESSED);

		}
		/* step 2: event included -> cancel */
		if((btn_activity & ((1<<BTN2_RELEASED) | (1<<BTN2_PRESSED))) & (btn_act_local & ((1<<BTN2_RELEASED) | (1<<BTN2_PRESSED))))
		{
			btn_activity &= ~((1<<BTN2_RELEASED) | (1<<BTN2_PRESSED));
		}

		sei();
	}
}

