/*
 * si4012.c
 *
 *  Created on: 18 Feb 2018
 *      Author: sid
 */

#include <stdio.h>
#include <stdbool.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>

#include "i2c.h"
#include "si4012.h"
#include "flexport.h"

#define SIQUICK

uint8_t si4012_send_request(uint8_t cmd, uint8_t *reqData, uint8_t reqLen)
{
	i2c_start();
	if(!i2c_write(SI4012_ADDR | I2C_WRITE))
	{
		i2c_stop();
		return CMD_ERROR;
	}

	i2c_write(cmd);
	for (int i = 0 ; i < reqLen && reqData; i++)
		i2c_write(reqData[i]);

	i2c_stop();

	return CMD_OK;
}

uint8_t si4012_read_response(uint8_t *resp, uint8_t resLen)
{
	i2c_start();
	if(!i2c_write(SI4012_ADDR | I2C_READ))
	{
		i2c_stop();
		return CMD_ERROR;
	}

	uint8_t status = i2c_read(!!resLen);
	for (int i=0; i<resLen && resp; i++)
		resp[i] = i2c_read(resLen>(i+1));

	i2c_stop();
	return status;
}


uint8_t si4012_set_property(uint8_t *data)
{
	if(si4012_send_request(SET_PROPERTY, data, (data[0] >> 4) + 1) == CMD_ERROR)
		return CMD_ERROR;
#ifdef SIQUICK
	return CMD_OK;
#else
	if (si4012_read_response(NULL, 0) == RESPONSE_STATUS_OK)
		return CMD_OK;
	return CMD_ERROR;
#endif
}

uint8_t si4012_get_interrupt_state(void)
{
	if(si4012_send_request(GET_INT_STATUS, NULL, 0) == CMD_ERROR)
		return CMD_ERROR;
	uint8_t resp;
	if (si4012_read_response(&resp, 1) == RESPONSE_STATUS_OK)
		return resp;

	return CMD_ERROR;
}

uint8_t si4012_enable_interrupts(uint8_t flags)
{
	if(si4012_send_request(SET_INT, &flags, 1) == CMD_ERROR)
		return CMD_ERROR;
#ifdef SIQUICK
	return CMD_OK;
#else
	if (si4012_read_response(NULL, 0) == RESPONSE_STATUS_OK)
		return CMD_OK;
	return CMD_ERROR;
#endif
}

uint8_t si4012_clear_tx_fifo(void)
{
	if(si4012_send_request(INIT_FIFO, NULL, 0) == CMD_ERROR)
		return CMD_ERROR;
#ifdef SIQUICK
	return CMD_OK;
#else
	if (si4012_read_response(NULL, 0) == RESPONSE_STATUS_OK)
		return CMD_OK;
	return CMD_ERROR;
#endif
}

/* props */
uint8_t si4012_set_chip_config(bool useXO, bool lsbFirst, bool fskDevPola)
{
	uint8_t cfg[2] = {PROP_CHIP_CONFIG, (useXO << 3) | (lsbFirst << 2) | fskDevPola};
	return si4012_set_property(cfg);
}

uint8_t si4012_set_modulation(bool fsk, uint8_t fskDeviation)
{
	uint8_t smod[3] = {PROP_MODULATION_FSKDEV, fsk, fskDeviation};
	return si4012_set_property(smod);
}

uint8_t si4012_set_bitrate(uint16_t rate, uint8_t rampRate)
{
	uint8_t cfg[4] = {PROP_BITRATE_CONFIG, (rate >> 10) & 0xFF, rate & 0xFF, rampRate & 0xF};
	return si4012_set_property(cfg);
}

uint8_t si4012_set_frequency(uint32_t frequency)
{
	uint8_t freq[5] = {PROP_TX_FREQ, (frequency >> 24) & 0xFF, (frequency >> 16) & 0xFF, (frequency >> 8) & 0xFF, frequency  & 0xFF};
	return si4012_set_property(freq);
}

uint8_t si4012_set_pa_config(bool pamaxdrv, uint8_t palevel, uint16_t pacap, uint8_t falpha, uint8_t fbeta)
{
	uint8_t cfg[7] = {PROP_PA_CONFIG, pamaxdrv, palevel&0x7f, (pacap>>8)&0x01, pacap&0xff, falpha, fbeta};
	return si4012_set_property(cfg);
}

