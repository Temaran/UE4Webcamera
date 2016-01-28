// Copyright (C) 2016 Fredrik Lindh. All Rights Reserved.
// Created by Fredrik Lindh (Temaran) on 2016-01-28

#include "WebcameraPrivatePCH.h"

#define LOCTEXT_NAMESPACE "FWebcameraModule"

void FWebcameraModule::StartupModule()
{
	CoInitialize(NULL);

	Devices = NULL;
	ReacquireDevices();
}

void FWebcameraModule::ShutdownModule()
{
	if (Devices)
	{
		delete[] Devices;
	}

	Devices = NULL;
}

int32 FWebcameraModule::ReacquireDevices()
{
	if (Devices)
	{
		delete[] Devices;
	}

	Graph = NULL;
	Capture = NULL;
	Control = NULL;
	Devices = NULL;
	Current = NULL;
	NumDevices = 0;
	IsPlaying = false;

	Devices = new FVideoDevice[MAX_DEVICES];

	InitializeGraph();
	InitializeVideo();

	// we have to use this construct, because other 
	// filters may have been added to the graph
	Control->Run();
	for (int DeviceIndex = 0; DeviceIndex < NumDevices; DeviceIndex++)
	{
		Devices[DeviceIndex].Stop();
	}

	return NumDevices;
}

UTexture2D* FWebcameraModule::Connect(int32 DeviceIndex)
{
	if (DeviceIndex < NumDevices)
	{
		return Devices[DeviceIndex].Start();
	}

	return NULL;
}

void FWebcameraModule::Disconnect(int32 DeviceIndex)
{
	if (DeviceIndex < NumDevices)
	{
		Devices[DeviceIndex].Stop();
	}
}

int32 FWebcameraModule::GetNumDevices()
{
	return NumDevices;
}

FVideoDevice* FWebcameraModule::GetDevices()
{
	return Devices;
}

void FWebcameraModule::InitializeGraph()
{
	HRESULT HResult;

	HResult = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IFilterGraph2, (void**)&Graph);
	if (HResult < 0)
	{
		throw HResult;
	}

	HResult = CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC_SERVER, IID_ICaptureGraphBuilder2, (void**)&Capture);
	if (HResult < 0)
	{
		throw HResult;
	}

	HResult = Graph->QueryInterface(IID_IMediaControl, (void**)&Control);
	if (HResult < 0)
	{
		throw HResult;
	}

	Capture->SetFiltergraph(Graph);
}

