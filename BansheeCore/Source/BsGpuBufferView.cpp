//__________________________ Banshee Project - A modern game development toolkit _________________________________//
//_____________________________________ www.banshee-project.com __________________________________________________//
//________________________ Copyright (c) 2014 Marko Pintera. All rights reserved. ________________________________//
#include "BsGpuBufferView.h"
#include "BsUtil.h"

namespace BansheeEngine
{
	size_t GpuBufferView::HashFunction::operator()(const GPU_BUFFER_DESC& key) const
	{
		size_t seed = 0;
		hash_combine(seed, key.elementWidth);
		hash_combine(seed, key.firstElement);
		hash_combine(seed, key.numElements);
		hash_combine(seed, key.useCounter);
		hash_combine(seed, key.usage);

		return seed;
	}

	bool GpuBufferView::EqualFunction::operator()
		(const GPU_BUFFER_DESC& a, const GPU_BUFFER_DESC& b) const
	{
		return a.elementWidth == b.elementWidth && a.firstElement == b.firstElement 
			&& a.numElements == b.numElements && a.useCounter == b.useCounter && a.usage == b.usage;
	}

	GpuBufferView::GpuBufferView()
	{

	}

	GpuBufferView::~GpuBufferView()
	{

	}

	void GpuBufferView::initialize(GpuBufferPtr buffer, GPU_BUFFER_DESC& desc)
	{
		mBuffer = buffer;
		mDesc = desc;
	}
}