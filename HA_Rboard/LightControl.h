/*
  LightControl.h - Home automation rboard room control
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

#ifndef LIGHTCONTRL_H
#define LIGHTCONTRL_H

#include "PressButtonInput.h"

class CLightControl
{
public:

  // Constructor
  CLightControl();

  // Enable this class (true) or free up the relays (false)
  void EnableLightControl( bool bEnable );

  // Initialize the relay output pins (arduino pin numbers)
  void InitLightOutput(uint8_t PinRelayEnable);

  // Initialize the push button inputs pins for Up and Down
  void AttatchLightButton(CPressButton * pOnOffButton)
  {
      OnOffButton = pOnOffButton;
  };

  // Control the window blinds
  void LoopLightControl();

  // Remote control of light status
  void SwitchLight( bool bOn );

  bool IsLightOn()
  {
    return mIsOn;
  };

  bool IsEnabled()
  {
    return mEnabled;
  };

  // Default is true, button is a toggle push button
  // false, if we are using a fixed (normal) light switch
  void SetToggleMode( bool bToggle )
  {
    mToggleMode = bToggle;
  };

  // Remote command interpreter
  // Commands supported are "ON" and "OFF"
  bool DoRemoteCommand( char * strByte );

   // Set the time that the light can switch on for (power save)
   void SetOnTime( unsigned int timeToSwitchOff )
   {
       mTimeToSwitchOffCfg = timeToSwitchOff;
   };

private:

  // Private member
  void EnablePower();
  void DisablePower();

  // On Off button for controlling the light
  CPressButton * OnOffButton;

  // Private Variables
  bool mEnabled;
  bool mIsOn;
  bool mIsPreviousButtonPressed;
  bool mToggleMode;

  unsigned char mPinRelayEnable;
  //unsigned char mActState;
  //unsigned long mTimestampStatemachine;

  // Power save funtion
  unsigned int  mTimeToSwitchOff;
  unsigned int  mTimeToSwitchOffCfg;
  unsigned long mTimestampSwitchOn;
  
};
#endif
