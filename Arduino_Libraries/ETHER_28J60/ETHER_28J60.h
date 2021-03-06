/*
  ETHER_28J60.h - Ethernet library
  Copyright (c) 2010 Simon Monk.  All right reserved.

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

#ifndef ETHER_28J60_h
#define ETHER_28J60_h

#include <inttypes.h>

#define ETHER_28J60_BUFFER_SIZE    1000
#define STR_BUFFER_SIZE            32

class ETHER_28J60
{
  public:

    ETHER_28J60();

    // Setup and initialize the hardware and receive filters
    bool setup(uint8_t macAddress[], uint8_t ipAddress[], uint16_t port, uint8_t csPin);

    // Get a reference to the ethershield class, so we can utilize the methods
    EtherShield & GetEtherShield();

    // Message request handling methods
    bool serviceRequest();      // returns a true on reception of UDP or TCP message
    void print(char* text);     // append the text to the response
    void print_p(const char * text PROGMEM);
    void print(int value);      // append the number to the response
    void respond();             // write the final response
    void respondUdp(char * data, uint8_t datalen);

    // Logging activation methods
    void ActivateLogging( bool bLogging );
    void ActivateLoggingEx( bool bLogging );

    // Logging methods
    void DumpFrame();
    void DumpFrameIpHeader();
    void DumpFrameUdpHeader();
    void DumpFrameTcpHeader();
    void DumpRegisters();

    // Check if the last received message was of UDP type
    bool IsUdpMessageReceived();

    // Check if the last received message was of UDP type
    bool IsTcpMessageReceived();

    // Fetch the last received UDP/TCP message (is zero terminated)
    char * GetLastMessageReceived();

    // Fetch the number of errors
    unsigned char GetErrorCount()
    {
	  return ErrorCount;
	}

    void ResetErrorCount()
    {
      ErrorCount = 0;
    }

    // Fetch the last error reason
    unsigned char GetErrorReason()
    {
      return ErrorReason;
    }

  private:

    void CheckAndLogSetupError();
    void CheckAndLogRuntimeError();
    unsigned char ErrorCount;
    unsigned char ErrorReason;
};

#endif
