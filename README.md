# attiny85-sleep-mcu

use a Attiny85 to switch power to e.g. a ESP8266, to build a power-efficient weather station with low-power pulse conting (e.g. rain meter).
for schematic, see docs/schematic.pdf

## I2C Protocol

### 0x01 - SLEEP 

power down the load and put the attiny into sleep mode.
the attiny will wake up after a specified time and power up the load again.
counting of pulses continues during sleep mode.

__parameters:__

`{4 bytes - sleep time seconds, big-endian}{1 byte - flags, optional}`

sleep time specifies the sleep duration in seconds, and is limited to 1 hour (set by `MAX_SLEEP_TIME`)

flags configures additional options `xxxx xxxW`:

| Bits | Description                                                                                         |
| ---- | --------------------------------------------------------------------------------------------------- |
| 0    | 0: wake up only after sleep time expires; 1: wake up on pulse (if any), or after sleep time expires |
| 1-7  | reserved, set to 0                                                                                  |

if flags are omitted, they are set to 0.

if all parameters are omitted, the attiny will enter sleep mode with the sleep time set by the previous powerup, or the default value (set by `DEFAULT_SLEEP_TIME`). flags are set to 0 in this case.


### 0x02 - GET_COUNTER

get the current value of the pulse counter, without resetting it.
after sending the command, the host should read back 4 bytes, which represent the counter value in big-endian format.

### 0x03 - GET_COUNTER_AND_RESET

same as `GET_COUNTER`, but resets the counter back to 0 after reading.
