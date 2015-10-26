#pragma once

#include "BsScriptEditorPrerequisites.h"
#include "BsScriptObject.h"
#include "BsPixelData.h"
#include "BsGpuProgram.h"

namespace BansheeEngine
{
	enum class FontRenderMode;

	/**
	 * @brief	Base class for all C++/CLR interop objects wrapping various
	 *			implementations of ImportOptions.
	 */
	class BS_SCR_BED_EXPORT ScriptImportOptionsBase : public ScriptObjectBase
	{
	public:
		/**
		 * @brief	Returns the internal native import options.
		 */
		SPtr<ImportOptions> getImportOptions() const { return mImportOptions; }

	protected:
		ScriptImportOptionsBase(MonoObject* instance);
		virtual ~ScriptImportOptionsBase() {}

		SPtr<ImportOptions> mImportOptions;
	};

	/**
	 * @brief	Interop class between C++ & CLR for ImportOptions.
	 */
	class BS_SCR_BED_EXPORT ScriptImportOptions : public ScriptObject <ScriptImportOptions, ScriptImportOptionsBase>
	{
	public:
		SCRIPT_OBJ(EDITOR_ASSEMBLY, "BansheeEditor", "ImportOptions")

		/**
		 * @brief	Creates a new managed ImportOptions instance containing
		 *			the provided import options.
		 */
		static MonoObject* create(const SPtr<ImportOptions>& importOptions);

	private:
		ScriptImportOptions(MonoObject* instance);
	};

	/**
	 * @brief	Interop class between C++ & CLR for TextureImportOptions.
	 */
	class BS_SCR_BED_EXPORT ScriptTextureImportOptions : public ScriptObject<ScriptTextureImportOptions, ScriptImportOptionsBase>
	{
	public:
		SCRIPT_OBJ(EDITOR_ASSEMBLY, "BansheeEditor", "TextureImportOptions")

		/**
		 * @brief	Creates a new managed TextureImportOptions instance containing the 
		 *			default import options for textures.
		 */
		static MonoObject* create();

		/**
		 * @brief	Creates a new managed TextureImportOptions instance containing 
		 *			the provided import options.
		 */
		static MonoObject* create(const SPtr<TextureImportOptions>& options);

	private:
		ScriptTextureImportOptions(MonoObject* instance);

		/**
		 * @brief	Returns the internal native import options.
		 */
		SPtr<TextureImportOptions> getTexImportOptions();

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void internal_CreateInstance(MonoObject* instance);
		static PixelFormat internal_GetPixelFormat(ScriptTextureImportOptions* thisPtr);
		static void internal_SetPixelFormat(ScriptTextureImportOptions* thisPtr, PixelFormat value);
		static bool internal_GetGenerateMipmaps(ScriptTextureImportOptions* thisPtr);
		static void internal_SetGenerateMipmaps(ScriptTextureImportOptions* thisPtr, bool value);
		static UINT32 internal_GetMaxMipmapLevel(ScriptTextureImportOptions* thisPtr);
		static void internal_SetMaxMipmapLevel(ScriptTextureImportOptions* thisPtr, UINT32 value);
		static bool internal_GetCPUReadable(ScriptTextureImportOptions* thisPtr);
		static void internal_SetCPUReadable(ScriptTextureImportOptions* thisPtr, bool value);
		static bool internal_GetIsSRGB(ScriptTextureImportOptions* thisPtr);
		static void internal_SetIsSRGB(ScriptTextureImportOptions* thisPtr, bool value);
	};

	/**
	* @brief	Interop class between C++ & CLR for MeshImportOptions.
	*/
	class BS_SCR_BED_EXPORT ScriptMeshImportOptions : public ScriptObject<ScriptMeshImportOptions, ScriptImportOptionsBase>
	{
	public:
		SCRIPT_OBJ(EDITOR_ASSEMBLY, "BansheeEditor", "MeshImportOptions")

		/**
		 * @brief	Creates a new managed MeshImportOptions instance containing the
		 *			default import options for textures.
		 */
		static MonoObject* create();

		/**
		 * @brief	Creates a new managed MeshImportOptions instance containing
		 *			the provided import options.
		 */
		static MonoObject* create(const SPtr<MeshImportOptions>& options);

	private:
		ScriptMeshImportOptions(MonoObject* instance);

