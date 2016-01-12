#pragma once

#include "BsCorePrerequisites.h"

namespace BansheeEngine
{
	/** @cond INTERNAL */
	/** @addtogroup Renderer
	 *  @{
	 */

	/**
	 * Factory class for creating Renderer objects. Implement this class for any custom renderer classes you may have, and 
	 * register it with renderer manager.
	 * 			
	 * @see		RendererManager
	 */
	class BS_CORE_EXPORT RendererFactory
	{
	public:
		virtual ~RendererFactory() { }

		/**	Creates a new instance of the renderer. */
		virtual CoreRendererPtr create() = 0;

		/**	Returns the name of the renderer this factory creates. */
		virtual const String& name() const = 0;
	};

	/** @} */
	/** @endcond */
}