# WiFi Serial Gateway for the RustyBugA

Firmware and tools for a Seeed Studio XIAO C3 based serial gateway that uses WiFi as wireless interface.

# Usage
## Hardware connections
TBD

## Connect from the PC using netcat
Connect the PC to the same network as the gateway and use the mDNS name to create a connection:
```
nc -u ${MDNS_NAME}.local 1234
```
now data can be exchanged with the robot.

## Connect from the PC using tools
TBD

