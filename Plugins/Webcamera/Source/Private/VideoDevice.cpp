// Copyright (C) 2016 Fredrik Lindh. All Rights Reserved.
// Created by Fredrik Lindh (Temaran) on 2016-01-28

#include "WebcameraPrivatePCH.h"

FVideoDevice::FVideoDevice()
{
	Friendlyname = (char*)FMemory::Malloc(MAX_DEVICE_NAME * sizeof(char));
	Filtername = (WCHAR*)FMemory::Malloc(MAX_DEVICE_NAME * sizeof(WCHAR));
	
	FMemory::Memset(Friendlyname, 0, MAX_DEVICE_NAME * sizeof(char));
	FMemory::Memset(Filtername, 0, MAX_DEVICE_NAME * sizeof(WCHAR));

	Id = -1;
	Sourcefilter = 0;
	Samplegrabberfilter = 0;
	Nullrenderer = 0;
	Callbackhandler = new FVideoCallbackHandler();
}

FVideoDevice::~FVideoDevice()
{
	FMemory::Free(Friendlyname);
	FMemory::Free(Filtername);
}

int FVideoDevice::GetId()
{
	return Id;
}

const char* FVideoDevice::GetFriendlyName()
{
	return Friendlyname;
}

UTexture2D* FVideoDevice::Start()
{
	HRESULT HResult;

	HResult = Nullrenderer->Run(0);
	if (HResult < 0) throw HResult;

	HResult = Samplegrabberfilter->Run(0);
	if (HResult < 0) throw HResult;

	HResult = Sourcefilter->Run(0);
	if (HResult < 0) throw HResult;

	return Callbackhandler->RecreateTexture(Width, Height);
}

void FVideoDevice::Stop()
{
	HRESULT HResult;

	HResult = Sourcefilter->Stop();
	if (HResult < 0) throw HResult;

	HResult = Samplegrabberfilter->Stop();
	if (HResult < 0) throw HResult;

	HResult = Nullrenderer->Stop();
	if (HResult < 0) throw HResult;
}
