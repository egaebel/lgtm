#!/usr/bin/sudo /bin/bash
WLAN_INTERFACE=$3
SLEEP_TIME=2
echo "Bringing down $WLAN_INTERFACE"
ifconfig $WLAN_INTERFACE down
sleep $SLEEP_TIME
echo "Deleting mon0.........."
iw dev mon0 del 2>/dev/null 1>/dev/null
sleep $SLEEP_TIME
echo "Bringing up firmware...."
modprobe -r iwlwifi mac80211 cfg80211
modprobe iwlwifi debug=0x40000
sleep $SLEEP_TIME
echo "Running ifconfig on $WLAN_INTERFACE, looping until success...."
ifconfig $WLAN_INTERFACE 2>/dev/null 1>/dev/null
sleep $SLEEP_TIME
while [ $? -ne 0 ]
do
    ifconfig $WLAN_INTERFACE 2>/dev/null 1>/dev/null
	sleep $SLEEP_TIME
done
echo "Adding monitor to $WLAN_INTERFACE...."
iw dev $WLAN_INTERFACE interface add mon0 type monitor
sleep $SLEEP_TIME
echo "Bringing mon0 up...................."
ifconfig mon0 up
sleep $SLEEP_TIME
echo "Killing the notorious wpa_supplicant....."
killall wpa_supplicant
sleep $SLEEP_TIME
ifconfig wlan0 down
echo "Setting channel on mon0 to $1 $2 ........."
iw dev mon0 set channel $1 $2
