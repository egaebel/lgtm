#/bin/bash

cd /lib/firmware
sudo rm /lib/firmware/iwlwifi-5000-2.ucode
sudo ln -s /lib/firmware/iwlwifi-5000-2.ucode.sigcomm2010 /lib/firmware/iwlwifi-5000-2.ucode
sudo modprobe -r iwlwifi mac80211
sleep 2
sudo modprobe iwlwifi connector_log=0x1
sudo killall wpa_supplicant
