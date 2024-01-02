/*
 * i2c.c
 *
 *  Created on: 18 Feb 2018
 *      Author: sid
 */


#include <avr/io.h>
//#include <util/delay.h>


#define SCLPORT	PORTA	//TAKE PORTD as SCL OUTPUT WRITE
#define SCLDDR	DDRA	//TAKE DDRB as SCL INPUT/OUTPUT configure

#define SDAPORT	PORTA	//TAKE PORTD as SDA OUTPUT WRITE
#define SDADDR	DDRA	//TAKE PORTD as SDA INPUT configure

#define SDAPIN	PINA	//TAKE PORTD TO READ DATA
#define SCLPIN	PINA	//TAKE PORTD TO READ DATA

#define SCL	PA1	//PORTD.0 PIN AS SCL PIN
#define SDA	PA0	//PORTD.1 PIN AS SDA PIN


#define SOFT_I2C_SDA_LOW		SDADDR|=((1<<SDA))
#define SOFT_I2C_SDA_HIGH	SDADDR&=(~(1<<SDA))

#define SOFT_I2C_SCL_LOW		SCLDDR|=((1<<SCL))
#define SOFT_I2C_SCL_HIGH	SCLDDR&=(~(1<<SCL))

//#define Q_DEL _delay_loop_2(1)		//3
//#define H_DEL _delay_loop_2(1)		//5
#define Q_DEL	{}//asm("nop")
#define H_DEL	asm("nop")


void i2c_init()
{
	SDAPORT&=(1<<SDA);
	SCLPORT&=(1<<SCL);

	SOFT_I2C_SDA_HIGH;
	SOFT_I2C_SCL_HIGH;
}
void i2c_start()
{
	SOFT_I2C_SCL_HIGH;
	H_DEL;

	SOFT_I2C_SDA_LOW;
	H_DEL;
}

void i2c_stop()
{
	 SOFT_I2C_SDA_LOW;
	 H_DEL;
	 SOFT_I2C_SCL_HIGH;
	 Q_DEL;
	 SOFT_I2C_SDA_HIGH;
	 H_DEL;
}

uint8_t i2c_write(uint8_t data)
{

	 uint8_t i;

	 for(i=0;i<8;i++)
	 {
		SOFT_I2C_SCL_LOW;
		Q_DEL;

		if(data & 0x80)
			SOFT_I2C_SDA_HIGH;
		else
			SOFT_I2C_SDA_LOW;

		H_DEL;

		SOFT_I2C_SCL_HIGH;
		H_DEL;

		while((SCLPIN & (1<<SCL)) == 0 );	// Hier kann er sich aufhaengen, wenn Slave tot !!!

		data=data<<1;
	}

	//The 9th clock (ACK Phase)
	SOFT_I2C_SCL_LOW;
	Q_DEL;

	SOFT_I2C_SDA_HIGH;
	H_DEL;

	SOFT_I2C_SCL_HIGH;
	H_DEL;
	while((SCLPIN & (1<<SCL)) == 0 );	// Hier kann er sich aufhaengen, wenn Slave tot !!!

	// ----- ACK des Slaves einlesen
	uint8_t ack =! (SDAPIN & (1<<SDA));

	// --- Bus loslassen
	SOFT_I2C_SCL_LOW;
	H_DEL;

	return ack;

}

// Lesefunktion
// 0 = Letztes Byte, sende NAK
// 1 = Es kommen noch welche, sende ACK
uint8_t i2c_read(uint8_t ack)
{
	uint8_t data=0x00;
	uint8_t i;

	for(i=0;i<8;i++)
	{
		// SCL clocken
		SOFT_I2C_SCL_LOW;
		H_DEL;
		SOFT_I2C_SCL_HIGH;
		H_DEL;

		while((SCLPIN & (1<<SCL)) == 0);

		if(SDAPIN & (1<<SDA))
			data |= (0x80>>i);

	}

	SOFT_I2C_SCL_LOW;
	Q_DEL;						//Soft_I2C_Put_Ack

	if(ack) {					// ACK
		SOFT_I2C_SDA_LOW;
	}
	else {
		SOFT_I2C_SDA_HIGH;			// NACK = Ende
	}
	H_DEL;

	SOFT_I2C_SCL_HIGH;
	H_DEL;
	while((SCLPIN & (1<<SCL)) == 0);

	SOFT_I2C_SCL_LOW;
	H_DEL;

	SOFT_I2C_SDA_HIGH; // was missing!!

	return data;
}
