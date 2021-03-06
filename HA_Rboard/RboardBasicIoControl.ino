/*
  RboardBasicIoControl.ino - Home automation rboard room control
  Copyright (c) 2021 Neil Parker.  All right reserved.

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

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void InitRelays(void)
{
  SetRelayState(1, false);
  SetRelayState(2, false);
  SetRelayState(3, false);
  SetRelayState(4, false);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void SetRelayOn(unsigned char NumRelay)
{
  SetRelayState(NumRelay, true);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void SetRelayOff(unsigned char NumRelay)
{
  SetRelayState(NumRelay, false);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void SetRelayState(uint8_t NumRelay, bool bState)
{
  if ( NumRelay >= 1 && NumRelay <= 4 )
  {
    pinMode(relay1 + NumRelay - 1, OUTPUT);
    if ( bState )
    {
      digitalWrite(relay1 + NumRelay - 1, HIGH);
    }
    else
    {
      digitalWrite(relay1 + NumRelay - 1, LOW);
    }
  }
}
