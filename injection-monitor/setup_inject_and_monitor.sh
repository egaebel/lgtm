
#!/usr/bin/sudo /bin/bash
WLAN_INTERFACE=$3
SLEEP_TIME=2
echo "Bringing down $WLAN_INTERFACE"
ifconfig $WLAN_INTERFACE down 2>/dev/null 1>/dev/null
sleep $SLEEP_TIME
echo "Deleting mon0.........."
iw mon0 del 2>/dev/null 1>/dev/null
sleep $SLEEP_TIME

echo "Bringing up firmware...."
modprobe -r iwlwifi mac80211 cfg80211
modprobe iwlwifi debug=0x40000
sleep $SLEEP_TIME

#echo "Running ifconfig on $WLAN_INTERFACE, looping until success...."
#ifconfig $WLAN_INTERFACE 2>/dev/null 1>/dev/null
#sleep $SLEEP_TIME
#while [ $? -ne 0 ]
#do
#    ifconfig $WLAN_INTERFACE 2>/dev/null 1>/dev/null
#	sleep $SLEEP_TIME
#done

echo "Setting $WLAN_INTERFACE into monitor mode....Looping until success..."
# Setup monitor mode, loop until it works
iwconfig $WLAN_INTERFACE mode monitor 2>/dev/null 1>/dev/null
while [ $? -ne 0 ]
do
    iwconfig $WLAN_INTERFACE mode monitor 2>/dev/null 1>/dev/null
done

echo "Adding monitor to $WLAN_INTERFACE...."
iw dev $WLAN_INTERFACE interface add mon0 type monitor
sleep $SLEEP_TIME

echo "Bringing mon0 up...................."
ifconfig mon0 up
sleep $SLEEP_TIME

echo "Bringing $WLAN_INTERFACE up......."
ifconfig $WLAN_INTERFACE up
sleep $SLEEP_TIME

echo "Killing the notorious wpa_supplicant....."
killall wpa_supplicant
sleep $SLEEP_TIME

ifconfig wlan0 down

echo "Setting channel on mon0 to $1 $2 ........."
iw mon0 set channel $1 $2
echo "Setting channel to monitor on $WLAN_INTERFACE to $1 $2" 
iw $WLAN_INTERFACE set channel $1 $2
