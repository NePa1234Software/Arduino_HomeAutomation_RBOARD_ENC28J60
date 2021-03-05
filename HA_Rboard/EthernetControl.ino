/*
  EthernetControl.ino - Home automation rboard room control
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
#include "enc28j60.h"
}
#include "EtherShield.h"
#include "ETHER_28J60.h"

static uint8_t mac[6] = { 0x54, 0x55, 0x58, 0x10, 0x00, 0x99 };                                                            
static uint8_t ip[4] = { 192, 168, 178, 99 };
static uint16_t port = 80;

static ETHER_28J60 * ethernet = 0;

#define DEF_STATE_INIT       0
#define DEF_STATE_INIT_LO    1
#define DEF_STATE_INIT_HI    2
#define DEF_STATE_INIT_SETUP 3
#define DEF_STATE_INIT_MON   4
static unsigned int stateInit = DEF_STATE_INIT;

static bool isEthernetInitialized = false;
static bool enableEthernetService = false;
static unsigned long mEthTimestamp;
static unsigned int mEthReinitCount = 0;
static unsigned char mEthReinitReason = 0;
static const unsigned long mEthTimeResetPin = 50;
static const unsigned long mEthTimeMonitor = 10000;
static unsigned int mEthWatchdog = 1;

static const uint8_t ethResetPin = 8;
static const uint8_t ethChipSelectPin = 9;

void InitEthernet( bool bEnable )
{
  stateInit = 0;
  isEthernetInitialized = false;
  mEthTimestamp = millis();
  enableEthernetService = bEnable;

  // initialize ethernet
  if ( ethernet == 0 )
  {
    Serial.println(F(" ..... Construct"));
    ethernet = new ETHER_28J60();
  }
  Serial.flush();
}

void LoopEthernetControl( void )
{
  if ( enableEthernetService )
  {
    DoInitialization();
    if ( isEthernetInitialized )
    {
      ServiceEthernet();
    }
  }
}

void DoInitialization(void)
{
  unsigned int prevStateInit = stateInit;
  
  switch( stateInit )
  {
    case DEF_STATE_INIT:
    {
      if ( (millis() - mEthTimestamp) >= mEthTimeResetPin )
      {
        Serial.print(F("\nEthernet Board setup .....\n"));
        isEthernetInitialized = false;
        
        // Set SS to high so a connected chip will be "deselected" by default
        digitalWrite(SS, HIGH);
  
        // When the SS pin is set as OUTPUT, it can be used as
        // a general purpose output port (it doesn't influence
        // SPI operations).
        pinMode(SS, OUTPUT);

        // Hardware reset pin, active low
        pinMode(ethResetPin, OUTPUT);
        digitalWrite(ethResetPin, HIGH);
        stateInit = DEF_STATE_INIT_LO;
      }
      break;
    }
    case DEF_STATE_INIT_LO:
    {
      if ( (millis() - mEthTimestamp) >= mEthTimeResetPin )
      {
        digitalWrite(ethResetPin, LOW);
        stateInit = DEF_STATE_INIT_HI;
      }
      break;
    }
    case DEF_STATE_INIT_HI:
    {
      if ( (millis() - mEthTimestamp) >= mEthTimeResetPin )
      {
        digitalWrite(ethResetPin, HIGH);
        stateInit = DEF_STATE_INIT_SETUP;
      }
      break;
    }
    case DEF_STATE_INIT_SETUP:
    {
      if ( (millis() - mEthTimestamp) >= mEthTimeResetPin )
      {
        isEthernetInitialized = ethernet->setup(mac, ip, port, ethChipSelectPin);
        mEthReinitCount++;
  
        if ( isEthernetInitialized )
        {
          Serial.println(F(" ..... OK"));
        }
        else
        {
          Serial.println(F(" ..... FAILED"));
        }
        Serial.flush();
        stateInit = DEF_STATE_INIT_MON;
      }
      break;
    }
    case DEF_STATE_INIT_MON:
    {
      // OK stay here
      if ( (millis() - mEthTimestamp) >= mEthTimeMonitor )
      {
        mEthTimestamp = millis();

        if ( !isEthernetInitialized )
        {
          stateInit = DEF_STATE_INIT;
        }
        else if ( ethernet->GetErrorCount() > 0 )
        {
          mEthReinitReason = ethernet->GetErrorReason();
          Serial.print(F("!"));
          Serial.print(mEthReinitReason);
          stateInit = DEF_STATE_INIT;
        }
        else if ( ethernet->GetEtherShield().ES_enc28j60linkup() == 0 )
        {
           Serial.print(F("*"));
           uint8_t result = ethernet->GetEtherShield().ES_enc28j60Revision();
           Serial.print(result);
           result =  ethernet->GetEtherShield().ES_enc28j60linkup();
           Serial.print(result);
           mEthReinitReason = 10;
           stateInit = DEF_STATE_INIT;
        }
        else if ( !IsEthernetChipConfigured() )
        {
           Serial.print(F("\n!ETH RESET!\n"));
           mEthReinitReason = 11;
           stateInit = DEF_STATE_INIT;
        }
        else
        {
          // all is good
          if (mEthWatchdog > 0)
          {
            Serial.print(F(":"));
            
            if (mEthWatchdog == 1)
            {
              Serial.print(F("\n!ETH WATCHDOG RESET!\n"));
              mEthReinitReason = 12;
              stateInit = DEF_STATE_INIT;
            }
            mEthWatchdog = 1;
          }
        }
      }
      break;
    }
    default:
      stateInit = DEF_STATE_INIT;
      break;
  }
  
  if ( prevStateInit != stateInit )
  {
      mEthTimestamp = millis();
  }
}

bool IsEthernetChipConfigured(void)
{
  bool bRetVal = true;
  uint8_t tmpReg;
  
  tmpReg = enc28j60Read(ECON1);
  if ( (tmpReg & ECON1_RXEN) == 0 )
  {
    bRetVal = false;
  }
  
  tmpReg = enc28j60Read(MAADR5);
  if ( tmpReg != mac[0] )
  {
    bRetVal = false;
  }

  tmpReg = enc28j60Read(MAADR4);
  if ( tmpReg != mac[1] )
  {
    bRetVal = false;
  }

  tmpReg = enc28j60Read(MAADR3);
  if ( tmpReg != mac[2] )
  {
    bRetVal = false;
  }

  tmpReg = enc28j60Read(MAADR2);
  if ( tmpReg != mac[3] )
  {
    bRetVal = false;
  }

  tmpReg = enc28j60Read(MAADR1);
  if ( tmpReg != mac[4] )
  {
    bRetVal = false;
  }

  tmpReg = enc28j60Read(MAADR0);
  if ( tmpReg != mac[5] )
  {
    bRetVal = false;
  }
  return bRetVal;
}


void ServiceEthernet(void)
{
  // Service the ethernet if it is initialized
  if ( ethernet != 0 && ethernet->serviceRequest() )
  {
    //Serial.print(F("\n---------- Msg ------------\n"));

    if ( ethernet->IsTcpMessageReceived() )
    {
      //Serial.print(F("\n------- TCP Command -------\n"));
      DoTcpCommand( ethernet->GetLastMessageReceived() );
      SetupWebpage(ethernet);
      if (mEthWatchdog == 1) mEthWatchdog = 2; 
    }
    else
    {
      if ( ethernet->IsUdpMessageReceived() )
      {
        //Serial.print(F("\n------- UDP Command -------\n"));
        DoUdpCommand( ethernet->GetLastMessageReceived() );
      }
    }
  }
}

void LogLevelEthernet( uint8_t LogLevel )
{
  if ( ethernet == 0 )
  {
  }
  else if ( LogLevel == 0 )
  {
    ethernet->ActivateLogging(false);
  }
  else if ( LogLevel == 1 )
  {
    ethernet->ActivateLogging(true);
  }
  else
  {
    ethernet->ActivateLoggingEx(true);
  }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
/*
void SetEthMacAddress( uint8_t * pmac )
 {
 if ( pmac != 0 )
 {
 memcpy( mac, pmac, 6);
 }
 }
 */

