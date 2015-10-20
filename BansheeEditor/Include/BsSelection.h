#pragma once

#include "BsEditorPrerequisites.h"
#include "BsModule.h"

namespace BansheeEngine
{
	/**
	 * @brief	Handles scene object and resource selection. Triggeres events when selection changes and allows the 
	 *			user to query current selection state.
	 */
	class BS_ED_EXPORT Selection : public Module<Selection>
	{
	public:
		Selection();
		~Selection();

		/**
		 * @brief	Returns a currently selected set of scene objects.
		 */
		const Vector<HSceneObject>& getSceneObjects() const;

		/**
		 * @brief	Sets a new set of scene objects to select, replacing the old ones.
		 */
		void setSceneObjects(const Vector<HSceneObject>& sceneObjects);

		/**
		 * @brief	Returns a currently selected set of resource paths.
		 */
		const Vector<Path>& getResourcePaths() const;

		/**
		 * @brief	Sets a new set of resource paths to select, replacing the old ones.
		 */
		void setResourcePaths(const Vector<Path>& paths);

		/**
		 * @brief	Returns a currently selected set of resource UUIDs.
		 */
		Vector<String> getResourceUUIDs() const;

		/**
		 * @brief	Sets a new set of resource UUIDs to select, replacing the old ones.
		 */
		void setResourceUUIDs(const Vector<String>& UUIDs);

		/**
		 * @brief	Deselects all currently selected scene objects.
		 */
		void clearSceneSelection();

		/**
		 * @brief	Deselects all currently selected resources.
		 */
		void clearResourceSelection();

		/**
		 * @brief	Pings the scene object, highlighting it in its respective editors.
		 */
		void ping(const HSceneObject& sceneObject);

		/**
		 * @brief	Pings the resource, highlighting it in its respective editors.
		 * 		
		 * @param	resourcePath	Resource path relative to the project library resources folder.
		 */
		void ping(const Path& resourcePath);

		/**
		 * @brief	Triggered whenever scene object or resource selection changes. The provided
		 *			parameters will contain the newly selected objects/resource paths.
		 */
		Event<void(const Vector<HSceneObject>&, const Vector<Path>&)> onSelectionChanged; 

		/**
		 * @brief	Triggered when a scene object ping is requested. Ping usually means the
		 *			object will be highlighted in its respective editors.
		 */
		Event<void(const HSceneObject&)> onSceneObjectPing; 

		/**
		 * @brief	Triggered when a resource ping is requested. Ping usually means the
		 *			object will be highlighted in its respective editors.
		 */
		Event<void(const Path&)> onResourcePing; 
	private:
		/**
		 * @brief	Triggered when the scene object selection in the scene tree view changes.
		 */
		void sceneSelectionChanged();

		/**
		 * @brief	Triggered when the resource selection in the resource tree view changes.
		 */
		void resourceSelectionChanged();

		/**
		 * @brief	Updates scene and resource tree views with new selection.
		 */
		void updateTreeViews();

		Vector<HSceneObject> mSelectedSceneObjects;
		Vector<Path> mSelectedResourcePaths;

		HMessage mSceneSelectionChangedConn;
		HMessage mResourceSelectionChangedConn;
	};
}