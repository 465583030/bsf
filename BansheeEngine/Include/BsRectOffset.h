#pragma once

#include "BsPrerequisites.h"

namespace BansheeEngine
{
	/**
	 * @brief	Rectangle represented in the form of offsets
	 *			from some parent rectangle.
	 */
	struct RectOffset
	{
		RectOffset()
			:left(0), right(0), top(0), bottom(0)
		{ }

		RectOffset(INT32 left, INT32 right, INT32 top, INT32 bottom)
			:left(left), right(right), top(top), bottom(bottom)
		{ }

		INT32 left, right, top, bottom;
	};

	BS_ALLOW_MEMCPY_SERIALIZATION(RectOffset);
}