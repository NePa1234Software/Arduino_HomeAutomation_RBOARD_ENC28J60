################################################################
#
#  Copyright notice
#
#  (c) 2017 Neil Parker
#
#  This script is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  The GNU General Public License can be found at
#  http://www.gnu.org/copyleft/gpl.html.
#  A copy is found in the textfile GPL.txt and important notices to the license
#  from the author is found in LICENSE.txt distributed with these scripts.
#
#  This script is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  This copyright notice MUST APPEAR in all copies of the script!
#
################################################################
# $Id:  $

##############################################
package main;

use strict;
use warnings;
#use Data::Dumper;
use HTTP::Request;
use HttpUtils;

my %commandsmap = (
  "on" => "ON",
  "off" => "OFF",
  "stop" => "STOP",
  "open" => "OPEN",
  "close" => "CLOSE",
  "force" => "CLOSE_FORCE",
  "up" => "OPEN",
  "down" => "CLOSE",
  "10" => "POS=10",
  "20" => "POS=20",
  "30" => "POS=30",
  "40" => "POS=40",
  "50" => "POS=50",
  "60" => "POS=60",
  "70" => "POS=70",
  "80" => "POS=80",
  "90" => "POS=90",
  "100" => "OPEN",
);

#  "CLOSED" => "signal_Fenster_Offen.off",
 # "OPEN" => "signal_Fenster_Offen.on",
my %statesmap = (
  "I_ON" => "closed",
  "I_OFF" => "open",
  "I_ONHOLD" => "closed",
  "ONHOLD" => "on",
  "ON" => "on",
  "OFF" => "off",
  "CLOSED" => "closed",
  "OPEN" => "open",
  "0" => "shutter_closed",
  "1" => "shutter_7",
  "2" => "shutter_7",
  "3" => "shutter_7",
  "4" => "shutter_7",
  "5" => "shutter_7",
  "6" => "shutter_7",
  "7" => "shutter_7",
  "8" => "shutter_7",
  "9" => "shutter_7",
  "10" => "shutter_7",
  "11" => "shutter_7",
  "12" => "shutter_7",
  "13" => "shutter_7",
  "14" => "shutter_7",
  "15" => "shutter_7",
  "16" => "shutter_6",
  "17" => "shutter_6",
  "18" => "shutter_6",
  "19" => "shutter_6",
  "20" => "shutter_6",
  "21" => "shutter_6",
  "22" => "shutter_6",
  "23" => "shutter_6",
  "24" => "shutter_6",
  "25" => "shutter_6",
  "26" => "shutter_6",
  "27" => "shutter_6",
  "28" => "shutter_6",
  "29" => "shutter_6",
  "30" => "shutter_5",
  "31" => "shutter_5",
  "32" => "shutter_5",
  "33" => "shutter_5",
  "34" => "shutter_5",
  "35" => "shutter_5",
  "36" => "shutter_5",
  "37" => "shutter_5",
  "38" => "shutter_5",
  "39" => "shutter_5",
  "40" => "shutter_5",
  "41" => "shutter_5",
  "42" => "shutter_5",
  "43" => "shutter_5",
  "44" => "shutter_4",
  "45" => "shutter_4",
  "46" => "shutter_4",
  "47" => "shutter_4",
  "48" => "shutter_4",
  "49" => "shutter_4",
  "50" => "shutter_4",
  "51" => "shutter_4",
  "52" => "shutter_4",
  "53" => "shutter_4",
  "54" => "shutter_4",
  "55" => "shutter_4",
  "56" => "shutter_4",
  "57" => "shutter_4",
  "58" => "shutter_3",
  "59" => "shutter_3",
  "60" => "shutter_3",
  "61" => "shutter_3",
  "62" => "shutter_3",
  "63" => "shutter_3",
  "64" => "shutter_3",
  "65" => "shutter_3",
  "66" => "shutter_3",
  "67" => "shutter_3",
  "68" => "shutter_3",
  "69" => "shutter_3",
  "70" => "shutter_3",
  "71" => "shutter_3",
  "72" => "shutter_2",
  "73" => "shutter_2",
  "74" => "shutter_2",
  "75" => "shutter_2",
  "76" => "shutter_2",
  "77" => "shutter_2",
  "78" => "shutter_2",
  "79" => "shutter_2",
  "80" => "shutter_2",
  "81" => "shutter_2",
  "82" => "shutter_2",
  "83" => "shutter_2",
  "84" => "shutter_2",
  "85" => "shutter_2",
  "86" => "shutter_1",
  "87" => "shutter_1",
  "88" => "shutter_1",
  "89" => "shutter_1",
  "90" => "shutter_1",
  "91" => "shutter_1",
  "92" => "shutter_1",
  "93" => "shutter_1",
  "94" => "shutter_1",
  "95" => "shutter_1",
  "96" => "shutter_1",
  "97" => "shutter_1",
  "98" => "shutter_1",
  "99" => "shutter_1",
  "100" => "shutter_open",
  "255" => "shutter_open",
);

