//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsAudio.h"

namespace BansheeEngine
{
	Audio& gAudio()
	{
		return Audio::instance();
	}
}