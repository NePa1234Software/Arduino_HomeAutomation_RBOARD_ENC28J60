/*
  WindowControl.cpp - Home automation rboard room control
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

#include "WindowControl.h"

// Relay states
#define DIR_UP    (LOW)
#define DIR_DOWN  (HIGH)
#define RELAY_OFF (LOW)
#define RELAY_ON  (HIGH)
#define RELAY_DELAY      100
#define RELAY_DELAY_OFF  500

const unsigned char ActStateInit = 0;
const unsigned char ActStateIdle = 1;
const unsigned char ActStateEnablePower = 2;
const unsigned char ActStateMoving = 3;
const unsigned char ActStatePowerOff = 4;

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

CWindowControl::CWindowControl()
 :  mEnabled(false),
    mPowerOn(false),
    mActState(ActStateInit),
    mTimeToOpenTenths(defaultTimeoutMovement),
    mTimeActualTenths(defaultTimeoutMovement),
    mPercentOpen(unknownOpenPercent),
    mPercentOpenRequest(unknownOpenPercent),
    mPinRelayEnable(0),
    mPinRelayUpDown(0),
    mDoingUpMovement(false),
    mTimestampStatemachine(0),
    localWindowCommand(windowCommandOff)
{
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void CWindowControl::EnableWindowControl( bool bEnable )
{
    // Free up the relays for other resources
    mActState = ActStateInit;
    mEnabled = bEnable;
    DisablePower();
    SelectOff();
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void CWindowControl::LoopWindowControl()
{
  if ( mEnabled )
  {
    CheckLocalRequests();
    
    // Monitor the direction and time the power is on to
    // Determine the percentage open or closed (approximately)
    CalculatePercentageOpen();

    // Control the relays
    unsigned char prevActState = mActState;
    switch(mActState)
    {
      default:
      case ActStateInit:
          DisablePower();
          SelectOff();
          mActState = ActStateIdle;
      break;

      case ActStateIdle:
          if ( (localWindowCommand != windowCommandOff) && (localWindowCommand != windowCommandWait) )
          { 
            DisablePower(); // Just in case

            if ((localWindowCommand >= windowCommandOpenMin) && (localWindowCommand <= windowCommandOpenMax))
            {
              Serial.println(F(" - Up selected - "));
              SelectUp();
              mActState = ActStateEnablePower;
            }
            else
            {
              if ((localWindowCommand >= windowCommandDownMin) && (localWindowCommand <= windowCommandDownMax))
              {
                Serial.println(F(" - Down selected - "));
                SelectDown();
                mActState = ActStateEnablePower;
              }
            }
          }
      break;

      case ActStateEnablePower:
        if ( (millis() - mTimestampStatemachine) >= RELAY_DELAY )
        {
          EnablePower();
          Serial.println(F(" - Power ON - "));
          mActState = ActStateMoving;
        }
      break;

      case ActStateMoving:
          if ( (localWindowCommand == windowCommandOpenHandsFree) || (localWindowCommand == windowCommandDownHandsFree) )
          {
            if ( (millis() - mTimestampStatemachine) >= (unsigned long)(mTimeToOpenTenths + addTimeoutOpenClose)*100 )
            {
              localWindowCommand = windowCommandOff;
              Serial.println(F(" - Power OFF (Timeout) - "));
              DisablePower();
              mActState = ActStatePowerOff;
            }
            else 
            {
              // Stop when the requested position is reached
              if (   (mPercentOpenRequest != unknownOpenPercent) 
                  && (mPercentOpen == mPercentOpenRequest)
                )
              {
                mPercentOpenRequest = unknownOpenPercent;
                localWindowCommand = windowCommandOff;
                Serial.println(F(" - Power OFF (Position reached) - "));
                DisablePower();
                mActState = ActStatePowerOff;
              }
            } 
          }
          if ( IsDoingUpMovement() )
          {
            if ( ! ((localWindowCommand >= windowCommandOpenMin) && (localWindowCommand <= windowCommandOpenMax) ) )
            {
              Serial.println(F(" - UP -> Power OFF (New Command) - "));
              DisablePower();
              mActState = ActStatePowerOff;
            }
          }
          if ( IsDoingDownMovement() )
          {
            if ( ! ((localWindowCommand >= windowCommandDownMin) && (localWindowCommand <= windowCommandDownMax) ) )
            {
              Serial.println(F(" - DOWN -> Power OFF (New Command) - "));
              DisablePower();
              mActState = ActStatePowerOff;
            }
          }
      break;

      case ActStatePowerOff:
        if ( (millis() - mTimestampStatemachine) >= RELAY_DELAY_OFF )
        {
          // Power is now assumed off after a delay (longer delay to allow motor to stop before reversing)
          Serial.println(F(" - Power is OFF - "));
          
          // Switch off the selector relay to save power 
          SelectOff();
          mActState = ActStateIdle;
        }
      break;
    }

    // Keep the state change time in mind
    if ( prevActState != mActState )
    {
      mTimestampStatemachine = millis();
      Serial.flush();
      Serial.print(F(" -> "));
      Serial.print(mActState);
      Serial.print(F("\n"));
      Serial.flush();
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void CWindowControl::CheckLocalRequests()
{
    // Determine what the local command is
    if ( (UpButton != 0) && (DownButton != 0) )
    {
      if ( DownButton->IsPressed() || UpButton->IsPressed() )
      {
        mPercentOpenRequest = unknownOpenPercent;
      }
      
      if ( localWindowCommand == windowCommandWait )
      {
        if ( DownButton->IsReleased() && UpButton->IsReleased() )
        {
          localWindowCommand = windowCommandOff;
        }
      }
      else if ( DownButton->IsPressed() && UpButton->IsPressed() )
      {
        localWindowCommand = windowCommandWait;
      }
      else if ( UpButton->IsHeld() )
      {
        localWindowCommand = windowCommandOpenManual;
      }
      else if ( DownButton->IsHeld() )
      {
        localWindowCommand = windowCommandDownManual;
      }
      else if ( UpButton->IsReleased() && (localWindowCommand == windowCommandOpen) )
      {
        localWindowCommand = windowCommandOpenHandsFree;
      }
      else if ( UpButton->IsReleased() && (localWindowCommand == windowCommandOpenManual) )
      {
        localWindowCommand = windowCommandOff;
      }
      else if ( DownButton->IsReleased() && (localWindowCommand == windowCommandDown) )
      {
        localWindowCommand = windowCommandDownHandsFree;
      }
      else if ( DownButton->IsReleased() && (localWindowCommand == windowCommandDownManual) )
      {
        localWindowCommand = windowCommandOff;
      }
      else if ( UpButton->IsPressed() && (localWindowCommand >= windowCommandDownMin) && (localWindowCommand <= windowCommandDownMax) )
      {
        localWindowCommand = windowCommandOpen;
      }
      else if ( DownButton->IsPressed() && (localWindowCommand >= windowCommandOpenMin) && (localWindowCommand <= windowCommandOpenMax) )
      {
        localWindowCommand = windowCommandDown;
      }
      else if ( UpButton->IsPressed() && (localWindowCommand == windowCommandOff) )
      {
        localWindowCommand = windowCommandOpen;
      }
      else if ( DownButton->IsPressed() && (localWindowCommand == windowCommandOff) )
      {
        localWindowCommand = windowCommandDown;
      }
      else if ( UpButton->IsPressed() && (localWindowCommand == windowCommandOpenHandsFree) )
      {
        localWindowCommand = windowCommandWait;
      }
      else if ( DownButton->IsPressed() && (localWindowCommand == windowCommandDownHandsFree) )
      {
        localWindowCommand = windowCommandWait;
      }
      else
      {
        // No change
      }
    }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void CWindowControl::CalculatePercentageOpen()
{
    // Monitor the direction and time the power is on to
    // Determine the percentage open or closed (approximately)
    if ( IsPowerOn() )
    {
        if ( (millis() - mTimestampStatemachineMonitor) >= 100 )
        {
            mTimestampStatemachineMonitor = millis();

            // One tenth second of movement has passed
            if ( IsDoingUpMovement() )
            {
                if ( mPercentOpen == unknownOpenPercent )
                {
                  mPercentOpen = 0;
                }
                if ( mTimeActualTenths < mTimeToOpenTenths )
                {
                    mTimeActualTenths++;
                }
            }
            else
            {
                if ( mPercentOpen == unknownOpenPercent )
                {
                  mPercentOpen = 100;
                }
                if ( mTimeActualTenths > 0 )
                {
                    mTimeActualTenths--;
                }
            }

            mPercentOpen = (unsigned char)((unsigned long)(100UL * (unsigned long)mTimeActualTenths)/mTimeToOpenTenths);
            if ( mPercentOpen > 100 )
            {
                mPercentOpen = 100;
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void CWindowControl::InitWindowOutputs(uint8_t PinRelayEnable, uint8_t PinRelayUpDown)
{
  mPinRelayEnable = PinRelayEnable;
  mPinRelayUpDown = PinRelayUpDown;
  mActState = 0;

  pinMode(mPinRelayEnable, OUTPUT);
  pinMode(mPinRelayUpDown, OUTPUT);
  digitalWrite(mPinRelayEnable, RELAY_OFF);
  digitalWrite(mPinRelayUpDown, DIR_UP);
}

////////////////////////////////////////////////////////////////////////////////
// Window contact (pulled down is closed status, just like the buttons)
////////////////////////////////////////////////////////////////////////////////

// Get the status of the window
bool CWindowControl::IsWindowOpen()
{
    bool bRetVal = false;
    if ( ContactInput && ContactInput->IsEnabled() )
    {
        bRetVal = ContactInput->IsReleased();
    }
    return bRetVal;
}

////////////////////////////////////////////////////////////////////////////////
// Relays
////////////////////////////////////////////////////////////////////////////////

void CWindowControl::EnablePower()
{
    if( !mPowerOn )
    {
        mTimestampStatemachineMonitor = millis();
        mPowerOn = true;
    }
    digitalWrite(mPinRelayEnable, RELAY_ON);
}

void CWindowControl::DisablePower()
{
    mPowerOn = false;
    digitalWrite(mPinRelayEnable, RELAY_OFF);
}

void CWindowControl::SelectOff()
{
    digitalWrite(mPinRelayUpDown, RELAY_OFF);
}

void CWindowControl::SelectUp()
{
    mDoingUpMovement = true;
    digitalWrite(mPinRelayUpDown, DIR_UP);
}

void CWindowControl::SelectDown()
{
    mDoingUpMovement = false;
    digitalWrite(mPinRelayUpDown, DIR_DOWN);
}

bool CWindowControl::IsPowerOn()
{
  return mPowerOn;
}

bool CWindowControl::IsDoingUpMovement()
{
  return mDoingUpMovement && mPowerOn;
}

bool CWindowControl::IsDoingDownMovement()
{
  return !mDoingUpMovement && mPowerOn;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

bool CWindowControl::DoRemoteCommand( char * strByte )
{
    bool bRetVal = false;
    if ( (strByte != 0) && mEnabled )
    {
        if (strncmp(strByte, "POS=", 4) == 0)
        {
            unsigned int tmpReq = atoi(&strByte[4]);
            if ( tmpReq == 0 )
            {
                mPercentOpenRequest = unknownOpenPercent;
                Close();
                bRetVal = true;
            }
            else if ( (tmpReq >= 100) || (mPercentOpen == unknownOpenPercent) )
            {
                mPercentOpenRequest = unknownOpenPercent;
                Open();
                bRetVal = true;
            }
            else
            {
                mPercentOpenRequest = (unsigned char)tmpReq;
                if ( mPercentOpen == mPercentOpenRequest )
                {
                    Stop();
                }
                else if ( mPercentOpen > mPercentOpenRequest )
                {
                    Close();
                }
                else
                {
                    Open();
                }
                bRetVal = true;
            }

        }
        else if (strncmp(strByte, "OPEN", 4) == 0)
        {
            Open();
            bRetVal = true;
        }
        else if (strncmp(strByte, "CLOSE_FORCE", 11) == 0)
        {
            Close();
            bRetVal = true;
        }
        else if (strncmp(strByte, "CLOSE", 5) == 0)
        {
            if ( !IsWindowOpen() )
            {
                Close();
                bRetVal = true;
            }
        }
        else if (strncmp(strByte, "STOP", 4) == 0)
        {
            Stop();
            bRetVal = true;
        }
        else
        {
        }
    }
    return bRetVal;
}

void CWindowControl::Open()
{
  localWindowCommand = windowCommandOpenHandsFree;
}

void CWindowControl::Close()
{
  localWindowCommand = windowCommandDownHandsFree;
}

void CWindowControl::Stop()
{
    localWindowCommand = windowCommandOff;
    DisablePower();
    mActState = ActStatePowerOff;
    mPercentOpenRequest = unknownOpenPercent;
}
