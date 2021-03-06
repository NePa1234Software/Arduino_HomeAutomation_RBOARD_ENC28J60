/*
ETHER_28J60.cpp - Ethernet library
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

/******************************************************************************
 * Includes
 ******************************************************************************/

extern "C" {
#include "enc28j60.h"
}
#include "etherShield.h"
#include "ETHER_28J60.h"

//#include "WProgram.h" // Arduino 0.23
#include "Arduino.h" // Arduino 1.0 -- 2011 12 15 # Ben Schueler

/******************************************************************************
 * Definitions
 ******************************************************************************/

EtherShield es=EtherShield();

static bool bConstructed = false;

/******************************************************************************
 * Constructors
 ******************************************************************************/

/******************************************************************************
 * User API
 ******************************************************************************/

uint16_t plen = 0;
uint8_t buf[ETHER_28J60_BUFFER_SIZE+1];
uint16_t _port = 0;
uint8_t  _initialized = false;
bool mLoggingActive = false;
bool mLoggingActiveEx = false;
bool bUdpReceived = false;
bool bTcpReceived = false;
char strbuf[STR_BUFFER_SIZE+1];

ETHER_28J60::ETHER_28J60()
: ErrorCount(0)
, ErrorReason(0)
{
    memset(strbuf, 0, sizeof(strbuf));
    memset(buf, 0, sizeof(buf));
}

bool ETHER_28J60::setup(uint8_t macAddress[], uint8_t ipAddress[], uint16_t port, uint8_t csPin)
{
    _port = port;
    _initialized = 0;
    ErrorCount = 0;

    Serial.flush();
    if ( bConstructed )
    {
        Serial.print(F("\n Constructed "));
    }

    if ( mLoggingActive )
    {
      Serial.print(F("\n InitEth "));
    }

    es.ES_enc28j60Init(macAddress, csPin);
    CheckAndLogSetupError();
    es.ES_enc28j60clkout(2); // change clkout from 6.25MHz to 12.5MHz
    CheckAndLogSetupError();
    delay(1);
    es.ES_enc28j60PhyWrite(PHLCON,0x880);
    CheckAndLogSetupError();
    delay(1);
    es.ES_enc28j60PhyWrite(PHLCON,0x990);
    CheckAndLogSetupError();
    delay(1);
    es.ES_enc28j60PhyWrite(PHLCON,0x880);
    CheckAndLogSetupError();
    delay(1);
    es.ES_enc28j60PhyWrite(PHLCON,0x990);
    CheckAndLogSetupError();
    delay(1);
    es.ES_enc28j60PhyWrite(PHLCON,0x476);
    CheckAndLogSetupError();
    delay(1);
    es.ES_init_ip_arp_udp_tcp(macAddress, ipAddress, _port);
    CheckAndLogSetupError();

    if ( mLoggingActive )
    {
      Serial.print(F(" Done\n"));
    }

    // Initialize UDP buffer
    bUdpReceived = false;
    bTcpReceived = false;
    memset(strbuf, 0, sizeof(strbuf));

    Serial.flush();
    return (_initialized == 0);
}

EtherShield & ETHER_28J60::GetEtherShield()
{
    return es;
}

void ETHER_28J60::CheckAndLogSetupError()
{
    uint8_t theError = 0;

    theError = es.ES_enc28j60GetLastError();
    if ( theError == 0 )
    {
       Serial.print(F("."));
    }
    else
    {
       Serial.print(char('0'+theError));
       _initialized++;
    }
}

void ETHER_28J60::CheckAndLogRuntimeError()
{
    uint8_t theError = 0;

    theError = es.ES_enc28j60GetLastError();

    if ( theError == 0 )
    {
      uint8_t result = es.ES_enc28j60Revision();
      if ( result == 0 || result == 0xff )
      {
        theError = 8;
      }
    }

    if ( theError != 0 )
    {
       ErrorReason = theError;
       if ( ErrorCount < 10 )
       {
         ErrorCount++;
         Serial.print(char('0'+theError));
       }
    }
}

