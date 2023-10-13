/*
  ==============================================================================

	SerialDevice.cpp
	Created: 15 Mar 2017 10:15:01am
	Author:  Ben-Portable

  ==============================================================================
*/

#include "JuceHeader.h"

#if SERIALSUPPORT
SerialDevice::SerialDevice(Serial* _port, SerialDeviceInfo* _info, PortMode _mode) :
	thread(_info->port, this),
	port(_port),
	info(_info),
	mode(_mode)
{
	open();
}
#else
SerialDevice::SerialDevice(SerialDeviceInfo* _info, PortMode _mode) :
	info(_info),
	mode(_mode),
	thread(_info->port, this)
{
	open();
}
#endif

SerialDevice::~SerialDevice()
{
	//cleanup
	close();
	listeners.call(&SerialDeviceListener::portRemoved, this);
}

void SerialDevice::setMode(PortMode _mode)
{
	if (mode == _mode) return; //do nothing if the same

	if (_mode == LINES) //must restart to make sure thread is not hanging in readLine
	{
		if (thread.isThreadRunning())
		{
			thread.stopThread(1000);
			thread.startThread();
		}
	}

	mode = _mode;
}

void SerialDevice::setDTR(bool val)
{
	if(port != nullptr) port->setDTR(val);
}

void SerialDevice::setRTS(bool val)
{
	if(port != nullptr) port->setRTS(val);
}

void SerialDevice::setBaudRate(int baudRate)
{
	if (port != nullptr)
	{
		DBG("Port is null here, not setting baudrate");
		if ((uint32_t)baudRate == port->getBaudrate()) return;

		port->setBaudrate(baudRate);
	}
}

void SerialDevice::setParity(int parity)
{
	if (port != nullptr)
	{
		DBG("Port is null here, not setting baudrate");
		if ((parity_t)parity == port->getParity()) return;

		port->setParity((parity_t)parity);
	}

}

void SerialDevice::setStopBits(int stopBits)
{
	if (port != nullptr)
	{
		DBG("Port is null here, not setting baudrate");
		if ((stopbits_t)stopBits == port->getStopbits()) return;

		port->setStopbits((stopbits_t)stopBits);
	}
}

void SerialDevice::setDataBits(int dataBits)
{
	if (port != nullptr)
	{
		DBG("Port is null here, not setting baudrate");
		if ((bytesize_t)dataBits == port->getBytesize()) return;

		port->setBytesize((bytesize_t)dataBits);
	}
}

void SerialDevice::open(int baud)
{
#if SERIALSUPPORT

	openedOk = false;

	if (port == nullptr) return;
	try
	{
		if (baud != -1) port->setBaudrate(baud);
		if (!port->isOpen())  port->open();
		//port->setDTR();
		//port->setRTS();


		if (!thread.isThreadRunning())
		{
			thread.addSerialListener(this);
			thread.startThread();
#ifdef SYNCHRONOUS_SERIAL_LISTENERS
#else
			thread.addAsyncSerialListener(this);
#endif
			listeners.call(&SerialDeviceListener::portOpened, this);
		}

		openedOk = true;
	}
	catch (std::exception e)
	{
		NLOGERROR("Serial", "Error opening the port " << info->description << ", try reconnecting the device.");
		openedOk = false;
	}

#endif
}

void SerialDevice::close()
{
#if SERIALSUPPORT
	if (isOpen())
	{


#ifdef SYNCHRONOUS_SERIAL_LISTENERS
		thread.removeSerialListener(this);
#else
		thread.removeAsyncSerialListener(this);
#endif

		thread.stopThread(1000);

		try
		{
			port->close();

		}
		catch (std::exception e)
		{
			NLOGWARNING("Serial", "Error closing port.");
		}

		listeners.call(&SerialDeviceListener::portClosed, this);
	}
#endif
}

bool SerialDevice::isOpen() {
#if SERIALSUPPORT
	if (port == nullptr) return false;
	return port->isOpen() && openedOk;
#else
	return false;
#endif
}

int SerialDevice::writeString(String message)
{
#if SERIALSUPPORT
	if (!port->isOpen()) return 0;

	try
	{
		return (int)port->write(message.toStdString());
	}
	catch (std::exception e)
	{
		LOGWARNING("Error writing to serial : " << e.what());
		return 0;
	}

#else
	return 0;
#endif
}

int SerialDevice::writeBytes(Array<uint8_t> data)
{
#if SERIALSUPPORT
	try
	{
		int result = (int)port->write(data.getRawDataPointer(), data.size());
		return result;
	}
	catch (std::exception e)
	{
		NLOGERROR("Serial", "Error writing to serial : " << e.what());
		return 0;
	}
#else
	return 0;
#endif
}

void SerialDevice::dataReceived(const var& data) {
	listeners.call(&SerialDeviceListener::serialDataReceived, this, data);
}

