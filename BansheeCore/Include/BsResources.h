#pragma once

#include "BsCorePrerequisites.h"
#include "BsModule.h"

namespace BansheeEngine
{
	/**
	 * @brief	Manager for dealing with all engine resources. It allows you to save 
	 *			new resources and load existing ones.
	 *
	 *			Used for manually dealing with resources but also for automatic resolving of
	 *			resource handles.
	 *
	 * @note	Sim thread only.
	 */
	class BS_CORE_EXPORT Resources : public Module<Resources>
	{
		struct ResourceLoadData
		{
			ResourceLoadData(const HResource& resource, UINT32 numDependencies)
				:resource(resource), remainingDependencies(numDependencies)
			{ }

			HResource resource;
			ResourcePtr loadedData;
			UINT32 remainingDependencies;
			bool notifyImmediately;
		};

	public:
		Resources();
		~Resources();

		/**
		 * @brief	Loads the resource from a given path. Returns an empty handle if resource can't be loaded.
		 *			Resource is loaded synchronously.
		 */
		HResource load(const Path& filePath, bool loadDependencies = true);

		/**
		 * @copydoc	load
		 */
		template <class T>
		ResourceHandle<T> load(const Path& filePath, bool loadDependencies = true)
		{
			return static_resource_cast<T>(load(filePath, loadDependencies));
		}

		/**
		 * @brief	Loads the resource asynchronously. Initially returned resource handle will be invalid
		 *			until resource loading is done.
		 *
		 * @param	filePath	Full pathname of the file.
		 * 						
		 * @note	You can use returned invalid handle in engine systems as the engine will check for handle 
		 *			validity before using it.
		 */
		HResource loadAsync(const Path& filePath, bool loadDependencies = true);

		/**
		 * @copydoc	loadAsync
		 */
		template <class T>
		ResourceHandle<T> loadAsync(const Path& filePath, bool loadDependencies = true)
		{
			return static_resource_cast<T>(loadAsync(filePath));
		}

		/**
		 * @brief	Loads the resource with the given UUID. Returns an empty handle if resource can't be loaded.
		 *
		 * @param	uuid	UUID of the resource to load.
		 * @param	async	If true resource will be loaded asynchronously. Handle to non-loaded
		 *					resource will be returned immediately while loading will continue in the background.
		 */
		HResource loadFromUUID(const String& uuid, bool async = false, bool loadDependencies = true);

		/**
		 * @brief	Unloads the resource that is referenced by the handle. Resource is unloaded regardless if it is 
		 *			referenced or not. Any dependencies held by the resource will also be unloaded, but only if the
		 *			resource is holding the last reference to them.
		 *
		 * @param	resourceHandle	Handle of the resource to unload.
		 * 							
		 * @note	GPU resources held by the resource will be scheduled to be destroyed on the core thread.
		 * 			Actual resource pointer wont be deleted until all user-held references to it are removed.
		 */
		void unload(HResource resource);

		/**
		 * @brief	Finds all resources that aren't being referenced anywhere and unloads them.
		 */
		void unloadAllUnused();

		/**
		 * @brief	Saves the resource at the specified location.
		 *
		 * @param	resource 	Handle to the resource.
		 * @param	filePath 	Full pathname of the file to save as.
		 * @param	overwrite	(optional) If true, any existing resource at the specified location will
		 * 						be overwritten.
		 * 						
		 * @note	If the resource is a GpuResource and you are in some way modifying it from the Core thread, make
		 * 			sure all those commands are submitted before you call this method. Otherwise an obsolete
		 * 			version of the resource might get saved.
		 *
		 *			If saving a core thread resource this is a potentially very slow operation as we must wait on the 
		 *			core thread and the GPU in order to read the resource.
		 */
		void save(HResource resource, const Path& filePath, bool overwrite);

		/**
		 * @brief	Saves an existing resource to its previous location.
		 *
		 * @param	resource 	Handle to the resource.
		 *
		 * @note	If the resource is a GpuResource and you are in some way modifying it from the Core thread, make
		 * 			sure all those commands are submitted before you call this method. Otherwise an obsolete
		 * 			version of the resource might get saved.
		 *
		 *			If saving a core thread resource this is a potentially very slow operation as we must wait on the
		 *			core thread and the GPU in order to read the resource.
		 */
		void save(HResource resource);

		/**
		 * @brief	Updates an existing resource handle with a new resource. Caller must ensure that
		 * 			new resource type matches the original resource type.
		 */
		void update(HResource& handle, const ResourcePtr& resource);

