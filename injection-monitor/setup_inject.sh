#!/usr/bin/sudo /bin/bash
WLAN_INTERFACE=$3
echo "Bringing up firmware...."
modprobe -r iwlwifi mac80211 cfg80211
modprobe iwlwifi debug=0x40000
echo "Running ifconfig on $WLAN_INTERFACE, looping until success...."
ifconfig $WLAN_INTERFACE 2>/dev/null 1>/dev/null
while [ $? -ne 0 ]
do
    ifconfig $WLAN_INTERFACE 2>/dev/null 1>/dev/null
done
echo "Adding monitor to $WLAN_INTERFACE...."
iw dev $WLAN_INTERFACE interface add mon0 type monitor
echo "Bringing mon0 up...................."
ifconfig mon0 up
echo "Setting channel on mon0 to $1 $2 ........."
iw mon0 set channel $1 $2
