# Automated Irrigation system
Homebrew automated Irrigation system based on ESP32

Bill of materials:
* ESP32 based controllers (AZ-Delivery ESP32 DevKitC V2 ESP32-WROOM-32)
* Capacitive Soil Moisture Sensor v 1.2 (AZ-Delivery)
* KF-301 Relay Module with Low-level Trigger (AZ-Delivery)
To be continued...

```mermaid
C4Component
title Automated Irrigation system
Enterprise_Boundary(bc, "Logic") {
  System_Boundary(blc, "Logic Controller") {
    System(SN, "NodeRed")
    Rel(SQ1, SN, "Get Sensor Readings")
    Rel(SQ2, SN, "Receive Status Readings")
    Rel(SN, CQ1, "Send open/close commands")
  }
}

Enterprise_Boundary(bi, "Irrigation") {
  System_Boundary(bmq, "MQTT Broker") {
    SystemQueue(SQ2, "stat/moisture1")
    SystemQueue(SQ1, "stat/relay1")
    SystemQueue(CQ1, "cmnd/relay1")
  }
  System_Boundary(bs, "Soil Sensor Controller") {
    System(C1, "Soil Sensor Controller")
    System(S1, "Soil Moisture Sensor")
    BiRel(S1, C1, "")
  }
  System_Boundary(bv, "Valve Controller") {
    System(V1, "Water Valve")
    System(R1, "Relay1")
    System(C2, "Water Valve Controller")
    BiRel(V1, R1, "")
    BiRel(R1, C2, "")
  }
  
Rel(C1, SQ2, "Send readings periodically")
Rel(CQ1, C2, "Receive open/close commands")
Rel(C2, SQ1, "Send status periodically")
}
UpdateLayoutConfig($c4ShapeInRow="4", $c4BoundaryInRow="1")
```
