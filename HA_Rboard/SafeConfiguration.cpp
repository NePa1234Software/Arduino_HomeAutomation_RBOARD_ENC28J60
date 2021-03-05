/*
 SafeConfiguration.cpp - Home automation rboard room control
 Copyright (c) 2021 Neil Parker.  All right reserved.
 
 Wrapper for EEPROM.cpp - EEPROM library
 Copyright (c) 2006 David A. Mellis.  All right reserved.
 
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

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "SafeConfiguration.h"
#include <avr/eeprom.h>
#include "Arduino.h"

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

const uint16_t EEPROMStoreClass::PosOfEepromDataMain = 0;
const uint16_t EEPROMStoreClass::PosOfEepromDataMainChecksum = 198;
const uint16_t EEPROMStoreClass::PosOfEepromDataCopy = 200;
const uint16_t EEPROMStoreClass::PosOfEepromDataCopyChecksum = 398;
const uint16_t EEPROMStoreClass::PosOfEepromDataErrors = 400;
const uint16_t EEPROMStoreClass::PosOfEepromDataErrorsAppl = 402;
const uint16_t EEPROMStoreClass::SizeOfEepromData = 200;
const uint16_t EEPROMStoreClass::SizeOfEeprom = 512;

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

EEPROMStoreClass::EEPROMStoreClass()
: bDumpAll(false)
, bInitializing(true)
, bInitializingFailed(false)
, bUpdateChecksum(false)
, tmpAddress(0)
, tmpState(0)
{
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void EEPROMStoreClass::LoopConfiguration()
{
  if ( bInitializing )
  {
    switch( tmpState )
    {
    case 0:

      // Try to repair main section or the backup copy in case there
      // was a power failure during the write process for example

      Serial.print(F("\nTest Eeprom ....."));

      if ( IsChecksumMainOk() )
      {
        if ( !IsChecksumCopyOk() )
        {
          RepareCopy();
          Serial.print(F("..... Repaired copy"));
        }
      }
      else
      {
        if ( IsChecksumCopyOk() )
        {
          RepareMain();
          Serial.print(F("..... Repaired main"));
        }
        else
        {
          // Both sections are defect....what to do?
          bInitializingFailed = true;
        }
      }
      tmpState++;
      break;

    case 1:
      if ( bInitializingFailed )
      {
        Serial.println(F("..... FAILED"));
      }
      else
      {
        Serial.println(F("..... OK"));
      }
      tmpState++;
      break;

    default:
      bInitializing = false;
      break;
    }
  }
  else if ( bDumpAll )
  {
    DumpAddress(tmpAddress);
    ++tmpAddress;
    if ( tmpAddress >= SizeOfEeprom )
    {
      tmpAddress = 0;
      bDumpAll = false;
      Serial.println();
    }
  }
  else if ( bUpdateChecksum )
  {
    // Update the checksum, then perform a copy
    bUpdateChecksum = false;
    uint16_t tmpChecksum = CalcChecksumMain();
    WriteBlock( PosOfEepromDataMainChecksum, (uint8_t *)&tmpChecksum, (size_t)2 );
    RepareCopy();
  }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void EEPROMStoreClass::EraseEeprom()
{
  uint16_t ii = 0;
  for (ii = 0; ii < SizeOfEeprom; ii++)
  {
    eeprom_write_byte((uint8_t *)ii, (uint8_t)0);
  }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void EEPROMStoreClass::DumpEeprom()
{
  Serial.println();
  Serial.println(F("DUMP"));
  Serial.flush();
  bDumpAll = true;
  tmpAddress = 0;
  Serial.println();
  Serial.println();
  Serial.flush();
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void EEPROMStoreClass::DumpAddress( uint16_t dumpAddress )
{
  if ( tmpAddress < SizeOfEeprom )
  {
    // read a byte from the current address of the EEPROM
    uint8_t value = Read(dumpAddress);

    if ( (dumpAddress % 32) == 0 )
    {
      Serial.println();
      Serial.print(dumpAddress);
      Serial.print(F(": "));
    }
    Serial.print(value, DEC);
    Serial.print(F(" "));
  }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

uint16_t EEPROMStoreClass::CalcChecksumMain()
{
  uint16_t ii;
  uint16_t tmpChecksum = 0;
  for ( ii = PosOfEepromDataMain; ii < PosOfEepromDataMainChecksum; ii++ )
  {
    tmpChecksum += (uint16_t)Read( ii );
  }
  //Serial.print("\nChecksum : ");
  //Serial.print(tmpChecksum);
  return tmpChecksum;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

uint16_t EEPROMStoreClass::CalcChecksumCopy()
{
  uint16_t ii;
  uint16_t tmpChecksum = 0;
  for ( ii = PosOfEepromDataCopy; ii < PosOfEepromDataCopyChecksum; ii++ )
  {
    tmpChecksum += (uint16_t)Read( ii );
  }
  return tmpChecksum;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

bool EEPROMStoreClass::IsChecksumMainOk()
{
  uint16_t tmpChecksum = 0;
  ReadBlock( PosOfEepromDataMainChecksum, (uint8_t *)&tmpChecksum, (size_t)2 );
  return tmpChecksum == CalcChecksumMain();
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

bool EEPROMStoreClass::IsChecksumCopyOk()
{
  uint16_t tmpChecksum = 0;
  ReadBlock( PosOfEepromDataCopyChecksum, (uint8_t *)&tmpChecksum, (size_t)2 );
  return tmpChecksum == CalcChecksumCopy();
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void EEPROMStoreClass::RepareMain()
{
  uint16_t ii;
  for ( ii = 0; ii < SizeOfEepromData; ii++ )
  {
    Write( PosOfEepromDataMain + ii, Read(PosOfEepromDataCopy + ii) );
  }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void EEPROMStoreClass::RepareCopy()
{
  uint16_t ii;
  for ( ii = 0; ii < SizeOfEepromData; ii++ )
  {
    Write( PosOfEepromDataCopy + ii, Read(PosOfEepromDataMain + ii) );
  }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

uint8_t EEPROMStoreClass::Read( uint16_t pos )
{
  return eeprom_read_byte( (const uint8_t *)pos );
}

void EEPROMStoreClass::ReadBlock(uint16_t pos, uint8_t * destData, size_t count )
{
  while( (destData != 0) && count )
  {
    *destData = Read( pos );
    destData++;
    pos++;
    count--;
  }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void EEPROMStoreClass::WriteNoChecksum( uint16_t pos, uint8_t writeData )
{
  eeprom_write_byte((uint8_t *)pos, writeData);
}

void EEPROMStoreClass::Write(uint16_t pos, uint8_t writeData )
{
  eeprom_write_byte((uint8_t *)pos, writeData);
  if ( (pos >= PosOfEepromDataMain) && (pos < PosOfEepromDataMainChecksum) )
  {
    bUpdateChecksum = true;
  }
}

void EEPROMStoreClass::WriteBlock(uint16_t pos, uint8_t * sourceData, size_t count )
{
  while( (sourceData != 0) && count )
  {
    Write( pos, *sourceData );
    sourceData++;
    pos++;
    count--;
  }
}
