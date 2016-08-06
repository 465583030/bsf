﻿//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using System;
using System.Collections.Generic;
using BansheeEngine;

namespace BansheeEditor
{
    /** @addtogroup Windows
     *  @{
     */

    /// <summary>
    /// Displays animation curve editor window.
    /// </summary>
    [DefaultSize(900, 500)]
    internal class AnimationWindow : EditorWindow
    {
        private const int FIELD_DISPLAY_WIDTH = 200;
        private const int DRAG_START_DISTANCE = 3;
        private const float DRAG_SCALE = 10.0f;
        private const float ZOOM_SCALE = 0.1f/120.0f; // One scroll step is usually 120 units, we want 1/10 of that

        private bool isInitialized;
        private SceneObject selectedSO;

        #region Overrides

        /// <summary>
        /// Opens the animation window.
        /// </summary>
        [MenuItem("Windows/Animation", ButtonModifier.CtrlAlt, ButtonCode.A, 6000)]
        private static void OpenGameWindow()
        {
            OpenWindow<AnimationWindow>();
        }

        /// <inheritdoc/>
        protected override LocString GetDisplayName()
        {
            return new LocEdString("Animation");
        }

        private void OnInitialize()
        {
            Selection.OnSelectionChanged += OnSelectionChanged;
            EditorInput.OnPointerPressed += OnPointerPressed;
            EditorInput.OnPointerMoved += OnPointerMoved;
            EditorInput.OnPointerReleased += OnPointerReleased;
            EditorInput.OnButtonUp += OnButtonUp;

            RebuildGUI();
        }

        private void OnEditorUpdate()
        {
            if (!isInitialized)
                return;

            HandleDragAndZoomInput();
        }

        private void OnDestroy()
        {
            Selection.OnSelectionChanged -= OnSelectionChanged;
            EditorInput.OnPointerPressed -= OnPointerPressed;
            EditorInput.OnPointerMoved -= OnPointerMoved;
            EditorInput.OnPointerReleased -= OnPointerReleased;
            EditorInput.OnButtonUp -= OnButtonUp;
        }

        protected override void WindowResized(int width, int height)
        {
            if (!isInitialized)
                return;

            ResizeGUI(width, height);
        }
        #endregion

        #region GUI
        private GUIButton playButton;
        private GUIButton recordButton;

        private GUIButton prevFrameButton;
        private GUIIntField frameInputField;
        private GUIButton nextFrameButton;

        private GUIButton addKeyframeButton;
        private GUIButton addEventButton;

        private GUIButton optionsButton;

        private GUIButton addPropertyBtn;
        private GUIButton delPropertyBtn;

        private GUILayout buttonLayout;

        private int buttonLayoutHeight;
        private int scrollBarWidth;
        private int scrollBarHeight;

        private GUIResizeableScrollBarH horzScrollBar;
        private GUIResizeableScrollBarV vertScrollBar;

        private GUIPanel editorPanel;
        private GUIAnimFieldDisplay guiFieldDisplay;
        private GUICurveEditor guiCurveEditor;

