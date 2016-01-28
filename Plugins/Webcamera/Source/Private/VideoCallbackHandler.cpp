// Copyright (C) 2016 Fredrik Lindh. All Rights Reserved.
// Created by Fredrik Lindh (Temaran) on 2016-01-28

#include "WebcameraPrivatePCH.h"

FVideoCallbackHandler::FVideoCallbackHandler()
	: TextureWidth(0.0f)
	, TextureHeight(0.0f)
{
}

FVideoCallbackHandler::~FVideoCallbackHandler()
{
}

UTexture2D* FVideoCallbackHandler::RecreateTexture(int32 SourceWidth, int32 SourceHeight)
{
	TextureWidth = SourceWidth;
	TextureHeight = SourceHeight;

	Texture = UTexture2D::CreateTransient(TextureWidth, TextureHeight);
	Texture->UpdateResource();

	CameraDataBuffer.Init(FColor(0, 0, 0, 255), TextureWidth * TextureHeight);
	
	return Texture.Get();
}

HRESULT FVideoCallbackHandler::SampleCB(double Time, IMediaSample* Sample)
{
	HRESULT HResult;
	unsigned char* Buffer;

	HResult = Sample->GetPointer((BYTE**)&Buffer);
	if (HResult != S_OK)
	{
		return S_OK;
	}

	if (Texture.IsValid())
	{
		//Dispatch to render thread here
		struct FUpdateTextureRegionsData
		{
			FTexture2DResource* Texture2DResource;
			int32 MipIndex;
			FUpdateTextureRegion2D Region;
			uint32 SrcPitch;
			uint32 SrcBpp;
			uint8* SrcData;
		};

		FUpdateTextureRegionsData* RegionData = new FUpdateTextureRegionsData;

		RegionData->Texture2DResource = (FTexture2DResource*)Texture->Resource;
		RegionData->MipIndex = 0;
		RegionData->Region = FUpdateTextureRegion2D(0, 0, 0, 0, TextureWidth, TextureHeight);;
		RegionData->SrcPitch = (uint32)(4 * TextureWidth);
		RegionData->SrcBpp = (uint32)4;
		RegionData->SrcData = (uint8*)CameraDataBuffer.GetData();
		
		//Copy Mat data to Data array
		for (int y = 0; y < TextureHeight; y++)
		{
			for (int x = 0; x < TextureWidth; x++)
			{
				int i = x + (y * TextureWidth);
				CameraDataBuffer[i].B = Buffer[i * 3 + 0];
				CameraDataBuffer[i].G = Buffer[i * 3 + 1];
				CameraDataBuffer[i].R = Buffer[i * 3 + 2];
			}
		}

		ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(
			UpdateTextureRegionsData,
			FUpdateTextureRegionsData*, RegionData, RegionData,
			{
				int32 CurrentFirstMip = RegionData->Texture2DResource->GetCurrentFirstMip();
				if (RegionData->MipIndex >= CurrentFirstMip)
				{
					RHIUpdateTexture2D(RegionData->Texture2DResource->GetTexture2DRHI(),
						RegionData->MipIndex - CurrentFirstMip,
						RegionData->Region,
						RegionData->SrcPitch,
						RegionData->SrcData	+ RegionData->Region.SrcY * RegionData->SrcPitch + RegionData->Region.SrcX * RegionData->SrcBpp);
				}

				delete RegionData;
			});
	}

	return S_OK;
}

HRESULT FVideoCallbackHandler::BufferCB(double Time, BYTE* Buffer, long Length)
{
	return S_OK;
}

HRESULT FVideoCallbackHandler::QueryInterface(const IID& Iid, LPVOID *VoidPtrPtr)
{
	if (Iid == IID_ISampleGrabberCB || Iid == IID_IUnknown)
	{
		*VoidPtrPtr = (void*) static_cast<ISampleGrabberCB*>(this);
		return S_OK;
	}

	return E_NOINTERFACE;
}

ULONG FVideoCallbackHandler::AddRef()
{
	return 1;
}

ULONG FVideoCallbackHandler::Release()
{
	return 2;
}
