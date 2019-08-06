#ifndef VR_MANAGER_HPP
#define VR_MANAGER_HPP

#include <string>
#include <vector>
#include <memory>

#include "openvr.h"
#include "Matrices.h"
#include "CGLRenderModel.hpp"
#include "VR_Helper.hpp"
#include "SystemInfo.hpp"

#ifdef __APPLE__ 
#include <GL/glew.h>
#include "GLFW/glfw3.h"
#elif _WIN32 
#include "GL/glew.h"
#include "glfw3.h"
#endif

class VR_Manager {

public:

	VR_Manager(std::unique_ptr<ExecutionFlags>& flagPtr);

	bool BInit();
	bool BInitCompositor();
	void ExitVR();
	bool HandleInput();
	void ProcessVREvent(const vr::VREvent_t &event);
	Matrix4 ConvertSteamVRMatrixToMatrix4(const vr::HmdMatrix34_t &matPose);
	CGLRenderModel* FindOrLoadRenderModel(const char* pchRenderModelName);

	bool BSetupCameras();
	void UpdateHMDMatrixPose();
	Matrix4 GetHMDMatrixProjectionEye(vr::Hmd_Eye nEye);
	Matrix4 GetHMDMatrixPoseEye(vr::Hmd_Eye nEye);
	Matrix4 GetCurrentViewProjectionMatrix(vr::Hmd_Eye nEye);
	Matrix4 GetCurrentViewEyeMatrix(vr::Hmd_Eye nEye);
	Matrix4 GetCurrentViewMatrix(vr::Hmd_Eye nEye);
	Matrix4 GetCurrentProjectionMatrix(vr::Hmd_Eye nEye);
	Matrix4 GetCurrentEyeMatrix(vr::Hmd_Eye nEye);

	bool BGetRotate3DTrigger();
	float GetNearClip();
	float GetFarClip();

	Vector4 GetFarPlaneDimensions(vr::Hmd_Eye nEye);

	enum EHand
	{
		Left = 0,
		Right = 1,
	};

	friend class Graphics;

private:

	std::unique_ptr<VR_Helper> helper;
	vr::IVRSystem *m_pHMD;		

	std::string m_strDriver;
	std::string m_strDisplay;

	struct ControllerInfo_t
	{
		vr::VRInputValueHandle_t m_source = vr::k_ulInvalidInputValueHandle;
		vr::VRActionHandle_t m_actionPose = vr::k_ulInvalidActionHandle;
		vr::VRActionHandle_t m_actionHaptic = vr::k_ulInvalidActionHandle;
		Matrix4 m_rmat4Pose;
		CGLRenderModel *m_pRenderModel = nullptr;
		std::string m_sRenderModelName;
		bool m_bShowController;
	};
	ControllerInfo_t m_rHand[2];

	Vector2 m_vAnalogValue;

	Matrix4 m_mat4eyePosLeft;
	Matrix4 m_mat4eyePosRight;

	Matrix4 m_mat4ProjectionLeft;
	Matrix4 m_mat4ProjectionRight;

	Matrix4 m_mat4HMDPose;

	std::vector<CGLRenderModel*> m_vecRenderModels;
	
	vr::VRActionHandle_t m_actionRotateStructure = vr::k_ulInvalidActionHandle;
	vr::VRActionHandle_t m_actionHideThisController = vr::k_ulInvalidActionHandle;
	vr::VRActionHandle_t m_actionTriggerHaptic = vr::k_ulInvalidActionHandle;
	vr::VRActionHandle_t m_actionAnalongInput = vr::k_ulInvalidActionHandle;
	
	vr::VRActionSetHandle_t m_actionsetAvr = vr::k_ulInvalidActionSetHandle;


	vr::TrackedDevicePose_t m_rTrackedDevicePose[ vr::k_unMaxTrackedDeviceCount ];
	int m_iValidPoseCount;
	std::string m_strPoseClasses;                            // what classes we saw poses for this frame
	Matrix4 m_rmat4DevicePose[vr::k_unMaxTrackedDeviceCount];
	char m_rDevClassChar[vr::k_unMaxTrackedDeviceCount];   // for each device, a character representing its class
	float m_fNearClip;
	float m_fFarClip;
	bool m_bDebugPrint;

	bool m_bRotate3D;
	
};
#endif
