/*
 SafeConfiguration.h - Home automation rboard room control
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

#ifndef SAFECONFIGURATION_h
#define SAFECONFIGURATION_h

#include <arduino.h>

class EEPROMStoreClass
{
public:

  EEPROMStoreClass();

  // Data access routines
  uint8_t Read( uint16_t pos );
  void ReadBlock( uint16_t pos, uint8_t * destData, size_t count );

  void Write( uint16_t pos, uint8_t writeData );
  void WriteBlock( uint16_t pos, uint8_t * sourceData, size_t count );

  // Just write without updating the checksum (for testing)
  void WriteNoChecksum( uint16_t pos, uint8_t writeData );

  // Set eeprom to 0
  void EraseEeprom();

  // Handle all requests as background task
  void LoopConfiguration();

  // Request dumping (in background)
  void DumpEeprom();

  // Wait until the initialization is complete (true)
  bool IsInitializingConfiguration(void)
  {
    return bInitializing;
  };

  // Define the position and size of the data stores
  static const uint16_t PosOfEepromDataMain;
  static const uint16_t PosOfEepromDataMainChecksum;
  static const uint16_t PosOfEepromDataCopy;
  static const uint16_t PosOfEepromDataCopyChecksum;
  static const uint16_t PosOfEepromDataErrors;
  static const uint16_t PosOfEepromDataErrorsAppl;
  static const uint16_t SizeOfEepromData;
  static const uint16_t SizeOfEeprom;

private:

  // Helper
  void DumpAddress( uint16_t dumpAddress );
  uint16_t CalcChecksumMain();
  uint16_t CalcChecksumCopy();
  bool IsChecksumMainOk();
  bool IsChecksumCopyOk();
  void RepareMain();
  void RepareCopy();

  // Temporary variables for initialization and dumping
  bool bDumpAll;
  bool bInitializing;
  bool bInitializingFailed;
  bool bUpdateChecksum;
  unsigned int tmpAddress;
  unsigned char tmpState;
};
#endif
