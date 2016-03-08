#!/usr/bin/sudo /bin/bash

channel_number=$1
channel_type=$2
WLAN_INTERFACE=$3
SLEEP_TIME=2
SWITCH_WAIT_TIME=5

injection_mode () {
    echo "Switching $WLAN_INTERFACE to inject........................................"
    ip link set $WLAN_INTERFACE down
    echo "Deleting mon0...................................................."
    iw dev mon0 del 2>/dev/null 1>/dev/null
    echo "Bringing up firmware............................................."
    modprobe -r iwlwifi mac80211 cfg80211
    modprobe iwlwifi debug=0x40000
    echo "Running ip link show on $WLAN_INTERFACE, looping until success............."  
    ip link show $WLAN_INTERFACE 2>/dev/null 1>/dev/null
    while [ $? -ne 0 ]; do
        ip link show $WLAN_INTERFACE 2>/dev/null 1>/dev/null
    done
    echo "Setting $WLAN_INTERFACE into monitor mode.................................."
    iw dev $WLAN_INTERFACE set type monitor 2>/dev/null 1>/dev/null
    mode_change=$?
    while [ $mode_change -ne 0 ]; do
        ip link set $WLAN_INTERFACE down 2>/dev/null 1>/dev/null
        iw dev $WLAN_INTERFACE set type monitor 2>/dev/null 1>/dev/null
        mode_change=$?
        echo mode change: $mode_change
    done
    echo "Bringing up $WLAN_INTERFACE ..............................................."
    ip link set $WLAN_INTERFACE up
    echo "Adding monitor to $WLAN_INTERFACE ........................................."
    iw dev $WLAN_INTERFACE interface add mon0 type monitor
    echo "Bringing up mon0................................................."
    ip link set mon0 up
    echo "Killing default wireless interface, wlan0........................"
    ip link set wlan0 down
    echo "Setting channel on mon0 to $channel_number $channel_type .............................."
    iw dev mon0 set channel $channel_number $channel_type
    channel_set=$?
    while [ $channel_set -ne 0 ]; do
        ip link set $WLAN_INTERFACE down 2>/dev/null 1>/dev/null
        iw dev $WLAN_INTERFACE set type monitor 2>/dev/null 1>/dev/null
        ip link set $WLAN_INTERFACE up
        iw dev mon0 set channel $channel_number $channel_type
        channel_set=$?
        if [ $channel_set -eq 0 ]; then
            echo "Fixed problem with set channel command..........................."
        fi
    done
    echo "Setting monitor_tx_rate.........................................."
    echo 0x4101 | sudo tee `sudo find /sys -name monitor_tx_rate`
    echo "Injection mode active!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
}

monitor_mode () {
    echo "Switching $WLAN_INTERFACE to monitor......................................."
    echo "Bringing up firmware............................................."
    modprobe -r iwlwifi mac80211 cfg80211
    modprobe iwlwifi connector_log=0x5
    echo "Bringing down $WLAN_INTERFACE ............................................."
    ip link set $WLAN_INTERFACE down 2>/dev/null 1>/dev/null
    echo "Setting $WLAN_INTERFACE into monitor mode.................................."
    iw dev $WLAN_INTERFACE set type monitor 2>/dev/null 1>/dev/null
    mode_change=$?
    while [ $mode_change -ne 0 ]; do
        ip link set $WLAN_INTERFACE down 2>/dev/null 1>/dev/null
        iw dev $WLAN_INTERFACE set type monitor 2>/dev/null 1>/dev/null
        mode_change=$?
    done
    echo "Bringing up $WLAN_INTERFACE ..............................................."
    ip link set $WLAN_INTERFACE up
    wlan_interface_up=$(ip link show up | grep $WLAN_INTERFACE | wc -l)
    while [ $wlan_interface_up -ne 1 ]
    do
        ip link set $WLAN_INTERFACE up
        wlan_interface_up=$(ip link show up | grep $WLAN_INTERFACE | wc -l)
    done
    echo "Bringing down default wireless interface wlan0..................."
    ip link set wlan0 down
    echo "Setting channel to monitor on $WLAN_INTERFACE to $channel_number $channel_type .................." 
    iw dev $WLAN_INTERFACE set channel $channel_number $channel_type
    channel_set=$?
    while [ $channel_set -ne 0 ]; do
        ip link set $WLAN_INTERFACE down 2>/dev/null 1>/dev/null
        iw dev $WLAN_INTERFACE set type monitor 2>/dev/null 1>/dev/null
        ip link set $WLAN_INTERFACE up 2>/dev/null 1>/dev/null
        ip link set wlan0 down 2>/dev/null 1>/dev/null
        iw dev $WLAN_INTERFACE set channel $channel_number $channel_type 2>/dev/null 1>/dev/null
        channel_set=$?
        if [ $channel_set -eq 0 ]; then
            echo "Fixed problem with set channel command..........................."
        fi
    done
    echo "Monitor mode active!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
}

