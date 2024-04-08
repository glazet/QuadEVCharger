# SmartQuadEVCharger
Sonoma State University: 2023-24 Senior Design Project


The Smart Quad EV Charger project aims to address the increasing demand for Electric Vehicle (EV) charging infrastructure in the face of surging EV ownership. Designed as a Level-2 Electric Vehicle Supply Equipment (EVSE), this innovation accommodates up to four standard EVs simultaneously, optimizing charging capabilities within constrained parking spaces. Utilizing SAE communication protocols such as control pilot signals, the charger integrates an algorithm that dynamically manages charging sessions to provide charge to each plugged in EV. Rigorous testing, covering parameters such as control pilot voltage and frequency tolerances, along with switching device control, verifies the successful implementation of essential engineering requirements. This project offers an innovative solution to the evolving landscape of EV charging needs.

The proposed remedy is constructing EVSE that satisfies two constraints: capability of serving four EVs and IEC compliance. The build will be an alpha model, meaning the circuit and testing will revolve around hardware as opposed to software. This choice was made to match the design process of hardware engineering who are not directly responsible for creating the software and will eventually hand the project to a software development team. It is still important to have a basic understanding of the software in order to supply enough supporting hardware. The first constraint requires that the EVSE be able to manage four EVs at one time. Since we have standards and examples for single port EVSE, the true difference in this task boils down to adapting previous systems fourfold. This first aspect is the control system. The control system will be managed by an ESP32-WROOM 2. The ESP32-WROOM 2 has 16 PWM capable pins with 16-bit timers and supports 0% to 100% duty cycle. This will not only allow the EVSE to generate the control pilot but also generate the control pilot to the IEC 61851 specifications. The ESP32-WROOM 2 also has two ADCs that support 18 pins that will read the modified control pilots and current sensor. The extra pins will be necessary for future attachments. Additionally, the display requires SPI while the NFC requires I2C, both which the ESP32-WROOM 2 has. Finally, the ESP32-WROOM 2 has an integrated 2.4 GHz Wi-Fi module. The ESP32-WROOM 2 will also manage four 450BXX50-12s that will control current to each EV. Since most EVSE only use two phase power the 450BXX50-12 is 2 Form A DPST rated at 240 VAC 50 A, (dictated by IEC 62196). Unfortunately it has a triggered voltage of 12 VDC but since the control pilot needs a 12V source that is acceptable.  Since the control pilot needs to be operated at 12V or less, the LT1491 will be in comparator configuration to step up the 3.3V control pilot signal from the ESP32-WROOM 2 to 12 V and scale it down when needed. The LT1491 is a four channel device. Finally, the charging process will consist of dividing charging time as opposed to dividing current. This process was determined in two steps; first, the group determined the six most popular EV and using ecalc.ch to compare mileage achieved by giving four EV full power one at a time (40 A) for one hour and giving four EV quarter power (10 A) simultaneously. The tests had the same outcome. (This was originally predicted by noting that if power is constant, then if energy changes, time must change proportionally in opposition P=E/t). Second, since dividing the current would require far more manipulation of the control pilot, increased risk of exceeding 50 A and triggering the breaker, and unexpected EV behavior at power levels lower than the EV controller is expecting, time division was chosen to be the charging schema.
