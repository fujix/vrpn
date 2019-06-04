#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
//#include "vrpn_MessageMacros.h"
#include "vrpn_Analog_Leptrino.h"

#ifdef	_WIN32
#pragma comment (lib, "user32.lib")
#pragma comment (lib, "SetupAPI.lib")
#endif
#include <SetupAPI.h>
#include <devguid.h>
#include <RegStr.h>

namespace {
	// CfsUsb
	extern "C" __declspec(dllexport) void __stdcall Initialize();
	extern "C" __declspec(dllexport) void __stdcall Finalize();
	extern "C" __declspec(dllexport) bool __stdcall PortOpen(int pPortNo);
	extern "C" __declspec(dllexport) void __stdcall PortClose(int pPortNo);
	extern "C" __declspec(dllexport) bool __stdcall SetSerialMode(int pPortNo, bool pSet);
	extern "C" __declspec(dllexport) bool __stdcall GetSerialData(int pPortNo, double *pData, char *pStatus);
	extern "C" __declspec(dllexport) bool __stdcall GetLatestData(int pPortNo, double *pData, char *pStatus);
	extern "C" __declspec(dllexport) bool __stdcall GetSensorLimit(int pPortNo, double *pData);
	extern "C" __declspec(dllexport) bool __stdcall GetSensorInfo(int pPortNo, char *psSerial);
	typedef void (CALLBACK *FUNC_Initialize)();
	typedef void (CALLBACK *FUNC_Finalize)();
	typedef bool (CALLBACK *FUNC_PortOpen)(int);
	typedef void (CALLBACK *FUNC_PortClose)(int);
	typedef bool (CALLBACK *FUNC_SetSerialMode)(int, bool);
	typedef bool (CALLBACK *FUNC_GetSerialData)(int, double *, char *);
	typedef bool (CALLBACK *FUNC_GetLatestData)(int, double *, char *);
	typedef bool (CALLBACK *FUNC_GetSensorLimit)(int, double *);
	typedef bool (CALLBACK *FUNC_GetSensorInfo)(int, char *);
	FUNC_Initialize LeptrinoDLL_Initialize;
	FUNC_Finalize LeptrinoDLL_Finalize;
	FUNC_PortOpen LeptrinoDLL_PortOpen;
	FUNC_PortClose LeptrinoDLL_PortClose;
	FUNC_SetSerialMode LeptrinoDLL_SetSerialMode;
	FUNC_GetSerialData LeptrinoDLL_GetSerialData;
	FUNC_GetLatestData LeptrinoDLL_GetLatestData;
	FUNC_GetSensorLimit LeptrinoDLL_GetSensorLimit;
	FUNC_GetSensorInfo LeptrinoDLL_GetSensorInfo;
	static bool gLeptrinoDDLLoaded = false;
	inline bool LoadLeptrinoDDL() {
		if (gLeptrinoDDLLoaded) {
			return gLeptrinoDDLLoaded;
		}
		HMODULE hDll = LoadLibrary("CfsUsb.dll");
		if (hDll != NULL)
		{
			LeptrinoDLL_Initialize = (FUNC_Initialize)GetProcAddress(hDll, "Initialize");
			LeptrinoDLL_Finalize = (FUNC_Finalize)GetProcAddress(hDll, "Finalize");
			LeptrinoDLL_PortOpen = (FUNC_PortOpen)GetProcAddress(hDll, "PortOpen");
			LeptrinoDLL_PortClose = (FUNC_PortClose)GetProcAddress(hDll, "PortClose");
			LeptrinoDLL_SetSerialMode = (FUNC_SetSerialMode)GetProcAddress(hDll, "SetSerialMode");
			LeptrinoDLL_GetSerialData = (FUNC_GetSerialData)GetProcAddress(hDll, "GetSerialData");
			LeptrinoDLL_GetLatestData = (FUNC_GetLatestData)GetProcAddress(hDll, "GetLatestData");
			LeptrinoDLL_GetSensorLimit = (FUNC_GetSensorLimit)GetProcAddress(hDll, "GetSensorLimit");
			LeptrinoDLL_GetSensorInfo = (FUNC_GetSensorInfo)GetProcAddress(hDll, "GetSensorInfo");
			LeptrinoDLL_Initialize();
			gLeptrinoDDLLoaded = true;
		}
		else {
			fprintf(stderr, "Load CfsUsb.DLL failed!\n");
		}
		return gLeptrinoDDLLoaded;
	}
	inline int FindLeptrinoCOMPort(int deviceIndex) {
		std::vector<int> foundComPorts;
		TCHAR buffer[65535];
		if (::QueryDosDevice(NULL, buffer, sizeof buffer) == 0) {
			return -1;
		}
		TCHAR *p = buffer;
		while (*p != 0) {
			if (p[0] == 'C' && p[1] == 'O' && p[2] == 'M' && p[3] != 0) {
				foundComPorts.push_back(atoi(p + 3));
			}
			p += strlen(p) + 1;
		}
		/*GUID portsClassGUID[1];
		DWORD dwRequiredSize;
		if (!::SetupDiClassGuidsFromName("PORTS", portsClassGUID, 1, &dwRequiredSize)) {
			fprintf(stderr, "SetupDiClassGuidsFromName failed.");
			return -1;
		}
		HDEVINFO deviceInfoSet = ::SetupDiGetClassDevs(&portsClassGUID[0], NULL, NULL, DIGCF_PRESENT | DIGCF_PROFILE);
		if (!deviceInfoSet) {
			fprintf(stderr, "SetupDiGetClassDevs failed.");
			return -1;
		}
		SP_DEVINFO_DATA deviceInfoData;
		for (DWORD i = 0; ::SetupDiEnumDeviceInfo(deviceInfoSet, i++, &deviceInfoData);) {
			TCHAR szFriendlyName[MAX_PATH];
			TCHAR szPortName[MAX_PATH];
			DWORD dwPropType;

			dwRequiredSize = 0;
			if (!::SetupDiGetDeviceRegistryProperty(deviceInfoSet, &deviceInfoData, SPDRP_FRIENDLYNAME, &dwPropType, (LPBYTE)szFriendlyName, sizeof(szFriendlyName), &dwRequiredSize)) {
				break;
			}
			if (_strnicmp(szFriendlyName, "STM Virtual", 11) == 0) {
				//continue;

			}
			auto regKey = ::SetupDiOpenDevRegKey(deviceInfoSet, &deviceInfoData, DICS_FLAG_GLOBAL, 0, DIREG_DEV, KEY_READ);
			DWORD dwType = REG_SZ;
			if (regKey) {
				dwRequiredSize = sizeof(szPortName);
				::RegQueryValueEx(regKey, "PortName", 0, &dwType, (LPBYTE)&szPortName, &dwRequiredSize);
				::RegCloseKey(regKey);
			}
			printf("%s %s\n", szFriendlyName, szPortName);
			if (_strnicmp(szPortName, "COM", 3) == 0) {  // COMポートドライバを発見
				int port = atoi(&szPortName[3]);
				foundComPorts.push_back(port);
			}
		}*/
		if ((int)foundComPorts.size() > deviceIndex) {
			std::sort(foundComPorts.begin(), foundComPorts.end());
			return foundComPorts[deviceIndex];
		}
		return -1;
	}
}

