/*
  PressButtonInput.h - Home automation rboard room control
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

#ifndef PRESSBUTTONINPUT_H
#define PRESSBUTTONINPUT_H

class CPressButton
{
public:

  // Constructor
  CPressButton();

  // Initialize the inputs pins (0 is not valid)
  void InitInputPin(uint8_t PinInput);

  // On enable all timestamps etc are initialized
  void Enable(bool bEnable);

  // Return false if bEnabled and pin number is not 0
  bool IsEnabled();

  // Statistics
  unsigned long GetPressedCount()
  {
    return mCountPressed;
  };

  // Check the input pin if released, pressed or held
  void Loop();

  // Getter
  bool IsReleased();
  bool IsPressed();
  bool IsHeld();

private:

  // Private Variables
  bool mEnable;
  uint8_t mPinInput;
  bool mButtonRawState;
  bool mButtonPressed;
  bool mButtonHeld;

  unsigned long mTimestamp;
  unsigned long mCountPressed;
};

#endif
