It is important that before setting up the Intel 5300 wireless adapter 
using the scripts in this directory that the user make the connection
used by the adapter unmanaged by the NetworkManager.

To do this, open the file /etc/network/interfaces
within, there will likely be the following lines, or something similar:


 # interfaces(5) file used by ifup(8) and ifdown(8)
 
 auto lo
 
 iface lo inet loopback
 
 

Insert the following line at the end of this: (you must have super user privileges to do this)

 iface wlan1 inet manual

Instead of "wlan1" above you may need to insert a different interface name, 
depending on the name assigned to the Intel 5300 wireless card on your machine.
You can check this by running ifconfig before inserting the card and then again
after you've inserted the card.
