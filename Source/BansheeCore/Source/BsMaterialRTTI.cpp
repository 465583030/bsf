//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsMaterialRTTI.h"
#include "BsMaterialManager.h"
#include "BsMaterialParams.h"

namespace BansheeEngine
{
	void MaterialRTTI::onDeserializationEnded(IReflectable* obj)
	{
		Material* material = static_cast<Material*>(obj);
		material->initialize();

		if(material->mRTTIData.empty())
			return;

		material->initBestTechnique();

		SPtr<MaterialParams> params = any_cast<SPtr<MaterialParams>>(material->mRTTIData);
		material->setParams(params);

		material->mRTTIData = nullptr; // Delete temporary data
	}

	SPtr<IReflectable> MaterialRTTI::newRTTIObject()
	{
		return MaterialManager::instance().createEmpty();
	}
}