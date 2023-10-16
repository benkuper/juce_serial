/*
  ==============================================================================

    SerialDeviceParameter.h
    Created: 15 Mar 2017 10:14:56am
    Author:  Ben-Portable

  ==============================================================================
*/

#pragma once

class SerialDeviceParameter :
	public EnumParameter,
	public SerialManager::SerialManagerListener
{
public:
	SerialDeviceParameter(const String &name, const String &description, bool enabled);
	~SerialDeviceParameter();

	SerialDevice * currentDevice;
	SerialDevice * getDevice();

	String ghostData;
	int baudRate;
	bool dtr;
	bool rts;
	Array<int> vidFilters;
	Array<int> pidFilters;

	bool openOnSet;

	void setValueFromDevice(SerialDevice* device);
	void setValueInternal(var &value) override;

	void setBaudrate(int val);
	void setDTR(bool val);
	void setRTS(bool val);
	void setVIDPIDFilters(Array<int> vidFilters, Array<int> pidFilters);
	void updatePortList();

	// Inherited via SerialManagerListener
	virtual void portAdded(SerialDeviceInfo * info) override;
	virtual void portRemoved(SerialDeviceInfo * info) override;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SerialDeviceParameter)
};