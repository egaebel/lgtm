#!/usr/bin/sudo /bin/bash

#sudo ifmetric wlan0 0
#sudo ifmetric wlan1 10
#sudo ifmetric wlan2 10

# The three lines below seem to be unncessary....killall wpa_supplicant seems to do the trick -_-
#sudo modprobe iwlwifi mac80211 cfg80211
#sudo modprobe iwlwifi mac80211 cfg80211
#sudo restart network-manager
sudo killall wpa_supplicant
