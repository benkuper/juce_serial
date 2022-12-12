/*
  ==============================================================================



  ==============================================================================
*/

#ifdef ORGANIC_SERIAL_H_INCLUDED
/* When you add this cpp file to your project, you mustn't include it in a file where you've
   already included any other headers - just put it inside a file on its own, possibly with your config
   flags preceding it, but don't include anything else. That also includes avoiding any automatic prefix
   header files that the compiler may be using.
*/
#error "Incorrect use of JUCE cpp file"
#endif

#include "juce_serial.h"

#include "lib/serial/impl/list_ports/list_ports_linux.cc"
#include "lib/serial/impl/list_ports/list_ports_osx.cc"
#include "lib/serial/impl/list_ports/list_ports_win.cc"

#include "lib/serial/impl/unix.cc"
#include "lib/serial/impl/win.cc"

#include "lib/serial/serial.cc"


#include "lib/cobs/cobs.cpp"
#include "SerialDevice.cpp"
#include "SerialManager.cpp"
#include "SerialDeviceParameter.cpp"