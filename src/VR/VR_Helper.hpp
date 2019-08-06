#ifndef VR_HELPER_HPP
#define VR_HELPER_HPP

#include <string>
#include "openvr.h"

class VR_Helper {

public :

	std::string GetTrackedDeviceString(vr::TrackedDeviceIndex_t unDevice, vr::TrackedDeviceProperty prop, vr::TrackedPropertyError *pError = NULL); 

	bool GetDigitalActionRisingEdge(vr::VRActionHandle_t action, vr::VRInputValueHandle_t *pDevicePath = nullptr);
	bool GetDigitalActionState(vr::VRActionHandle_t action, vr::VRInputValueHandle_t *pDevicePath = nullptr);

	void ThreadSleep(unsigned long nMilliseconds);
};
#endif
