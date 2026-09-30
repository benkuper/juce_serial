#if defined(_WIN32)

/*
 * Copyright (c) 2014 Craig Lilley <cralilley@gmail.com>
 * This software is made available under the terms of the MIT licence.
 * A copy of the licence can be obtained from:
 * http://opensource.org/licenses/MIT
 */

#include "../../../../serial/serial.h"
#include <windows.h>
#include <setupapi.h>
#include <initguid.h>
#include <devguid.h>

using serial::PortInfo;
using std::vector;
using std::string;

static const size_t port_name_max_length = 256;
static const size_t friendly_name_max_length = 256;
static const size_t hardware_id_max_length = 256;

// The serial library exposes UTF-8 strings on every platform.
static string utf8_encode(const wchar_t* text)
{
	const int size_needed = WideCharToMultiByte(CP_UTF8, 0, text, -1, NULL, 0, NULL, NULL);
	if (size_needed <= 1)
		return {};

	string result(size_needed, '\0');
	if (WideCharToMultiByte(CP_UTF8, 0, text, -1, &result[0], size_needed, NULL, NULL) == 0)
		return {};

	result.pop_back(); // Remove the terminator included by WideCharToMultiByte.
	return result;
}

vector<PortInfo>
serial::list_ports()
{
	vector<PortInfo> devices_found;

	HDEVINFO device_info_set = SetupDiGetClassDevsW(
		(const GUID *) &GUID_DEVCLASS_PORTS,
		NULL,
		NULL,
		DIGCF_PRESENT);

	unsigned int device_info_set_index = 0;
	SP_DEVINFO_DATA device_info_data;

	device_info_data.cbSize = sizeof(SP_DEVINFO_DATA);

	while(SetupDiEnumDeviceInfo(device_info_set, device_info_set_index, &device_info_data))
	{
		device_info_set_index++;

		// Get port name

		HKEY hkey = SetupDiOpenDevRegKey(
			device_info_set,
			&device_info_data,
			DICS_FLAG_GLOBAL,
			0,
			DIREG_DEV,
			KEY_READ);

		if (hkey == INVALID_HANDLE_VALUE)
			continue;

		wchar_t port_name[port_name_max_length];
		DWORD port_name_length = sizeof(port_name);

		LONG return_code = RegQueryValueExW(
					hkey,
					L"PortName",
					NULL,
					NULL,
					(LPBYTE)port_name,
					&port_name_length);

		RegCloseKey(hkey);

		if(return_code != EXIT_SUCCESS)
			continue;

		if(port_name_length >= sizeof(wchar_t) && port_name_length <= sizeof(port_name))
			port_name[port_name_length / sizeof(wchar_t) - 1] = L'\0';
		else
			port_name[0] = L'\0';

		// Ignore parallel ports

		if(wcsstr(port_name, L"LPT") != NULL)
			continue;

		// Get port friendly name

		wchar_t friendly_name[friendly_name_max_length];
		DWORD friendly_name_actual_length = 0;

		BOOL got_friendly_name = SetupDiGetDeviceRegistryPropertyW(
					device_info_set,
					&device_info_data,
					SPDRP_FRIENDLYNAME,
					NULL,
					(PBYTE)friendly_name,
					sizeof(friendly_name),
					&friendly_name_actual_length);

		if(got_friendly_name == TRUE && friendly_name_actual_length >= sizeof(wchar_t)
			&& friendly_name_actual_length <= sizeof(friendly_name))
			friendly_name[friendly_name_actual_length / sizeof(wchar_t) - 1] = L'\0';
		else
			friendly_name[0] = L'\0';

		// Get hardware ID

		wchar_t hardware_id[hardware_id_max_length];
		DWORD hardware_id_actual_length = 0;

		BOOL got_hardware_id = SetupDiGetDeviceRegistryPropertyW(
					device_info_set,
					&device_info_data,
					SPDRP_HARDWAREID,
					NULL,
					(PBYTE)hardware_id,
					sizeof(hardware_id),
					&hardware_id_actual_length);

		if(got_hardware_id == TRUE && hardware_id_actual_length >= sizeof(wchar_t)
			&& hardware_id_actual_length <= sizeof(hardware_id))
			hardware_id[hardware_id_actual_length / sizeof(wchar_t) - 1] = L'\0';
		else
			hardware_id[0] = L'\0';

		PortInfo port_entry;
		port_entry.port = utf8_encode(port_name);
		port_entry.description = utf8_encode(friendly_name);
		port_entry.hardware_id = utf8_encode(hardware_id);

		devices_found.push_back(port_entry);
	}

	SetupDiDestroyDeviceInfoList(device_info_set);

	return devices_found;
}

#endif // #if defined(_WIN32)
