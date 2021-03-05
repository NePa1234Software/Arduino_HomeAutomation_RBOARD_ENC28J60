/*
  HA_RBoard.ino - Home automation rboard room control
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

extern "C" {
#include <websrv_help_functions.h>
#include <avr/wdt.h>
}
extern "C" {
#include "enc28j60.h"
}

#include "MemoryFree.h"

#include "EtherShield.h"
#include "ETHER_28J60.h"

#include "WindowControl.h"
#include "LightControl.h"
#include "PressButtonInput.h"

#include "SafeConfiguration.h"

////////////////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////////////////

#define FS(x) (const __FlashStringHelper*)(x)

////////////////////////////////////////////////////////////////////////////////
// Methods
////////////////////////////////////////////////////////////////////////////////

void SetupWebpage(ETHER_28J60 * pEth);
void SetupWebpageHelp(ETHER_28J60 * pEth);
void SetupWebpageStatus(ETHER_28J60 * pEth);

////////////////////////////////////////////////////////////////////////////////
// Config items in Eeprom for external read and write access
////////////////////////////////////////////////////////////////////////////////

EEPROMStoreClass fhemConfiguration = EEPROMStoreClass();

// Define the IOs for the 4 relays
const uint8_t relay1 = 4;
const uint8_t relay2 = 5;
const uint8_t relay3 = 6;
const uint8_t relay4 = 7;

const int inputPin1 = A0;
const int inputPin2 = A1;
const int inputPin3 = A2;
const int inputPin4 = A3;
const int inputPin5 = A4;
const int inputPin6 = A5;

const uint8_t outputResetPin = 8;

// Modes supported to date
const unsigned char ModeLights          = 0;
const unsigned char ModeLightsGong      = 1;
const unsigned char ModeLightsWindow    = 2;
const unsigned char ModeTwoWindows      = 3;
const unsigned char ModeCount           = 4;

// Positions of configuration data
const uint16_t CfgItemPosMode      = 0;    // 0: default all relays are lights, 1: One window, 2: Two windows 
const uint16_t CfgItemPosEthEnable = 10;   // size 1, 0: none, 1: enabled 
const uint16_t CfgItemPosEthLog    = 11;   // size 1, 0: none, 1: full 
const uint16_t CfgItemPosMac       = 12;   // Size 6
const uint16_t CfgItemPosIp        = 18;   // Size 4
const uint16_t CfgItemPosTimeWin1  = 30;   // Size 1
const uint16_t CfgItemPosTimeWin2  = 40;   // Size 1
const uint16_t CfgItemPosTimeLight1 = 50;   // Size 1
const uint16_t CfgItemPosTimeLight2 = 60;   // Size 1
const uint16_t CfgItemPosTimeLight3 = 70;   // Size 1
const uint16_t CfgItemPosTimeLight4 = 80;   // Size 1
const uint16_t CfgItemPosBoardName    = 100;  // Size 32
const uint16_t CfgItemPosTestEeprom1  = 190;   // Size 1
const uint16_t CfgItemPosTestEeprom2  = 390;   // Size 1

// Modes and initialization variables
static unsigned char theMode = ModeLights;
static unsigned char theEthEnable = 0;
static unsigned char theLogLevelEth = 0;
static unsigned char bWaitForEeprom = true;

// Lights and window handlers
#define MAX_BOARDNAME_SIZE 32
#define NUM_INPUTS  6
#define NUM_LIGHTS  4
#define NUM_WINDOWS 2
CWindowControl theWindows[NUM_WINDOWS];
CLightControl theLights[NUM_LIGHTS];
CPressButton theInputs[NUM_INPUTS];

// Command buffer for serial input
#define COMMAND_BUFFER_SIZE 32
static char cmdReceived[COMMAND_BUFFER_SIZE+1];
static unsigned char cmdIndex = 0;

// Menu 
const char UserString_Line [] PROGMEM = "-----------------------------------------------";
const char UserString_NewLine[] PROGMEM = "\n";
const char UserString_Title[] PROGMEM = "Fhem Relay Board";
const char UserString_FwVersion[] PROGMEM = "Version: 1.4";
const char UserString_Author[] PROGMEM = "Author: Neil Parker";
const char UserString_H[] PROGMEM = "H - Help menu";
const char UserString_D[] PROGMEM = "D - Dump Eeprom";
const char UserString_M[] PROGMEM = "M - Mode change";
const char UserString_L[] PROGMEM = "L - Ethernet logging";
const char UserString_R[] PROGMEM = "R - Dump Ethernet registers";
const char UserString_0[] PROGMEM = "0 - Relays off";
const char UserString_1[] PROGMEM = "1-4 - Relay ON";
const char UserString_MenuCfgIp[] PROGMEM        = "?CMD=IP=192.168.178.99 : set IP and last 2 bytes of MAC";
const char UserString_MenuCfgErase[] PROGMEM     = "?CMD=EEPROM : erase the eeprom";
const char UserString_MenuCfgReset[] PROGMEM     = "?CMD=RESET : trigger a watchdog timeout (8s)";
const char UserString_MenuCfgName[] PROGMEM      = "?CMD=NAME=Max32Char : Board name used for webpage";
const char UserString_MenuCfgMode[] PROGMEM      = "?CMD=MODE=n : n: 0=Lights, 1=LightsGong, 2=LightsWindow, 3=TwoWindows";
const char UserString_MenuCfgEth[] PROGMEM       = "?CMD=ETH_ON / ETH_OFF / ETH_RESET : enable, disable, reset ethernet";
const char UserString_MenuCfgWinTime[] PROGMEM   = "?CMD=TIMEBn=90 (n=1-2) : Time that the blind need to open";
const char UserString_MenuCfgLightTime[] PROGMEM = "?CMD=TIMELn=90 (n=1-4) : Time that the light is on for";
const char UserString_MenuCfgLight[] PROGMEM     = "?CMD_Ln=LightCommand(n=1-4)";
const char UserString_MenuCfgLightCmd[] PROGMEM  = "...ON,OFF : switch light on or off";
const char UserString_MenuCfgWin[] PROGMEM       = "?CMD_Bn=BlindCommand(n=1-2)";
const char UserString_MenuCfgWinCmd[] PROGMEM    = "...OPEN, CLOSE, CLOSE_FORCE, STOP, POS=50 : control the shutters";
const char UserString_Help[] PROGMEM             = "help : ";

const char * const UserStringMenuEth[] PROGMEM=
{
  UserString_Help,
  UserString_MenuCfgName,
  UserString_MenuCfgWinTime,
  UserString_MenuCfgLightTime,
  UserString_MenuCfgLight,
  UserString_MenuCfgLightCmd,
  UserString_MenuCfgWin,
  UserString_MenuCfgWinCmd,
  0
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void setup()
{ 
  /*
   * Enable the watchdog with the largest prescaler.  Will cause a
   * watchdog reset after approximately 2 s @ Vcc = 5 V
   */
  wdt_enable(WDTO_8S);

  Serial.begin(115200);

  wdt_reset();

  DumpMemoryFree(true);

  ShowMenu();

  wdt_reset();
  InitRelays();

  wdt_reset();
  InitResources();
  
  DumpMemoryFree(true);
}

