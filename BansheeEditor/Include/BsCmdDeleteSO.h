#pragma once

#include "BsEditorPrerequisites.h"
#include "BsEditorCommand.h"
#include "BsUndoRedo.h"
#include "BsCmdUtility.h"

namespace BansheeEngine
{
	/**
	 * @brief	A command used for undo/redo purposes. Deletes a scene object
	 *			and restores it as an undo operation.
	 */
	class CmdDeleteSO : public EditorCommand
	{
	public:
		~CmdDeleteSO();

		/**
		 * @brief	Creates and executes the command on the provided scene object.
		 *			Automatically registers the command with undo/redo system.
		 *
		 * @param	inputField	Input field to modify the value on.
		 * @param	value		New value for the field.
		 */
		static void execute(const HSceneObject& sceneObject);

		/**
		 * @copydoc	EditorCommand::commit
		 */
		void commit() override;

		/**
		 * @copydoc	EditorCommand::revert
		 */
		void revert() override;

	private:
		friend class UndoRedo;

		CmdDeleteSO(const HSceneObject& sceneObject);

		/**
		 * @brief	Saves the state of the specified object, all of its children
		 *			and components. Make sure to call "clear" when you no longer need
		 *			the data, or wish to call this method again.
		 */
		void recordSO(const HSceneObject& sceneObject);

		/**
		 * @brief	Clears all the stored data and frees memory.
		 */
		void clear();

		HSceneObject mSceneObject;
		CmdUtility::SceneObjProxy mSceneObjectProxy;

		UINT8* mSerializedObject;
		UINT32 mSerializedObjectSize;
		UINT64 mSerializedObjectParentId;
	};
}