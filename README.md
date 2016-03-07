# Openwrt-Z1

Bringup repo for the Cisco Meraki Z1 on OpenWRT!

Currently based on commit http://git.openwrt.org/?p=openwrt.git;a=commit;h=63bce15a60d6a307540662f87ce1b2515c553800

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
  * Merge upstream!

Working
-----
##### Z1
  * Sysupgrade & Caldata generation
  * WiFi (PCIe loaded via module)
  * Ethernet
  * LED's/GPIOs
  * System Integration
  * Kernel Device Profile
  * USB

Notice
------
No promises this won't brick your AP, and no promises that this will even work!