sub
WEBIO_NP_Initialize($)
{
  my ($hash) = @_;

  $hash->{SetFn}     = "WEBIO_NP_Set";
  $hash->{DefFn}     = "WEBIO_NP_Define";
  $hash->{AttrList}  = "loglevel:0,1,2,3,4,5,6 setList:on,off,stop,up,down,force,90,80,70,60,50,30,20,10 ";

  #print "WEBIO_NP_Initialize\n";
}

###################################
sub
WEBIO_NP_Define($$)
{
  my ($hash, $def) = @_;
  my @a = split("[ \t][ \t]*", $def);

  if((int(@a) == 5) && ($a[2] eq "MASTER"))
  {
    $hash->{Master_Slave}=$a[2];
    $hash->{Host_Port} = 80;
    $hash->{Host_Port} = $a[3] if $a[3];
    $hash->{Delay}=5;
    $hash->{Delay}=$a[4] if $a[4];
    $hash->{Delaytmp} = $hash->{Delay};
    $modules{WEBIO_NP}{defptr}{Master_Ptr} = $hash;

    print "WEBIO_NP_Define (MASTER) ".$hash->{NAME}."\n";

    InternalTimer(gettimeofday()+$hash->{Delay}, "WEBIO_NP_GetStatus", $hash, 1);
  }
  elsif((int(@a) == 5) && ($a[2] eq "SLAVE"))
  {
    return "Error. Create the master first." if (!$modules{WEBIO_NP}{defptr}{Master_Ptr});
    $hash->{Master_Slave}=$a[2];
    $hash->{Connection}="connecting...";
    $hash->{Channel} = $a[3];
    $hash->{Host} = $a[4];
    $hash->{URL} = "http://".$hash->{Host}.":".$modules{WEBIO_NP}{defptr}{Master_Ptr}->{Host_Port}."/";
    $hash->{Delay}=$modules{WEBIO_NP}{defptr}{Master_Ptr}->{Delay};
    $hash->{Delaytmp} = $hash->{Delay};
    $hash->{STATE} = "Init";
    $hash->{READINGS}{LastError}{VAL} = "???";
    $modules{WEBIO_NP}{defptr}{Slave_Ptrs}{$hash->{NAME}} = $hash;

    print "WEBIO_NP_Define (SLAVE) ".$hash->{NAME}." ".$hash->{URL}." Channel = ".$hash->{Channel}."\n";
  }
  else
  {
    return "Wrong syntax: use define <name> WEBIO_NP <1: MASTER, SLAVE> <2: L1..4, B2..2, W1..2, I1..8> <3: ip-address (Slave)> <2: port-nr (Master)> <3:   poll-delay (Master)> ";
  }
 
  return undef;
}

###################################
sub
WEBIO_NP_Set($@)
{
  my ($hash, @a) = @_;

  return "no set value specified" if(int(@a) != 2);
 
  # set up the set list depending on channel type
  if ($a[1] eq "?")
  {
    return "Unknown argument $a[1], choose one of on off" if($hash->{Channel} =~ /L/);
    return "Unknown argument $a[1], choose one of open closed" if($hash->{Channel} =~ /W/);
    return "Unknown argument $a[1], choose one of none" if($hash->{Channel} =~ /I/);
    return "Unknown argument $a[1], choose one of stop up down force 90 80 70 60 50 40 30 20 10" if($hash->{Channel} =~ /B/);
    return "Unknown argument $a[1], choose one of on off stop up down force 90 80 70 50 40 30 20 10";
  }

  WEBIO_NP_execute($hash,$a[1]);

  Log GetLogLevel($a[0],2), "WEBIO_NP set @a";
  return undef;
}

