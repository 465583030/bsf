//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPrerequisites.h"
#include "BsGUIMouseEvent.h"
#include "BsGUITextInputEvent.h"
#include "BsGUICommandEvent.h"
#include "BsGUIVirtualButtonEvent.h"
#include "BsSprite.h"
#include "BsModule.h"
#include "BsColor.h"
#include "BsMatrix4.h"
#include "BsEvent.h"
#include "BsMaterialParam.h"

namespace BansheeEngine
{
	/** @cond INTERNAL */
	/** @addtogroup GUI
	 *  @{
	 */

	class GUIManagerCore;

	/**
	 * Manages the rendering and input of all GUI widgets in the scene. 
	 * 			
	 * @note	
	 * If adding or modifying GUIManager functionality ensure that GUIManager data never gets modified outside of update()
	 * method or Input callbacks. If you need such functionality add temporary variables that store you changes and then
	 * execute them delayed in update().
	 * @par
	 * This ensures that GUIElements don't recursively modify GUIManager while GUIManager is still using that data.
	 * @par
	 * for example setFocus() usually gets called from within GUIElements, however we don't want elements in focus be 
	 * modified immediately since that setFocus() call could have originated in sendCommandEvent and elements in focus array
	 * would be modified while still being iterated upon.
	 * @note
	 * Internal class. Unless modifying internal engine systems you should have no need to access this class.
	 */
	class BS_EXPORT GUIManager : public Module<GUIManager>
	{
		/**	Valid states of a drag and drop operation. */
		enum class DragState
		{
			NoDrag,
			HeldWithoutDrag,
			Dragging
		};

		/**	GUI render data for a single viewport. */
		struct GUIRenderData
		{
			GUIRenderData()
				:isDirty(true)
			{ }

			Vector<TransientMeshPtr> cachedMeshes;
			Vector<SpriteMaterialInfo> cachedMaterials;
			Vector<GUIWidget*> cachedWidgetsPerMesh;
			Vector<GUIWidget*> widgets;
			bool isDirty;
		};

		/**	Render data for a single GUI group used for notifying the core GUI renderer. */
		struct GUICoreRenderData
		{
			SPtr<TransientMeshCore> mesh;
			SPtr<TextureCore> texture;
			SpriteMaterial materialType;
			Color tint;
			Matrix4 worldTransform;
		};

		/**	Container for a GUI widget. */
		struct WidgetInfo
		{
			WidgetInfo(GUIWidget* _widget)
				:widget(_widget)
			{ }

			GUIWidget* widget;
		};

		/**	Container for data about a single GUI element and its widget. */
		struct ElementInfo
		{
			ElementInfo(GUIElement* element, GUIWidget* widget)
				:element(element), widget(widget)
			{ }

			GUIElement* element;
			GUIWidget* widget;
		};

		/**	Container for data about a single GUI element and its widget currently under the pointer. */
		struct ElementInfoUnderPointer
		{
			ElementInfoUnderPointer(GUIElement* element, GUIWidget* widget)
				:element(element), widget(widget), usesMouseOver(false), 
				receivedMouseOver(false), isHovering(false)
			{ }

			GUIElement* element;
			GUIWidget* widget;
			bool usesMouseOver;
			bool receivedMouseOver;
			bool isHovering;
		};

		/**	Container for GUI element in focus. */
		struct ElementFocusInfo
		{
			ElementFocusInfo(GUIElement* element, GUIWidget* widget, bool usesFocus)
				:element(element), widget(widget), usesFocus(usesFocus)
			{ }

			GUIElement* element;
			GUIWidget* widget;
			bool usesFocus;
		};

		/**	Container for GUI elements that need to have their focus state forcefully changed. */
		struct ElementForcedFocusInfo
		{
			GUIElement* element;
			bool focus;
			bool usesFocus;
		};

	public:
		GUIManager();
		~GUIManager();

		/** Registers a newly created widget with the GUI manager. This should be called by every GUI widget on creation. */
		void registerWidget(GUIWidget* widget);