        private void RebuildGUI()
        {
            GUI.Clear();
            selectedFields.Clear();
            curves.Clear();
            isInitialized = false;

            if (selectedSO != Selection.SceneObject)
            {
                zoomAmount = 0.0f;
                selectedSO = Selection.SceneObject;
            }

            if (selectedSO == null)
            {
                GUILabel warningLbl = new GUILabel(new LocEdString("Select an object to animate in the Hierarchy or Scene windows."));

                GUILayoutY vertLayout = GUI.AddLayoutY();
                vertLayout.AddFlexibleSpace();
                GUILayoutX horzLayout = vertLayout.AddLayoutX();
                vertLayout.AddFlexibleSpace();

                horzLayout.AddFlexibleSpace();
                horzLayout.AddElement(warningLbl);
                horzLayout.AddFlexibleSpace();

                return;
            }

            // TODO - Retrieve Animation & AnimationClip from the selected object, fill curves dictionary
            //  - If not available, show a button to create new animation clip

            // Top button row
            GUIContent playIcon = new GUIContent(EditorBuiltin.GetAnimationWindowIcon(AnimationWindowIcon.Play),
                new LocEdString("Play"));
            GUIContent recordIcon = new GUIContent(EditorBuiltin.GetAnimationWindowIcon(AnimationWindowIcon.Record),
                new LocEdString("Record"));

            GUIContent prevFrameIcon = new GUIContent(EditorBuiltin.GetAnimationWindowIcon(AnimationWindowIcon.FrameBack),
                new LocEdString("Previous frame"));
            GUIContent nextFrameIcon = new GUIContent(EditorBuiltin.GetAnimationWindowIcon(AnimationWindowIcon.FrameForward),
                new LocEdString("Next frame"));

            GUIContent addKeyframeIcon = new GUIContent(EditorBuiltin.GetAnimationWindowIcon(AnimationWindowIcon.AddKeyframe),
                new LocEdString("Add keyframe"));
            GUIContent addEventIcon = new GUIContent(EditorBuiltin.GetAnimationWindowIcon(AnimationWindowIcon.AddEvent),
                new LocEdString("Add event"));

            GUIContent optionsIcon = new GUIContent(EditorBuiltin.GetLibraryWindowIcon(LibraryWindowIcon.Options),
                new LocEdString("Options"));

            playButton = new GUIButton(playIcon);
            recordButton = new GUIButton(recordIcon);

            prevFrameButton = new GUIButton(prevFrameIcon);
            frameInputField = new GUIIntField();
            nextFrameButton = new GUIButton(nextFrameIcon);

            addKeyframeButton = new GUIButton(addKeyframeIcon);
            addEventButton = new GUIButton(addEventIcon);

            optionsButton = new GUIButton(optionsIcon);

            playButton.OnClick += () =>
            {
                // TODO
                // - Record current state of the scene object hierarchy
                // - Evaluate all curves manually and update them
                // - On end, restore original values of the scene object hierarchy
            };

            recordButton.OnClick += () =>
            {
                // TODO
                // - Every frame read back current values of all the current curve's properties and assign it to the current frame
            };

            prevFrameButton.OnClick += () =>
            {
                SetCurrentFrame(currentFrameIdx - 1);
            };

            frameInputField.OnChanged += SetCurrentFrame;

            nextFrameButton.OnClick += () =>
            {
                SetCurrentFrame(currentFrameIdx + 1);
            };

            addKeyframeButton.OnClick += () =>
            {
                guiCurveEditor.AddKeyFrameAtMarker();
            };

            addEventButton.OnClick += () =>
            {
                // TODO - Add event
            };

            optionsButton.OnClick += () =>
            {
                Vector2I openPosition = ScreenToWindowPos(Input.PointerPosition);
                AnimationOptions dropDown = DropDownWindow.Open<AnimationOptions>(this, openPosition);
                dropDown.Initialize(this);
            };

            // Property buttons
            addPropertyBtn = new GUIButton(new LocEdString("Add property"));
            delPropertyBtn = new GUIButton(new LocEdString("Delete selected"));

            addPropertyBtn.OnClick += () =>
            {
                Vector2I windowPos = ScreenToWindowPos(Input.PointerPosition);
                FieldSelectionWindow fieldSelection = DropDownWindow.Open<FieldSelectionWindow>(this, windowPos);
                fieldSelection.OnFieldSelected += OnFieldAdded;
            };

            delPropertyBtn.OnClick += () =>
            {
                LocEdString title = new LocEdString("Warning");
                LocEdString message = new LocEdString("Are you sure you want to remove all selected fields?");

                DialogBox.Open(title, message, DialogBox.Type.YesNo, x =>
                {
                    if (x == DialogBox.ResultType.Yes)
                    {
                        RemoveSelectedFields();
                    }
                });
            };

            GUILayout mainLayout = GUI.AddLayoutY();

            buttonLayout = mainLayout.AddLayoutX();
            buttonLayout.AddSpace(5);
            buttonLayout.AddElement(playButton);
            buttonLayout.AddElement(recordButton);
            buttonLayout.AddSpace(5);
            buttonLayout.AddElement(prevFrameButton);
            buttonLayout.AddElement(frameInputField);
            buttonLayout.AddElement(nextFrameButton);
            buttonLayout.AddSpace(5);
            buttonLayout.AddElement(addKeyframeButton);
            buttonLayout.AddElement(addEventButton);
            buttonLayout.AddSpace(5);
            buttonLayout.AddElement(optionsButton);
            buttonLayout.AddFlexibleSpace();

            buttonLayoutHeight = playButton.Bounds.height;

            GUILayout contentLayout = mainLayout.AddLayoutX();
            GUILayout fieldDisplayLayout = contentLayout.AddLayoutY(GUIOption.FixedWidth(FIELD_DISPLAY_WIDTH));

            guiFieldDisplay = new GUIAnimFieldDisplay(fieldDisplayLayout, FIELD_DISPLAY_WIDTH,
                Height - buttonLayoutHeight * 2, selectedSO);
            guiFieldDisplay.OnEntrySelected += OnFieldSelected;

            GUILayout bottomButtonLayout = fieldDisplayLayout.AddLayoutX();
            bottomButtonLayout.AddElement(addPropertyBtn);
            bottomButtonLayout.AddElement(delPropertyBtn);

            horzScrollBar = new GUIResizeableScrollBarH();
            horzScrollBar.OnScrollOrResize += OnHorzScrollOrResize;

            vertScrollBar = new GUIResizeableScrollBarV();
            vertScrollBar.OnScrollOrResize += OnVertScrollOrResize;

            GUILayout curveLayout = contentLayout.AddLayoutY();
            GUILayout curveLayoutHorz = curveLayout.AddLayoutX();
            GUILayout horzScrollBarLayout = curveLayout.AddLayoutX();
            horzScrollBarLayout.AddElement(horzScrollBar);
            horzScrollBarLayout.AddFlexibleSpace();

            editorPanel = curveLayoutHorz.AddPanel();
            curveLayoutHorz.AddElement(vertScrollBar);
            curveLayoutHorz.AddFlexibleSpace();

            scrollBarHeight = horzScrollBar.Bounds.height;
            scrollBarWidth = vertScrollBar.Bounds.width;

            Vector2I curveEditorSize = GetCurveEditorSize();
            guiCurveEditor = new GUICurveEditor(this, editorPanel, curveEditorSize.x, curveEditorSize.y);
            guiCurveEditor.OnFrameSelected += OnFrameSelected;
            guiCurveEditor.Redraw();

            horzScrollBar.SetWidth(curveEditorSize.x);
            vertScrollBar.SetHeight(curveEditorSize.y);

            SetCurrentFrame(currentFrameIdx);
            UpdateScrollBarSize();

            isInitialized = true;
        }

