# Openwrt-Z1

Bringup repo for the Cisco Meraki Z1 on OpenWRT!

Currently based on commit http://git.openwrt.org/?p=openwrt.git;a=commit;h=2e4094eb214f8214a98b8354e8cd0b4e7d8b84a5

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
