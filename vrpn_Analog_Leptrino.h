#pragma once
#include "vrpn_Analog.h"

class VRPN_API vrpn_Analog_Leptrino : public vrpn_Analog
{
public:
	vrpn_Analog_Leptrino(const char* name, int deviceIndex = 0, vrpn_Connection *c = 0);
	virtual ~vrpn_Analog_Leptrino();
	virtual void mainloop();
	//std::string get_description() const;

protected:
	//void report_changes(vrpn_uint32 class_of_service = vrpn_CONNECTION_LOW_LATENCY);
	//void report(vrpn_uint32 class_of_service = vrpn_CONNECTION_LOW_LATENCY);

	void go();
	void captureOne();

	int comPortNumber;
	double sensorLimit[6];
	double base[6];
	struct timeval zeroTime;  //< When the time counter was zeroed
	long reviceMicrosec;
	long lastFrameCount;
};
