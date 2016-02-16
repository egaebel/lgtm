#!/bin/sh

echo Setting up 5300 wifi firmware with logging....
sudo ./set-5300-wifi.sh

echo Sleeping to allow wifi firmware to take hold..
sleep 2

echo Setting up wifi connection....................
sudo ./5300-network-setup.sh