void SerialDevice::addSerialDeviceListener(SerialDeviceListener* newListener) { listeners.add(newListener); }

void SerialDevice::removeSerialDeviceListener(SerialDeviceListener* listener) {
	listeners.remove(listener);
	if (listeners.size() == 0) {
		SerialManager* manager = SerialManager::getInstance();
		manager->removePort(this);
	}
}

SerialReadThread::SerialReadThread(String name, SerialDevice* _port) :
	Thread(name + "_thread"),
	port(_port)
{
}

SerialReadThread::~SerialReadThread()
{
	stopThread(1000);
}

void SerialReadThread::run()
{
#if SERIALSUPPORT

	std::vector<uint8_t> byteBuffer; //for cobs and data255

	while (!threadShouldExit())
	{
		sleep(2); //500fps

		if (port == nullptr) return;
		if (!port->isOpen()) return;

		try
		{

			size_t numBytes = (int)port->port->available();
			if (numBytes == 0) continue;

			switch (port->mode)
			{

			case SerialDevice::PortMode::LINES:
			{
				while (port->port->available() && port->mode == SerialDevice::PortMode::LINES)
				{
					std::string line = port->port->readline();
					if (line.size() > 0) serialThreadListeners.call(&SerialThreadListener::dataReceived, var(line));
				}
			}
			break;

			case SerialDevice::PortMode::DIRECT:
			{
				std::vector<uint8_t> data;
				port->port->read(data, numBytes);
				String s(std::string(data.begin(), data.end()));
				serialThreadListeners.call(&SerialThreadListener::dataReceived, var(s));
			}
			break;

			case SerialDevice::PortMode::RAW:
			{
				std::vector<uint8_t> data;
				port->port->read(data, numBytes);
				//for (int i = 0; i < data.size(); ++i) DBG("Data " << data[i]);
				serialThreadListeners.call(&SerialThreadListener::dataReceived, var(data.data(), numBytes));
			}
			break;

			case SerialDevice::PortMode::DATA255:
			{
				while (port->port->available() && port->mode == SerialDevice::PortMode::DATA255)
				{
					uint8_t b = port->port->read(1)[0];
					if (b == 255)
					{
						serialThreadListeners.call(&SerialThreadListener::dataReceived, var(byteBuffer.data(), byteBuffer.size()));
						byteBuffer.clear();
					}
					else
					{
						byteBuffer.push_back(b);
					}
				}
			}
			break;


			case SerialDevice::PortMode::JSON:
			{
				std::vector<uint8_t> data;
				port->port->read(data, numBytes);
				byteBuffer.insert(byteBuffer.end(), data.begin(), data.end());

				String s(std::string(byteBuffer.begin(), byteBuffer.end()));
				var o = JSON::parse(s);
				if (o.isObject())
				{
					serialThreadListeners.call(&SerialThreadListener::dataReceived, var(s));
					byteBuffer.clear();
				}
			}
			break;

			case SerialDevice::PortMode::COBS:
			{
				while (port->port->available() && port->mode == SerialDevice::PortMode::COBS)
				{
					uint8_t b = port->port->read(1)[0];
					byteBuffer.push_back(b);
					if (b == 0)
					{
						uint8_t decodedData[255];
						size_t numDecoded = cobs_decode(byteBuffer.data(), byteBuffer.size(), decodedData);
						serialThreadListeners.call(&SerialThreadListener::dataReceived, var(decodedData, numDecoded - 1));
						byteBuffer.clear();
					}
				}
			}
			break;
			}
		}
		catch (...)
		{
			DBG("### Serial Problem ");
		}
	}

	DBG("END SERIAL THREAD");
#endif

}

SerialDeviceInfo::SerialDeviceInfo(String _port, String _description, String _hardwareID) :
	port(_port), description(_description), hardwareID(_hardwareID)
{
#if JUCE_WINDOWS
	vid = hardwareID.substring(8, 12).getHexValue32();
	pid = hardwareID.substring(17, 21).getHexValue32();
	deviceID = description;
	uniqueDescription = description; //COM port integrated in description
#else
	String sn = "not found";
	vid = 0;
	pid = 0;

	if (hardwareID.startsWith("USB VID:PID=")) // USB COM port
	{
		StringArray eqSplit;
		eqSplit.addTokens(hardwareID, "=", "\"");
		if (eqSplit.size() > 1)
		{
			vid = eqSplit[1].substring(0, 4).getHexValue32();
			pid = eqSplit[1].substring(5, 9).getHexValue32();
			sn = eqSplit[eqSplit.size() - 1];
		}
	}
	else if (hardwareID.startsWith("PNP")) // Hardware COM port
	{
		pid = hardwareID.substring(3, 7).getHexValue32();
		sn = hardwareID;
	}

	deviceID = hardwareID;
	uniqueDescription = (vid == 0 && pid == 0) ? _port : description + "(SN : " + sn + ")";
#endif
}
