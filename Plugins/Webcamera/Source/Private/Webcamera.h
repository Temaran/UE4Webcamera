// Copyright (C) 2016 Fredrik Lindh. All Rights Reserved.
// Created by Fredrik Lindh (Temaran) on 2016-01-28

#pragma once

#include "IWebcamera.h"
#include "COMTypes.h"
#include "Runtime/Engine/Classes/Engine/Texture2D.h"

class FWebcameraModule : public IWebcamera
{

	/************************************************************************/
	/* IWebcamera                                                           */
	/************************************************************************/
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	virtual UTexture2D* Connect(int32 DeviceIndex) override;
	virtual void Disconnect(int32 DeviceIndex) override;
	virtual int32 GetNumDevices() override;

protected:
	int32 ReacquireDevices();
	class FVideoDevice* GetDevices();
	void InitializeGraph();
	void InitializeVideo();

private:
	IFilterGraph2* Graph;
	ICaptureGraphBuilder2* Capture;
	IMediaControl* Control;
	FVideoDevice* Devices;
	FVideoDevice* Current;

	int32 NumDevices;
	bool IsPlaying;
};