###################################
sub
WEBIO_NP_execute($@)
{
    my ($hash,$cmd) = @_;

    #Map the command to the interface HTTP else take command directly
    my $cmdmap = $cmd;
    $cmdmap = $commandsmap{$cmd};
    $cmdmap = $cmd if(!defined($cmdmap));
    print $cmd." -> ".$cmdmap."\n";

    my $URL=$hash->{URL}."?CMD_".$hash->{Channel}."=".$cmdmap;
    #print "WEBIO_NP_execute ".$hash->{NAME}." ".$URL."\n";

    #$hash->{Delaytmp} = $hash->{Delay}; 
      
    HttpUtils_NonblockingGet({
                                url        => $URL,
                                timeout    => 3,
                                noshutdown => 1,
                                loglevel   => 5,
                                hash       => $hash,
                                callback   => \&WEBIO_NP_ParseResponse
                            });

    return undef;
}

#####################################

sub
WEBIO_NP_GetStatus($)
{
  my ($hash) = @_;
  #print "Timer\n";
  InternalTimer(gettimeofday()+$hash->{Delay}, "WEBIO_NP_GetStatus", $hash, 0);
  return if (!$init_done);
  
  my $tmpURL = "Dummy";
  foreach my $mydevname (%{$modules{WEBIO_NP}{defptr}{Slave_Ptrs}}) 
  {
    my $mydev = $modules{WEBIO_NP}{defptr}{Slave_Ptrs}{$mydevname};
    #print $mydev->{NAME}." - Found -> ".$mydev->{URL}."\n";
    #print "Found -> ".$modules{WEBIO_NP}{defptr}{Slave_Ptrs}{$mydevname}->{URL}."\n";
    
    if (!$mydev->{URL})
    {
        #print $mydev->{NAME}." - No URL Found !!!\n";
    }
    elsif ($tmpURL ne $mydev->{URL})
    {
      $tmpURL = $mydev->{URL};
      
      my $URL_Command=$tmpURL."?GET";
  
      Log3 $hash, 5, "Sending -> ".$mydev->{NAME}." - ".$URL_Command;

      HttpUtils_NonblockingGet({
                            url        => $URL_Command,
                            timeout    => 3,
                            noshutdown => 1,
                            loglevel   => 5,
                            hash       => $hash,
                            callback   => \&WEBIO_NP_ParseResponse
                        });
       
       
      #Log3 $hash, 5, "Sent -> ".$mydev->{NAME}." - ".$URL_Command;
    }
  }
}

#############################
# parses the receiver response
sub 
WEBIO_NP_ParseResponse($$$)
{
    my ( $param, $err, $data ) = @_;    
    my $hash = $param->{hash};
    
    if ($hash->{Master_Slave} eq "MASTER")
    {
      foreach my $mydevname (%{$modules{WEBIO_NP}{defptr}{Slave_Ptrs}}) 
      {
        my $mydev = $modules{WEBIO_NP}{defptr}{Slave_Ptrs}{$mydevname};
        #print "Found -> ".$modules{WEBIO_NP}{defptr}{Slave_Ptrs}{$mydevname}->{URL}."\n";
    
        my $URL_Command="???";
        $URL_Command=$mydev->{URL}."?GET" if ($mydev->{URL});

        if ($URL_Command eq $param->{url})
        {
          Log3 $hash, 5, $mydev->{NAME}." - Parse GET Response -> ".$mydev->{URL};
          WEBIO_NP_ParseResponseInt($mydev, $err, $data);
        }      
      }
    }
    else
    {
      Log3 $hash, 5, $hash->{NAME}." - Parse CMD Response -> ".$hash->{URL};
      WEBIO_NP_ParseResponseInt($hash, $err, $data);
    }
}