vrpn_Analog_Leptrino::vrpn_Analog_Leptrino(const char* name, int deviceIndex, vrpn_Connection *c)
	: vrpn_Analog(name, c), lastFrameCount(0), reviceMicrosec(0), base{0}
{
	if (!LoadLeptrinoDDL()) {
		send_text_message("Could not load CfsUsb.dll.", timestamp, vrpn_TEXT_ERROR);
	}
	vrpn_gettimeofday(&timestamp, NULL);

	comPortNumber = FindLeptrinoCOMPort(deviceIndex);
	if (comPortNumber < 0) {
		send_text_message("Could not find COM port.", timestamp, vrpn_TEXT_ERROR);
	}
	if (!LeptrinoDLL_PortOpen(comPortNumber)) {
		send_text_message("Could not open COM port.", timestamp, vrpn_TEXT_ERROR);
	}
	LeptrinoDLL_SetSerialMode(comPortNumber, false);
	char info[1024] = { 0 };
	if (LeptrinoDLL_GetSensorInfo(comPortNumber, info)) {
		char message[1024];
		snprintf(message, sizeof message, "[vrpn_Analog_Leptrino]info:%s Fx[N] Fy[N] Fz[N] Mx[Nm] My[Nm] Mz[Nm]", info);
		send_text_message(message, timestamp, vrpn_TEXT_NORMAL);
	}
	if (LeptrinoDLL_GetSensorLimit(comPortNumber, sensorLimit)) {
		for (int i = 0; i < 6; i++) {
			sensorLimit[i] /= 10000.0;
		}
		char message[1024];
		snprintf(message, sizeof message, "[vrpn_Analog_Leptrino]sensorlimit:%+09.6lf %+09.6lf %+09.6lf %+09.6lf %+09.6lf %+09.6lf",
			sensorLimit[0], sensorLimit[1], sensorLimit[2], sensorLimit[3], sensorLimit[4], sensorLimit[5]);
		send_text_message(message, timestamp, vrpn_TEXT_NORMAL);
	} else {
		send_text_message("GetSensorLimit failed.", timestamp, vrpn_TEXT_ERROR);
	}
	char status;
	/*if (LeptrinoDLL_GetSerialData(comPortNumber, base, &status)) {
		VRPN_MSG_ERROR("Internal Error. serialmode!=false");
	}*/
	if (!LeptrinoDLL_GetLatestData(comPortNumber, base, &status)) {
		send_text_message("GetLatestData failed. (init)", timestamp, vrpn_TEXT_ERROR);
	}
}