        private void ResizeGUI(int width, int height)
        {
            guiFieldDisplay.SetSize(FIELD_DISPLAY_WIDTH, height - buttonLayoutHeight * 2);

            Vector2I curveEditorSize = GetCurveEditorSize();
            guiCurveEditor.SetSize(curveEditorSize.x, curveEditorSize.y);
            guiCurveEditor.Redraw();

            horzScrollBar.SetWidth(curveEditorSize.x);
            vertScrollBar.SetHeight(curveEditorSize.y);

            UpdateScrollBarSize();
            UpdateScrollBarPosition();
        }
        #endregion

        #region Scroll, drag, zoom
        private Vector2I dragStartPos;
        private bool isButtonHeld;
        private bool isDragInProgress;

        private float zoomAmount;

        private void HandleDragAndZoomInput()
        {
            // Handle middle mouse dragging
            if (isDragInProgress)
            {
                float dragX = Input.GetAxisValue(InputAxis.MouseX) * DRAG_SCALE;
                float dragY = Input.GetAxisValue(InputAxis.MouseY) * DRAG_SCALE;

                Vector2 offset = guiCurveEditor.Offset;
                offset.x = Math.Max(0.0f, offset.x + dragX);
                offset.y += dragY;
                
                guiCurveEditor.Offset = offset;
                UpdateScrollBarSize();
                UpdateScrollBarPosition();
            }

            // Handle zoom in/out
            float scroll = Input.GetAxisValue(InputAxis.MouseZ);
            if (scroll != 0.0f)
            {
                Vector2I windowPos = ScreenToWindowPos(Input.PointerPosition);
                Vector2 curvePos;
                if (guiCurveEditor.WindowToCurveSpace(windowPos, out curvePos))
                {
                    float zoom = scroll * ZOOM_SCALE;
                    Zoom(curvePos, zoom);
                }
            }
        }

