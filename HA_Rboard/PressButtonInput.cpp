/*
  PressButtonInput.cpp - Home automation rboard room control
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
#include "PressButtonInput.h"

const unsigned int holdTimeMs = 1000;
const unsigned int pressTimeMs = 70;

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

CPressButton::CPressButton()
:
mEnable(false),
mPinInput(0),
mButtonRawState(false),
mButtonPressed(false),
mButtonHeld(false),
mTimestamp(0)
{
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void CPressButton::Enable(bool bEnable)
{
  mEnable = bEnable;
  mButtonRawState = false;
  mButtonPressed = false;
  mButtonHeld = false;
  mTimestamp = millis();
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

bool CPressButton::IsEnabled()
{
  return mEnable && (mPinInput != 0);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void CPressButton::Loop()
{
  if ( IsEnabled() )
  {
    ///////////////////////////////////////////
    // Debouncing of the button
    ///////////////////////////////////////////

    // Determine if a button is release, pressed or held
    bool tmpState = (digitalRead(mPinInput) == 0);

    if ( tmpState != mButtonRawState )
    {
      mButtonRawState = tmpState;
      mTimestamp = millis();
    }
    else if ( (millis() - mTimestamp) < pressTimeMs )
    {
      // Just wait to ensure this was not just interference
    }
    else
    {
      if ( mButtonRawState )
      {
        if ( !mButtonPressed )
        {
          mButtonPressed = true;
          mCountPressed++;
          Serial.println(F(" - Button pressed - "));
        }
        else
        {
          if ( !mButtonHeld )
          {
            if ( (millis() - mTimestamp) >= holdTimeMs )
            {
              mButtonHeld = true;
              Serial.println(F(" - Button held - "));
            }
          }
        }
      }
      else
      {
        if ( mButtonPressed )
        {
          mButtonPressed = false;
          mButtonHeld = false;
          Serial.println(F(" - Button released - "));
        }
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void CPressButton::InitInputPin(uint8_t PinButton)
{
  mPinInput = PinButton;

  // set pin to input and turn on pullup resistors (buttons are input to ground)
  pinMode(mPinInput, INPUT);
  digitalWrite(mPinInput, HIGH);

  mButtonRawState = false;
  mButtonPressed = false;
  mButtonHeld = false;
  mTimestamp = millis();
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

bool CPressButton::IsReleased()
{
  return !mButtonPressed;
}

bool CPressButton::IsPressed()
{
  return mButtonPressed;
}

bool CPressButton::IsHeld()
{
  return mButtonHeld;
}
