/*  Simple Bicolor LED library for Arduino, v1.1
    Copyright (C) 2012 Wolfgang Faust

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

    Email: wolf@wolfgang.site40.net
*/
#ifndef BiColorLED_H
#define BiColorLED_H 1.1

#include "Arduino.h"            // If using an old (pre-1.0) version of Arduino, use WConstants.h instead of Arduino.h

#define BICOLOR_OFF 0
#define BICOLOR_RED 1
#define BICOLOR_GREEN 2
#define BICOLOR_YELLOW 3

class BiColorLED
{
public:
    BiColorLED(uint8_t ledPin1, uint8_t ledPin2);
    void drive();
    void off(uint8_t force = false);
    void notify(uint8_t color, uint16_t duration, uint8_t force = false);
    void setColor(uint8_t toColor, uint8_t force = false);
    uint8_t getColor();
private:
    void assign(uint8_t toColor);
    uint8_t color, assigned, notifyColor;
    uint8_t pin1, pin2;
    uint16_t notifyDuration;
    uint32_t _time;
};

#endif