        private void SetVertScrollbarProperties(float position, float size)
        {
            Vector2 visibleRange = guiCurveEditor.Range;
            Vector2 totalRange = GetTotalRange();

            visibleRange.y = totalRange.y*size;
            guiCurveEditor.Range = visibleRange;

            float scrollableRange = totalRange.y - visibleRange.y;

            Vector2 offset = guiCurveEditor.Offset;
            offset.y = -scrollableRange * (position * 2.0f - 1.0f);

            guiCurveEditor.Offset = offset;
        }

        private void SetHorzScrollbarProperties(float position, float size)
        {
            Vector2 visibleRange = guiCurveEditor.Range;
            Vector2 totalRange = GetTotalRange();

            visibleRange.x = totalRange.x * size;
            guiCurveEditor.Range = visibleRange;

            float scrollableRange = totalRange.x - visibleRange.x;

            Vector2 offset = guiCurveEditor.Offset;
            offset.x = scrollableRange * position;

            guiCurveEditor.Offset = offset;
        }

        private void UpdateScrollBarSize()
        {
            Vector2 visibleRange = guiCurveEditor.Range;
            Vector2 totalRange = GetTotalRange();

            horzScrollBar.HandleSize = visibleRange.x / totalRange.x;
            vertScrollBar.HandleSize = visibleRange.y / totalRange.y;
        }

        private void UpdateScrollBarPosition()
        {
            Vector2 visibleRange = guiCurveEditor.Range;
            Vector2 totalRange = GetTotalRange();
            Vector2 scrollableRange = totalRange - visibleRange;

            Vector2 offset = guiCurveEditor.Offset;
            if (scrollableRange.x > 0.0f)
                horzScrollBar.Position = offset.x / scrollableRange.x;
            else
                horzScrollBar.Position = 0.0f;

            if (scrollableRange.y > 0.0f)
            {
                float pos = offset.y/scrollableRange.y;
                float sign = MathEx.Sign(pos);
                pos = sign*MathEx.Clamp01(MathEx.Abs(pos));
                pos = (1.0f - pos) /2.0f;

                vertScrollBar.Position = pos;
            }
            else
                vertScrollBar.Position = 0.0f;
        }

        private Vector2 GetZoomedRange()
        {
            float zoomLevel = MathEx.Pow(2, zoomAmount);

            Vector2 optimalRange = GetOptimalRange();
            return optimalRange / zoomLevel;
        }

        private Vector2 GetTotalRange()
        {
            // Return optimal range (that covers the visible curve)
            Vector2 totalRange = GetOptimalRange();

            // Increase range in case user zoomed out
            Vector2 zoomedRange = GetZoomedRange();
            totalRange = Vector2.Max(totalRange, zoomedRange);

            // Increase range in case user dragged outside of the optimal range
            Vector2 visibleRange = guiCurveEditor.Range;
            Vector2 draggedRange = guiCurveEditor.Offset;
            draggedRange.x += visibleRange.x;
            draggedRange.y = Math.Abs(draggedRange.y) + visibleRange.y;

            return Vector2.Max(totalRange, draggedRange);
        }

