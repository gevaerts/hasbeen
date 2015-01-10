/*
  BV4627 8 way multi-interface relay board from ByVac
  Copyright (c) 2011 Jim Spence.  All right reserved.
  www.byvac.com - see terms and conditions for using hardware
  
  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

*/  

#include "Arduino.h"
#include "bv4627.h"
 
BV4627::BV4627(char i2adr)
{
  _i2rlyadr=i2adr;  
  I2c.begin();
}

// ==============================================================
// I2C utilities
// ==============================================================
// ==============================================================
// **************************************************************
// gets a 16 bit value from the i2c bus
// **************************************************************
int BV4627::i2_16bit()
{
int rv;
  I2c.read(_i2rlyadr, 2); // returns 2 bytes
  rv=I2c.receive()*256; // high byte
  rv+=I2c.receive(); // low byte
  return rv;
}

// ==============================================================
// I2C 8 WAY RELAY
// ==============================================================
// ==============================================================

// **************************************************************
// Switches an individual relay after a given time, 
// relay A=1, H=8
// **************************************************************
void BV4627::click(char rly, char on, int del)
{
  if(!del) del=1; // 0 is not allowed
  
  if(rly <1) rly = 1;
  if(rly>8) rly = 8;
  
  rly+=9; // relays range 10 to 18
  
  // format is rly, on/off, high_del, low_del
  uint8_t message[3];
  message[0]=on;
  message[1]=(del>>8)&0xff;
  message[2]=del&0xff;
  I2c.write(_i2rlyadr, rly, message, 3);
} 

// **************************************************************
// gets the value of an individual relay timer
// **************************************************************
int BV4627::timer(char rly)
{
  I2c.write(_i2rlyadr, 30, rly);
  // The device will now send data
  return i2_16bit();
}

// **************************************************************
// turns all relays off
// **************************************************************
void BV4627::off()
{
  I2c.write(_i2rlyadr, 18);
}

// **************************************************************
// turns all relays on
// **************************************************************
void BV4627::on()
{
  I2c.write(_i2rlyadr, 19);
}

// **************************************************************
// Binary on - switches relays in accord with the individual bits
// set in param rly. LSB is relay A
// **************************************************************
void BV4627::bin(char rly)
{
  I2c.write(_i2rlyadr, 20, rly);
}

// **************************************************************
// sets I2C address - note will not respond to old I2C address
// after a reset
// **************************************************************
void BV4627::setaddress(char newAddress)
{
  I2c.write(_i2rlyadr, 82, newAddress);
}

// **************************************************************
// get device id, this should be 4627
// **************************************************************
int BV4627::deviceid()
{
  I2c.write(_i2rlyadr, 83);
  return i2_16bit();
}

// **************************************************************
// get device firmware version, this is a string!!
// **************************************************************
void BV4627::version(char *ver)
{
char b, tmp[20];  
  I2c.write(_i2rlyadr, 84);
  // return as a string in the format x.x
  I2c.read(_i2rlyadr, 2); // returns 2 bytes
  b=I2c.receive(); // major byte
  itoa(b,tmp,10); // convert to string
  *ver=0; // just in case
  strcpy(ver,tmp);
  strcat(ver,".");
  b=I2c.receive(); // minor byte
  itoa(b,tmp,10);
  strcat(ver,tmp);
}

// **************************************************************
// reset device
// **************************************************************
void BV4627::reset()
{
  I2c.write(_i2rlyadr, 85);
}