		/**
		 * @brief	Returns a list of dependencies from the resources at the specified path. Resource will not be loaded
		 *			or parsed, but instead the saved list of dependencies will be read from the file and returned.
		 *
		 * @param	filePath	Full path to the resource to get dependencies for.
		 *
		 * @returns	List of dependencies represented as UUIDs.
		 */
		Vector<String> getDependencies(const Path& filePath);

		/**
		 * @brief	Checks is the resource with the specified UUID loaded.
		 *
		 * @param	uuid			UUID of the resource to check.
		 * @param	checkInProgress	Should this method also check resources that are in progress of being asynchronously loaded.
		 * 							
		 * @return	True if loaded or loading in progress, false otherwise.
		 */
		bool isLoaded(const String& uuid, bool checkInProgress = true);

		/**
		 * @brief	Creates a new resource handle from a resource pointer. 
		 *
		 * @note	Internal method used primarily be resource factory methods.
		 */
		HResource _createResourceHandle(const ResourcePtr& obj);

		/**
		 * @brief	Returns an existing handle of a resource that has already been loaded,
		 *			or is currently being loaded.
		 */
		HResource _getResourceHandle(const String& uuid);

		/**
		 * @brief	Allows you to set a resource manifest containing UUID <-> file path mapping that is
		 * 			used when resolving resource references.
		 *
		 * @note	If you want objects that reference resources (using ResourceHandles) to be able to
		 * 			find that resource even after application restart, then you must save the resource
		 * 			manifest before closing the application and restore it upon startup.
		 * 			Otherwise resources will be assigned brand new UUIDs and references will be broken.
		 */
		void registerResourceManifest(const ResourceManifestPtr& manifest);

		/**
		 * @brief	Unregisters a resource manifest previously registered with ::registerResourceManifest.
		 */
		void unregisterResourceManifest(const ResourceManifestPtr& manifest);

		/**
		 * @brief	Allows you to retrieve resource manifest containing UUID <-> file path mapping that is
		 * 			used when resolving resource references.
		 * 			
		 * @note	Resources module internally holds a "Default" manifest that it automatically updated whenever
		 * 			a resource is saved.
		 *
		 * @see		registerResourceManifest
		 */
		ResourceManifestPtr getResourceManifest(const String& name) const;

		/**
		 * @brief	Attempts to retrieve file path from the provided UUID. Returns true
		 *			if successful, false otherwise.
		 */
		bool getFilePathFromUUID(const String& uuid, Path& filePath) const;

		/**
		 * @brief	Attempts to retrieve UUID from the provided file path. Returns true
		 *			if successful, false otherwise.
		 */
		bool getUUIDFromFilePath(const Path& path, String& uuid) const;

		/**
		 * @brief	Called when the resource has been successfully loaded. 
		 *
		 * @note	It is undefined from which thread this will get called from.
		 *			Most definitely not the sim thread if resource was being loaded
		 *			asynchronously.
		 */
		Event<void(const HResource&)> onResourceLoaded;

		/**
		 * @brief	Called when the resource has been destroyed.
		 *
		 * @note	It is undefined from which thread this will get called from.
		 */
		Event<void(const HResource&)> onResourceDestroyed;

		/**
		 * @brief	Called when the internal resource the handle is pointing to has changed.
		 *
		 * @note	It is undefined from which thread this will get called from.
		 */
		Event<void(const HResource&)> onResourceModified;
	private:
		/**
		 * @brief	Starts resource loading or returns an already loaded resource. Both UUID and filePath must match the
		 * 			same resource, although you may provide an empty path in which case the resource will be retrieved
		 * 			from memory if its currently loaded.
		 */
		HResource loadInternal(const String& UUID, const Path& filePath, bool synchronous, bool loadDependencies);

		/**
		 * @brief	Performs actually reading and deserializing of the resource file. 
		 *			Called from various worker threads.
		 */
		ResourcePtr loadFromDiskAndDeserialize(const Path& filePath);

		/**
		 * @brief	Triggered when individual resource has finished loading.
		 */
		void loadComplete(HResource& resource);

		/**
		 * @brief	Callback triggered when the task manager is ready to process the loading task.
		 */
		void loadCallback(const Path& filePath, HResource& resource);

	private:
		Vector<ResourceManifestPtr> mResourceManifests;
		ResourceManifestPtr mDefaultResourceManifest;

		BS_MUTEX(mInProgressResourcesMutex);
		BS_MUTEX(mLoadedResourceMutex);

		UnorderedMap<String, HResource> mLoadedResources;
		UnorderedMap<String, ResourceLoadData*> mInProgressResources; // Resources that are being asynchronously loaded
		UnorderedMap<String, Vector<ResourceLoadData*>> mDependantLoads;
	};

	/**
	 * @brief	Provides global access to the resource manager.
	 */
	BS_CORE_EXPORT Resources& gResources();
}