vrpn_Analog_Leptrino::~vrpn_Analog_Leptrino()
{
	int ttl = 100;
	while (LeptrinoDLL_SetSerialMode(comPortNumber, false) == 0) {
		vrpn_SleepMsecs(10);
		if (--ttl <= 0) {
			send_text_message("SetSerialMode(off) failed.", timestamp, vrpn_TEXT_ERROR);
			break;
		}
	}
	LeptrinoDLL_PortClose(comPortNumber);
}

void vrpn_Analog_Leptrino::mainloop() {
	if (lastFrameCount == 0) {
		go();
	}

	captureOne();

	server_mainloop();
}

void vrpn_Analog_Leptrino::go()
{
	char status;
	/*if (LeptrinoDLL_GetLatestData(comPortNumber, base, &status)) {
		char message[1024];
		snprintf(message, sizeof message, "[vrpn_Analog_Leptrino]base:%+09.6lf %+09.6lf %+09.6lf %+09.6lf %+09.6lf %+09.6lf",
			base[0], base[1], base[2], base[3], base[4], base[5]);
		send_text_message(message, timestamp, vrpn_TEXT_NORMAL);
	}
	else {
		send_text_message("GetLatestData failed. (base)", timestamp, vrpn_TEXT_ERROR);
	}*/
	if (!LeptrinoDLL_SetSerialMode(comPortNumber, 1)) {
		send_text_message("SetSerialMode failed.", timestamp, vrpn_TEXT_ERROR);
	}
	vrpn_gettimeofday(&zeroTime, NULL);
	reviceMicrosec = 0;
	lastFrameCount = 1;
}

void vrpn_Analog_Leptrino::captureOne()
{
	auto savedLastFrameCount = lastFrameCount;
	auto maxFrameCountForYeild = lastFrameCount + 100;
	double force[6];
	char	status;

	num_channel = 6;
	while (LeptrinoDLL_GetSerialData(comPortNumber, force, &status)) {
		channel[0] = (force[0] - base[0])*sensorLimit[0];
		channel[1] = (force[1] - base[1])*sensorLimit[1];
		channel[2] = (force[2] - base[2])*sensorLimit[2];
		channel[3] = (force[3] - base[3])*sensorLimit[3];
		channel[4] = (force[4] - base[4])*sensorLimit[4];
		channel[5] = (force[5] - base[5])*sensorLimit[5];

		timeval delta;
		delta.tv_sec = lastFrameCount / 1200; // 1200Hz
		delta.tv_usec = long(round((lastFrameCount % 1200) * 1000000.0 / 1200.0)) + reviceMicrosec;
		timestamp = vrpn_TimevalSum(zeroTime, delta);

		report(vrpn_CONNECTION_LOW_LATENCY, timestamp);

		lastFrameCount++;
		if (lastFrameCount > maxFrameCountForYeild) {
			return; // yeild for another device
		}
	}
	if (lastFrameCount == savedLastFrameCount) {
		// LeptrinoDLL_GetSerialData failed. Maybe not connected.
		return; // do not need to revice clock.
	}
	// not yeild, just no more serial data
	timeval now;
	vrpn_gettimeofday(&now, NULL);
	long delay = now.tv_usec - timestamp.tv_usec;
	delay += 1000000L * (now.tv_sec - timestamp.tv_sec);
	if (delay > 1000) { // > 1ms
		reviceMicrosec += delay/100*100+1;

		if (delay > 2000) { // > 2ms
			char message[1024];
			snprintf(message, sizeof message, "[vrpn_Analog_Leptrino]revicedMicrosec:%ld %+07ld", lastFrameCount, reviceMicrosec);
			send_text_message(message, timestamp, vrpn_TEXT_WARNING);
		}
	}
}
