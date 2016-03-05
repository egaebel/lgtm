#!/usr/bin/sudo /bin/bash

channel_number=$1
channel_type=$2
WLAN_INTERFACE=$3
SLEEP_TIME=2
SWITCH_WAIT_TIME=5

injection_mode () {
    echo "Setting $WLAN_INTERFACE into injection mode......................"
    ifconfig $WLAN_INTERFACE down
    iw mon0 del
    echo "Adding monitor to $WLAN_INTERFACE ..............................."
    iw dev $WLAN_INTERFACE interface add mon0 type monitor
    ifconfig mon0 up
    echo "Killing the notorious wpa_supplicant............................."
    killall wpa_supplicant
    sleep $SLEEP_TIME
    echo "Killing default wireless interface, wlan0........................"
    ifconfig wlan0 down
    echo "Setting channel on mon0 to $channel_number $channel_type ................................"
    iw dev mon0 set channel $channel_number $channel_type
    echo "Injection mode active!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
}

monitor_mode () {
    echo "Bringing down $WLAN_INTERFACE ..................................."
    ifconfig $WLAN_INTERFACE down
    while [[ $! -ne 0 ]]
    do
        ifconfig $WLAN_INTERFACE down
    done
    echo "Setting $WLAN_INTERFACE into monitor mode........................"
    # Setup monitor mode, loop until it works
    iwconfig $WLAN_INTERFACE mode monitor
    while [[ $! -ne 0 ]]
    do
        iwconfig $WLAN_INTERFACE mode monitor
    done
    echo "Bringing $WLAN_INTERFACE up......................................"
    ifconfig $WLAN_INTERFACE up
    sleep $SLEEP_TIME
    sleep $SLEEP_TIME
    echo "Killing the notorious wpa_supplicant............................."
    killall wpa_supplicant
    #sleep $SLEEP_TIME
    echo "Setting channel to monitor on $WLAN_INTERFACE to $channel_number $channel_type .........." 
    iw dev $WLAN_INTERFACE set channel $channel_number $channel_type
    echo "Killing default wireless interface, wlan0........................"
    ifconfig wlan0 down
    echo "Monitor mode active!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
}


echo "Bringing up firmware............................................."
modprobe -r iwlwifi mac80211 cfg80211
modprobe iwlwifi debug=0x40000
sleep $SLEEP_TIME

monitor_mode

echo "Waiting for LGTM initiation......................................"
./log_to_file.sh lgtm-monitor.dat &

# Wait for key press or special token to appear in lgtm-monitor.dat
echo "Press space to initiate LGTM from this computer.................."
begin_lgtm=0
input='a'
while [[ $input != ' ' ]] && [[ $lgtm_data < 1 ]]
do
    read -n 1 -s -t 10 input
    # TODO: lgtm-monitor.dat and lgtm-monitor-check are statically set in matlab files and here...make this better?
    # TODO: Later this token, "begin-lgtm-protocol", will also include a public key
    lgtm_data=$(cat lgtm-monitor.dat | wc -l)
done

# Key pressed to initiate LGTM
if [[ $input == ' ' ]]
then
    echo "Initiating LGTM protocol....................................."
    pkill log_to_file.sh
    # Sleep for 5 seconds to ensure other party has switched into monitor mode
    sleep $SWITCH_WAIT_TIME
    # undo monitor mode settings....(which are what exactly?)
    # Setup Injection mode
    injection_mode
    # Send "begin-lgtm-protocol", TODO: later this will include a public key
    echo begin-lgtm-protocol > .begin-lgtm-protocol
    ./packets_from_file .begin-lgtm-protocol
    rm .begin-lgtm-protocol
    # Switch to monitor mode
    monitor_mode
    # Wait for acknowledgement + facial recognition params, TODO: later it will be ack + recog params + public key
    rm lgtm-monitor.dat
    ./log_to_file.sh lgtm-monitor.dat &
    lgtm_ack=0
    while [[ $lgtm_ack < 1 ]]
    do
        # Receive ack + params
        lgtm_ack=$(cat lgtm-monitor.dat | wc -l)
    done
    # Sleep for 5 seconds to ensure other party has switched into monitor mode....
    sleep $SWITCH_WAIT_TIME
    # Switch to injection mode
    injection_mode
    # Send facial recognition params
    echo second-level-lgtm-protocol > .lgtm-protocol-continued
    ./packets_from_file .lgtm-protocol-continued
    # Done!
fi

# Token received from other party to initiate LGTM
if [[ begin_lgtm > 0 ]]
then
    echo "Other party initiated LGTM protocol........................."
    # Setup Injection mode
    injection_mode
    # Sleep for 5 seconds to ensure other party has switched into monitor mode....
    sleep $SWITCH_WAIT_TIME
    # Send acknowledgement + facial recognition params, TODO: later this will inlcude a public key
    echo begin-lgtm-protocol > .begin-lgtm-protocol
    ./packets_from_file .begin-lgtm-protocol
    rm .begin-lgtm-protocol
    # Setup Monitor mode
    monitor_mode
    # Await facial recognition params
    rm lgtm-monitor.dat
    ./log-to-file lgtm-monitor.dat &
    lgtm_ack=0
    while [[ $lgtm_ack < 1 ]]
    do
        # Receive ack + params
        lgtm_ack=$(cat lgtm-monitor.dat | wc -l)
    done
    # Done!
fi
