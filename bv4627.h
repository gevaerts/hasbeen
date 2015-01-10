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

#ifndef _H_bv4627_h
#define _H_bv4627_h

#include "I2C.h"
#include "Arduino.h"



class BV4627
{
    public:
        BV4627(char i2adr);
        void click(char rly, char on, int del);
        int timer(char rly);
        void off();
        void on();
        void bin(char rly);
        void setaddress(char newAddress);
        int deviceid();
        void version(char *ver);
        void reset();
    private:    
        int i2_16bit();
        char _i2rlyadr;
};

#endif