        private void Zoom(Vector2 curvePos, float amount)
        {
            // Increase or decrease the visible range depending on zoom level
            Vector2 oldZoomedRange = GetZoomedRange();
            zoomAmount = MathEx.Clamp(zoomAmount + amount, -10.0f, 10.0f);
            Vector2 zoomedRange = GetZoomedRange();

            Vector2 zoomedDiff = zoomedRange - oldZoomedRange;
            zoomedDiff.y *= 0.5f;

            Vector2 currentRange = guiCurveEditor.Range;
            Vector2 newRange = currentRange + zoomedDiff;
            guiCurveEditor.Range = newRange;

            // When zooming, make sure to focus on the point provided, so adjust the offset
            Vector2 rangeScale = newRange;
            rangeScale.x /= currentRange.x;
            rangeScale.y /= currentRange.y;

            Vector2 relativeCurvePos = curvePos - guiCurveEditor.Offset;
            Vector2 newCurvePos = relativeCurvePos * rangeScale;
            Vector2 diff = newCurvePos - relativeCurvePos;

            guiCurveEditor.Offset -= diff;

            UpdateScrollBarSize();
            UpdateScrollBarPosition();
        }
        #endregion

        #region Curve display
        /// <summary>
        /// A set of animation curves for a field of a certain type.
        /// </summary>
        private struct FieldCurves
        {
            public SerializableProperty.FieldType type;
            public EdAnimationCurve[] curves;
        }

        private int currentFrameIdx;
        private int fps = 1;
        private Dictionary<string, FieldCurves> curves = new Dictionary<string, FieldCurves>();

        internal int FPS
        {
            get { return fps; }
            set { guiCurveEditor.SetFPS(value); fps = MathEx.Max(value, 1); }
        }

        private void SetCurrentFrame(int frameIdx)
        {
            currentFrameIdx = Math.Max(0, frameIdx);

            frameInputField.Value = currentFrameIdx;
            guiCurveEditor.SetMarkedFrame(currentFrameIdx);

            float time = guiCurveEditor.GetTimeForFrame(currentFrameIdx);

            List<GUIAnimFieldPathValue> values = new List<GUIAnimFieldPathValue>();
            foreach (var kvp in curves)
            {
                SerializableProperty property = GUIAnimFieldDisplay.FindProperty(selectedSO, kvp.Key);
                if (property != null)
                {
                    GUIAnimFieldPathValue fieldValue = new GUIAnimFieldPathValue();
                    fieldValue.path = kvp.Key;

                    switch (kvp.Value.type)
                    {
                        case SerializableProperty.FieldType.Vector2:
                            {
                                Vector2 value = new Vector2();

                                for (int i = 0; i < 2; i++)
                                    value[i] = kvp.Value.curves[i].Evaluate(time, false);

                                fieldValue.value = value;
                            }
                            break;
                        case SerializableProperty.FieldType.Vector3:
                            {
                                Vector3 value = new Vector3();

                                for (int i = 0; i < 3; i++)
                                    value[i] = kvp.Value.curves[i].Evaluate(time, false);

                                fieldValue.value = value;
                            }
                            break;
                        case SerializableProperty.FieldType.Vector4:
                            {
                                Vector4 value = new Vector4();

                                for (int i = 0; i < 4; i++)
                                    value[i] = kvp.Value.curves[i].Evaluate(time, false);

                                fieldValue.value = value;
                            }
                            break;
                        case SerializableProperty.FieldType.Color:
                            {
                                Color value = new Color();

                                for (int i = 0; i < 4; i++)
                                    value[i] = kvp.Value.curves[i].Evaluate(time, false);

                                fieldValue.value = value;
                            }
                            break;
                        case SerializableProperty.FieldType.Bool:
                        case SerializableProperty.FieldType.Int:
                        case SerializableProperty.FieldType.Float:
                            fieldValue.value = kvp.Value.curves[0].Evaluate(time, false); ;
                            break;
                    }

                    values.Add(fieldValue);
                }
            }

            guiFieldDisplay.SetDisplayValues(values.ToArray());
        }

