#pragma once

#include "BsEditorPrerequisites.h"
#include "BsModule.h"

namespace BansheeEngine
{
	/**
	 * @brief	Project library is the primary location for interacting with all the resources
	 *			in the current project. A complete hierarchy of resources is provided which
	 *			can be interacted with by importing new ones, deleting them, moving, renaming
	 *			and similar.
	 */
	class BS_ED_EXPORT ProjectLibrary : public Module<ProjectLibrary>
	{
	public:
		struct LibraryEntry;
		struct ResourceEntry;
		struct DirectoryEntry;

		/**
		 * @brief	Types of elements in the library, either a file
		 *			or a folder.
		 */
		enum class LibraryEntryType
		{
			File,
			Directory
		};

		/**
		 * @brief	A generic library entry that may be a file or a folder
		 *			depending on its type.
		 */
		struct LibraryEntry
		{
			LibraryEntry();
			LibraryEntry(const Path& path, const WString& name, DirectoryEntry* parent, LibraryEntryType type);

			LibraryEntryType type; /**< Specific type of this entry. */
			Path path; /**< Absolute path to the entry. */
			WString elementName; /**< Name of the entry. */

			DirectoryEntry* parent; /**< Folder this entry is located in. */
		};

		/**
		 * @brief	A library entry representing a resource.
		 */
		struct ResourceEntry : public LibraryEntry
		{
			ResourceEntry();
			ResourceEntry(const Path& path, const WString& name, DirectoryEntry* parent);

			ProjectResourceMetaPtr meta; /**< Meta file containing various information about the resource. */
			std::time_t lastUpdateTime; /**< Timestamp of when we last imported the resource. */
		};

		/**
		 * @brief	A library entry representing a folder that contains other entries.
		 */
		struct DirectoryEntry : public LibraryEntry
		{
			DirectoryEntry();
			DirectoryEntry(const Path& path, const WString& name, DirectoryEntry* parent);

			Vector<LibraryEntry*> mChildren; /**< Child files or folders. */
		};

	public:
		ProjectLibrary(const Path& projectFolder);
		~ProjectLibrary();

		/**
		 * @brief	To be called once per frame.
		 *
		 * @note	Internal methods.
		 */
		void update();

		/**
		 * @brief	Checks if any resources at the specified path have been modified, added or deleted,
		 *			and updates the internal hierarchy accordingly. 
		 *
		 * @param	path	Absolute path of the file or folder to check. If a folder is provided
		 *					all its children will be checked recursively.
		 */
		void checkForModifications(const Path& path);

		/**
		 * @brief	Checks if any resources at the specified path have been modified, added or deleted,
		 *			and updates the internal hierarchy accordingly. 
		 *
		 * @param	path			Absolute path of the file or folder to check. If a folder is provided
		 *							all its children will be checked recursively.
		 * @param	import			Should the dirty resources be automatically reimported.
		 * @param	dirtyResources	A list of resources that should be reimported.
		 */
		void checkForModifications(const Path& path, bool import, Vector<Path>& dirtyResources);

		/**
		 * @brief	Returns the root library entry that references the entire library hierarchy.
		 */
		const LibraryEntry* getRootEntry() const { return mRootEntry; }

		/**
		 * @brief	Attempts to a find a library entry at the specified path.
		 *
		 * @param	path	Path to the entry, either absolute or relative to resources folder.
		 *
		 * @return	Found entry, or null if not found.
		 */
		LibraryEntry* findEntry(const Path& path) const;

		/**
		 * @brief	Searches the library for a pattern.
		 *
		 * @param	pattern	Pattern to search for. Use wildcard * to match any character(s).
		 *
		 * @return	A list of entries matching the pattern.
		 */
		Vector<LibraryEntry*> search(const WString& pattern);

		/**
		 * @brief	Searches the library for a pattern, but only among specific resource types.
		 *
		 * @param	pattern	Pattern to search for. Use wildcard * to match any character(s).
		 * @param	typeIds	RTTI type IDs of the resource types we're interested in searching.
		 *
		 * @return	A list of entries matching the pattern.
		 */
		Vector<LibraryEntry*> search(const WString& pattern, const Vector<UINT32>& typeIds);

