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
webcam_id=$5

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
    echo "Setting channel on mon0 to $channel_number $channel_type ............................."
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
    echo "Setting channel to monitor on $wlan_interface to $channel_number $channel_type ................." 
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

send_facial_recognition_params () {
    echo "Sending 'facial recognition params'.............................."
    # Setup Injection mode
    injection_mode
    # Sleep for 5 seconds to ensure other party has switched into monitor mode....
    sleep $SWITCH_WAIT_TIME
    # Send facial recognition params
    rm .lgtm-facial-recognition-params
    echo $FACIAL_RECOGNITION_HEADER > .lgtm-facial-recognition-params
    dd if=$facial_recognition_file of=.lgtm-facial-recognition-params seek=${#FACIAL_RECOGNITION_HEADER} bs=1
    echo $FACIAL_RECOGNITION_FOOTER | dd of=.lgtm-facial-recognition-params bs=1 oflag=append conv=notrunc

    ./packets-from-file/packets_from_file .lgtm-facial-recognition-params 1
    echo "Sent 'facial recognition params'!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
}

receive_facial_recognition_params () {
    echo "Receiving and processing 'facial recognition params'............."
    # Switch to monitor mode
    monitor_mode

    # Listen for facial recognition parameters
    echo "Awaiting 'facial recognition params'............................."
    rm .lgtm-monitor.dat
    rm .lgtm-received-facial-recognition-params
    ./log-to-file/log_to_file .lgtm-monitor.dat &
    lgtm_ack=0
    # Figure this out to use with sudo -u below
    logged_on_user=$(who | head -n1 | awk '{print $1;}')
    while [ $lgtm_ack -lt 1 ]; do
        # Count the file size in bytes using wc, and cut off the file name from the output (only want the number!)
        file_size=$(wc --bytes .lgtm-monitor.dat | cut -d ' ' -f 1)
        echo file size: $file_size
        if [ "${file_size:0}" -gt 0 ]; then
            # Extract data from mpdus in packets
            echo "Extracting data from received packets............................"
            # Extract data on facial recognition params from received data
            sudo -u $logged_on_user matlab -nojvm -nodisplay -nosplash -r "run('read_mpdu_file.m'), exit"    
            echo "Data extracted!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
            # Receive ack + params
            lgtm_ack=$(cat .lgtm-received-facial-recognition-params | grep $FACIAL_RECOGNITION_FOOTER | wc -l)
        fi
        sleep 3
    done
    pkill log_to_file
    chmod 644 .lgtm-monitor.dat
    echo "Received 'facial recognition params'!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
}

localize_wireless_signal () {
    # Localize wireless signal
    echo "Localizing signal source........................................."
    logged_on_user=$(who | head -n1 | awk '{print $1;}')
    sudo -u $logged_on_user matlab -nojvm -nodisplay -nosplash -r "run('../csi-code/lgtm_spotfi_runner.m'), exit"
    echo "Successfully localized signal source!"
}

compare_wireless_location_with_face_location () {
    echo "Checking for face/signal overlap................................."

    rm .lgtm-received-facial-recognition-params--no-header
    rm .lgtm-received-facial-recognition-params--no-header--no-footer
    # Strip off $FACIAL_RECOGNITION_HEADER, $FACIAL_RECOGNITION_FOOTER, and anything before or after
    # Plus one for the string terminator ('\0')
    num_header_bytes=${#FACIAL_RECOGNITION_HEADER}
    dd if=.lgtm-received-facial-recognition-params bs=1 skip=$num_header_bytes of=.lgtm-received-facial-recognition-params--no-header
    byte_offset=$(dd if=.lgtm-received-facial-recognition-params--no-header bs=1 | grep --byte-offset --only-matching --text $FACIAL_RECOGNITION_FOOTER | grep --only-matching [0-9]*)
    dd if=.lgtm-received-facial-recognition-params--no-header bs=1 count=$byte_offset of=.lgtm-received-facial-recognition-params--no-header--no-footer
    
    # Extract files from tar archive
    tar xvf .lgtm-received-facial-recognition-params--no-header--no-footer   
    facial_recognition_params_folder=$(tar xvf .lgtm-received-facial-recognition-params--no-header--no-footer | head -n 1)
    echo "facial_recognition_params_folder: " $facial_recognition_params_folder
    rm -rf .lgtm-facial-recognition-training-photos
    mv $facial_recognition_params_folder .lgtm-facial-recognition-training-photos

    # Generate csv file with paths to images for training and labels
    rm .lgtm-facial-recognition-training-photo-paths.csv
    ./create_yalefaces_csv.py .lgtm-facial-recognition-training-photos > .lgtm-facial-recognition-training-photo-paths.csv

    # Grab the label from the first entry (they're assumed to all be the same)
    # The label is from after the semi-colon to the end of the line
    face_id=$(cat .lgtm-facial-recognition-training-photo-paths.csv | head -n1 | grep -o ";.*$" | cut -c 2-)
    top_aoas=$(cat .lgtm-top-aoas)

    # Change folder to run facial recognition program
    old_dir=$(pwd)
    cd ../facial-recognition/lgtm-recognition/

    # Run facial recognition
    ./run_lgtm_facial_recognition.sh $webcam_id $old_dir/.lgtm-facial-recognition-training-photo-paths.csv $face_id $top_aoas 2>/dev/null

    # Return to original directory
    cd $old_dir
}

# Main code-----------------------------------------------------------------------------------------
pkill log_to_file
monitor_mode
# Sleep to ensure other party has also switched into monitor mode
sleep $SWITCH_WAIT_TIME

echo "Waiting for LGTM initiation......................................"
rm .lgtm-begin-monitor.dat
./log-to-file/log_to_file .lgtm-begin-monitor.dat &

# Wait for key press or special token to appear in lgtm-begin-monitor.dat
echo "Press 'L' to initiate LGTM from this computer...................."
begin_lgtm=0
input='a'
while [[ $input != 'l' ]] && [[ $begin_lgtm -lt 1 ]]; do
    read -n 1 -s -t 1 -r input
    begin_lgtm=$(cat .lgtm-begin-monitor.dat | grep $LGTM_BEGIN_TOKEN | wc -l)
done

start_time=$(date +%s)
# Key pressed to initiate LGTM
if [[ $input == 'l' ]]; then
    echo "Initiating LGTM protocol........................................."
    pkill log_to_file

    # Sleep to ensure other party has switched into monitor mode
    sleep $SWITCH_WAIT_TIME

    # Setup Injection mode
    injection_mode

    # Send "begin-lgtm-protocol"
    rm .lgtm-begin-protocol
    echo $LGTM_BEGIN_TOKEN > .lgtm-begin-protocol
    ./packets-from-file/packets_from_file .lgtm-begin-protocol 1 $PACKET_DELAY
    
    receive_facial_recognition_params
    send_facial_recognition_params
    localization_start_time=$(date +%s)
    localize_wireless_signal
    localization_end_time=$(date +%s)
    localization_elapsed_time=$(expr $localization_end_time - $localization_start_time)
    echo Localization ran in time: $localization_elapsed_time seconds
    compare_wireless_location_with_face_location

    # Done!
    echo "LGTM COMPLETE!"
    exit
fi

# Token received from other party to initiate LGTM
if [ $begin_lgtm -gt 0 ]; then
    echo "Other party initiated LGTM protocol.............................."
    
    send_facial_recognition_params
    receive_facial_recognition_params
    localization_start_time=$(date +%s)
    localize_wireless_signal
    localization_end_time=$(date +%s)
    localization_elapsed_time=$(expr $localization_end_time - $localization_start_time)
    echo Localization ran in time: $localization_elapsed_time seconds
    compare_wireless_location_with_face_location
    
    # Done!
    echo "LGTM COMPLETE!"
    exit
fi
end_time=$(date +%s)
elapsed_time=$(expr $end_time - $start_time)
echo LGTM ran in time: $elapsed_time seconds