uint8_t si4012_set_xo_config(uint32_t frequency, bool lowcap)
{
	uint8_t cfg[6] = {PROP_XO_CONFIG, (frequency >> 24) & 0xFF, (frequency >> 16) & 0xFF, (frequency >> 8) & 0xFF, frequency  & 0xFF, lowcap};
	return si4012_set_property(cfg);
}

/*
 *
 * Interrupt
 *
 */
volatile uint8_t si_por;
uint8_t si_pktsize;

//exclusive for si4012
ISR(SI4012_IRQ_ISR)
{
	if(GET(SI4012_nIRQp))
		return;

	uint8_t interrupt = si4012_get_interrupt_state();	//clean any pending interrupt
	if(interrupt & INT_POWER_ON_RESET)
		si_por = 1;
	else if(interrupt & INT_PACKET_SENT)
		SI4012_SDN;
}
ISR(TIM0_OVF_vect)
{

}

/* public */

void si4012_init(void)
{
	/* I2C */
	i2c_init();

	/* Pins */
	INPUT(SI4012_nIRQp);
	SET(SI4012_nIRQp);		//activate pull-up
	SI4012_SDN;
	OUTPUT(SI4012_SDNp);
	asm("nop");

	/* enable interrupt */
	SI4012_IRQ_REG = 1<<SI4012_IRQ_INT;
	GIMSK |= 1<<SI4012_IRQ_EN;

}

void si4012_configure(void)
{
	/* timer on */
	set_sleep_mode(SLEEP_MODE_IDLE);
	TIMSK0 = (1<<TOIE0);
	TCCR0B = (2<<CS00);	//overflow every 2ms -> int -> wakeup

	/* wait until last tx done -> SDN high */
	while(!GET(SI4012_SDNp))
		sleep_mode();

	/* wait for interrrupt */
	si_por = 0;
	SI4012_WAKE;
	while(si_por == 0)
		sleep_mode();

	/* timer off, normal sleep mode */
	TCCR0B = 0;
	set_sleep_mode(SLEEP_MODE_PWR_DOWN);
	/* config */
	si4012_enable_interrupts(ENPKSENT);
	si4012_set_chip_config(true, false, false);

	si4012_set_pa_config(false, 10, 128, 125, 127);
	si4012_set_modulation(true, 0x68);
	si4012_set_bitrate(96, 4);
	si4012_set_frequency(870110000);
	si4012_set_xo_config(10000000, true);
}


uint8_t si4012_set_data(uint8_t *data, uint8_t packetSize)
{
	si4012_clear_tx_fifo();
	if(si4012_send_request(SET_FIFO, data, packetSize) == CMD_ERROR)
		return CMD_ERROR;
#ifdef SIQUICK
	si_pktsize = packetSize;
	return CMD_OK;
#else
	if (si4012_read_response(NULL, 0) == RESPONSE_STATUS_OK)
	{
		si_pktsize = packetSize;
		return CMD_OK;
	}
	return CMD_ERROR;
#endif
}

uint8_t si4012_tx(void)
{
	uint8_t cfg[5] = {0, si_pktsize, (IDLESTATE & 0x03), STBYMODE & 0x07, FIFOMODE & 0x03};
	if(si4012_send_request(TX_START, cfg, 5) == CMD_ERROR)
		return CMD_ERROR;
#ifdef SIQUICK
	return CMD_OK;
#else
	if (si4012_read_response(NULL, 0) == RESPONSE_STATUS_OK)
		return CMD_OK;
	return CMD_ERROR;
#endif
}

uint16_t si4012_get_battery(bool under_load)
{
	if(si4012_send_request(GET_BAT_STATUS, (uint8_t*) &under_load, 1) == CMD_ERROR)
		return CMD_ERROR;
	uint8_t raw[2];
	if (si4012_read_response(raw, 2) == RESPONSE_STATUS_OK)
	{
		uint16_t mv = raw[0]<<8 | raw[1];
		return mv;
	}
	return 0;
}