        private Vector2 GetOptimalRange()
        {
            List<EdAnimationCurve> displayedCurves = new List<EdAnimationCurve>();
            for (int i = 0; i < selectedFields.Count; i++)
            {
                EdAnimationCurve curve;
                if (TryGetCurve(selectedFields[i], out curve))
                    displayedCurves.Add(curve);
            }

            float xRange;
            float yRange;
            CalculateRange(displayedCurves, out xRange, out yRange);

            // Add padding to y range
            yRange *= 1.05f;

            // Don't allow zero range
            if (xRange == 0.0f)
                xRange = 60.0f;

            if (yRange == 0.0f)
                yRange = 10.0f;

            return new Vector2(xRange, yRange);
        }

        private void UpdateDisplayedCurves()
        {
            List<EdAnimationCurve> curvesToDisplay = new List<EdAnimationCurve>();
            for (int i = 0; i < selectedFields.Count; i++)
            {
                EdAnimationCurve curve;
                if (TryGetCurve(selectedFields[i], out curve))
                    curvesToDisplay.Add(curve);
            }

            guiCurveEditor.SetCurves(curvesToDisplay.ToArray());

            Vector2 newRange = GetOptimalRange();

            // Don't reduce visible range
            newRange.x = Math.Max(newRange.x, guiCurveEditor.Range.x);
            newRange.y = Math.Max(newRange.y, guiCurveEditor.Range.y);

            guiCurveEditor.Range = newRange;
            UpdateScrollBarSize();
        }
        #endregion 

        #region Field display
        private List<string> selectedFields = new List<string>();

        private void AddNewField(string path, SerializableProperty.FieldType type)
        {
            guiFieldDisplay.AddField(path);

            switch (type)
            {
                case SerializableProperty.FieldType.Vector4:
                    {
                        FieldCurves fieldCurves = new FieldCurves();
                        fieldCurves.type = type;
                        fieldCurves.curves = new EdAnimationCurve[4];

                        string[] subPaths = { ".x", ".y", ".z", ".w" };
                        for (int i = 0; i < subPaths.Length; i++)
                        {
                            string subFieldPath = path + subPaths[i];
                            fieldCurves.curves[i] = new EdAnimationCurve();
                            selectedFields.Add(subFieldPath);
                        }

                        curves[path] = fieldCurves;
                    }
                    break;
                case SerializableProperty.FieldType.Vector3:
                    {
                        FieldCurves fieldCurves = new FieldCurves();
                        fieldCurves.type = type;
                        fieldCurves.curves = new EdAnimationCurve[3];

                        string[] subPaths = { ".x", ".y", ".z" };
                        for (int i = 0; i < subPaths.Length; i++)
                        {
                            string subFieldPath = path + subPaths[i];
                            fieldCurves.curves[i] = new EdAnimationCurve();
                            selectedFields.Add(subFieldPath);
                        }

                        curves[path] = fieldCurves;
                    }
                    break;
                case SerializableProperty.FieldType.Vector2:
                    {
                        FieldCurves fieldCurves = new FieldCurves();
                        fieldCurves.type = type;
                        fieldCurves.curves = new EdAnimationCurve[2];

                        string[] subPaths = { ".x", ".y" };
                        for (int i = 0; i < subPaths.Length; i++)
                        {
                            string subFieldPath = path + subPaths[i];
                            fieldCurves.curves[i] = new EdAnimationCurve();
                            selectedFields.Add(subFieldPath);
                        }

                        curves[path] = fieldCurves;
                    }
                    break;
                case SerializableProperty.FieldType.Color:
                    {
                        FieldCurves fieldCurves = new FieldCurves();
                        fieldCurves.type = type;
                        fieldCurves.curves = new EdAnimationCurve[4];

                        string[] subPaths = { ".r", ".g", ".b", ".a" };
                        for (int i = 0; i < subPaths.Length; i++)
                        {
                            string subFieldPath = path + subPaths[i];
                            fieldCurves.curves[i] = new EdAnimationCurve();
                            selectedFields.Add(subFieldPath);
                        }

                        curves[path] = fieldCurves;
                    }
                    break;
                default: // Primitive type
                    {
                        FieldCurves fieldCurves = new FieldCurves();
                        fieldCurves.type = type;
                        fieldCurves.curves = new EdAnimationCurve[1];

                        fieldCurves.curves[0] = new EdAnimationCurve();
                        selectedFields.Add(path);

                        curves[path] = fieldCurves;
                    }
                    break;
            }

            UpdateDisplayedCurves();
        }