pkill log_to_file
monitor_mode

echo "Waiting for LGTM initiation......................................"
rm .lgtm-monitor.dat
./log-to-file/log_to_file .lgtm-monitor.dat &

# Wait for key press or special token to appear in lgtm-monitor.dat
echo "Press 'L' to initiate LGTM from this computer...................."
begin_lgtm=0
input='a'
while [[ $input != 'l' ]] && [[ $begin_lgtm -lt 1 ]]; do
    read -n 1 -s -t 2 -r input
    #echo "Input received ||$input||"
    # TODO: Later this token, "begin-lgtm-protocol", will also include a public key
    begin_lgtm=$(cat .lgtm-monitor.dat | grep "lgtm-begin-protocol" | wc -l)
    cat .lgtm-monitor.dat
    #cat lgtm-monitor.dat | grep begin-lgtm-protocol
done

# Key pressed to initiate LGTM
if [[ $input == 'l' ]]; then
    echo "Initiating LGTM protocol........................................."
    pkill log_to_file
    # Sleep for 5 seconds to ensure other party has switched into monitor mode
    sleep $SWITCH_WAIT_TIME
    # undo monitor mode settings....(which are what exactly?)
    # Setup Injection mode
    injection_mode
    # Send "begin-lgtm-protocol", TODO: later this will include a public key
    rm .lgtm-begin-protocol
    echo lgtm-begin-protocol > .lgtm-begin-protocol
    ./packets-from-file/packets_from_file .lgtm-begin-protocol 1
    # Switch to monitor mode
    monitor_mode
    # Wait for acknowledgement + facial recognition params, TODO: later it will be ack + recog params + public key
    echo "Awaiting 'facial recognition params'!"
    rm .lgtm-monitor.dat
    ./log-to-file/log_to_file .lgtm-monitor.dat &
    lgtm_ack=0
    while [ $lgtm_ack -lt 1 ]; do
        # Receive ack + params
        lgtm_ack=$(cat .lgtm-monitor.dat | grep "facial-recognition-params" | wc -l)
    done
    echo "Received 'facial recognition params'!"
    pkill log_to_file
    echo "Localizing signal source!"
    chmod 644 .lgtm-monitor.dat
    sudo -u $(whoami) matlab -nojvm -nodisplay -nosplash -r "run('../csi-code/spotfi.m'), exit"
    echo "Successfully localized signal source!"
    # Sleep for 5 seconds to ensure other party has switched into monitor mode.... TODO: Shorten or remove this....
    sleep $SWITCH_WAIT_TIME
    # Switch to injection mode
    injection_mode
    # Send facial recognition params
    rm .lgtm-facial-recognition-params
    echo facial-recognition-params > .lgtm-facial-recognition-params
    cat facial-recognition-model >> .lgtm-facial-recognition-params
    ./packets-from-file/packets_from_file .lgtm-facial-recognition-params 1
    # Done!
    echo "LGTM COMPLETE!"
    exit
fi

# Token received from other party to initiate LGTM
if [ $begin_lgtm -gt 0 ]; then
    echo "Other party initiated LGTM protocol.............................."
    # Setup Injection mode
    injection_mode
    # Sleep for 5 seconds to ensure other party has switched into monitor mode....
    sleep $SWITCH_WAIT_TIME
    # Send acknowledgement + facial recognition params, TODO: later this will inlcude a public key
    echo "Sending 'facial recognition params'!"
    rm .lgtm-facial-recognition-params
    echo facial-recognition-params > .lgtm-facial-recognition-params
    cat facial-recognition-model >> .lgtm-facial-recognition-params
    ./packets-from-file/packets_from_file .lgtm-facial-recognition-params 1
    # Setup Monitor mode
    monitor_mode
    # Await facial recognition params
    echo "Awaiting 'facial recognition params'!"
    rm .lgtm-monitor.dat
    ./log-to-file/log_to_file .lgtm-monitor.dat &
    lgtm_ack=0
    while [[ $lgtm_ack -lt 1 ]]; do
        # Receive ack + params
        lgtm_ack=$(cat .lgtm-monitor.dat | grep "facial-recognition-params" | wc -l)
    done
    pkill log_to_file
    echo "Received 'facial recognition params'!"
    # Done!
    echo "LGTM COMPLETE!"
    exit
fi
