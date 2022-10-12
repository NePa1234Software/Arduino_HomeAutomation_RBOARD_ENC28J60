# HomeAutomation\_RBOARD_ENC28J60
## Introduction

Before I start, it is only fair to mention that I have (2022) replaced this entire setup with Loxone miniservers (in the 24x28 in wall boxes) and Loxone wired tree devices (14x14 in wall boxes). This system is fun to configure and use, DIY hobby friendly, its extremely reliable and the App is professionally implemented rich in features.

This project was inspired by FHEM [Link](https://wiki.fhem.de/wiki/Hauptseite) homeautomation webio module. The arduino relay board from itead - RBOARD [Link](http://www.itead.cc/wiki/RBoard) - using an external ethernet ENC28J60 module provides the cheapest possible homeautomation. The relay board provides 4 relays and a large number of digital inputs for light switches (rockers) and window sensors. 

This is a homebrew arduino based home automation software with http ethernet control. 
The software has been running on 14 boards installed into my house for a few years now. The ethernet chip is very sensitive and can disconnect from time to time. 
The ethernet library from simon monk is modified to ensure maximum possible robustness, in addition a watchdog functionality on top catches the last occasional "problem". 
My newest just started project (coming soon) is generation 2 - using the compact relay board from KMP [Link](http://kmpelectronics.eu/products/prodino-mkr-zero-ethernet-v1/).
Note also the RBOARD is not really wire up friendly, CAT cable with passive PoE can be made using a helpful keystone module etc.

The board is designed to work "stand alone" w/o a home automation server. Each board has its own light switches, window sensor, relays. 

Depending on the number of windows / lights in one room, either 1 or 2 boards are installed inside a "Kaiser connection box 24x14 cm" [Link](https://www.kaiser-elektro.de/de_DE/produkte/elektro-installation/unterputz/verbindungskaesten/83/verbindungskasten-240?c=14). 2 boxes can be linked to one big 24x28 cm in wall box. 

The electricians did all the wiring preparation and are qualified to do so!

The utility room (electrical cupboard telecoms area) ONLY consists of a large patch field, router, 2 x 8 port passive (ALFA) PoE injector modules, 2 x 12 Volts power supplies. Thats it... 

See pictures in the docs folder.

## Features
+ Mode select - 4 lights, 3 lights + gong, 2 lights + window, 2 windows
+ Blinds - configurable up and down time for good positioning. Local control and remote.
+ Windows - open / closed - blinds are not closed if window is open (this ensures I am not locked out when sat on the patio (-; )
+ Gong - switch relay only as long as the bell button is pressed (no toggle)
+ Light - on / off - configurable timer for stairs etc. Press for > 1s will switch to on without timer.
+ Extra safe double eeprom configuration for restoring errors that may occur during write operations (e.g. power cut during write)
+ Maximum possible stability of ENC28J60 http usage. Be warned, this is a really BAD chip (but dirt cheap). There are still the occasional "reconnects".
+ HTML homepage for each board provides a rudimentary control and partially configure the board. 

## FHEM
I am not brilliant at programming perl. The script is still missing some error logging and light toggle function etc. I does its job, but is not pretty ! The module uses a master slave principle. The master is responsible for the board, slave mode just has access to one light, window, sensor etc.
The perl module is in the fhem folder together with my favourite excerts from my fhem configuration file.

## Libraries "borrowed"
Thanks to the authors or the libraries I have used. I should have forked the projects but I am new to github, so will do eventually (-;

+ EEPROM library from David A. Mellis
+ ETHER_28J60 from Simon Monk - this was modified to make it "fit" to my project. Adding error codes and ensuring the while loops end eventually. Disconnected LAN cable and chip (SPI) is detected, configurable chip select pins etc etc.
+ Ethershield from Guido Socher - if I remember correctly, I made a few tweeks to this too.
+ Memory free - taken from the arduino playground - this was a great help as my software really didn't want to fit into the 32 KB !!!!!!! Thanks.

## Licenses
My code is open source (LGPL) and is in no way bug free - so do not use for life critical devices !!

## About me
My name is Neil Parker living near Hamburg in Germany. I have spent over 30 years programming almost everything - well almost ....

## Thanks
Thanks go to the numerous opensource module authors. 

Special thanks to the fhem home automation group of enthusiasts (although I still hate perl, it does go places where other HA servers can only dream of). 

Thanks to itead for providing a board and ethernet breakout for a mega low price. I am so sorry it is retired. I am really lucky to find a new company with nice relay boards (KMP - with wiznet 5500 chip ! [Link](http://kmpelectronics.eu/products)). The new gen 2 project will clean up the wiring as well !!

Also thanks to the kids that loved drilling holes and preparing cables etc with me.
 


