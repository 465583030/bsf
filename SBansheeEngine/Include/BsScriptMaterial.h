#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptResource.h"
#include "BsMaterial.h"
#include "BsVector2.h"
#include "BsVector3.h"
#include "BsVector4.h"
#include "BsMatrix3.h"
#include "BsMatrix4.h"
#include "BsColor.h"

namespace BansheeEngine
{
	/**
	 * @brief	Interop class between C++ & CLR for Material.
	 */
	class BS_SCR_BE_EXPORT ScriptMaterial : public TScriptResource <ScriptMaterial, Material>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "Material")

	private:
		friend class ScriptResourceManager;

		ScriptMaterial(MonoObject* instance, const HMaterial& material);

		/**
		 * @copydoc	ScriptObjectBase::_createManagedInstance
		 */
		MonoObject* _createManagedInstance(bool construct) override;

		/**
		 * @brief	Creates an empty, uninitialized managed instance of the resource interop object.
		 */
		static MonoObject* createInstance();

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void internal_CreateInstance(MonoObject* instance, ScriptShader* shader);
		static MonoObject* internal_Clone(ScriptMaterial* nativeInstance);

		static MonoObject* internal_GetShader(ScriptMaterial* nativeInstance);
		static void internal_SetShader(ScriptMaterial* nativeInstance, ScriptShader* shader);

		static void internal_SetFloat(ScriptMaterial* nativeInstance, MonoString* name, float value);
		static void internal_SetVector2(ScriptMaterial* nativeInstance, MonoString* name, Vector2* value);
		static void internal_SetVector3(ScriptMaterial* nativeInstance, MonoString* name, Vector3* value);
		static void internal_SetVector4(ScriptMaterial* nativeInstance, MonoString* name, Vector4* value);
		static void internal_SetMatrix3(ScriptMaterial* nativeInstance, MonoString* name, Matrix3* value);
		static void internal_SetMatrix4(ScriptMaterial* nativeInstance, MonoString* name, Matrix4* value);
		static void internal_SetColor(ScriptMaterial* nativeInstance, MonoString* name, Color* value);
		static void internal_SetTexture2D(ScriptMaterial* nativeInstance, MonoString* name, ScriptTexture2D* value);
		static void internal_SetTexture3D(ScriptMaterial* nativeInstance, MonoString* name, ScriptTexture3D* value);
		static void internal_SetTextureCube(ScriptMaterial* nativeInstance, MonoString* name, ScriptTextureCube* value);

		static float internal_GetFloat(ScriptMaterial* nativeInstance, MonoString* name);
		static void internal_GetVector2(ScriptMaterial* nativeInstance, MonoString* name, Vector2* value);
		static void internal_GetVector3(ScriptMaterial* nativeInstance, MonoString* name, Vector3* value);
		static void internal_GetVector4(ScriptMaterial* nativeInstance, MonoString* name, Vector4* value);
		static void internal_GetMatrix3(ScriptMaterial* nativeInstance, MonoString* name, Matrix3* value);
		static void internal_GetMatrix4(ScriptMaterial* nativeInstance, MonoString* name, Matrix4* value);
		static void internal_GetColor(ScriptMaterial* nativeInstance, MonoString* name, Color* value);
		static MonoObject* internal_GetTexture2D(ScriptMaterial* nativeInstance, MonoString* name);
		static MonoObject* internal_GetTexture3D(ScriptMaterial* nativeInstance, MonoString* name);
		static MonoObject* internal_GetTextureCube(ScriptMaterial* nativeInstance, MonoString* name);
	};
}