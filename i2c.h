/*
 * i2c.h
 *
 *  Created on: 18 Feb 2018
 *      Author: sid
 */

#ifndef I2C_H_
#define I2C_H_

#define I2C_WRITE	0
#define I2C_READ		1

void i2c_init();

void i2c_start();
void i2c_stop();
uint8_t i2c_write(uint8_t data);
uint8_t i2c_read(uint8_t ack);

#endif /* I2C_H_ */
