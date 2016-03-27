#!/usr/bin/sudo /bin/bash

##
# The MIT License (MIT)
# Copyright (c) 2016 Ethan Gaebel <egaebel@vt.edu>
# 
# Permission is hereby granted, free of charge, to any person obtaining a 
# copy of this software and associated documentation files (the "Software"), 
# to deal in the Software without restriction, including without limitation 
# the rights to use, copy, modify, merge, publish, distribute, sublicense, 
# and/or sell copies of the Software, and to permit persons to whom the 
# Software is furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included 
# in all copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS 
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
# DEALINGS IN THE SOFTWARE.

channel_number=$1
channel_type=$2
wlan_interface=$3
SLEEP_TIME=2
SWITCH_WAIT_TIME=5
PACKET_DELAY=0

injection_mode () {
    echo "Switching $wlan_interface to inject........................................"
    ip link set $wlan_interface down
    echo "Deleting mon0...................................................."
    iw dev mon0 del 2>/dev/null 1>/dev/null
    echo "Bringing up firmware............................................."
    modprobe -r iwlwifi mac80211 cfg80211
    modprobe iwlwifi debug=0x40000
    echo "Running ip link show on $wlan_interface, looping until success............."  
    ip link show $wlan_interface 2>/dev/null 1>/dev/null
    while [ $? -ne 0 ]; do
        ip link show $wlan_interface 2>/dev/null 1>/dev/null
    done
    echo "Setting $wlan_interface into monitor mode.................................."
    iw dev $wlan_interface set type monitor 2>/dev/null 1>/dev/null
    mode_change=$?
    while [ $mode_change -ne 0 ]; do
        ip link set $wlan_interface down 2>/dev/null 1>/dev/null
        iw dev $wlan_interface set type monitor 2>/dev/null 1>/dev/null
        mode_change=$?
    done
    echo "Bringing up $wlan_interface ..............................................."
    ip link set $wlan_interface up
    echo "Adding monitor to $wlan_interface ........................................."
    iw dev $wlan_interface interface add mon0 type monitor
    echo "Bringing up mon0................................................."
    ip link set mon0 up
    echo "Killing default wireless interface, wlan0........................"
    ip link set wlan0 down
    echo "Setting channel on mon0 to $channel_number $channel_type .............................."
    iw dev mon0 set channel $channel_number $channel_type
    channel_set=$?
    while [ $channel_set -ne 0 ]; do
        ip link set $wlan_interface down 2>/dev/null 1>/dev/null
        iw dev $wlan_interface set type monitor 2>/dev/null 1>/dev/null
        ip link set $wlan_interface up
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
    echo "Switching $wlan_interface to monitor......................................."
    echo "Bringing up firmware............................................."
    modprobe -r iwlwifi mac80211 cfg80211
    modprobe iwlwifi connector_log=0x5
    echo "Bringing down $wlan_interface ............................................."
    ip link set $wlan_interface down 2>/dev/null 1>/dev/null
    echo "Setting $wlan_interface into monitor mode.................................."
    iw dev $wlan_interface set type monitor 2>/dev/null 1>/dev/null
    mode_change=$?
    while [ $mode_change -ne 0 ]; do
        ip link set $wlan_interface down 2>/dev/null 1>/dev/null
        iw dev $wlan_interface set type monitor 2>/dev/null 1>/dev/null
        mode_change=$?
    done
    echo "Bringing up $wlan_interface ..............................................."
    ip link set $wlan_interface up
    wlan_interface_up=$(ip link show up | grep $wlan_interface | wc -l)
    while [ $wlan_interface_up -ne 1 ]
    do
        ip link set $wlan_interface up
        wlan_interface_up=$(ip link show up | grep $wlan_interface | wc -l)
    done
    echo "Bringing down default wireless interface wlan0..................."
    ip link set wlan0 down
    echo "Setting channel to monitor on $wlan_interface to $channel_number $channel_type .................." 
    iw dev $wlan_interface set channel $channel_number $channel_type
    channel_set=$?
    while [ $channel_set -ne 0 ]; do
        ip link set $wlan_interface down 2>/dev/null 1>/dev/null
        iw dev $wlan_interface set type monitor 2>/dev/null 1>/dev/null
        ip link set $wlan_interface up 2>/dev/null 1>/dev/null
        ip link set wlan0 down 2>/dev/null 1>/dev/null
        iw dev $wlan_interface set channel $channel_number $channel_type 2>/dev/null 1>/dev/null
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
    # TODO: Later this token, "begin-lgtm-protocol", will also include a public key
    begin_lgtm=$(cat .lgtm-monitor.dat | grep "lgtm-begin-protocol" | wc -l)
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
    ./packets-from-file/packets_from_file .lgtm-begin-protocol 1 $PACKET_DELAY
    # Switch to monitor mode
    monitor_mode
    # Wait for acknowledgement + facial recognition params, TODO: later it will be ack + recog params + public key
    echo "Awaiting 'facial recognition params'!"
    rm .lgtm-monitor.dat
    ./log-to-file/log_to_file .lgtm-monitor.dat &
    lgtm_ack=0
    while [ $lgtm_ack -lt 1 ]; do
        # Receive ack + params
        lgtm_ack=$(cat .lgtm-monitor.dat | grep "facial-recognition-params-finished" | wc -l)
    done
    pkill log_to_file
    echo "Received 'facial recognition params'!"
    echo "Localizing signal source!"
    chmod 644 .lgtm-monitor.dat
    logged_on_user=$(who | head -n1 | awk '{print $1;}')
    sudo -u $logged_on_user matlab -nojvm -nodisplay -nosplash -r "run('../csi-code/lgtm_spotfi_runner.m'), exit"
    cat .lgtm-top-aoas
    echo "Successfully localized signal source!"
    # Sleep for 5 seconds to ensure other party has switched into monitor mode.... TODO: Shorten or remove this....
    sleep $SWITCH_WAIT_TIME
    # Switch to injection mode
    injection_mode
    # Send facial recognition params
    echo "Sending 'facial recognition params'!"
    rm .lgtm-facial-recognition-params
    echo facial-recognition-params > .lgtm-facial-recognition-params
    #cat facial-recognition-model >> .lgtm-facial-recognition-params
    cat subject01.tar.gz >> .lgtm-facial-recognition-params
    echo facial-recognition-params-finished >> .lgtm-facial-recognition-params
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
    #cat facial-recognition-model >> .lgtm-facial-recognition-params
    cat subject01.tar.gz >> .lgtm-facial-recognition-params
    echo facial-recognition-params-finished >> .lgtm-facial-recognition-params
    ./packets-from-file/packets_from_file .lgtm-facial-recognition-params 1 $PACKET_DELAY
    # Setup Monitor mode
    monitor_mode
    # Await facial recognition params
    echo "Awaiting 'facial recognition params'!"
    rm .lgtm-monitor.dat
    ./log-to-file/log_to_file .lgtm-monitor.dat &
    lgtm_ack=0
    while [[ $lgtm_ack -lt 1 ]]; do
        # Receive ack + params
        lgtm_ack=$(cat .lgtm-monitor.dat | grep "facial-recognition-params-finished" | wc -l)
    done
    pkill log_to_file
    echo "Received 'facial recognition params'!"
    echo "Localizing signal source!"
    chmod 644 .lgtm-monitor.dat
    logged_on_user=$(who | head -n1 | awk '{print $1;}')
    sudo -u $logged_on_user matlab -nojvm -nodisplay -nosplash -r "run('../csi-code/lgtm_spotfi_runner.m'), exit"
    cat .lgtm-top-aoas
    echo "Successfully localized signal source!"
    # Done!
    echo "LGTM COMPLETE!"
    exit
fi
