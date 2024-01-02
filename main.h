/*
 * main.h
 *
 *  Created on: 20 Feb 2018
 *      Author: sid
 */

#ifndef MAIN_H_
#define MAIN_H_

#define BTN1p		B,0
#define BTN1_PCINT	PCINT8
#define BTN2p		B,1
#define BTN2_PCINT	PCINT9

#define BTN_PC_REG	PCMSK1
#define BTN_PC_EN	PCIE1
#define BTN_ISR		PCINT1_vect
#define BTN_MASK		0xF0

#define BTN1bm		(1<<BIT(BTN1p))
#define BTN2bm		(1<<BIT(BTN2p))
#define BTNpin		(PIN(BTN1p))
#define BTNpinMASK	(BTN1bm|BTN2bm)

/* in MSB */
#define BTN1_RELEASED	4
#define BTN1_PRESSED	5
#define BTN2_RELEASED	6
#define BTN2_PRESSED	7

#endif /* MAIN_H_ */