sub 
WEBIO_NP_ParseResponseInt($$$)
{
    my ( $hash, $err, $data ) = @_;    
    
    my $state="???";
    my $channel = $hash->{Channel};
    my $channelstate="STATE_".$channel;
    
    #print "Error: ".$err."\n";
    #print "Data : ".$data."\n";
    
    if($err ne "")
    {
        Log3 $hash, 0, "WEBIO_NP error ".$hash->{NAME}." - ".$err if ($hash->{Delaytmp} < 60);
        $hash->{Delaytmp} += 5 if ($hash->{Delaytmp} < 60); 
        $hash->{Connection} = "disconnected" if ($hash->{Delaytmp} >= 60);
        $state = "noreply";
        $state = "disconnected" if ($hash->{Delaytmp} >= 60);
    }
    elsif($data eq "")
    {
        Log3 $hash, 0, "WEBIO_NP data error ".$hash->{NAME};
        $hash->{Delaytmp} += 5 if ($hash->{Delaytmp} < 60); 
        $hash->{Connection}="disconnected(2)" if ($hash->{Delaytmp} >= 60);
        $state = "noreply(2)";
        $state = "disconnected" if ($hash->{Delaytmp} >= 60);
    }
    else
    {
        Log3 $hash, 5, "WEBIO_NP data - ".$hash->{NAME}." - ".$data;
 
        $hash->{Delaytmp} = $hash->{Delay};
        Log3 $hash->{NAME}, 0, "WEBIO_NP device - ".$hash->{NAME}." - Connected." if ($hash->{Connection} ne "connected");
        $hash->{Connection} = "connected";

        my $body =  $data;
        my $staterx="???";
        my @bodysplit = split("<br>", $body);
      
        #print $channel.">\n";
        #$hash->{READINGS}{$channel}{URL} = $body;
        $hash->{READINGS}{$channel}{VAL} = $body;

        foreach my $line (@bodysplit) 
        {
            #print "\n: ".$line;
         
            if ( $line =~ /$channelstate/ )
            {
                my @linesplit = split(" ", $line);
                my @statussplit = split("=", $linesplit[0]);
                #print $channelstate." ---> ".$line."\n";
                #print $channelstate."  ---> ".$linesplit[0]."\n";
                $staterx = $statussplit[1];
                $state = $statesmap{$staterx};
                $state = $statesmap{"I_".$staterx} if($hash->{Channel} =~ /I/);
                #print $channelstate."    ---> ".$statussplit[0]." -> ".$staterx." -> ".$state."\n";
            }
            
            if ( $line =~ /ErrorCount/ )
            {
                #ErrorCount=1, LastError=0

                if($line ne $hash->{READINGS}{LastError}{VAL})
                {
                    $hash->{READINGS}{LastError}{TIME} = TimeNow();
                    $hash->{READINGS}{LastError}{VAL} = $line;
                    Log3 $hash, 0, "RECEIVE Error> ".$hash->{NAME}." : ".$channel." : ".$hash->{READINGS}{LastError}{VAL};
                }
            }
        }
    }
     
    if($state ne "???")
    {
        if($state ne $hash->{STATE})
        {
            Log3 $hash, 0, "RECEIVE> ".$hash->{NAME}." : ".$channel." : ".$hash->{STATE}."->".$state;
            $hash->{STATE} = $state;
            $hash->{CHANGED}[0] = $state;
            $hash->{READINGS}{$state}{TIME} = TimeNow();
            $hash->{READINGS}{$state}{VAL} = $state;
            DoTrigger($hash->{NAME}, undef) if($init_done);
        }
    }
}

1;

=pod
=begin html

<a name="WEBIO_NP"></a>
<h3>WEBIO_NP</h3>
<ul>
  Note: 03.06.2015 this module needs the HTTP::Request and LWP::UserAgent perl modules.
  <br><br>
  <a name="WEBIO_NPdefine"></a>
  <b>Define</b>
  <ul>
    <code>define &lt;name&gt; WEBIO_NP &lt;channel&gt; &lt;ip-address&gt; &lt;port&gt; &lt;delay&gt;</code>
    <br><br>
    Define a web based RBoard from ITead (Arduino based). The boards are used to control Lights, Blinds, Windows, Doors, Switches etc.
    <br>
    channel W1...2 (window contact open:closed), L1...4 (light on:off), B1...2 (Blind stop:up:90:80:70:60:50:40:30:20:10:down:force)
    <br><br>

    Example:
    <ul>
      <code>define og_B_Bathroom_B1 WEBIO_NP B1 192.168.178.104 80 5</code><br>
      <code>attr og_B_Bathroom_B1 room Bathroom</code><br>
      <code>attr og_B_Bathroom_B1 webCmd stop:up:90:80:70:60:50:40:30:20:10:down:force</code><br>
    </ul>
  
  <br>
  </ul>

</ul>

=end html
=cut