		/**
		 * Unregisters a GUI widget from the GUI manager. This should be called by every GUI widget before getting deleted.
		 */
		void unregisterWidget(GUIWidget* widget);

		/**	Called once per frame. */
		void update();

		/** Queues the GUI element for destruction. Element will be destroyed during the next call to update(). */
		void queueForDestroy(GUIElement* element);

		/**	Change the GUI element focus state. */
		void setFocus(GUIElement* element, bool focus);

		/**	Changes the color of the input caret used in input boxes and similar controls. */
		void setCaretColor(const Color& color) { mCaretColor = color; updateCaretTexture(); }

		/**	Changes the text selection highlight color used in input boxes and similar controls. */
		void setTextSelectionColor(const Color& color) { mTextSelectionColor = color; updateTextSelectionTexture(); }

		/**	Returns the default caret texture used for rendering the input caret sprite. */
		const HSpriteTexture& getCaretTexture() const { return mCaretTexture; }

		/**	Returns the default selection highlight texture used for rendering the selection highlight sprites. */
		const HSpriteTexture& getTextSelectionTexture() const { return mTextSelectionTexture; }

		/**	Checks is the input caret visible this frame. */
		bool getCaretBlinkState() const { return mIsCaretOn; }

		/**
		 * Returns input caret helper tool that allows you to easily position and show an input caret in your GUI controls.
		 */
		GUIInputCaret* getInputCaretTool() const { return mInputCaret; }

		/**
		 * Returns input selection helper tool that allows you to easily position and show an input selection highlight in
		 * your GUI controls.
		 */
		GUIInputSelection* getInputSelectionTool() const { return mInputSelection; }

		/**
		 * Allows you to bridge GUI input from a GUI element into another render target.
		 *
		 * @param[in]	renderTex 	The render target to which to bridge the input.
		 * @param[in]	element		The element from which to bridge input. Input will be transformed according to this 
		 *							elements position and size. Provide nullptr if you want to remove a bridge for the
		 *							specified widget.
		 * 					
		 * @note	
		 * This is useful if you use render textures, where your GUI is rendered off-screen. In such case you need to 
		 * display the render texture within another GUIElement in a GUIWidget, but have no way of sending input to the
		 * render texture (normally input is only sent to render windows). This allows you to change that - any GUIWidget
		 * using the bridged render texture as a render target will then receive input when mouse is over the specified
		 * element.
		 * @note			
		 * Bridged element needs to remove itself as the bridge when it is destroyed.
		 */
		void setInputBridge(const RenderTexture* renderTex, const GUIElement* element);

		/** Gets the core thread portion of the GUI manager, responsible for rendering of GUI elements. */
		GUIManagerCore* getCore() const { return mCore.load(std::memory_order_relaxed); }

	private:
		friend class GUIManagerCore;

		/**	Recreates all dirty GUI meshes and makes them ready for rendering. */
		void updateMeshes();

		/**	Recreates the input caret texture. */
		void updateCaretTexture();

		/**	Recreates the input text selection highlight texture. */
		void updateTextSelectionTexture();

		/**
		 * Destroys the core thread counterpart of the GUI manager.
		 *
		 * @param[in]	core	Previously constructed core thread GUI manager instance.
		 */
		void destroyCore(GUIManagerCore* core);

		/**
		 * Destroys any elements or widgets queued for destruction.
		 *
		 * @note	
		 * Returns true if more elements have been added for destruction (will happen when destruction of one element
		 * queues up destruction of another).
		 */
		bool processDestroyQueue();

		/**
		 * Finds a GUI element under the pointer at the specified screen position. This method will also trigger pointer
		 * move/hover/leave events.
		 *
		 * @param[in]	screenMousePos	Position of the pointer in screen coordinates.
		 * @param[in]	buttonStates	States of the three mouse buttons (left, right, middle).
		 * @param[in]	shift			Is shift key held.
		 * @param[in]	control			Is control key held.
		 * @param[in]	alt				Is alt key held.
		 */
		bool findElementUnderPointer(const Vector2I& screenMousePos, bool buttonStates[3], bool shift, bool control, bool alt);

