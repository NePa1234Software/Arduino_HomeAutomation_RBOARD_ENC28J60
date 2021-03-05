/*
  LightControl.cpp - Home automation rboard room control
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

#include "Arduino.h"

#include "LightControl.h"

// Relay states
#define RELAY_OFF (LOW)
#define RELAY_ON  (HIGH)


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

CLightControl::CLightControl()
:
mEnabled(false),
mIsOn(false),
mIsPreviousButtonPressed(false),
mPinRelayEnable(0),
mToggleMode(true),
mTimeToSwitchOff(0),
mTimeToSwitchOffCfg(0),
mTimestampSwitchOn(0)
{
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void CLightControl::EnableLightControl( bool bEnable )
{
  if ( mEnabled && !bEnable )
  {
    SwitchLight(false);
  }
  mEnabled = bEnable;
  mIsPreviousButtonPressed = false;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void CLightControl::LoopLightControl()
{
  if ( mEnabled && OnOffButton )
  {
    // Control the relay on button press
    if ( mIsPreviousButtonPressed != OnOffButton->IsPressed() )
    {
      mIsPreviousButtonPressed = OnOffButton->IsPressed();

      if ( mToggleMode )
      {
        if ( mIsPreviousButtonPressed )
        {
          if ( IsLightOn() )
          {
            SwitchLight(false);
            Serial.println(F(" - Light off - "));
          }
          else
          {
            mTimestampSwitchOn = millis();
            mTimeToSwitchOff = mTimeToSwitchOffCfg;
            SwitchLight(true);
            Serial.println(F(" - Light on - "));
          }
        }
      }
      else
      {
        mTimeToSwitchOff = 0;
        
        if ( OnOffButton->IsPressed() )
        {
          SwitchLight(true);
          Serial.println(F(" - Light on - "));
        }
        else
        {
          SwitchLight(false);
          Serial.println(F(" - Light off - "));
        }
      }
    }
    else
    {
      // Handle the power save timer
      if ( mTimeToSwitchOff != 0 )
      {
        if (OnOffButton->IsHeld())
        {
          // Override the timer and switch to infinite time
          mTimeToSwitchOff = 0;
        }
        else
        {
          if ( IsLightOn() )
          { 
            if ( (millis() - mTimestampSwitchOn) >= ((unsigned long)mTimeToSwitchOff * 1000) )
            {
              SwitchLight(false);
              Serial.println(F(" - Light off (Power save) - "));
            }
          }
        }
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void CLightControl::SwitchLight( bool bOn )
{
  if ( mEnabled )
  {
    mIsOn = bOn;
    if (mIsOn)
    {
      EnablePower();
    }
    else
    {
      DisablePower();
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void CLightControl::InitLightOutput(uint8_t PinRelayEnable)
{
  mPinRelayEnable = PinRelayEnable;

  pinMode(mPinRelayEnable, OUTPUT);
  digitalWrite(mPinRelayEnable, RELAY_OFF);
}

////////////////////////////////////////////////////////////////////////////////
// Relay
////////////////////////////////////////////////////////////////////////////////

void CLightControl::EnablePower()
{
  digitalWrite(mPinRelayEnable, RELAY_ON);
}

void CLightControl::DisablePower()
{
  digitalWrite(mPinRelayEnable, RELAY_OFF);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

bool CLightControl::DoRemoteCommand( char * strByte )
{
  bool bRetVal = false;
  if ( strByte != 0 )
  {
    if (strncmp(strByte, "ON", 2) == 0)
    {
      mTimeToSwitchOff = 0;
      SwitchLight(true);
      bRetVal = true;
    }
    else if (strncmp(strByte, "OFF", 3) == 0)
    {
      SwitchLight(false);
      bRetVal = true;
    }
    else
    {
    }
  }
  return bRetVal;
}
