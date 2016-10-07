#include "RFProtocolMotionSensor.h"

static const range_type g_timing_pause[7] =
{
	{ 380, 750 },
	{ 851, 1400 },
	{ 1800, 2300 },
	{ 2500, 2700 },
	{ 0,0 }
};  // TODO FIXIT 500us, 1000us

static const range_type g_timing_pulse[8] =
{
	{ 200, 650 },
	{ 700, 1100 },
	{ 1800, 2300 },
	{ 2500, 2700 },
	{ 0,0 }
};   // TODO FIXIT 500us, 1000us


CRFProtocolMotionSensor::CRFProtocolMotionSensor()
	:CRFProtocol(g_timing_pause, g_timing_pulse, 0, 1, "aAaAaAaAaAaAaAaAaAaAaA")
{
	m_Debug = true;

}


CRFProtocolMotionSensor::~CRFProtocolMotionSensor()
{
}