void loop()
{
  wdt_reset();
  LoopMenu();
  DumpMemoryFree(false);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void InitResources(void)
{
  ////////////////////////////////////////////////////////
  // Just initialize pins, not enable
  ////////////////////////////////////////////////////////

  int ii = 0;
  for( ii = 0; ii < NUM_INPUTS; ii++ )
  {
    theInputs[ii].InitInputPin(inputPin1 + ii);
    theInputs[ii].Enable(true);
  }
  
  for( ii = 0; ii < NUM_LIGHTS; ii++ )
  {
    theLights[ii].InitLightOutput(relay1 + ii);
    theLights[ii].AttatchLightButton(&theInputs[ii]);
  }

  theWindows[0].InitWindowOutputs(relay1,relay2);
  theWindows[0].AttatchWindowButtons(&theInputs[0], &theInputs[1]);
  theWindows[0].AttatchWindowContact(&theInputs[4]);

  theWindows[1].InitWindowOutputs(relay3,relay4);
  theWindows[1].AttatchWindowButtons(&theInputs[2], &theInputs[3]);
  theWindows[1].AttatchWindowContact(&theInputs[5]);

}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void ShowMenu( void )
{
    // Print out the menu to the serial port
    Serial.flush(); Serial.println( FS(UserString_Line) ); 
    Serial.flush(); Serial.println( FS(UserString_Title) );
    Serial.flush(); Serial.println( FS(UserString_FwVersion) );
    Serial.flush(); Serial.println( FS(UserString_Author) );
    Serial.flush(); Serial.println( FS(UserString_Line) );
    Serial.flush(); Serial.println( FS(UserString_H) );
    Serial.flush(); Serial.println( FS(UserString_D) );
    Serial.flush(); Serial.println( FS(UserString_M) );
    Serial.flush(); Serial.println( FS(UserString_L) );
    Serial.flush(); Serial.println( FS(UserString_R) );
    Serial.flush(); Serial.println( FS(UserString_0) );
    Serial.flush(); Serial.println( FS(UserString_1) );
    Serial.flush(); Serial.println( FS(UserString_Line) );
    Serial.flush(); Serial.println( FS(UserString_MenuCfgIp) );
    Serial.flush(); Serial.println( FS(UserString_MenuCfgErase) );
    Serial.flush(); Serial.println( FS(UserString_MenuCfgReset) );
    Serial.flush(); Serial.println( FS(UserString_MenuCfgName) );
    Serial.flush(); Serial.println( FS(UserString_MenuCfgMode) ); 
    Serial.flush(); Serial.println( FS(UserString_MenuCfgEth) );
    Serial.flush(); Serial.println( FS(UserString_MenuCfgWinTime) );
    Serial.flush(); Serial.println( FS(UserString_MenuCfgLightTime) );
    Serial.flush(); Serial.println( FS(UserString_Line) );
    Serial.flush(); Serial.println( FS(UserString_MenuCfgLight) );
    Serial.flush(); Serial.println( FS(UserString_MenuCfgLightCmd) );
    Serial.flush(); Serial.println( FS(UserString_Line) );
    Serial.flush(); Serial.println( FS(UserString_MenuCfgWin) );
    Serial.flush(); Serial.println( FS(UserString_MenuCfgWinCmd) );
    Serial.flush(); Serial.println( FS(UserString_Help) );
    Serial.flush(); Serial.println( FS(UserString_Line) );
    LogModes();
    DumpEthSettings();
    DumpMemoryFree(true);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void ConfigureModes(void)
{
  ////////////////////////////////////////////////////////
  // First disable unneeded resources (switching off relays etc)
  // Then enable the resources need with the modes 
  ////////////////////////////////////////////////////////

  switch ( theMode )
  {
  default:
  case ModeLights:
    {
      theWindows[0].EnableWindowControl(false);
      theWindows[1].EnableWindowControl(false);
      theLights[0].EnableLightControl(true);
      theLights[1].EnableLightControl(true);
      theLights[2].EnableLightControl(true);
      theLights[3].EnableLightControl(true);
      theLights[3].SetToggleMode(true);
      break;
    }
  case ModeLightsGong:
    {
      theWindows[0].EnableWindowControl(false);
      theWindows[1].EnableWindowControl(false);
      theLights[0].EnableLightControl(true);
      theLights[1].EnableLightControl(true);
      theLights[2].EnableLightControl(true);
      theLights[3].EnableLightControl(true);

      // The gong is activated when the door bell is pressed (or remotely)
      theLights[3].SetToggleMode(false);
      break;
    }
  case ModeLightsWindow:
    {
      theWindows[0].EnableWindowControl(false);
      theLights[2].EnableLightControl(false);
      theLights[3].EnableLightControl(false);

      theWindows[1].EnableWindowControl(true);
      theLights[0].EnableLightControl(true);
      theLights[1].EnableLightControl(true);
      break;
    }
  case ModeTwoWindows:
    {
      theLights[0].EnableLightControl(false);
      theLights[1].EnableLightControl(false);
      theLights[2].EnableLightControl(false);
      theLights[3].EnableLightControl(false);

      theWindows[0].EnableWindowControl(true);
      theWindows[1].EnableWindowControl(true);
      break;
    }
  }

  LogModes();
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void LogModes(void)
{
  Serial.print(F("Mode set to : "));

  switch ( theMode )
  {
  default:
  case ModeLights:
    {
      Serial.println(F("4 Lights"));
      break;
    }
  case ModeLightsGong:
    {
      Serial.println(F("3 Lights + 1 Gong"));
      break;
    }
  case ModeLightsWindow:
    {
      Serial.println(F("2 Lights + 1 Windows"));
      break;
    }
  case ModeTwoWindows:
    {
      Serial.println(F("2 Windows"));
      break;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void LoopMenu( void )
{
  fhemConfiguration.LoopConfiguration();
  
  // Input control
  int ii;
  for( ii = 0; ii < NUM_INPUTS; ii++ )
  {
    theInputs[ii].Loop();
  }

  // Wait for the Eeprom initialization to finish  
  if ( bWaitForEeprom )
  {
    if ( !fhemConfiguration.IsInitializingConfiguration() )
    {
      Serial.flush();
      bWaitForEeprom = false;

      // Get configuration from the eeprom
      theMode = fhemConfiguration.Read(CfgItemPosMode);
      theLogLevelEth = fhemConfiguration.Read(CfgItemPosEthLog);
      theEthEnable = fhemConfiguration.Read(CfgItemPosEthEnable);

      // Initialize ethernet is done in the loop, just trigger here
      InitEthernet(theEthEnable != 0);
      LogLevelEthernet(theLogLevelEth);

      //IP adress
      uint8_t tmp_ip[4] = { 0, 0, 0, 0 };
      fhemConfiguration.ReadBlock(CfgItemPosIp, tmp_ip, 4 );
      if ( (tmp_ip[0] != 0) &&
           (tmp_ip[1] != 0) &&
           (tmp_ip[2] != 0) &&
           (tmp_ip[3] != 0) )
      {
        Serial.println(F("Set IP and MAC."));
        SetEthIpAddress(tmp_ip);
        //SetEthMacAddress(tmp_mac);
        DumpEthSettings();
      }
      else
      {
        Serial.println(F("BAD IP in EEPROM."));
      }
      Serial.flush();

      // Update accordingly
      ConfigureModes();

      // Configure the windows
      theWindows[0].SetTimeToOpen(fhemConfiguration.Read(CfgItemPosTimeWin1));
      theWindows[1].SetTimeToOpen(fhemConfiguration.Read(CfgItemPosTimeWin2));

      // Configure the lights
      theLights[0].SetOnTime(fhemConfiguration.Read(CfgItemPosTimeLight1));
      theLights[1].SetOnTime(fhemConfiguration.Read(CfgItemPosTimeLight2));
      theLights[2].SetOnTime(fhemConfiguration.Read(CfgItemPosTimeLight3));
      theLights[3].SetOnTime(fhemConfiguration.Read(CfgItemPosTimeLight4));
    }
  }
  else
  {
    for( ii = 0; ii < NUM_WINDOWS; ii++ )
    {
      LoopEthernetControl();
      LoopSerialRead();
      theWindows[ii].LoopWindowControl();
    }

    for( ii = 0; ii < NUM_LIGHTS; ii++ )
    {
      LoopEthernetControl();
      LoopSerialRead();
      theLights[ii].LoopLightControl();
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void LoopSerialRead( void )
{
  // read the serial input
  if (Serial.available() > 0) 
  {
    bool bDone = false;
    int inByte = Serial.read();
    //Serial.println(inByte);

    ////////////////////////////////////////////////
    // Stream in the command into the command buffer
    ////////////////////////////////////////////////

    if ( inByte < ' ' )
    {
      if ( inByte == '\n' )
      {
        cmdReceived[cmdIndex] = 0;
        bDone = true;
      }
    }
    else
    {
      cmdReceived[cmdIndex] = (char)inByte;
      if ( ++cmdIndex >= COMMAND_BUFFER_SIZE )
      {
        cmdIndex = COMMAND_BUFFER_SIZE;
        cmdReceived[cmdIndex] = 0;
        bDone = true;
      }
    }

    ////////////////////////////////////////////////
    // Call the command handler
    ////////////////////////////////////////////////

    if ( bDone )
    {
      if ( DoCommand( cmdReceived ) )
      {
        Serial.println(F("GOOD..."));
      }
      cmdIndex = 0;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

bool DoCommand( char * strByte )
{
  bool bRetVal = false;
  if ( strByte != 0 )
  {
    if ( (strByte[0] == 0) || (strByte[1] == 0) || (strByte[1] == ' ') )
    {
      bRetVal = DoMenuCommand( strByte[0] );
    }
    else
    {
      Serial.println(strByte);
      if (strncmp(strByte, "?CMD=", 5) == 0)
      {
        bRetVal = DoRemoteCommand(&strByte[5]);
      }
      else if (strncmp(strByte, "?CMD_L1=", 8) == 0)
      {
        bRetVal = theLights[0].DoRemoteCommand(&strByte[8]);
      }
      else if (strncmp(strByte, "?CMD_L2=", 8) == 0)
      {
        bRetVal = theLights[1].DoRemoteCommand(&strByte[8]);
      }
      else if (strncmp(strByte, "?CMD_L3=", 8) == 0)
      {
        bRetVal = theLights[2].DoRemoteCommand(&strByte[8]);
      }
      else if (strncmp(strByte, "?CMD_L4=", 8) == 0)
      {
        bRetVal = theLights[3].DoRemoteCommand(&strByte[8]);
      }
      else if (strncmp(strByte, "?CMD_B1=", 8) == 0)
      {
        bRetVal = theWindows[0].DoRemoteCommand(&strByte[8]);
      }
      else if (strncmp(strByte, "?CMD_B2=", 8) == 0)
      {
        bRetVal = theWindows[1].DoRemoteCommand(&strByte[8]);
      }
      else if (strncmp(strByte, "help", 4) == 0)
      {
        bRetVal = true;
      }
      else if (strncmp(strByte, "?GET", 4) == 0)
      {
        bRetVal = true;
      }
      else
      {
      }
    }
  }
  return bRetVal;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

bool DoRemoteCommand( char * strByte )
{
  bool bRetVal = false;
  if ( strByte != 0 )
  {
    if ( (strByte[0] == 0) || (strByte[1] == 0) || (strByte[1] == ' ') )
    {
      bRetVal = DoMenuCommand( strByte[0] );
    }
    else
    {
      if (strncmp(strByte, "NAME=", 5) == 0)
      {
        char tmpName[MAX_BOARDNAME_SIZE+1];
        memset(tmpName, 0, MAX_BOARDNAME_SIZE+1);
        strncpy(tmpName, &strByte[5], MAX_BOARDNAME_SIZE);
        fhemConfiguration.WriteBlock(CfgItemPosBoardName, (uint8_t *)tmpName, (size_t)MAX_BOARDNAME_SIZE);
        bRetVal = true;
      }
      else if (strncmp(strByte, "MODE=", 5) == 0)
      {
        char tmpMode = strByte[5] - '0';
        if ( (tmpMode >= 0) && (tmpMode < ModeCount) )
        {
          fhemConfiguration.Write(CfgItemPosMode, (uint8_t)tmpMode);
          bWaitForEeprom = true;
          bRetVal = true;
        }
      }
      else if (strncmp(strByte, "IP=", 3) == 0)
      {
        uint8_t tmp_ip[4] = { 0, 0, 0, 0 };
        if ( 0 == parse_ip(tmp_ip,&strByte[3]) )
        {
          if ( (tmp_ip[0] != 0) ||
               (tmp_ip[1] != 0) ||
               (tmp_ip[2] != 0) ||
               (tmp_ip[3] != 0) )
          {
            // Set to the eeprom
            fhemConfiguration.WriteBlock(CfgItemPosIp, tmp_ip, 4);
            
            // Set ethernet on
            theEthEnable = 1;
            fhemConfiguration.Write(CfgItemPosEthEnable, theEthEnable);
            bWaitForEeprom = true;
            bRetVal = true;
          }
        }
      }
      /*
            else if (strncmp(strByte, "MAC=", 4) == 0)
       {
       uint8_t tmp_mac[4] = {0, 0, 0, 0, 0, 0};
       if ( 0 == parse_mac(tmp_mac,&strByte[4]) )
       {
       if ( (tmp_mac[0] != 0) &&
       (tmp_mac[1] != 0) &&
       (tmp_mac[2] != 0) &&
       (tmp_mac[3] != 0) &&
       (tmp_mac[4] != 0) &&
       (tmp_mac[5] != 0) )
       {
         Set to the eeprom
         fhemConfiguration.WriteBlock(CfgItemPosMac, tmp_mac, 6);
         bWaitForEeprom = true;
         bRetVal = true;
       }
       }
       }
       */
      else if (strncmp(strByte, "ETH_ON", 6) == 0)
      {
        // Set to the eeprom
        theEthEnable = 1;
        fhemConfiguration.Write(CfgItemPosEthEnable, theEthEnable);
        bWaitForEeprom = true;
        bRetVal = true;
      }
      else if (strncmp(strByte, "ETH_OFF", 7) == 0)
      {
        // Set to the eeprom
        theEthEnable = 0;
        fhemConfiguration.Write(CfgItemPosEthEnable, theEthEnable);
        bWaitForEeprom = true;
        bRetVal = true;
      }
      else if (strncmp(strByte, "ETH_RESET", 7) == 0)
      {
        // Force a reset of the chip
        Serial.print(F("Reset ETH..."));
        InitEthernet(theEthEnable != 0);
        bRetVal = true;
      }
      else if (strncmp(strByte, "TIMEB1=", 7) == 0)
      {
        // Set to the eeprom
        unsigned int tmpTime = atoi(&strByte[7]);
        fhemConfiguration.Write(CfgItemPosTimeWin1, (unsigned char)tmpTime);
        bWaitForEeprom = true;
        bRetVal = true;
      }
      else if (strncmp(strByte, "TIMEB2=", 7) == 0)
      {
        // Set to the eeprom
        unsigned int tmpTime = atoi(&strByte[7]);
        fhemConfiguration.Write(CfgItemPosTimeWin2, (unsigned char)tmpTime);
        bWaitForEeprom = true;
        bRetVal = true;
      }
      else if (strncmp(strByte, "TIMEL1=", 7) == 0)
      {
        // Set to the eeprom
        unsigned int tmpTime = atoi(&strByte[7]);
        fhemConfiguration.Write(CfgItemPosTimeLight1, (unsigned char)tmpTime);
        bWaitForEeprom = true;
        bRetVal = true;
      }
      else if (strncmp(strByte, "TIMEL2=", 7) == 0)
      {
        // Set to the eeprom
        unsigned int tmpTime = atoi(&strByte[7]);
        fhemConfiguration.Write(CfgItemPosTimeLight2, (unsigned char)tmpTime);
        bWaitForEeprom = true;
        bRetVal = true;
      }
      else if (strncmp(strByte, "TIMEL3=", 7) == 0)
      {
        // Set to the eeprom
        unsigned int tmpTime = atoi(&strByte[7]);
        fhemConfiguration.Write(CfgItemPosTimeLight3, (unsigned char)tmpTime);
        bWaitForEeprom = true;
        bRetVal = true;
      }
      else if (strncmp(strByte, "TIMEL4=", 7) == 0)
      {
        // Set to the eeprom
        unsigned int tmpTime = atoi(&strByte[7]);
        fhemConfiguration.Write(CfgItemPosTimeLight4, (unsigned char)tmpTime);
        bWaitForEeprom = true;
        bRetVal = true;
      }
      else if (strncmp(strByte, "EEPROM", 6) == 0)
      {
        // Init eeprom
        fhemConfiguration.EraseEeprom();
        fhemConfiguration.DumpEeprom();
        bRetVal = true;
      }
      else if (strncmp(strByte, "RESET", 5) == 0)
      {
        // Wait for the watchdog timer
        // doesn' work with this bootloader 
        // while(1){};
        
        digitalWrite(outputResetPin, LOW);
        delay(50);
        digitalWrite(outputResetPin, HIGH);
        delay(500);
        bRetVal = false;
      }
    }
  }
  return bRetVal;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

bool DoMenuCommand( char inByte )
{
  bool bRetVal = true;
  switch(inByte)
  {
  default:
    bRetVal = false;
    break;

  case 'H':
    ShowMenu();
    break;

  case 'S':
    DumpMemoryFree(true);
    break;
    
  case 'D':
    fhemConfiguration.DumpEeprom();
    break;

  case 'C':
    break;

  case '0':
    InitRelays();
    break;

  case '1':
    SetRelayOn(1);
    break;

  case '2':
    SetRelayOn(2);
    break;

  case '3':
    SetRelayOn(3);
    break;

  case '4':
    SetRelayOn(4);
    break;

  case '5':
    fhemConfiguration.WriteNoChecksum(CfgItemPosTestEeprom1, (uint8_t)0x55);
    break;

  case '6':
    fhemConfiguration.WriteNoChecksum(CfgItemPosTestEeprom2, (uint8_t)0x55);
    break;

  case 'L':
    ++theLogLevelEth;
    if ( theLogLevelEth >= 3 )
    {
      theLogLevelEth = 0;
    }

    // Set to the eeprom
    fhemConfiguration.Write(CfgItemPosEthLog, theLogLevelEth);

    LogLevelEthernet(theLogLevelEth);
    break;

  case 'R':
    DumpEthSettings();
    DumpEthRegisters();
    break;

  case 'M':
    ++theMode;
    if ( theMode >= ModeCount )
    {
      theMode = 0;
    }

    // Set to the eeprom
    fhemConfiguration.Write(CfgItemPosMode, theMode);

    ConfigureModes();
    break;
  }
  return bRetVal;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void DumpMemoryFree(bool bImmediate)
{
  static int freeMemoryKeep = 2000;
  
  if ( freeMemory() < freeMemoryKeep )
  {
    freeMemoryKeep = freeMemory();
    bImmediate = true;
  }
  
  if (bImmediate)
  {
    Serial.print(F("freeMemory()="));
    Serial.println(freeMemory());
    Serial.flush();
    Serial.print(F("lowestMemory()="));
    Serial.println(freeMemoryKeep);
    Serial.flush();
  }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void SetupWebpageHelp(ETHER_28J60 * pEth)
{
  if ( pEth == 0 ) return;

  unsigned char ii = 0;
  char buffer[80];
  while( pgm_read_word(&UserStringMenuEth[ii]) != 0 )
  {
    strcpy_P(buffer, (char*)pgm_read_word(&(UserStringMenuEth[ii]))); 
    pEth->print( buffer );
    pEth->print_p(PSTR("<br>"));
    ii++;
  };
  pEth->respond();
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void SetupWebpageStatus(ETHER_28J60 * pEth)
{

  ////////////////////////////////////////
  // Title and Board Name  
  ////////////////////////////////////////
  pEth->print_p(PSTR("<br><h1>"));
  pEth->print_p(UserString_Title);
  pEth->print_p(PSTR("</h1><h4>"));
  pEth->print_p(UserString_FwVersion);
  pEth->print_p(PSTR("<br>"));
  pEth->print_p(UserString_Author);

  char tmpName[MAX_BOARDNAME_SIZE+1];
  memset(tmpName, 0, MAX_BOARDNAME_SIZE+1);
  fhemConfiguration.ReadBlock(CfgItemPosBoardName, (uint8_t *)tmpName, (size_t)MAX_BOARDNAME_SIZE );
  pEth->print_p(PSTR("<br>Board Name : "));
  pEth->print(tmpName);
  pEth->print_p(PSTR("</h4><br>"));

  pEth->print_p(PSTR("<A HREF='help'>Help</A>"));
  pEth->print_p(PSTR(" <A HREF='?GET'>Update</A><br>"));

  ////////////////////////////////////////
  // Light
  ////////////////////////////////////////
  unsigned char ii;
  for ( ii = 0; ii < NUM_LIGHTS; ii++ )
  {
    if ( theLights[ii].IsEnabled() )
    {
      pEth->print_p(PSTR("<br>STATE_L"));
      pEth->print(ii+1);
      if ( theLights[ii].IsLightOn() )
      {
        pEth->print_p(PSTR("=ON "));
        pEth->print_p(PSTR("(<A HREF='?CMD_L"));
        pEth->print(ii+1);
        pEth->print_p(PSTR("=OFF'>Turn off</A>)"));
      }
      else
      {
        pEth->print_p(PSTR("=OFF "));
        pEth->print_p(PSTR("(<A HREF='?CMD_L"));
        pEth->print(ii+1);
        pEth->print_p(PSTR("=ON'>Turn on</A>)"));
      }
    }
  }

  ////////////////////////////////////////
  // Window
  ////////////////////////////////////////

  for ( ii = 0; ii < NUM_WINDOWS; ii++ )
  {
    if ( theWindows[ii].IsEnabled() )
    {
      pEth->print_p(PSTR("<br>STATE_W"));
      pEth->print(ii+1);
      if ( theWindows[ii].IsWindowOpen() )
      {
        pEth->print_p(PSTR("=OPEN"));
      }
      else
      {
        pEth->print_p(PSTR("=CLOSED"));
      }

      pEth->print_p(PSTR("<br>STATE_B"));
      pEth->print(ii+1);
      pEth->print_p(PSTR("="));
      pEth->print(theWindows[ii].GetBlindOpenPercent());

      pEth->print_p(PSTR(" (<A HREF='?CMD_B"));
      pEth->print(ii+1);
      pEth->print_p(PSTR("=OPEN'>Open,</A>)"));

      pEth->print_p(PSTR(" (<A HREF='?CMD_B"));
      pEth->print(ii+1);
      pEth->print_p(PSTR("=CLOSE'>Close,</A>)"));

      pEth->print_p(PSTR(" (<A HREF='?CMD_B"));
      pEth->print(ii+1);
      pEth->print_p(PSTR("=CLOSE_FORCE'>Force!</A>)"));

      pEth->print_p(PSTR(" (<A HREF='?CMD_B"));
      pEth->print(ii+1);
      pEth->print_p(PSTR("=STOP'>Stop</A>)"));
    }
  }

  ////////////////////////////////////////
  // Inputs
  ////////////////////////////////////////

  for ( ii = 0; ii < NUM_INPUTS; ii++ )
  {
    pEth->print_p(PSTR("<br>STATE_I"));
    pEth->print(ii+1);
    if ( theInputs[ii].IsHeld() )
    {
      pEth->print_p(PSTR("=ONHOLD"));
    }
    else if ( theInputs[ii].IsPressed() )
    {
      pEth->print_p(PSTR("=ON"));
    }
    else
    {
      pEth->print_p(PSTR("=OFF"));
    }
    
    // Statistics
    pEth->print_p(PSTR(" Count="));
    pEth->print(theInputs[ii].GetPressedCount());
  }
  pEth->print_p(PSTR("<br>"));

  ////////////////////////////////////////
  // Errors
  ////////////////////////////////////////
  pEth->print_p(PSTR("<br>ErrorCount="));
  pEth->print(GetEthReinitCount());
  pEth->print_p(PSTR(", LastError="));
  pEth->print(GetEthReinitReason());
  pEth->print_p(PSTR("<br>"));

  pEth->respond();
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void SetupWebpage(ETHER_28J60 * pEth)
{
  if ( pEth == 0 ) return;

  if (strncmp(pEth->GetLastMessageReceived(), "help", 4) == 0)
  {
    SetupWebpageHelp(pEth);
  }
  else
  {
    SetupWebpageStatus(pEth);
  }
}