		/**
		* @brief	Returns the internal native import options.
		*/
		SPtr<MeshImportOptions> getMeshImportOptions();

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void internal_CreateInstance(MonoObject* instance);
		static bool internal_GetCPUReadable(ScriptMeshImportOptions* thisPtr);
		static void internal_SetCPUReadable(ScriptMeshImportOptions* thisPtr, bool value);
		static bool internal_GetImportNormals(ScriptMeshImportOptions* thisPtr);
		static void internal_SetImportNormals(ScriptMeshImportOptions* thisPtr, bool value);
		static bool internal_GetImportTangents(ScriptMeshImportOptions* thisPtr);
		static void internal_SetImportTangents(ScriptMeshImportOptions* thisPtr, bool value);
		static bool internal_GetImportSkin(ScriptMeshImportOptions* thisPtr);
		static void internal_SetImportSkin(ScriptMeshImportOptions* thisPtr, bool value);
		static bool internal_GetImportAnimation(ScriptMeshImportOptions* thisPtr);
		static void internal_SetImportAnimation(ScriptMeshImportOptions* thisPtr, bool value);
		static bool internal_GetImportBlendShapes(ScriptMeshImportOptions* thisPtr);
		static void internal_SetImportBlendShapes(ScriptMeshImportOptions* thisPtr, bool value);
		static float internal_GetScale(ScriptMeshImportOptions* thisPtr);
		static void internal_SetScale(ScriptMeshImportOptions* thisPtr, float value);
	};

	/**
	 * @brief	Interop class between C++ & CLR for FontImportOptions.
	 */
	class BS_SCR_BED_EXPORT ScriptFontImportOptions : public ScriptObject <ScriptFontImportOptions, ScriptImportOptionsBase>
	{
	public:
		SCRIPT_OBJ(EDITOR_ASSEMBLY, "BansheeEditor", "FontImportOptions")

		/**
		 * @brief	Creates a new managed FontImportOptions instance containing the 
		 *			default import options for fonts.
		 */
		static MonoObject* create();

		/**
		 * @brief	Creates a new managed FontImportOptions instance containing 
		 *			the provided import options.
		 */
		static MonoObject* create(const SPtr<FontImportOptions>& options);

	private:
		ScriptFontImportOptions(MonoObject* instance);

		/**
		 * @brief	Returns the internal native import options.
		 */
		SPtr<FontImportOptions> getFontImportOptions();

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void internal_CreateInstance(MonoObject* instance);
		static MonoArray* internal_GetFontSizes(ScriptFontImportOptions* thisPtr);
		static void internal_SetFontSizes(ScriptFontImportOptions* thisPtr, MonoArray* value);
		static UINT32 internal_GetDPI(ScriptFontImportOptions* thisPtr);
		static void internal_SetDPI(ScriptFontImportOptions* thisPtr, UINT32 value);
		static FontRenderMode internal_GetRenderMode(ScriptFontImportOptions* thisPtr);
		static void internal_SetRenderMode(ScriptFontImportOptions* thisPtr, FontRenderMode value);
		static bool internal_GetBold(ScriptFontImportOptions* thisPtr);
		static void internal_SetBold(ScriptFontImportOptions* thisPtr, bool value);
		static bool internal_GetItalic(ScriptFontImportOptions* thisPtr);
		static void internal_SetItalic(ScriptFontImportOptions* thisPtr, bool value);
		static MonoArray* internal_GetCharRanges(ScriptFontImportOptions* thisPtr);
		static void internal_SetCharRanges(ScriptFontImportOptions* thisPtr, MonoArray* value);
	};

	/**
	 * @brief	Interop class between C++ & CLR for ScriptCodeImportOptions.
	 */
	class BS_SCR_BED_EXPORT ScriptScriptCodeImportOptions : public ScriptObject <ScriptScriptCodeImportOptions, ScriptImportOptionsBase>
	{
	public:
		SCRIPT_OBJ(EDITOR_ASSEMBLY, "BansheeEditor", "ScriptCodeImportOptions")

		/**
		 * @brief	Creates a new managed ScriptCodeImportOptions instance containing the 
		 *			default import options for script code files.
		 */
		static MonoObject* create();

		/**
		 * @brief	Creates a new managed ScriptCodeImportOptions instance containing 
		 *			the provided import options.
		 */
		static MonoObject* create(const SPtr<ScriptCodeImportOptions>& options);

	private:
		ScriptScriptCodeImportOptions(MonoObject* instance);

		/**
		 * @brief	Returns the internal native import options.
		 */
		SPtr<ScriptCodeImportOptions> getCodeImportOptions();

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void internal_CreateInstance(MonoObject* instance);
		static bool internal_IsEditorScript(ScriptScriptCodeImportOptions* thisPtr);
		static void internal_SetEditorScript(ScriptScriptCodeImportOptions* thisPtr, bool value);
	};
}