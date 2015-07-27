#pragma once

#include "BsEditorPrerequisites.h"
#include "BsIReflectable.h"

namespace BansheeEngine
{
	/**
	 * @brief	Available platforms we can build for
	 */
	enum class PlatformType
	{
		Windows,
		Count // Keep at end
	};

	/**
	 * @brief	Contains per-platform information used primarily for build purposes.
	 */
	struct BS_ED_EXPORT PlatformInfo : public IReflectable
	{
		PlatformInfo();
		virtual ~PlatformInfo();

		PlatformType type;
		WString defines;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class PlatformInfoRTTI;
		static RTTITypeBase* getRTTIStatic();
		virtual RTTITypeBase* getRTTI() const override;
	};

	/**
	 * @brief	Contains Windows specific per-platform information used
	 *			primarily for build purposes.
	 */
	struct BS_ED_EXPORT WinPlatformInfo : public PlatformInfo
	{
		WinPlatformInfo();

		bool is32bit;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class WinPlatformInfoRTTI;
		static RTTITypeBase* getRTTIStatic();
		virtual RTTITypeBase* getRTTI() const override;
	};
}