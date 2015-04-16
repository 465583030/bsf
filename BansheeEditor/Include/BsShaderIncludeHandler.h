#pragma once

#include "BsEditorPrerequisites.h"
#include "BsShaderManager.h"

namespace BansheeEngine
{
	/**
	 * @brief	Shader include handler for the editor. It uses project library to find
	 *			the includes, and supports special keywords for built-in includes that
	 *			reside outside of the project library.
	 */
	class BS_ED_EXPORT EditorShaderIncludeHandler : public IShaderIncludeHandler
	{
	public:
		/**
		 * @copydoc	IShaderIncludeHandler::findInclude
		 */
		virtual HShaderInclude findInclude(const String& name) const override;
	};
}