**NOTE: This repo is NO LONGER MAINTAINED as these changes were applied upstream to LEDE. Refer to https://github.com/lede-project/source/commit/68d649f5cd9ee2f6fda00df7bde4b9074474becf and enjoy the official nightles!** 

# Openwrt-Z1

Bringup repo for the Cisco Meraki Z1 on OpenWRT!

Currently based on commit http://git.openwrt.org/?p=openwrt.git;a=commit;h=cccc6363725cab791014e9d38850f74091e9553c

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
