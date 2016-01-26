//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsTimerQuery.h"
#include "BsQueryManager.h"

namespace BansheeEngine
{
	TimerQueryPtr TimerQuery::create()
	{
		return QueryManager::instance().createTimerQuery();
	}
}