		/**
		 * @brief	Finds the resource meta file for a resource with the specified UUID.
		 *
		 * @param	uuid	UUID of the resource to look for.
		 *
		 * @return	Resource meta data if found, null otherwise.
		 */
		ProjectResourceMetaPtr findResourceMeta(const String& uuid) const;

		/**
		 * @brief	Returns resource path based on its UUID.
		 *
		 * @param	uuid	UUID of the resource to look for.
		 *
		 * @return	Absolute path to the resource.
		 */
		Path uuidToPath(const String& uuid) const;

		/**
		 * @brief	Registers a new resource in the library.
		 *
		 * @param	resource	Resource instance to add to the library. A copy of the
		 *						resource will be saved at the provided path.
		 * @param	path		Absolute path to where to store the resource.
		 */
		void createEntry(const HResource& resource, const Path& path);

		/**
		 * @brief	Creates a new folder in the library.
		 *
		 * @param	path		Absolute path to where to store the folder.
		 */
		void createFolderEntry(const Path& path);

		/**
		 * @brief	Updates a resource that is already in the library.
		 */
		void saveEntry(const HResource& resource);

		/**
		 * @brief	Moves a library entry from one path to another.
		 *
		 * @param	oldPath		Source path of the entry, absolute or relative to resources folder.
		 * @param	newPath		Destination path of the entry, absolute or relative to resources folder.
		 * @param	overwrite	If an entry already exists at the destination path, should it be overwritten.
		 */
		void moveEntry(const Path& oldPath, const Path& newPath, bool overwrite = true);

		/**
		 * @brief	Copies a library entry from one path to another.
		 *
		 * @param	oldPath		Source path of the entry, absolute or relative to resources folder.
		 * @param	newPath		Destination path of the entry, absolute or relative to resources folder.
		 * @param	overwrite	If an entry already exists at the destination path, should it be overwritten.
		 */
		void copyEntry(const Path& oldPath, const Path& newPath, bool overwrite = true);

		/**
		 * @brief	Deletes an entry from the library.
		 *
		 * @param	path		Path of the entry, absolute or relative to resources folder.
		 */
		void deleteEntry(const Path& path);

		/**
		 * @brief	Triggers a reimport of a resource using the provided import options, if needed.
		 *
		 * @param	path			Path to the resource to reimport, absolute or relative to resources folder.
		 * @param	importOptions	Optional import options to use when importing the resource. Caller must ensure the
		 *							import options are of the correct type for the resource in question. If null is provided
		 *							default import options are used.
		 * @param	forceReimport	Should the resource be reimported even if we detect no changes. This should be true
		 *							if import options changed since last import.
		 */
		void reimport(const Path& path, const ImportOptionsPtr& importOptions = nullptr, bool forceReimport = false);

		/**
		 * @brief	Loads a resource at the specified path, synchronously.
		 *
		 * @param	path	Path of the resource, absolute or relative to resources folder.
		 *
		 * @return	Loaded resource, or null handle if one is not found.
		 */
		HResource load(const Path& path);

		/**
		 * @brief	Returns the path to the project's resource folder where all
		 *			the assets are stored.
		 */
		const Path& getResourcesFolder() const { return mResourcesFolder; }

		Event<void(const Path&)> onEntryRemoved; /**< Triggered whenever an entry is removed from the library. Path provided is absolute. */
		Event<void(const Path&)> onEntryAdded; /**< Triggered whenever an entry is added to the library. Path provided is absolute. */
	private:
		/**
		 * @brief	Saves all the project library data so it may be restored later,
		 *			at the default save location in the project folder.
		 */
		void save();

		/**
		 * @brief	Loads previously saved project library data from the default save 
		 *			location in the project folder. Nothing is loaded if it doesn't
		 *			exist.
		 */
		void load();

		/**
		 * @brief	Common code for adding a new resource entry to the library.
		 *
		 * @param	parent			Parent of the new entry.
		 * @param	filePath		Absolute path to the resource.
		 * @param	importOptions	Optional import options to use when importing the resource. Caller must ensure the
		 *							import options are of the correct type for the resource in question. If null is provided
		 *							default import options are used.
		 * @param	forceReimport	Should the resource be reimported even if we detect no changes. This should be true
		 *							if import options changed since last import.
		 *
		 * @return	Newly added resource entry.
		 */
		ResourceEntry* addResourceInternal(DirectoryEntry* parent, const Path& filePath, const ImportOptionsPtr& importOptions = nullptr, bool forceReimport = false);

