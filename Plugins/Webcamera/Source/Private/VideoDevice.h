// Copyright (C) 2016 Fredrik Lindh. All Rights Reserved.
// Created by Fredrik Lindh (Temaran) on 2016-01-28

#pragma once

#pragma comment(lib, "strmiids")

#include "COMTypes.h"
#include "VideoCallbackHandler.h"

class FVideoDevice
{
	friend FWebcameraModule;

public:
	FVideoDevice();
	virtual ~FVideoDevice();

	int32 GetId();
	const char*	GetFriendlyName();

	UTexture2D* Start();
	void Stop();

private:
	int32 Id;
	int32 Width;
	int32 Height;
	char* Friendlyname;
	WCHAR* Filtername;
	
	IBaseFilter* Sourcefilter;
	IBaseFilter* Samplegrabberfilter;
	IBaseFilter* Nullrenderer;

	ISampleGrabber* Samplegrabber;

	IFilterGraph2* Graph;
	FVideoCallbackHandler* Callbackhandler;
};