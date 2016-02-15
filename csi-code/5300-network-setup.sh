#!/bin/sh

echo Killing wpa_supplicant instances..........................
sleep 2
sudo killall wpa_supplicant
echo Bringing wlan1 up.........................................
sudo ip link set wlan1 up
sleep 2
#echo Connecting to NSA.surveillance.gov, assuming no auth......
#sudo iw dev wlan1 connect NSA.surveillance.gov
echo Connecting to ASUS_5G, assuming no auth...................
sudo iw dev wlan1 connect ASUS_5G
sleep 5
echo Setting up DHCP on wlan1..................................
sudo pkill dhcpcd
sleep 4
sudo dhcpcd wlan1
sleep 2
