/*
  ==============================================================================

  ==============================================================================
*/


/*******************************************************************************
 The block below describes the properties of this module, and is read by
 the Projucer to automatically generate project code that uses it.
 For details about the syntax and how to create or use a module, see the
 JUCE Module Format.txt file.


 BEGIN_JUCE_MODULE_DECLARATION

  ID:               juce_serial
  vendor:           benkuper
  version:          1.0.0
  name:             Serial
  description:      Serial Handling, using OrganicUI
  website:          https://github.com/benkuper/juce_serial
  license:          GPLv3

  dependencies:    juce_organicui
  windowsLibs:	   Setupapi
  OSXLibs:         libserial
 
 END_JUCE_MODULE_DECLARATION

*******************************************************************************/

#pragma once
#define ORGANIC_SERIAL_H_INCLUDED

//==============================================================================

#ifdef _MSC_VER
#pragma warning (push)
// Disable warnings for long class names, padding, and undefined preprocessor definitions.
#pragma warning (disable: 4251 4786 4668 4820)
#endif


#include <juce_organicui/juce_organicui.h>
using namespace juce;



#include "serial/serial.h"
#include "serial/v8stdint.h"

#include "serial/impl/unix.h"
#include "serial/impl/win.h"

#include "lib/cobs/cobs.h"
#include "SerialDevice.h"
#include "SerialManager.h"
#include "SerialDeviceParameter.h"