void SetEthIpAddress( uint8_t * pip )
{
  if ( pip != 0 )
  {
    memcpy( ip, pip, 4 );

    // Trick just until we can parse the mac configuration
    mac[4] = pip[2];
    mac[5] = pip[3];
  }
}

unsigned int GetEthReinitCount()
{
  return mEthReinitCount;
}

unsigned char GetEthReinitReason()
{
  return mEthReinitReason;
}

void DumpEthSettings()
{
  if ( ethernet != 0 )
  {
    char tmpStr[20];
    Serial.flush();
    ethernet->GetEtherShield().ES_mk_net_str(tmpStr,mac,6,'.',16);
    Serial.println(tmpStr);
    Serial.flush();
    ethernet->GetEtherShield().ES_mk_net_str(tmpStr,ip,4,'.',10);
    Serial.println(tmpStr);
    Serial.flush();
  }
}

void DumpEthRegisters()
{
  if ( ethernet != 0 )
  {
    ethernet->DumpRegisters();
  }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void DoUdpCommand( char * strByte )
{
  if ( strByte != 0 && ethernet != 0 )
  {
    if ( DoCommand( strByte ) )
    {
      ethernet->respondUdp("GOOD____",8);
    }
    else
    {
      ethernet->respondUdp("BAD____",8);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void DoTcpCommand( char * strByte )
{
  if ( strByte != 0 && ethernet != 0)
  {
    if ( DoCommand( strByte ) )
    {
      //ethernet->print_p(PSTR("<H1>Command OK</H1>"));
    }
    else
    {
      ethernet->print_p(PSTR("<H1>Command Failed!</H1>"));
    }
    //ethernet->respond();
  }
}
