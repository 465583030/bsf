#pragma once

#include "BsPrerequisites.h"
#include "BsModule.h"
#include "BsInput.h"
#include "BsEvent.h"
#include <atomic>

namespace BansheeEngine
{
	/**
	 * @brief	Holds data returned by DragAndDropManager callbacks.
	 */
	struct BS_EXPORT DragCallbackInfo
	{
		DragCallbackInfo()
			:processed(false)
		{ }

		bool processed;
	};

	/**
	 * @brief	Handles GUI drag and drop operations. When active GUI elements
	 *			will be notified of any drag events and will be able to retrieve
	 *			dragged data.
	 *
	 * @note	Sim thread only.
	 */
	class BS_EXPORT DragAndDropManager : public Module<DragAndDropManager>
	{
	public:
		DragAndDropManager();
		~DragAndDropManager();

		/**
		 * @brief	Starts a drag operation of the specified type. This means GUI elements will start receiving
		 * 			drag and drop related events and they may choose to handle them.
		 *
		 * @param	typeId					Type of the drag and drop operation that other objects may query and decide if they want
		 * 									to handle it. User defined.
		 * @param 	data					Some operation specific data that is just passed through to however needs it.
		 * @param	dropCallback			The drop callback that gets triggered whenever mouse button is released and drag operation ends.
		 * 									You should perform any cleanup here.
		 * @param	needsValidDropTarget	(optional) Determines whether the drop operation may happen anywhere or does the GUI element need to specifically
		 * 									accept the drag of this type. If false all GUI elements we mouse over will receive drag/drop events, otherwise only
		 * 									those that specifically subscribe to the specified drag operation of this typeId will.
		 * 									
		 *									Additionally this will determine the cursor displayed (whether or not it can have a "denied" state).
		 */
		void startDrag(UINT32 typeId, void* data, std::function<void(bool)> dropCallback, bool needsValidDropTarget = false);

		/**
		 * @brief	Returns true if drag is currently in progress.
		 */
		bool isDragInProgress() const { return mIsDragInProgress; }

		/**
		 * @brief	Get type ID of drag currently in progress. Only valid if drag is in progress.
		 */
		UINT32 getDragTypeId() const { return mDragTypeId; }

		/**
		 * @brief	Gets drag specific data specified when the drag started. Only valid if drag is in progress.
		 */
		void* getDragData() const { return mData; }

		/**
		 * @brief	Determines whether the drop operation may happen anywhere or does the GUI element need to specifically
		 * 			accept the drag of this type. If false all GUI elements we mouse over will receive drag/drop events, otherwise only
		 * 			those that specifically subscribe to the specified drag operation of this typeId will.
		 * 									
		 *			Additionally this will determine the cursor displayed (whether or not it can have a "denied" state).
		 */
		bool needsValidDropTarget() const { return mNeedsValidDropTarget; }

		/**
		 * @brief	Registers a new callback that will be triggered when dragged item is dropped.
		 *			Provided parameter specifies if the drop operation was handled by anyone or not.
		 */
		void addDropCallback(std::function<void(bool)> dropCallback);

		/**
		 * @brief	Called once per frame.
		 *
		 * @note	Internal method.
		 */
		void _update();

		/**
		 * @brief	Triggers a callback when user releases the pointer and the drag operation ends.
		 *			Provided parameters inform the subscriber where the pointer was released, and allows
		 *			the subscriber to note whether the drag operation was processed or not.
		 *
		 * @note	Internal event. You should use addDropCallback for normal use.
		 */
		Event<void(const PointerEvent&, DragCallbackInfo&)> onDragEnded;
	private:

		/**
		 * @brief	Triggers any drop callbacks and clears callback data.
		 */
		void endDrag(bool processed);

		/**
		 * @brief	Called by the core thread whenever mouse capture state changes.
		 *			This can happen when window loses focus (e.g. alt+tab). In that 
		 *			case we want to end the drag even if the user is still holding 
		 *			the dragged item.
		 *
		 * @note	Core thread.
		 */
		void mouseCaptureChanged();

		/**
		 * @brief	Called by the input system when pointer is released.
		 */
		void cursorReleased(const PointerEvent& event);

	private:
		UINT32 mDragTypeId;
		void* mData;
		Vector<std::function<void(bool)>> mDropCallbacks;
		bool mIsDragInProgress;
		bool mNeedsValidDropTarget;
		HEvent mMouseCaptureChangedConn;

		std::atomic<bool> mCaptureChanged;
		std::atomic<int> mCaptureActive;
		std::atomic<unsigned long> mCaptureChangeFrame;
	};
}