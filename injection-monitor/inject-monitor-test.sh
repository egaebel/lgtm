#!/usr/bin/sudo /bin/bash

channel_number=$1
channel_type=$2
WLAN_INTERFACE=$3
SLEEP_TIME=2
SWITCH_WAIT_TIME=5

injection_mode () {
    echo "Setting $WLAN_INTERFACE into injection mode................................"
    ip link set $WLAN_INTERFACE down
    echo "Deleting mon0...................................................."
    iw dev mon0 del 2>/dev/null 1>/dev/null
    echo "Bringing up firmware............................................."
    modprobe -r iwlwifi mac80211 cfg80211
    modprobe iwlwifi debug=0x40000
    echo "Running ifconfig on $WLAN_INTERFACE, looping until success................."  
    ip link show $WLAN_INTERFACE 2>/dev/null 1>/dev/null
    while [ $? -ne 0 ]
    do
        ip link show $WLAN_INTERFACE 2>/dev/null 1>/dev/null
        sleep $SLEEP_TIME
    done
    echo "Adding monitor to $WLAN_INTERFACE ..............................."
    iw dev $WLAN_INTERFACE interface add mon0 type monitor
    echo "Bringing mon0 up................................................."
    ip link set mon0 up
    sleep $SLEEP_TIME
    sleep $SLEEP_TIME
    #echo "Killing the notorious wpa_supplicant............................."
    #killall wpa_supplicant
    #sleep $SLEEP_TIME
    echo "Killing default wireless interface, wlan0........................"
    ip link set wlan0 down
    sleep $SLEEP_TIME
    echo "Setting channel on mon0 to $channel_number $channel_type ................................"
    iw dev mon0 set channel $channel_number $channel_type
    echo "Setting monitor_tx_rate.........................................."
    echo 0x4101 | sudo tee `sudo find /sys -name monitor_tx_rate`
    echo "Injection mode active!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
}

monitor_mode () {
    echo "Setting $WLAN_INTERFACE into monitor mode.................................."
    echo "Bringing up firmware............................................."
    modprobe -r iwlwifi mac80211 cfg80211
    modprobe iwlwifi connector_log=0x5
    echo "Bringing down $WLAN_INTERFACE ..................................."
    ip link set $WLAN_INTERFACE down 2>/dev/null 1>/dev/null
    echo "Setting $WLAN_INTERFACE into monitor mode........................"
    # Setup monitor mode, loop until it works
    iw dev $WLAN_INTERFACE set type monitor 2>/dev/null 1>/dev/null
    mode_change=$?
    while [ $mode_change -ne 0 ]; do
        ip link set $WLAN_INTERFACE down 2>/dev/null 1>/dev/null
        iw dev $WLAN_INTERFACE set type monitor 
#2>/dev/null 1>/dev/null
        mode_change=$?
        echo mode change: $mode_change
    done
    #sleep $SLEEP_TIME
    echo "Bringing $WLAN_INTERFACE up......................................"
    ip link set $WLAN_INTERFACE up
    wlan_interface_up=$(ip link show up | grep $WLAN_INTERFACE | wc -l)
    while [ $wlan_interface_up -ne 1 ]
    do
        ip link set $WLAN_INTERFACE up
        wlan_interface_up=$(ip link show up | grep $WLAN_INTERFACE | wc -l)
    done
    #sleep $SLEEP_TIME
    #sleep $SLEEP_TIME
    #echo "Killing the notorious wpa_supplicant............................."
    #killall wpa_supplicant
    #sleep $SLEEP_TIME
    echo "Killing default wireless interface, wlan0........................"
    ip link set wlan0 down
    #wlan0_up=$(ip link show up | grep wlan0 | wc -l)
    #while [ $wlan0_up -gt 0 ]
    #do
    #    ip link set wlan0 down
    #    wlan0_up=$(ip link show up | grep wlan0 | wc -l)
    #done
    #sleep $SLEEP_TIME
    echo "Setting channel to monitor on $WLAN_INTERFACE to $channel_number $channel_type .........." 
    iw dev $WLAN_INTERFACE set channel $channel_number $channel_type
    echo "Monitor mode active!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
}

monitor_mode

echo "Waiting for LGTM initiation......................................"
rm lgtm-monitor.dat
./log_to_file.sh lgtm-monitor.dat &

# Wait for key press or special token to appear in lgtm-monitor.dat
echo "Press 'L' to initiate LGTM from this computer.................."
begin_lgtm=0
input='a'
while [[ $input != 'l' ]] && [[ $lgtm_data < 1 ]]
do
    read -n 1 -s -t 10 -r input
    echo "Input received ||$input||"
    # TODO: lgtm-monitor.dat and lgtm-monitor-check are statically set in matlab files and here...make this better?
    # TODO: Later this token, "begin-lgtm-protocol", will also include a public key
    lgtm_data=$(cat lgtm-monitor.dat | wc -l)
done

# Key pressed to initiate LGTM
if [[ $input == 'l' ]]
then
    echo "Initiating LGTM protocol....................................."
    pkill log_to_file
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
    while [[ $lgtm_ack -lt 1 ]]
    do
        # Receive ack + params
        lgtm_ack=$(cat lgtm-monitor.dat | wc -l)
        echo lgtm_ack: $lgtm_ack
        sleep $SLEEP_TIME
        sleep $SLEEP_TIME
        sleep $SLEEP_TIME
    done
    pkill log_to_file
    # Sleep for 5 seconds to ensure other party has switched into monitor mode....
    sleep $SWITCH_WAIT_TIME
    # Switch to injection mode
    injection_mode
    # Send facial recognition params
    echo second-level-lgtm-protocol > .lgtm-protocol-continued
    ./packets_from_file .lgtm-protocol-continued
    # Done!
    echo "LGTM COMPLETE!"
fi

# Token received from other party to initiate LGTM
if [[ $begin_lgtm > 0 ]]
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
    ./log_to_file.sh lgtm-monitor.dat &
    lgtm_ack=0
    while [[ $lgtm_ack -lt 1 ]]
    do
        # Receive ack + params
        lgtm_ack=$(cat lgtm-monitor.dat | wc -l)
        sleep $SLEEP_TIME
    done
    # Done!
    echo "LGTM COMPLETE!"
fi
