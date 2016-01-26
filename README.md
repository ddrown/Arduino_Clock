Clock is based on the Arduino Time Library (https://github.com/PaulStoffregen/Time)

new functions:

void now\_ms(struct timems \*tms); -- return the current time with millisecond accuracy

void setTime\_ms(struct timems \*tms); -- set the current time with millisecond accuracy

void adjustTime\_ms(int16\_t X); -- jump the time forward or back X ms

int adjustClockSpeed(uint16\_t StepSeconds, int8\_t new\_AddRemoveMS); -- run the clock faster or slower by adding/removing 1 millisecond every StepSeconds. new\_AddRemoveMS 1=local clock is fast, -1=local clock is slow
