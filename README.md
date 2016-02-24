# Openwrt-Z1

Bringup repo for the Cisco Meraki Z1 on OpenWRT!

Building
-----
#### Build Only
`./build.sh`

#### Modify Configs and Build
`./build.sh modify`

Note that you will need to run a modify on the first compile to select the NAND image in the OpenWRT menuconfig.

To Do
-----
##### Z1
  * Fixup WiFi on PCIe
  * Cleanup Eth config? (maybe)
  * Merge upstream!

Working
-----
##### Z1
  * Sysupgrade & Caldata generation
  * WiFi (mostly)
  * Ethernet
  * LED's/GPIOs
  * System Integration
  * Kernel Device Profile
  * USB

Notice
------
No promises this won't brick your AP, and no promises that this will even work!