        private void SelectField(string path, bool additive)
        {
            if (!additive)
                selectedFields.Clear();

            if (!string.IsNullOrEmpty(path))
            {
                selectedFields.RemoveAll(x => { return x == path || IsPathParent(x, path); });
                selectedFields.Add(path);
            }

            guiFieldDisplay.SetSelection(selectedFields.ToArray());

            UpdateDisplayedCurves();
        }

        private void RemoveSelectedFields()
        {
            for (int i = 0; i < selectedFields.Count; i++)
            {
                selectedFields.Remove(selectedFields[i]);
                curves.Remove(GetSubPathParent(selectedFields[i]));
            }

            List<string> existingFields = new List<string>();
            foreach (var KVP in curves)
                existingFields.Add(KVP.Key);

            guiFieldDisplay.SetFields(existingFields.ToArray());

            selectedFields.Clear();
            UpdateDisplayedCurves();
        }
        #endregion

        #region Helpers
        private Vector2I GetCurveEditorSize()
        {
            Vector2I output = new Vector2I();
            output.x = Math.Max(0, Width - FIELD_DISPLAY_WIDTH - scrollBarWidth);
            output.y = Math.Max(0, Height - buttonLayoutHeight - scrollBarHeight);

            return output;
        }

        private static void CalculateRange(List<EdAnimationCurve> curves, out float xRange, out float yRange)
        {
            xRange = 0.0f;
            yRange = 0.0f;

            foreach (var curve in curves)
            {
                KeyFrame[] keyframes = curve.KeyFrames;

                foreach (var key in keyframes)
                {
                    xRange = Math.Max(xRange, key.time);
                    yRange = Math.Max(yRange, Math.Abs(key.value));
                }
            }
        }

        private bool TryGetCurve(string path, out EdAnimationCurve curve)
        {
            int index = path.LastIndexOf(".");
            string parentPath;
            string subPathSuffix = null;
            if (index == -1)
            {
                parentPath = path;
            }
            else
            {
                parentPath = path.Substring(0, index);
                subPathSuffix = path.Substring(index, path.Length - index);
            }

            FieldCurves fieldCurves;
            if (curves.TryGetValue(parentPath, out fieldCurves))
            {
                if (!string.IsNullOrEmpty(subPathSuffix))
                {
                    if (subPathSuffix == ".x" || subPathSuffix == ".r")
                    {
                        curve = fieldCurves.curves[0];
                        return true;
                    }
                    else if (subPathSuffix == ".y" || subPathSuffix == ".g")
                    {
                        curve = fieldCurves.curves[1];
                        return true;
                    }
                    else if (subPathSuffix == ".z" || subPathSuffix == ".b")
                    {
                        curve = fieldCurves.curves[2];
                        return true;
                    }
                    else if (subPathSuffix == ".w" || subPathSuffix == ".a")
                    {
                        curve = fieldCurves.curves[3];
                        return true;
                    }
                }
                else
                {
                    curve = fieldCurves.curves[0];
                    return true;
                }
            }

            curve = null;
            return false;
        }

        private bool IsPathParent(string child, string parent)
        {
            string[] childEntries = child.Split('/', '.');
            string[] parentEntries = parent.Split('/', '.');

            if (parentEntries.Length >= child.Length)
                return false;

            int compareLength = Math.Min(childEntries.Length, parentEntries.Length);
            for (int i = 0; i < compareLength; i++)
            {
                if (childEntries[i] != parentEntries[i])
                    return false;
            }

            return true;
        }

