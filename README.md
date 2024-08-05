# Onyib USB2DaqsA to RaspberryPi on Python
The way we found to bring data from the Onyib USB2DaqsA board, which includes the AD7606 ADC module, to a python code inside a Raspberry Pi

## Functioning

build.sh compiles main.cpp, ad7606.cpp and type.h in a single (call it executable) file called "readings"

"readings" comes from the secuence of main.cpp (you can edit it to your convenience), which manage the use of the libusb library to communicate the board.

(We couldn't made the libusb or pyusb work in python for this board.)

To obtain the readings, firstly we need to compile the c++ and then, by python, execute it to obtain the output that the file prints.

So, the file read_ad7606.py makes: 
    1. the compilation, 
    2. the execution 
    3. the reading and processing of the prints that comes from the output of "readings".