void ETHER_28J60::DumpRegisters()
{
    Serial.flush();
    Serial.print(F("--- ETH Registers ---\n"));
    Serial.flush();

    uint8_t tmpReg;

    tmpReg = es.ES_enc28j60Revision();
    Serial.print(F("\n REV   = 0x"));
    Serial.print(tmpReg, HEX);

    tmpReg = enc28j60Read(EIE);
    Serial.print(F("\n EIE   = 0x"));
    Serial.print(tmpReg, HEX);

    tmpReg = enc28j60Read(EIR);
    Serial.print(F("\n EIR   = 0x"));
    Serial.print(tmpReg, HEX);

    tmpReg = enc28j60Read(ESTAT);
    Serial.print(F("\n ESTAT = 0x"));
    Serial.print(tmpReg, HEX);

    tmpReg = enc28j60Read(ECON1);
    Serial.print(F("\n ECON1 = 0x"));
    Serial.print(tmpReg, HEX);

    tmpReg = enc28j60Read(ECON2);
    Serial.print(F("\n ECON2 = 0x"));
    Serial.print(tmpReg, HEX);

    tmpReg = enc28j60Read(MAADR5);
    Serial.print(F("\n MAADR5 = 0x"));
    Serial.print(tmpReg, HEX);

    tmpReg = enc28j60Read(MAADR4);
    Serial.print(F("\n MAADR4 = 0x"));
    Serial.print(tmpReg, HEX);

    tmpReg = enc28j60Read(MAADR3);
    Serial.print(F("\n MAADR3 = 0x"));
    Serial.print(tmpReg, HEX);

    tmpReg = enc28j60Read(MAADR2);
    Serial.print(F("\n MAADR2 = 0x"));
    Serial.print(tmpReg, HEX);

    tmpReg = enc28j60Read(MAADR1);
    Serial.print(F("\n MAADR1 = 0x"));
    Serial.print(tmpReg, HEX);

    tmpReg = enc28j60Read(MAADR0);
    Serial.print(F("\n MAADR0 = 0x"));
    Serial.print(tmpReg, HEX);

    Serial.print(F("\n---- "));
}

void ETHER_28J60::DumpFrame()
{
    char tmpStr[32];   // Be careful that the buffer doesnt overflow when using ES_mk_net_str
    unsigned int startData = 0;

    Serial.print(F("--- ETH Frame Length = "));
    Serial.println(plen);
    Serial.flush();
    Serial.print(F("--- ETH Header ---\n"));
    Serial.flush();

    es.ES_mk_net_str(tmpStr,&buf[ETH_DST_MAC],6,'.',16);
    Serial.println(tmpStr);
    Serial.flush();

    es.ES_mk_net_str(tmpStr,&buf[ETH_SRC_MAC],6,'.',16);
    Serial.println(tmpStr);
    Serial.flush();

    es.ES_mk_net_str(tmpStr,&buf[ETH_TYPE_H_P],2,'.',10);
    Serial.println(tmpStr);
    Serial.flush();

    Serial.print(F("- IP -\n"));
    Serial.flush();

    es.ES_mk_net_str(tmpStr,&buf[IP_P],6,'.',10);
    Serial.print(tmpStr);
    es.ES_mk_net_str(tmpStr,&buf[IP_P+6],6,'.',10);
    Serial.println(tmpStr);
    Serial.flush();

    es.ES_mk_net_str(tmpStr,&buf[IP_SRC_IP_P],4,'.',10);
    Serial.println(tmpStr);
    Serial.flush();

    es.ES_mk_net_str(tmpStr,&buf[IP_DST_IP_P],4,'.',10);
    Serial.println(tmpStr);
    Serial.flush();

    if (buf[IP_PROTO_P]==IP_PROTO_TCP_V)
    {
        Serial.print(F("- TCP -\n"));
        Serial.flush();

        es.ES_mk_net_str(tmpStr,&buf[TCP_SRC_PORT_H_P],2,'.',10);
        Serial.println(tmpStr);
        Serial.flush();

        es.ES_mk_net_str(tmpStr,&buf[TCP_DST_PORT_H_P],2,'.',10);
        Serial.println(tmpStr);
        Serial.flush();

        //es.ES_mk_net_str(tmpStr,&buf[TCP_SEQ_P],4,'.',10);
        //Serial.println(tmpStr);
        //es.ES_mk_net_str(tmpStr,&buf[TCP_FLAGS_P],7,'.',10);
        //Serial.println(tmpStr);
        startData = TCP_DATA_P;
    }
    else if (buf[IP_PROTO_P]==IP_PROTO_UDP_V)
    {
        Serial.print(F("- UDP -\n"));
        Serial.flush();

        es.ES_mk_net_str(tmpStr,&buf[UDP_SRC_PORT_H_P],2,'.',10);
        Serial.println(tmpStr);
        Serial.flush();

        es.ES_mk_net_str(tmpStr,&buf[UDP_DST_PORT_H_P],2,'.',10);
        Serial.println(tmpStr);
        Serial.flush();

        es.ES_mk_net_str(tmpStr,&buf[UDP_LEN_H_P],4,'.',10);
        Serial.println(tmpStr);
        Serial.flush();

        startData = UDP_DATA_P;
    }
    else
    {
        startData = 0x22;
    }

    // Data

    Serial.print(F("- DATA -"));
    Serial.flush();

    for ( unsigned int ii=startData;ii<plen;ii++)
    {
      if ( (ii-startData)%50 == 0 )
      {
        Serial.print(F("\n"));
        Serial.flush();
      }
      Serial.print((char)buf[ii]);
    }
    Serial.println(F("\n--"));
    Serial.flush();
}

