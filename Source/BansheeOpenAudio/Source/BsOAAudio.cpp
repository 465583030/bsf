//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsOAAudio.h"

namespace BansheeEngine
{
	OAAudio::OAAudio()
	{
		
	}

	OAAudio::~OAAudio()
	{
		
	}

	void OAAudio::setVolume(float volume)
	{
		// TODO
	}

	float OAAudio::getVolume() const
	{
		// TODO
		return 1.0f;
	}

	void OAAudio::setPaused(bool paused)
	{
		// TODO
	}

	bool OAAudio::isPaused() const
	{
		// TODO
		return false;
	}

	void OAAudio::update()
	{
		// TODO
	}

	void OAAudio::setActiveDevice(const AudioDevice& device)
	{
		// TODO
	}

	AudioDevice OAAudio::getActiveDevice() const
	{
		// TODO
		return AudioDevice();
	}

	AudioDevice OAAudio::getDefaultDevice() const
	{
		// TODO
		return AudioDevice();
	}

	Vector<AudioDevice> OAAudio::getAllDevices() const
	{
		// TODO
		return Vector<AudioDevice>();
	}
}