#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"

namespace BansheeEngine
{
	/**
	 * @brief	Types of resources accessible from script code.
	 */
	enum class ScriptResourceType // Note: Must be the same as C# enum ScriptResourceType
	{
		Texture, SpriteTexture, Mesh, Font, Shader, Material, Prefab, PlainText, ScriptCode, StringTable, GUISkin, Undefined
	};

	/**
	 * @brief	Base class for all resource interop objects.
	 */
	class BS_SCR_BE_EXPORT ScriptResourceBase : public PersistentScriptObjectBase
	{
	public:
		/**
		 * @brief	Returns a generic handle to the internal wrapped resource.
		 */
		virtual HResource getGenericHandle() const = 0;

		/**
		 * @brief	Sets the internal resource this object wraps.
		 */
		virtual void setResource(const HResource& resource) = 0;

		/**
		 * @copydoc	ScriptObjectBase::beginRefresh
		 */
		virtual ScriptObjectBackup beginRefresh() override;

		/**
		 * @copydoc	ScriptObjectBase::endRefresh
		 */
		virtual void endRefresh(const ScriptObjectBackup& backupData) override;

	protected:
		friend class ScriptResourceManager;

		ScriptResourceBase(MonoObject* instance);
		virtual ~ScriptResourceBase() {}

		/**
		 * @brief	Triggered by the script resource managed when the native resource handle this object point to has
		 * 			been destroyed.
		 */
		virtual void notifyResourceDestroyed() { }

		/**
		 * @brief	Destroys the interop object, unless refresh is in progress in which case it is just prepared
		 * 			for re-creation.
		 */
		void destroy();

		bool mRefreshInProgress;
	};

	/**
	 * @brief	Base class for a specific resource's interop object.
	 */
	template<class ScriptClass, class ResType>
	class BS_SCR_BE_EXPORT TScriptResource : public ScriptObject <ScriptClass, ScriptResourceBase>
	{
	public:
		/**
		 * @copydoc	ScriptResourceBase::getGenericHandle
		 */
		HResource getGenericHandle() const override { return mResource; }

		/**
		 * @copydoc	ScriptResourceBase::setResource
		 */
		void setResource(const HResource& resource) override { mResource = static_resource_cast<ResType>(resource); }

		/**
		 * @brief	Returns a handle to the internal wrapped resource.
		 */
		const ResourceHandle<ResType>& getHandle() const { return mResource; }

	protected:
		friend class ScriptResourceManager;

		TScriptResource(MonoObject* instance, const ResourceHandle<ResType>& resource)
			:ScriptObject(instance), mResource(resource)
		{
			mManagedHandle = mono_gchandle_new(instance, false);

			BS_DEBUG_ONLY(mHandleValid = true);
		}

		virtual ~TScriptResource() {}

		/**
		 * @copydoc	ScriptObjectBase::endRefresh
		 */
		virtual void endRefresh(const ScriptObjectBackup& backupData) override
		{
			BS_ASSERT(!mHandleValid);
			mManagedHandle = mono_gchandle_new(mManagedInstance, false);

			ScriptObject::endRefresh(backupData);
		}

		/**
		 * @copydoc	ScriptObjectBase::notifyResourceDestroyed
		 */
		void notifyResourceDestroyed() override
		{
			mono_gchandle_free(mManagedHandle);
			BS_DEBUG_ONLY(mHandleValid = false);
		}

		/**
		 * @copydoc	ScriptObjectBase::_onManagedInstanceDeleted
		 */
		void _onManagedInstanceDeleted() override
		{
			mono_gchandle_free(mManagedHandle);
			BS_DEBUG_ONLY(mHandleValid = false);

			destroy();
		}

		ResourceHandle<ResType> mResource;
		uint32_t mManagedHandle;
		BS_DEBUG_ONLY(bool mHandleValid);
	};

	/**
	 * @brief	Interop class between C++ & CLR for Resource.
	 */
	class BS_SCR_BE_EXPORT ScriptResource : public ScriptObject<ScriptResource, ScriptResourceBase>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "Resource")

		/**
		 * @brief	Converts a RTTI id belonging to a resource type into a ScriptResourceType.
		 */
		static ScriptResourceType getTypeFromTypeId(UINT32 typeId);

		/**
		 * @brief	Converts a ScriptResourceType into a RTTI id belonging to that resource type.
		 */
		static UINT32 getTypeIdFromType(ScriptResourceType type);
	private:
		ScriptResource(MonoObject* instance)
			:ScriptObject(instance)
		{ }
	};
}