bool ETHER_28J60::serviceRequest()
{
    uint16_t dat_p;
    //int8_t cmd;
    bUdpReceived = false;
    bTcpReceived = false;

    // Log any errors occuring that may be caused by interference etc
    CheckAndLogRuntimeError();
    
    memset(buf,'*',ETHER_28J60_BUFFER_SIZE);
    buf[ETHER_28J60_BUFFER_SIZE] = 0;
    
    plen = es.ES_enc28j60PacketReceive(ETHER_28J60_BUFFER_SIZE, buf);

    /*plen will ne unequal to zero if there is a valid packet (without crc error) */
    if(plen!=0)
    {
        // arp is broadcast if unknown but a host may also verify the mac address by sending it to a unicast address.
        if (es.ES_eth_type_is_arp_and_my_ip(buf, plen))
        {
          es.ES_make_arp_answer_from_request(buf);
          if (mLoggingActive)
          {
              Serial.print(F("\nARP!\n"));
          }
          if (mLoggingActiveEx)
          {
              DumpFrame();
          }
          return false;
        }

        // check if ip packets are for us:
        if (es.ES_eth_type_is_ip_and_my_ip(buf, plen) == 0)
        {
          if (mLoggingActiveEx == true)
          {
              Serial.print(F("\nNOT For ME::::!\n"));
              DumpFrame();
          }
          return false;
        }

        if (mLoggingActive == true)
        {
            Serial.print(F("\n ... IP match ... \n"));
            if (mLoggingActiveEx) DumpFrame();
        }

        // Check of ICMP type and respond
        if (buf[IP_PROTO_P]==IP_PROTO_ICMP_V && buf[ICMP_TYPE_P]==ICMP_TYPE_ECHOREQUEST_V)
        {
          es.ES_make_echo_reply_from_request(buf, plen);
          if (mLoggingActive) Serial.print(F("Echo Request!"));
          return false;
        }

        // UDP message reception
        if (buf[IP_PROTO_P]==IP_PROTO_UDP_V&&buf[TCP_DST_PORT_H_P]==0&&buf[TCP_DST_PORT_L_P] == _port)
        {
            bUdpReceived = true;
            memset(strbuf, 0, sizeof(strbuf));
            unsigned char tmpLength = buf[UDP_LEN_L_P];
            if ( tmpLength > STR_BUFFER_SIZE )
            {
                tmpLength = STR_BUFFER_SIZE;
            }
            memcpy(strbuf, &buf[UDP_DATA_P], tmpLength);
            if (mLoggingActive)
            {
                Serial.print(F("UDP Message: "));
                Serial.println(strbuf);
            }
            return true;
        }

        // tcp port www start, compare only the lower byte
        if (buf[IP_PROTO_P]==IP_PROTO_TCP_V&&buf[TCP_DST_PORT_H_P]==0&&buf[TCP_DST_PORT_L_P] == _port)
        {
            if (buf[TCP_FLAGS_P] & TCP_FLAGS_SYN_V)
            {
                es.ES_make_tcp_synack_from_syn(buf); // make_tcp_synack_from_syn does already send the syn,ack
                if (mLoggingActive) Serial.print(F("4!"));
                return false;
            }
            if (buf[TCP_FLAGS_P] & TCP_FLAGS_ACK_V)
            {
                es.ES_init_len_info(buf); // init some data structures
                dat_p=es.ES_get_tcp_data_pointer();
                if (dat_p==0)
                {   // we can possibly have no data, just ack:
                    if (buf[TCP_FLAGS_P] & TCP_FLAGS_FIN_V)
                    {
                        es.ES_make_tcp_ack_from_any(buf);
                    }
                    if (mLoggingActive) Serial.print(F("\nTCP NO DATA!\n"));
                    return false;
                }

                if (strncmp("GET ",(char *)&(buf[dat_p]),4)!=0)
                {
                    if (mLoggingActive) Serial.print(F("\nNot HTTP GET !\n"));
                    Serial.flush();
                    // head, post and other methods for possible status codes see:
                    // http://www.w3.org/Protocols/rfc2616/rfc2616-sec10.html
                    plen=es.ES_fill_tcp_data_p(buf,0,PSTR("HTTP/1.0 200 OK\r\nContent-Type: text/html\r\nPragma: no-cache\r\n\r\n"));
                    plen=es.ES_fill_tcp_data_p(buf,plen,PSTR("<!DOCTYPE html>"));
                    plen=es.ES_fill_tcp_data_p(buf,plen,PSTR("<h1>200 OK</h1>"));
                    //es.ES_www_server_reply(buf,plen); // send web page data
                    respond();
                    return false;
                }

                if (strncmp("/",(char *)&(buf[dat_p+4]),1)==0) // was "/ " and 2
                {
                    // Copy the request action before we overwrite it with the response
                    int i = 0;
                    while (buf[dat_p+5+i] != ' ' && i < STR_BUFFER_SIZE)
                    {
                        strbuf[i] = buf[dat_p+5+i];
                        i++;
                    }
                    strbuf[i] = '\0';

                    // Prepare the response header
                    plen=es.ES_fill_tcp_data_p(buf,0,PSTR("HTTP/1.0 200 OK\r\nContent-Type: text/html\r\nPragma: no-cache\r\n\r\n"));
                    plen=es.ES_fill_tcp_data_p(buf,plen,PSTR("<!DOCTYPE html>"));
                    if (mLoggingActive)
                    {
                        Serial.print(F("\nHTTP Passed to application.\n"));
                        Serial.flush();
                    }
                    bTcpReceived = true;
                    return true;
                }
                else
                {
                    if (mLoggingActive) Serial.print(F("\nHTTP GET FORMAT ERROR!\n"));
                    return false;
                }
            }
        }
        else
        {
          if (mLoggingActive)
          {
              Serial.flush();
              Serial.print(F("\nWrong Proto / Port! "));
              Serial.print(F(" Proto "));
              Serial.flush();
              Serial.print(buf[IP_PROTO_P]);
              Serial.print(F(" Rx "));
              Serial.flush();
              Serial.print(buf[TCP_DST_PORT_L_P]);
              Serial.print(F(" != Exp ("));
              Serial.flush();
              Serial.print(_port);
              Serial.print(F(")\n"));
              Serial.flush();
              if (mLoggingActiveEx) DumpFrame();
           }
        }
    }
    return false;
}