        private string GetSubPathParent(string path)
        {
            int index = path.LastIndexOf(".");
            if (index == -1)
                return path;

            return path.Substring(0, index);
        }

        #endregion

        #region Input callbacks
        private void OnPointerPressed(PointerEvent ev)
        {
            if (!isInitialized)
                return;

            guiCurveEditor.OnPointerPressed(ev);

            if (ev.button == PointerButton.Middle)
            {
                Vector2I windowPos = ScreenToWindowPos(ev.ScreenPos);
                Vector2 curvePos;
                if (guiCurveEditor.WindowToCurveSpace(windowPos, out curvePos))
                {
                    dragStartPos = windowPos;
                    isButtonHeld = true;
                }
            }
        }

        private void OnPointerMoved(PointerEvent ev)
        {
            if (!isInitialized)
                return;

            guiCurveEditor.OnPointerMoved(ev);

            if (isButtonHeld)
            {
                Vector2I windowPos = ScreenToWindowPos(ev.ScreenPos);

                int distance = Vector2I.Distance(dragStartPos, windowPos);
                if (distance >= DRAG_START_DISTANCE)
                {
                    isDragInProgress = true;

                    Cursor.Hide();

                    Rect2I clipRect;
                    clipRect.x = ev.ScreenPos.x - 2;
                    clipRect.y = ev.ScreenPos.y - 2;
                    clipRect.width = 4;
                    clipRect.height = 4;

                    Cursor.ClipToRect(clipRect);
                }
            }
        }

        private void OnPointerReleased(PointerEvent ev)
        {
            if (isDragInProgress)
            {
                Cursor.Show();
                Cursor.ClipDisable();
            }

            isButtonHeld = false;
            isDragInProgress = false;

            if (!isInitialized)
                return;

            guiCurveEditor.OnPointerReleased(ev);
        }

        private void OnButtonUp(ButtonEvent ev)
        {
            if (!isInitialized)
                return;

            guiCurveEditor.OnButtonUp(ev);
        }
        #endregion

        #region General callbacks
        private void OnFieldAdded(string path, SerializableProperty.FieldType type)
        {
            AddNewField(path, type);
        }

        private void OnHorzScrollOrResize(float position, float size)
        {
            SetHorzScrollbarProperties(position, size);
        }

        private void OnVertScrollOrResize(float position, float size)
        {
            SetVertScrollbarProperties(position, size);
        }

        private void OnFieldSelected(string path)
        {
            bool additive = Input.IsButtonHeld(ButtonCode.LeftShift) || Input.IsButtonHeld(ButtonCode.RightShift);
            SelectField(path, additive);
        }

        private void OnSelectionChanged(SceneObject[] sceneObjects, string[] resourcePaths)
        {
            RebuildGUI();
        }

        private void OnFrameSelected(int frameIdx)
        {
            SetCurrentFrame(frameIdx);
        }
        #endregion
    }

    /// <summary>
    /// Drop down window that displays options used by the animation window.
    /// </summary>
    [DefaultSize(100, 50)]
    internal class AnimationOptions : DropDownWindow
    {
        /// <summary>
        /// Initializes the drop down window by creating the necessary GUI. Must be called after construction and before
        /// use.
        /// </summary>
        /// <param name="parent">Animation window that this drop down window is a part of.</param>
        internal void Initialize(AnimationWindow parent)
        {
            GUIIntField fpsField = new GUIIntField(new LocEdString("FPS"), 40);
            fpsField.Value = parent.FPS;
            fpsField.OnChanged += x => { parent.FPS = x; };
            
            GUILayoutY vertLayout = GUI.AddLayoutY();

            vertLayout.AddFlexibleSpace();
            GUILayoutX contentLayout = vertLayout.AddLayoutX();
            contentLayout.AddFlexibleSpace();
            contentLayout.AddElement(fpsField);
            contentLayout.AddFlexibleSpace();
            vertLayout.AddFlexibleSpace();
        }
    }

    /** @} */
}
