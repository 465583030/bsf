#include "BsGUISpace.h"

namespace BansheeEngine
{
	LayoutSizeRange GUIFixedSpace::_calculateLayoutSizeRange() const
	{ 
		LayoutSizeRange range;
		range.optimal = _getOptimalSize();
		range.min = range.optimal;
		range.max = range.optimal;

		return range;
	}

	GUIFixedSpace* GUIFixedSpace::create(UINT32 size)
	{
		return bs_new<GUIFixedSpace>(size);
	}

	void GUIFixedSpace::destroy(GUIFixedSpace* space)
	{
		bs_delete(space);
	}

	LayoutSizeRange GUIFlexibleSpace::_calculateLayoutSizeRange() const
	{
		LayoutSizeRange range;
		range.optimal = _getOptimalSize();
		range.min = range.optimal;
		range.max = range.optimal;

		return range;
	}

	GUIFlexibleSpace* GUIFlexibleSpace::create()
	{
		return bs_new<GUIFlexibleSpace>();
	}

	void GUIFlexibleSpace::destroy(GUIFlexibleSpace* space)
	{
		bs_delete(space);
	}
}