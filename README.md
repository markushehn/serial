# Serial-library implemented in C++

This library implements the access for the serial port based on the ```termios.h``` library and runs only on Linux systems.
The functionality and syntax of this library is similar to PySerial.
The supported serial data format is 8 data Bits, no parity and one stop bit and the supported baudrates are 9600, 19200, 38400, 57600, 115200, 1000000.
The function ```read``` waits for a specified data size.
The function ```readline``` waits until a ```'\n'```-character is received and returns the received line.
The line can be maximum 256 Bytes long, which is hardcoded.
A timeout value for the read operation is also supported.
If the timeout value is negative, the program is blocked as long as the requested data size is received in case of the ```read```-function or a ```'\n'```-character is received by usage of the ```readline```-function.



# Repository structure

```.kateproject``` includes the project definition for the editor "Kate".
```./src``` include the source files and ```./inc``` the header files.
```./src/main.c``` executes the library test and shows the basic usage of the library.



