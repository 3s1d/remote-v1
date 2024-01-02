/*
 * si4012.h
 *
 *  Created on: 18 Feb 2018
 *      Author: sid
 */

#ifndef SI4012_H_
#define SI4012_H_


#define SI4012_ADDR 				0xE0		//shifted
#define SI4012_SDNp				A,2
#define SI4012_nIRQp				A,3
#define SI4012_IRQ_INT				PCINT3
#define SI4012_IRQ_REG				PCMSK0
#define SI4012_IRQ_EN				PCIE0
#define SI4012_IRQ_ISR				PCINT0_vect

#define SI4012_SDN				SET(SI4012_SDNp)
#define SI4012_WAKE				CLR(SI4012_SDNp)

#define DEFAULT_FSK_DEV 				63
#define DEFAULT_FIFO_ALMOST_FULL_THR 		0xF0
#define DEFAULT_FIFO_ALMOST_EMPTY_THR 		0x10
#define DEFAULT_FIFO_AUTO_TX_THR 		0x20

#define RESPONSE_STATUS_OK 			0x80
#define CMD_OK 					0
#define CMD_ERROR 				0xFF

/*******************************************************************************
    Command opcodes
 *******************************************************************************/
/*    	Command     				Opcode    Description */
#define GET_REV   				0x10    /* Device revision information */
#define SET_PROPERTY  				0x11    /* Sets device properties */
#define GET_PROPERTY  				0x12    /* Gets device properties */
#define LED_CTRL    				0x13    /* LED Control */
#define CHANGE_STATE  				0x60    /* Configures device mode */
#define GET_STATE   				0x61    /* Get device mode */
#define TX_START    				0x62    /* Start data transmission */
#define SET_INT     				0x63    /* Enable interrupts */
#define GET_INT_STATUS  				0x64    /* Read & clear interrupts */
#define INIT_FIFO   				0x65    /* Clears Tx FIFO */
#define SET_FIFO    				0x66    /* Stores data in FIFO for Tx */
#define TX_STOP     				0x67    /* Stops transmission */
#define GET_BAT_STATUS  				0x68    /* Gets battery status//alpha and beta steps */

/*******************************************************************************
    Property codes
 *******************************************************************************/
/*    Property    				ID    Description */
#define PROP_CHIP_CONFIG  			0x10
#define PROP_LED_INTENSITY  			0x11
#define PROP_MODULATION_FSKDEV  			0x20
#define PROP_FIFO_THRESHOLD  			0x30
#define PROP_BITRATE_CONFIG 			0x31
#define PROP_TX_FREQ  				0x40
#define PROP_XO_CONFIG				0x50
#define PROP_PA_CONFIG  				0x60

/********************************************************************************
   Command arguments and response data
 ********************************************************************************/
/* COMMAND: LED_CTRL */
#define LED_OFF      				0x00
#define LED_LOW      				0x01
#define LED_MED      				0x02
#define LED_ON       				0x03

/* COMMAND: SET_INT */
#define ENFFUNDER   				0x80    /* Enable FIFO Underflow */
#define ENTXFFAFULL   				0x40    /* Enable TX FIFO Almost Full */
#define ENTXFFAEM   				0x20    /* Enable TX FIFO Almost Empty */
#define ENFFOVER    				0x10    /* Enable FIFO Overflow */
#define ENPKSENT    				0x08    /* Enable Packet Sent */
#define ENLBD     				0x04    /* Enable Low Battery Detect */
#define ENTUNE      				0x02    /* Enable Tune Complete */

/* COMMAND: TX_START */
#define AUTOTX      				0x04
#define IDLESTATE   				0x00
#define SHDNSTATE   				0x01
#define STBYMODE    				0x00
#define SENSORMODE    				0x01
#define TUNEMODE    				0x02
#define FIFOMODE    				0x00
#define CWMODE      				0x01
#define PN9_0MODE   				0x02
#define PN9_1MODE 				0x03

/* COMMAND: GET_INT_STATUS */
#define INT_FIFO_UNDERFLOW			0x80
#define	INT_TX_FIFO_ALMOST_FULL			0x40
#define INT_TX_FIFO_ALMOST_EMPTY			0x20
#define INT_FIFO_OVERFLOW			0x10
#define INT_PACKET_SENT				0x08
#define INT_LOW_BAT_DETECT			0x04
#define INT_TUNE_COMPLETE			0x02
#define INT_POWER_ON_RESET			0x01

void si4012_init(void);
 void si4012_configure(void);

 uint8_t si4012_set_data(uint8_t *data, uint8_t packetSize);
uint8_t si4012_tx(void);

uint16_t si4012_get_battery(bool under_load);

#endif /* SI4012_H_ */