void FWebcameraModule::InitializeVideo()
{
	HRESULT HResult;
	VARIANT Name;
	WCHAR FilterName[MAX_DEVICE_NAME + 2];

	LONGLONG Start = 0;
	LONGLONG Stop = MAXLONGLONG;

	ICreateDevEnum* DevEnum;
	IEnumMoniker* EnumMoniker;
	IMoniker* Moniker;
	IPropertyBag* PropertyBag;

	//create an enumerator for video input devices
	HResult = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, (void**)&DevEnum);
	if (HResult < 0)
	{
		throw HResult;
	}	

	HResult = DevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &EnumMoniker, NULL);
	if (HResult < 0)
	{
		throw HResult;
	}

	if (HResult == S_FALSE)
	{
		return;
	}

	NumDevices = 0;
	while (EnumMoniker->Next(1, &Moniker, 0) == S_OK)
	{
		HResult = Moniker->BindToStorage(0, 0, IID_IPropertyBag, (void**)&PropertyBag);
		if (HResult >= 0)
		{
			VariantInit(&Name);

			//get the description
			HResult = PropertyBag->Read(L"Description", &Name, 0);
			if (HResult < 0)
			{
				HResult = PropertyBag->Read(L"FriendlyName", &Name, 0);
			}

			if (HResult >= 0)
			{
				//Initialize the VideoDevice struct
				FVideoDevice* VideoDevice = Devices + NumDevices++;
				BSTR StringPtr = Name.bstrVal;

				for (int c = 0; *StringPtr; c++, StringPtr++)
				{
					//bit hacky, but i don't like to include ATL
					VideoDevice->Filtername[c] = *StringPtr;
					VideoDevice->Friendlyname[c] = *StringPtr & 0xFF;
				}

				//add a filter for the device
				HResult = Graph->AddSourceFilterForMoniker(Moniker, 0, VideoDevice->Filtername, &VideoDevice->Sourcefilter);
				if (HResult != S_OK)
				{
					throw HResult;
				}

				//create a samplegrabber filter for the device
				HResult = CoCreateInstance(CLSID_SampleGrabber, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&VideoDevice->Samplegrabberfilter);
				if (HResult < 0)
				{
					throw HResult;
				}

				//set mediatype on the samplegrabber
				HResult = VideoDevice->Samplegrabberfilter->QueryInterface(IID_ISampleGrabber, (void**)&VideoDevice->Samplegrabber);
				if (HResult != S_OK)
				{
					throw HResult;
				}

				wcscpy_s(FilterName, MAX_DEVICE_NAME + 2, L"SG ");
				wcscpy_s(FilterName + 3, MAX_DEVICE_NAME + 5, VideoDevice->Filtername);
				Graph->AddFilter(VideoDevice->Samplegrabberfilter, FilterName);

				//set the media type
				AM_MEDIA_TYPE MediaType;
				FMemory::Memset(&MediaType, 0, sizeof(AM_MEDIA_TYPE));

				MediaType.majortype = MEDIATYPE_Video;
				MediaType.subtype = MEDIASUBTYPE_RGB24;
				// setting the above to 32 bits fails consecutive Select for some reason
				// and only sends one single callback (flush from previous one ???)
				// must be deeper problem. 24 bpp seems to work fine for now.

				HResult = VideoDevice->Samplegrabber->SetMediaType(&MediaType);
				if (HResult != S_OK)
				{
					throw HResult;
				}

				//add the callback to the sample grabber
				HResult = VideoDevice->Samplegrabber->SetCallback(VideoDevice->Callbackhandler, 0);
				if (HResult != S_OK)
				{
					throw HResult;
				}

				//set the null renderer
				HResult = CoCreateInstance(CLSID_NullRenderer, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&VideoDevice->Nullrenderer);
				if (HResult < 0)
				{
					throw HResult;
				}

				wcscpy_s(FilterName, MAX_DEVICE_NAME + 2, L"NR ");
				wcscpy_s(FilterName + 3, MAX_DEVICE_NAME + 5, VideoDevice->Filtername);
				Graph->AddFilter(VideoDevice->Nullrenderer, FilterName);

				//set the render path
#ifdef SHOW_DEBUG_RENDERER
				HResult = Capture->RenderStream(&PIN_CATEGORY_PREVIEW, &MEDIATYPE_Video, VideoDevice->Sourcefilter, VideoDevice->Samplegrabberfilter, NULL);
#else
				HResult = Capture->RenderStream(&PIN_CATEGORY_PREVIEW, &MEDIATYPE_Video, VideoDevice->Sourcefilter, VideoDevice->Samplegrabberfilter, VideoDevice->Nullrenderer);
#endif
				if (HResult < 0)
				{
					throw HResult;
				}

				//if the stream is started, start capturing immediatly
				HResult = Capture->ControlStream(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video, VideoDevice->Sourcefilter, &Start, &Stop, 1, 2);
				if (HResult < 0)
				{
					throw HResult;
				}

				// look up the media type:
				VideoDevice->Width = 0;
				VideoDevice->Height = 0;

				AM_MEDIA_TYPE MediaTypeInfo;
				HResult = VideoDevice->Samplegrabber->GetConnectedMediaType(&MediaTypeInfo);
				if (HResult == S_OK)
				{
					if (MediaTypeInfo.formattype == FORMAT_VideoInfo)
					{
						const VIDEOINFOHEADER * VideoInfo = reinterpret_cast<VIDEOINFOHEADER*>(MediaTypeInfo.pbFormat);
						VideoDevice->Width = VideoInfo->bmiHeader.biWidth;
						VideoDevice->Height = VideoInfo->bmiHeader.biHeight;
					}
				}

				//reference the graph
				VideoDevice->Graph = Graph;
				VideoDevice->Id = NumDevices;
			}
			VariantClear(&Name);
			PropertyBag->Release();
		}
		Moniker->Release();
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FWebcameraModule, Webcamera)