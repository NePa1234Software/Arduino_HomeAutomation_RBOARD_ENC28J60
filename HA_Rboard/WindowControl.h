/*
  WindowControl.h - Home automation rboard room control
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

#include "PressButtonInput.h"

class CWindowControl
{
  public:

     // Constructor
     CWindowControl();

     // Enable this class (true) or free up the relays (false)
     void EnableWindowControl( bool bEnable );

     // Initialize the relay output pins (arduino pin numbers)
     void InitWindowOutputs(uint8_t PinRelayEnable, uint8_t PinRelayUpDown);

     // Initialize the push button inputs pins for Up and Down
     void AttatchWindowButtons(CPressButton * pUpButton, CPressButton * pDownButton)
     {
         UpButton = pUpButton;
         DownButton = pDownButton;
     };

     // Initialize the window contact inputs pin (pulled down is closed)
     // set pin to 0 if this feature is not supported
     void AttatchWindowContact(CPressButton * pContactInput)
     {
         ContactInput = pContactInput;
     };

     bool IsEnabled()
     {
         return mEnabled;
     };

     // Get the status of the window
     bool IsWindowOpen();

     // Set the time that it take to open and close the window
     // so that we can calculate the percentage open
     void SetTimeToOpen( unsigned char timeToOpenSeconds )
     {
         mTimeToOpenTenths = (unsigned int)timeToOpenSeconds * 10;
         if (mTimeToOpenTenths == 0)
         {
             mTimeToOpenTenths = defaultTimeoutMovement;
         }
     };

     // Get the status of the window
     unsigned char GetBlindOpenPercent()
     {
         return mPercentOpen;
     };

     // Control the window blinds
     void LoopWindowControl();

     // Remote command interpreter
     // Commands supported are "OPEN" and "CLOSE" and "STOP"
     bool DoRemoteCommand( char * strByte );

  private:

    // Default time to open and close in tenths
    static const unsigned int  defaultTimeoutMovement = 300;
    static const unsigned int  addTimeoutOpenClose = 100;
    
    // Flag no move to position requested
    static const unsigned char unknownOpenPercent = 0xff;

    // Private methods
    void CheckLocalRequests();
    void EnablePower();
    void DisablePower();
    void SelectOff();
    void SelectUp();
    void SelectDown();

    // Remote commands
    void Open();
    void Close();
    void Stop();
    
    // Called regularely to keep track of the time the motor is moving up or down
    void CalculatePercentageOpen();

    // Up and Down buttons for controlling the window
    CPressButton * UpButton;
    CPressButton * DownButton;

    // Window contact
    CPressButton * ContactInput;

    // Private Variables
    bool mEnabled;
    bool mPowerOn;
    bool mDoingUpMovement;
    bool IsPowerOn();
    bool IsDoingUpMovement();
    bool IsDoingDownMovement();

    unsigned char mActState;
    unsigned int  mTimeToOpenTenths;
    unsigned int  mTimeActualTenths;
    unsigned char mPercentOpen;
    unsigned char mPercentOpenRequest;
    unsigned char mPinRelayEnable;
    unsigned char mPinRelayUpDown;
    unsigned long mTimestampStatemachine;
    unsigned long mTimestampStatemachineMonitor;
    
    // Commands to perform (remote or using key presses)
    static const unsigned char windowCommandWait  = 0;
    static const unsigned char windowCommandOff  = 1;

    static const unsigned char windowCommandOpenMin = 2;
    static const unsigned char windowCommandOpen = 2;
    static const unsigned char windowCommandOpenManual = 3;
    static const unsigned char windowCommandOpenHandsFree = 4;
    static const unsigned char windowCommandOpenMax = 4;
    
    static const unsigned char windowCommandDownMin = 5;
    static const unsigned char windowCommandDown = 5;
    static const unsigned char windowCommandDownManual = 6;
    static const unsigned char windowCommandDownHandsFree = 7;
    static const unsigned char windowCommandDownMax = 7;

    // Store the actual and new requests
    unsigned char localWindowCommand;
};
