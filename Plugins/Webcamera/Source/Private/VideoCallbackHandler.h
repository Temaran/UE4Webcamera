// Copyright (C) 2016 Fredrik Lindh. All Rights Reserved.
// Created by Fredrik Lindh (Temaran) on 2016-01-28

#pragma once

#include "COMTypes.h"
#include "Runtime/Engine/Classes/Engine/Texture2D.h"
#include "Runtime/Core/Public/UObject/WeakObjectPtrTemplates.h"

class FVideoDevice;
typedef void(*FVideoCaptureCallback)(unsigned char* Data, int Length, int BitsPerPixel, FVideoDevice* Device);

class FVideoCallbackHandler : public ISampleGrabberCB
{
public:
	FVideoCallbackHandler();
	~FVideoCallbackHandler();

	UTexture2D* RecreateTexture(int32 SourceWidth, int32 SourceHeight);

	virtual HRESULT __stdcall SampleCB(double Time, IMediaSample* Sample);
	virtual HRESULT __stdcall BufferCB(double Time, BYTE* Buffer, long Length);
	virtual HRESULT __stdcall QueryInterface(REFIID Iid, LPVOID *VoidPtrPtr);
	virtual ULONG __stdcall AddRef();
	virtual ULONG __stdcall Release();

private:
	TArray<FColor> CameraDataBuffer;
	TWeakObjectPtr<UTexture2D> Texture;
	int32 TextureWidth;
	int32 TextureHeight;
};
