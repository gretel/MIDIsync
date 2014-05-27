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
#include "BiColorLED.h"
#include <digitalIOPerformance.h>

BiColorLED::BiColorLED(uint8_t ledPin1, uint8_t ledPin2)
{
    pin1 = ledPin1;
    pin2 = ledPin2;
    pinMode(pin1, OUTPUT);
    pinMode(pin2, OUTPUT);
}

void BiColorLED::notify(uint8_t color, uint16_t duration, uint8_t force)
{
    notifyColor = color;
    notifyDuration = duration;
    _time = millis();
    if (force)
        drive();
}

void BiColorLED::setColor(uint8_t toColor, uint8_t force)
{
    if (color != toColor)
    {
        color = toColor;
        if (force)
            drive();
    }
}

uint8_t BiColorLED::getColor()
{
    return assigned;
}

void BiColorLED::off(uint8_t force)
{
    color = 0;
    if (force)
        drive();
}

void BiColorLED::assign(uint8_t toColor)
{
    if (assigned == toColor)
        return;
    assigned = toColor;

    switch (toColor)
    {
    case BICOLOR_RED:
        digitalWriteFast(pin1, HIGH);
        digitalWriteFast(pin2, LOW);
        break;
    case BICOLOR_GREEN:
        digitalWriteFast(pin1, LOW);
        digitalWriteFast(pin2, HIGH);
        break;
    case BICOLOR_YELLOW:
        digitalWriteFast(pin1, HIGH);
        digitalWriteFast(pin2, HIGH);
        break;
    default:
        digitalWriteFast(pin1, LOW);
        digitalWriteFast(pin2, LOW);
        break;
    }
}

void BiColorLED::drive()
{
    if (notifyColor > 0)
    {
        if (millis() - _time >= notifyDuration)
        {
            notifyColor = 0;
            notifyDuration = 0;
            assign(color);
        }
        else
        {
            assign(notifyColor);
        }
    }
    else
        assign(color);
}
