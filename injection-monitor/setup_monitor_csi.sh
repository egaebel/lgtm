#!/usr/bin/sudo /bin/bash
WLAN_INTERFACE=$3
echo "Loading up firmware......"
modprobe -r iwlwifi mac80211 cfg80211
modprobe iwlwifi connector_log=0x1
echo "Setting $WLAN_INTERFACE into monitor mode....Looping until success..."
# Setup monitor mode, loop until it works
iwconfig $WLAN_INTERFACE mode monitor 2>/dev/null 1>/dev/null
while [ $? -ne 0 ]
do
    iwconfig $WLAN_INTERFACE mode monitor 2>/dev/null 1>/dev/null
done
echo "Bringing $WLAN_INTERFACE up......."
ifconfig $WLAN_INTERFACE up
echo "Setting channel to monitor on $WLAN_INTERFACE to $1 $2" 
iw $WLAN_INTERFACE set channel $1 $2
