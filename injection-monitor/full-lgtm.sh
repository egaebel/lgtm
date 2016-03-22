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

# Parameters----------------------------------------------------------------------------------------
channel_number=$1
channel_type=$2
wlan_interface=$3
facial_recognition_file=$4

# Constants-----------------------------------------------------------------------------------------
SLEEP_TIME=2
SWITCH_WAIT_TIME=5
PACKET_DELAY=1000

# LGTM Magic String Constants-----------------------------------------------------------------------
LGTM_BEGIN_TOKEN="lgtm-begin-protocol"
FACIAL_RECOGNITION_HEADER="facial-recognition-params"
FACIAL_RECOGNITION_FOOTER="facial-recognition-params-finished"

# Functions-----------------------------------------------------------------------------------------
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

# Main code-----------------------------------------------------------------------------------------
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
    begin_lgtm=$(cat .lgtm-monitor.dat | grep $LGTM_BEGIN_TOKEN | wc -l)
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
    echo $LGTM_BEGIN_TOKEN > .lgtm-begin-protocol
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
        lgtm_ack=$(cat .lgtm-monitor.dat | grep $FACIAL_RECOGNITION_FOOTER | wc -l)
    done
    pkill log_to_file
    echo "Received 'facial recognition params'!"
    echo "Localizing signal source!"
    chmod 644 .lgtm-monitor.dat

    logged_on_user=$(who | head -n1 | awk '{print $1;}')
    sudo -u $logged_on_user matlab -nojvm -nodisplay -nosplash -r "run('../csi-code/spotfi.m'), exit"
    echo "Successfully localized signal source!"

    echo "Extracting data from received packets............................"
    # Extract data on facial recognition params from received data
    sudo -u $logged_on_user matlab -nojvm -nodisplay -nosplash -r "run('read_mpdu_file.m'), exit"    
    echo "Data extracted!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
    
    # Sleep for 5 seconds to ensure other party has switched into monitor mode.... TODO: Shorten or remove this....
    sleep $SWITCH_WAIT_TIME
    # Switch to injection mode
    injection_mode

    # Send facial recognition params
    echo "Sending 'facial recognition params'!"
    rm .lgtm-facial-recognition-params
    cp $facial_recognition_file .lgtm-received-facial-recognition-params
    echo $FACIAL_RECOGNITION_HEADER > .lgtm-facial-recognition-params
    cat .lgtm-received-facial-recognition-params >> .lgtm-facial-recognition-params
    echo $FACIAL_RECOGNITION_FOOTER >> .lgtm-facial-recognition-params
    ./packets-from-file/packets_from_file .lgtm-facial-recognition-params 1
    
    echo "Checking for face/signal overlap................................."
    # Strip off $FACIAL_RECOGNITION_FOOTER and $FACIAL_RECOGNITION_HEADER and anything before and after
    #cat .lgtm-received-facial-recognition-params | cut -c $((${#FACIAL_RECOGNITION_HEADER} + 1))- > .lgtm-received-facial-recognition-params--no-header
    # Plus one for the string terminator
    num_header_bytes=$((${#FACIAL_RECOGNITION_HEADER} + 1))
    cat .lgtm-received-facial-recognition-params | dd bs=1 skip=$num_header_bytes > .lgtm-received-facial-recognition-params--no-header
    byte_offset=$(cat .lgtm-received-facial-recognition-params--no-header | grep --byte-offset --only-matching --text $FACIAL_RECOGNITION_FOOTER | grep --only-matching [0-9]*)
    cat .lgtm-received-facial-recognition-params--no-header | dd bs=1 count=$byte_offset > .lgtm-received-facial-recognition-params--no-header--no-footer
    #cat .lgtm-received-facial-recognition-params--no-header | rev | cut -c $((${#FACIAL_RECOGNITION_FOOTER} + 1))- | rev > .lgtm-received-facial-recognition-params--no-header--no-footer
    tar xf .lgtm-received-facial-recognition-params--no-header--no-footer
    # Create CSV file for just-received photos
    ./../facial-recognition/lgtm-recognition/create_yalefaces_csv.py .lgtm-received-facial-recognition-params--no-header--no-footer > .lgtm-facial-recognition-training-photo-paths.csv
    # Grab the characters after the semi-colon to the end of the line on the first line
    face_id=$(cat .lgtm-facial-recognition-training-photo-paths.csv | head -n1 | grep -o ";.*$" | cut -c 2-)
    top_aoas=$(cat .lgtm-top-aoas)
    old_dir=$(pwd)
    cd ../facial-recognition/lgtm-recognition/
    ls $old_dir/
    ./run_lgtm_facial_recognition.sh $old_dir/.lgtm-facial-recognition-training-photo-paths.csv $face_id top_aoas
    # Echo exit status of LGTM facial recognition command
    echo $?
    cd $old_dir
    pwd
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
    # Send acknowledgement + facial recognition params, TODO: later this will include a public key
    # Send facial recognition params
    echo "Sending 'facial recognition params'!"
    rm .lgtm-facial-recognition-params
    cp $facial_recognition_file .lgtm-received-facial-recognition-params
    echo $FACIAL_RECOGNITION_HEADER > .lgtm-facial-recognition-params
    cat .lgtm-received-facial-recognition-params >> .lgtm-facial-recognition-params
    echo $FACIAL_RECOGNITION_FOOTER >> .lgtm-facial-recognition-params
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
        lgtm_ack=$(cat .lgtm-monitor.dat | grep $FACIAL_RECOGNITION_FOOTER | wc -l)
    done
    pkill log_to_file
    echo "Received 'facial recognition params'!"
    echo "Localizing signal source!"
    chmod 644 .lgtm-monitor.dat

    logged_on_user=$(who | head -n1 | awk '{print $1;}')
    sudo -u $logged_on_user matlab -nojvm -nodisplay -nosplash -r "run('../csi-code/spotfi.m'), exit"
    echo "Successfully localized signal source!"

    echo "Extracting data from received packets............................"
    # Extract data on facial recognition params from received data
    sudo -u $logged_on_user matlab -nojvm -nodisplay -nosplash -r "run('read_mpdu_file.m'), exit"
    echo "Data extracted!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"

    echo "Checking for face/signal overlap................................."
    # Strip off $FACIAL_RECOGNITION_FOOTER and $FACIAL_RECOGNITION_HEADER and anything before and after
    #cat .lgtm-received-facial-recognition-params | cut -c $((${#FACIAL_RECOGNITION_HEADER} + 1))- > .lgtm-received-facial-recognition-params--no-header
    # Plus one for the string terminator
    num_header_bytes=$((${#FACIAL_RECOGNITION_HEADER} + 1))
    cat .lgtm-received-facial-recognition-params | dd bs=1 skip=$num_header_bytes > .lgtm-received-facial-recognition-params--no-header
    byte_offset=$(cat .lgtm-received-facial-recognition-params--no-header | grep --byte-offset --only-matching --text $FACIAL_RECOGNITION_FOOTER | grep --only-matching [0-9]*)
    cat .lgtm-received-facial-recognition-params--no-header | dd bs=1 count=$byte_offset > .lgtm-received-facial-recognition-params--no-header--no-footer
    #cat .lgtm-received-facial-recognition-params--no-header | rev | cut -c $((${#FACIAL_RECOGNITION_FOOTER} + 1))- | rev > .lgtm-received-facial-recognition-params--no-header--no-footer
    tar xf .lgtm-received-facial-recognition-params--no-header--no-footer
    # Create CSV file for just-received photos
    ./../facial-recognition/lgtm-recognition/create_yalefaces_csv.py .lgtm-received-facial-recognition-params--no-header--no-footer > .lgtm-facial-recognition-training-photo-paths.csv
    # Grab the characters after the semi-colon to the end of the line on the first line
    face_id=$(cat .lgtm-facial-recognition-training-photo-paths.csv | head -n1 | grep -o ";.*$" | cut -c 2-)
    top_aoas=$(cat .lgtm-top-aoas)
    ./../facial-recognition/lgtm-recognition/run_lgtm_facial_recognition.sh .lgtm-facial-recognition-training-photo-paths.csv $face_id top_aoas
    # Echo exit status of LGTM facial recognition command
    echo $?
    # Done!
    echo "LGTM COMPLETE!"
    exit
fi
