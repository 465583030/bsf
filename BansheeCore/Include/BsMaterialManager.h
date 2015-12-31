#pragma once

#include "BsCorePrerequisites.h"
#include "BsModule.h"

namespace BansheeEngine
{
	/** @cond INTERNAL */
	/** @addtogroup Material
	 *  @{
	 */

	/**	Material manager handles material creation. */
	class BS_CORE_EXPORT MaterialManager : public Module<MaterialManager>
	{
	public:
		/**
		 * Creates a new material without any assigned shader.
		 *
		 * @note	Make sure to call Material::setShader() before using it.
		 */
		MaterialPtr create() const;

		/** Creates a new material with the specified shader. */
		MaterialPtr create(const HShader& shader) const;

		/**
		 * Creates a new empty material without initializing it.
		 *
		 * @note	You must manually call Material::initialize() after creation.
		 */
		MaterialPtr createEmpty() const;
	};

	/** @} */
	/** @endcond */
}