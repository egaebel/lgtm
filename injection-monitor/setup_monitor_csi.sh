#!/usr/bin/sudo /bin/bash
SLEEP_TIME=2
WLAN_INTERFACE=$3
echo "Bringing $WLAN_INTERFACE down....."
ifconfig $WLAN_INTERFACE down
sleep $SLEEP_TIME
echo "Loading up firmware......"
modprobe -r iwlwifi mac80211 cfg80211
#modprobe iwlwifi connector_log=0x1
modprobe iwlwifi connector_log=0x5
echo "Setting $WLAN_INTERFACE into monitor mode....Looping until success..."
# Setup monitor mode, loop until it works
iwconfig $WLAN_INTERFACE mode monitor 2>/dev/null 1>/dev/null
while [ $? -ne 0 ]
do
    iwconfig $WLAN_INTERFACE mode monitor 2>/dev/null 1>/dev/null
done
sleep $SLEEP_TIME
echo "Bringing $WLAN_INTERFACE up......."
ifconfig $WLAN_INTERFACE up
sleep $SLEEP_TIME
sleep $SLEEP_TIME
echo "Killing the notorious wpa_supplicant"
killall wpa_supplicant
sleep $SLEEP_TIME
echo "Bringing wlan0 (default interface) down...."
ifconfig wlan0 down
sleep $SLEEP_TIME
echo "Setting channel to monitor on $WLAN_INTERFACE to $1 $2" 
iw dev $WLAN_INTERFACE set channel $1 $2