		/**	Called whenever a pointer (for example mouse cursor) is moved. */
		void onPointerMoved(const PointerEvent& event);

		/**	Called whenever a pointer button (for example mouse button) is released. */
		void onPointerReleased(const PointerEvent& event);

		/**	Called whenever a pointer button (for example mouse button) is pressed. */
		void onPointerPressed(const PointerEvent& event);

		/**	Called whenever a pointer button (for example mouse button) is double clicked. */
		void onPointerDoubleClick(const PointerEvent& event);

		/**	Called whenever a text is input. */
		void onTextInput(const TextInputEvent& event);

		/**	Called whenever an input command is input. */
		void onInputCommandEntered(InputCommandType commandType);

		/**	Called whenever a virtual button is pressed. */
		void onVirtualButtonDown(const VirtualButton& button, UINT32 deviceIdx);

		/**	Called by the drag and drop managed to notify us the drag ended. */
		void onMouseDragEnded(const PointerEvent& event, DragCallbackInfo& dragInfo);

		/**	Called when the specified window gains focus. */
		void onWindowFocusGained(RenderWindow& win);

		/**	Called when the specified window loses focus. */
		void onWindowFocusLost(RenderWindow& win);

		/**	Called when the mouse leaves the specified window. */
		void onMouseLeftWindow(RenderWindow& win);

		/**	Converts pointer buttons to mouse buttons. */
		GUIMouseButton buttonToGUIButton(PointerEventButton pointerButton) const;

		/**	Converts screen coordinates to coordinates relative to the specified widget. */
		Vector2I getWidgetRelativePos(const GUIWidget* widget, const Vector2I& screenPos) const;

		/**
		 * Converts window coordinates to coordinates relative to the specified bridged widget. Returned coordinates will
		 * be relative to the bridge element.
		 *
		 * @param[in]	If provided widget has no bridge, coordinates are returned as is.
		 */
		Vector2I windowToBridgedCoords(const GUIWidget& widget, const Vector2I& windowPos) const;

		/**	Returns the parent render window of the specified widget. */
		const RenderWindow* getWidgetWindow(const GUIWidget& widget) const;

		/**	Hides the tooltip if any is shown. */
		void hideTooltip();

		/**
		 * Sends a mouse event to the specified GUI element.
		 *
		 * @param[in]	element	Element to send the event to.
		 * @param[in]	event	Event data.
		 */
		bool sendMouseEvent(GUIElement* element, const GUIMouseEvent& event);

		/**
		 * Sends a text input event to the specified GUI element.
		 *
		 * @param[in]	element	Element to send the event to.
		 * @param[in]	event	Event data.
		 */
		bool sendTextInputEvent(GUIElement* element, const GUITextInputEvent& event);

		/**
		 * Sends a command event to the specified GUI element.
		 *
		 * @param[in]	element	Element to send the event to.
		 * @param[in]	event	Event data.
		 */
		bool sendCommandEvent(GUIElement* element, const GUICommandEvent& event);

		/**
		 * Sends a virtual button event to the specified GUI element.
		 *
		 * @param[in]	element	Element to send the event to.
		 * @param[in]	event	Event data.
		 */
		bool sendVirtualButtonEvent(GUIElement* element, const GUIVirtualButtonEvent& event);

		static const UINT32 DRAG_DISTANCE;
		static const float TOOLTIP_HOVER_TIME;

		static const UINT32 MESH_HEAP_INITIAL_NUM_VERTS;
		static const UINT32 MESH_HEAP_INITIAL_NUM_INDICES;

		Vector<WidgetInfo> mWidgets;
		UnorderedMap<const Viewport*, GUIRenderData> mCachedGUIData;
		MeshHeapPtr mMeshHeap;

		std::atomic<GUIManagerCore*> mCore;
		bool mCoreDirty;

		VertexDataDescPtr mVertexDesc;

		Stack<GUIElement*> mScheduledForDestruction;

		// Element and widget pointer is currently over
		Vector<ElementInfoUnderPointer> mElementsUnderPointer;
		Vector<ElementInfoUnderPointer> mNewElementsUnderPointer;

