// Copyright (C) 2016 Fredrik Lindh. All Rights Reserved.
// Created by Fredrik Lindh (Temaran) on 2016-01-28

#pragma once

#include "ModuleManager.h"
#include "Runtime/Engine/Classes/Engine/Texture2D.h"

class IWebcamera : public IModuleInterface
{
public:

	/**
	* Singleton-like access to this module's interface.  This is just for convenience!
	* Beware of calling this during the shutdown phase, though. Your module might have been
	* unloaded already.
	*
	* @return Returns singleton instance, loading the module on demand if needed
	*/
	static inline IWebcamera& Get()
	{
		return FModuleManager::LoadModuleChecked<IWebcamera>("Webcamera");
	}

	/**
	* Checks to see if this module is loaded and ready.  It is only valid to call Get() if
	* IsAvailable() returns true.
	*
	* @return True if the module is loaded and ready to use
	*/
	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("Webcamera");
	}

	//==============================================================================
	// Public Webcamera API								
	//==============================================================================

public:
	/**
	* Destroys all connections and tries to find new ones. You need to reconnect to previously connected devices if you do this.
	*
	* @return The number of devices found.
	*/
	virtual int32 ReacquireDevices() = 0;

	/**
	* Connects to a device. Please check how many devices are available before you connect to an index.
	*
	* DeviceIndex The index of the camera to connect to. There is no order or stability guarantee for these indices.
	* @return The camera that the camera will continuously write updates to. If you don't store this in a UPROPERTY, it will get garbage collected, so watch out for that!
	*/
	virtual UTexture2D* Connect(int32 DeviceIndex) = 0;

	/**
	* Disconnects from a device. Please check how many devices are available before you attempt this.
	*
	* DeviceIndex The index of the camera to disconnect from. There is no order or stability guarantee for these indices.
	*/
	virtual void Disconnect(int32 DeviceIndex) = 0;

	/**
	* Returns the number of web cameras the plugin could find connected to the system.
	*
	* @return The number of devices found.
	*/
	virtual int32 GetNumDevices() = 0;
};
