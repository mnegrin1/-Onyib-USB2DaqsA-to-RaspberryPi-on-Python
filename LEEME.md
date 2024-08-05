# Onyib USB2DaqsA a Raspberry Pi en Python
La forma que encontramos para transferir datos desde la placa Onyib USB2DaqsA, que incluye el módulo ADC AD7606, a un código en Python dentro de una Raspberry Pi.

## Funcionamiento
El script build.sh compila main.cpp, ad7606.cpp y type.h en un único archivo (llámalo ejecutable) llamado "readings".

"readings" se genera a partir de la secuencia de main.cpp (puedes editarlo según te convenga), que gestiona el uso de la biblioteca libusb para comunicar la placa.

(No pudimos hacer que libusb o pyusb funcionaran en Python para esta placa).

Para obtener las lecturas, primero necesitamos compilar el código C++ y luego, mediante Python, ejecutarlo para obtener la salida que el archivo imprime.

Entonces, el archivo read_ad7606.py realiza:

La compilación,
La ejecución,
La lectura y el procesamiento de las impresiones que provienen de la salida de "readings".





