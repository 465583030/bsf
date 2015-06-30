#pragma once

#include "BsCorePrerequisites.h"
#include "BsGameObject.h"

namespace BansheeEngine
{
	/**
	 * @brief	Handles various prefab specific operations.
	 */
	class BS_CORE_EXPORT PrefabUtility
	{
	private:
		/**
		 * @brief	Contains saved Component instance data.
		 */
		struct ComponentProxy
		{
			GameObjectInstanceDataPtr instanceData;
			INT32 linkId;
		};

		/**
		 * @brief	Contains saved SceneObject instance data, as well as
		 *			saved instance data for all its children and components.
		 */
		struct SceneObjectProxy
		{
			GameObjectInstanceDataPtr instanceData;
			INT32 linkId;

			Vector<ComponentProxy> components;
			Vector<SceneObjectProxy> children;
		};

	public:
		/**
		 * @brief	Updates all of the objects belonging to the same prefab instance
		 *			as the provided object (if any). The update will remove any instance
		 *			specific changes to the hierarchy and restore it to the exact copy of 
		 *			linked prefab.
		 *
		 * @param[in]	so	Object to revert.
		 */
		static void revertToPrefab(const HSceneObject& so);

		/**
		 * @brief	Updates all of the objects belonging to the same prefab instance
		 *			as the provided object (if any). The update will apply any changes
		 *			from the linked prefab to the hierarchy (if any).
		 *
		 * @param[in]	so	Object to update.
		 */
		static void updateFromPrefab(const HSceneObject& so);

		/**
		 * @brief	Generates prefab "link" ID that can be used for tracking which game object
		 *			in a prefab instance corresponds to an object in the prefab.
		 *
		 * @note	If any children of the provided object belong to another prefab they will 
		 *			not have IDs generated.
		 */
		static void generatePrefabIds(const HSceneObject& sceneObject);

		/**
		 * @brief	Clears all prefab "link" IDs in the provided object and its children.
		 *
		 * @note	If any of its children belong to another prefab they will not be cleared.
		 */
		static void clearPrefabIds(const HSceneObject& sceneObject);

	private:
		/**
	     * @brief	Traverses the object hierarchy, finds all child objects and components
		 *			and records their instance data, as well as their original place in the hierarchy.
		 *			Instance data essentially holds the object's "identity" and by restoring it we
		 *			ensure any handles pointing to the object earlier will still point to the new version.
		 *
		 * @param[in]	so					Object to traverse and record.
		 * @param[out]	output				Contains the output hierarchy of instance data.
		 * @param[out]	linkedInstanceData	A map of link IDs to instance data. Objects without
		 *									link IDs will not be included here.
		 */
		static void recordInstanceData(const HSceneObject& so, SceneObjectProxy& output, 
			UnorderedMap<UINT32, GameObjectInstanceDataPtr>& linkedInstanceData);

		/**
		 * @brief	Restores instance data in the provided hierarchy, using link ids to determine
		 *			what data maps to which objects. 
		 *
		 * @param[in]	so					Object to traverse and restore the instance data.
		 * @param[in]	linkedInstanceData	A map of link IDs to instance data, returned by "recordInstanceData" method.
		 */
		static void restoreLinkedInstanceData(const HSceneObject& so, UnorderedMap<UINT32, GameObjectInstanceDataPtr>& linkedInstanceData);

		/**
		 * @brief	Restores instance data in the provided hierarchy, but only for objects without a link id.
		 *			Since the objects do not have a link ID we rely on their sequential order to find out
		 *			which instance data belong to which object.
		 *
		 * @param[in]	so		Object to traverse and restore the instance data.
		 * @param[in]	proxy	Hierarchy containing instance data for all objects and components, returned by
		 *						"recordInstanceData" method.
		 */
		static void restoreUnlinkedInstanceData(const HSceneObject& so, SceneObjectProxy& proxy);
	};
}