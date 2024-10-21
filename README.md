# attiny13-sleep-mcu

use a Attiny13a to switch power to e.g. a ESP8266, to build a power-efficient weather station
for schematic, see docs/schematic.pdf

## Power Usage

the following table shows the power usage of the Attiny13a during sleep mode (attiny13_slow environment):

| Supply Voltage | Power Usage | Notes                                                    |
| -------------- | ----------- | -------------------------------------------------------- |
| 1.8 V          | 4.3 uA      | On the low end of the voltage range, may not be reliable |
| 2.0 V          | 4.7 uA      |
| 2.5 V          | 5.3 uA      |
| 3.0 V          | 5.8 uA      |
| 3.3 V          | 6.0 uA      |
| 3.5 V          | 6.1 uA      |
| 4.0 V          | 6.7 uA      |
| 4.5 V          | 7.4 uA      |
| 5.0 V          | 8.2 uA      |

> [!NOTE]
> measurement were taken with a UNI-T UT61C multimeter, and may not be accurate.
>
> test procedure:
>
> 1. set voltage to desired value
> 2. power on device under test
> 3. wait for voltage to settle
> 4. assert `POWER_DOWN` pin
> 5. wait for current to settle
> 6. measure current
> 7. power off device under test
