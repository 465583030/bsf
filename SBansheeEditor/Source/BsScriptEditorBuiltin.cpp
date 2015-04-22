#include "BsScriptEditorBuiltin.h"
#include "BsMonoManager.h"
#include "BsMonoClass.h"
#include "BsMonoMethod.h"
#include "BsMonoUtil.h"
#include "BsBuiltinEditorResources.h"
#include "BsScriptSpriteTexture.h"
#include "BsScriptResourceManager.h"

namespace BansheeEngine
{
	ScriptEditorBuiltin::ScriptEditorBuiltin(MonoObject* instance)
		:ScriptObject(instance)
	{ }

	void ScriptEditorBuiltin::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_GetFolderIcon", &ScriptEditorBuiltin::internal_getFolderIcon);
		metaData.scriptClass->addInternalCall("Internal_GetMeshIcon", &ScriptEditorBuiltin::internal_getMeshIcon);
		metaData.scriptClass->addInternalCall("Internal_GetFontIcon", &ScriptEditorBuiltin::internal_getFontIcon);
		metaData.scriptClass->addInternalCall("Internal_GetTextureIcon", &ScriptEditorBuiltin::internal_getTextureIcon);
	}

	MonoObject* ScriptEditorBuiltin::internal_getFolderIcon()
	{
		HSpriteTexture tex = HSpriteTexture(); // TODO - Get actual texture

		return ScriptSpriteTexture::toManaged(tex);
	}

	MonoObject* ScriptEditorBuiltin::internal_getMeshIcon()
	{
		HSpriteTexture tex = HSpriteTexture(); // TODO - Get actual texture

		return ScriptSpriteTexture::toManaged(tex);
	}

	MonoObject* ScriptEditorBuiltin::internal_getFontIcon()
	{
		HSpriteTexture tex = HSpriteTexture(); // TODO - Get actual texture

		return ScriptSpriteTexture::toManaged(tex);
	}

	MonoObject* ScriptEditorBuiltin::internal_getTextureIcon()
	{
		HSpriteTexture tex = HSpriteTexture(); // TODO - Get actual texture

		return ScriptSpriteTexture::toManaged(tex);
	}
}