		/**
		 * @brief	Common code for adding a new folder entry to the library.
		 *
		 * @param	parent	Parent of the new entry.
		 * @param	dirPath	Absolute path to the directory.
		 *
		 * @return	Newly added directory entry.
		 */
		DirectoryEntry* addDirectoryInternal(DirectoryEntry* parent, const Path& dirPath);

		/**
		 * @brief	Common code for deleting a resource from the library. This code only removes
		 *			the library entry, not the actual resource file.
		 *
		 * @param	resource	Entry to delete.
		 */
		void deleteResourceInternal(ResourceEntry* resource);

		/**
		 * @brief	Common code for deleting a directory from the library. This code only removes
		 *			the library entry, not the actual directory.
		 *
		 * @param	resource	Entry to delete.
		 */
		void deleteDirectoryInternal(DirectoryEntry* directory);

		/**
		 * @brief	Triggers a reimport of a resource using the provided import options, if needed. Doesn't import dependencies.
		 *
		 * @param	path			Absolute Path to the resource to reimport.
		 * @param	importOptions	Optional import options to use when importing the resource. Caller must ensure the
		 *							import options are of the correct type for the resource in question. If null is provided
		 *							default import options are used.
		 * @param	forceReimport	Should the resource be reimported even if we detect no changes. This should be true
		 *							if import options changed since last import.
		 */
		void reimportResourceInternal(ResourceEntry* resource, const ImportOptionsPtr& importOptions = nullptr, bool forceReimport = false);

		/**
		 * @brief	Creates a full hierarchy of directory entries up to the provided directory, if any are needed.
		 *
		 * @param	fullPath			Absolute path to a directory we are creating the hierarchy to.
		 * @param	newHierarchyRoot	First directory entry that already existed in our hierarchy.
		 * @param	newHierarchyLeaf	Leaf entry corresponding to the exact entry at \p path.
		 */
		void createInternalParentHierarchy(const Path& fullPath, DirectoryEntry** newHierarchyRoot, DirectoryEntry** newHierarchyLeaf);

		/**
		 * @brief	Checks has the resource been modified since the last import.
		 */
		bool isUpToDate(ResourceEntry* resource) const;

		/**
		 * @brief	Returns a path to a .meta file based on the resource path.
		 *
		 * @param	path	Absolute path to the resource.
		 *
		 * @return	Path to the .meta file.
		 */
		Path getMetaPath(const Path& path) const;
		
		/**
		 * @brief	Checks does the path represent a .meta file.
		 */
		bool isMeta(const Path& fullPath) const;

		/**
		 * @brief	Triggered whenever a library entry is removed.
		 */
		void doOnEntryRemoved(const LibraryEntry* entry);

		/**
		 * @brief	Triggered whenever a library entry is added.
		 */
		void doOnEntryAdded(const LibraryEntry* entry);

		/**
		 * @brief	Returns a set of resource paths that are dependent on the provided
		 *			resource entry. (e.g. a shader file might be dependent on shader include file).
		 */
		Vector<Path> getImportDependencies(const ResourceEntry* entry);

		/**
		 * @brief	Registers any import dependencies for the specified resource.
		 */
		void addDependencies(const ResourceEntry* entry);

		/**
		 * @brief	Removes any import dependencies for the specified resource.
		 */
		void removeDependencies(const ResourceEntry* entry);

		/**
		 * @brief	Finds dependants resource for the specified resource entry and queues
		 *			them for reimport next frame.
		 */
		void queueDependantForReimport(const ResourceEntry* entry);

		static const Path RESOURCES_DIR;
		static const Path INTERNAL_RESOURCES_DIR;
		static const WString LIBRARY_ENTRIES_FILENAME;
		static const WString RESOURCE_MANIFEST_FILENAME;

		ResourceManifestPtr mResourceManifest;
		DirectoryEntry* mRootEntry;
		Path mProjectFolder;
		Path mResourcesFolder;

		UnorderedMap<Path, Vector<Path>> mDependencies;
		UnorderedSet<Path> mReimportQueue;
	};
}