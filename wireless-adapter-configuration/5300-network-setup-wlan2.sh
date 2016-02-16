#!/bin/sh

echo Killing wpa_supplicant instances..........................
sleep 2
sudo killall wpa_supplicant
echo Bringing wlan2 up.........................................
sudo ip link set wlan2 up
sleep 2
#echo Connecting to NSA.surveillance.gov, assuming no auth......
#sudo iw dev wlan1 connect NSA.surveillance.gov
echo Connecting to ASUS_5G, assuming no auth...................
sudo iw dev wlan2 connect ASUS_5G
sleep 5
echo Setting up DHCP on wlan2..................................
sudo pkill dhcpcd
sleep 4
sudo dhcpcd wlan2
sleep 2
