# CovertChannel

This application is written in C code for a Linux operating system. It uses the UDP protocol to hide
data which is transferred across the internet.
It takes a covert data file and a dummy data file as parameters. The covert data is encoded into the UDP
source and destination port values. The dummy data is then split up into the data portion of the UDP
payload.
To encode the covert data into such a small size, the characters are compressed into 5-bits each instead
of the regular 8 bits. This allows 6 characters to be squeezed into the port fields instead of 4 with 2
extra bits. These bits are the most significant bits for each of the ports and are both set to 1. This is so
that the minimum port value is 32767 where there is a rare chance that another process will be using
this port and firewalls will usually let these packets through