		// Element and widget that's being clicked on
		GUIMouseButton mActiveMouseButton;
		Vector<ElementInfo> mActiveElements;
		Vector<ElementInfo> mNewActiveElements;

		// Element and widget that currently have the keyboard focus
		Vector<ElementFocusInfo> mElementsInFocus;
		Vector<ElementFocusInfo> mNewElementsInFocus;

		Vector<ElementForcedFocusInfo> mForcedFocusElements;

		// Tooltip
		bool mShowTooltip;
		float mTooltipElementHoverStart;

		GUIInputCaret* mInputCaret;
		GUIInputSelection* mInputSelection;

		bool mSeparateMeshesByWidget;
		Vector2I mLastPointerScreenPos;

		DragState mDragState;
		Vector2I mLastPointerClickPos;
		Vector2I mDragStartPos;

		GUIMouseEvent mMouseEvent;
		GUITextInputEvent mTextInputEvent;
		GUICommandEvent mCommandEvent;
		GUIVirtualButtonEvent mVirtualButtonEvent;

		HSpriteTexture mCaretTexture;
		Color mCaretColor;
		float mCaretBlinkInterval;
		float mCaretLastBlinkTime;
		bool mIsCaretOn;
		CursorType mActiveCursor;

		HSpriteTexture mTextSelectionTexture;
		Color mTextSelectionColor;

		Map<const RenderTexture*, const GUIElement*> mInputBridge;

		HEvent mOnPointerMovedConn;
		HEvent mOnPointerPressedConn;
		HEvent mOnPointerReleasedConn;
		HEvent mOnPointerDoubleClick;
		HEvent mOnTextInputConn;
		HEvent mOnInputCommandConn;
		HEvent mOnVirtualButtonDown;

		HEvent mDragEndedConn;

		HEvent mWindowGainedFocusConn;
		HEvent mWindowLostFocusConn;

		HEvent mMouseLeftWindowConn;
	};

	/**	Handles GUI rendering on the core thread. */
	class BS_EXPORT GUIManagerCore
	{
		friend class GUIManager;

		/** Material reference and parameter handles for a specific material type. */
		struct MaterialInfo
		{
			MaterialInfo() { }
			MaterialInfo(const SPtr<MaterialCore>& material);

			SPtr<MaterialCore> material;

			MaterialParamMat4Core worldTransformParam;
			MaterialParamFloatCore invViewportWidthParam;
			MaterialParamFloatCore invViewportHeightParam;
			MaterialParamColorCore tintParam;
			MaterialParamTextureCore textureParam;
			MaterialParamSampStateCore samplerParam;
		};

	public:
		~GUIManagerCore();

		/**
		 * Initializes the object. Must be called right after construction.
		 *
		 * @param[in]	textMat			Material used for drawing text sprites.
		 * @param[in]	imageMat		Material used for drawing non-transparent image sprites.
		 * @param[in]	imageAlphaMat	Material used for drawing transparent image sprites.
		 */
		void initialize(const SPtr<MaterialCore>& textMat, const SPtr<MaterialCore>& imageMat,
			const SPtr<MaterialCore>& imageAlphaMat);

	private:
		/**
		 * Updates the internal data that determines what will be rendered on the next render() call.
		 *
		 * @param[in]	data	GUI mesh/material per viewport.
		 */
		void updateData(const UnorderedMap<SPtr<CameraCore>, Vector<GUIManager::GUICoreRenderData>>& perCameraData);

		/**	Triggered by the Renderer when the GUI should be rendered. */
		void render(const SPtr<CameraCore>& camera);

		UnorderedMap<SPtr<CameraCore>, Vector<GUIManager::GUICoreRenderData>> mPerCameraData;

		// Immutable
		MaterialInfo mTextMaterialInfo;
		MaterialInfo mImageMaterialInfo;
		MaterialInfo mImageAlphaMaterialInfo;

		SPtr<SamplerStateCore> mSamplerState;
	};

	/** Provides easier access to GUIManager. */
	BS_EXPORT GUIManager& gGUIManager();

	/** @} */
	/** @endcond */
}