void ETHER_28J60::print(char* text)
{
    int j = 0;
    while (text[j])
    {
        buf[TCP_CHECKSUM_L_P+3+plen]=text[j++];
        plen++;
    }
}

void ETHER_28J60::print_p(const char * text PROGMEM)
{
    int j = 0;
    char c;
    while ( (c = pgm_read_byte(&text[j]) ) )
    {
        buf[TCP_CHECKSUM_L_P+3+plen]=c;
        j++;
        plen++;
    }
}

void ETHER_28J60::print(int number)
{
  char tempString[9];
  itoa(number, tempString, 10);
  print(tempString);
}

void ETHER_28J60::respond()
{
    es.ES_www_server_reply(buf,plen); // send web page data

    //es.ES_make_tcp_ack_from_any(buf); // send ack for http get
    //es.ES_make_tcp_ack_with_data(buf,plen); // send data

    if (mLoggingActive)
    {
        Serial.println(F("\nSEND:"));
        DumpFrame();
    }
}

void ETHER_28J60::respondUdp(char * data, uint8_t datalen)
{
    es.ES_make_udp_reply_from_request(buf,data,datalen,_port);
}

void ETHER_28J60::ActivateLogging( bool bLogging )
{
    mLoggingActive = bLogging;
    if ( mLoggingActive )
    {
        Serial.print(F("\nLogging ethernet ON\n"));
    }
    else
    {
        mLoggingActiveEx = false;
        Serial.print(F("\nLogging ethernet OFF\n"));
    }
}

void ETHER_28J60::ActivateLoggingEx( bool bLogging )
{
    mLoggingActiveEx = bLogging;
    if ( mLoggingActiveEx )
    {
        mLoggingActive = true;
        Serial.print(F("\nLogging ethernet Extended\n"));
    }
}

bool ETHER_28J60::IsUdpMessageReceived()
{
    return bUdpReceived;
}

// Check if the last received message was of UDP type
bool ETHER_28J60::IsTcpMessageReceived()
{
    return bTcpReceived;
}

char * ETHER_28J60::GetLastMessageReceived()
{
    return strbuf;
}
