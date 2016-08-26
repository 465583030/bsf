﻿//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using System;
using System.Collections.Generic;
using System.Text;
using BansheeEngine;

namespace BansheeEditor
{
    /** @addtogroup AnimationEditor
     *  @{
     */

    internal class GUIAnimFieldDisplay
    {
        private SceneObject root;
        private int width;
        private int height;

        private GUIScrollArea scrollArea;
        private List<AnimFieldInfo> fieldInfos = new List<AnimFieldInfo>();

        private GUIAnimFieldEntry[] fields;
        private GUIAnimFieldLayouts layouts;

        public GUIAnimFieldDisplay(GUILayout layout, int width, int height, SceneObject root)
        {
            this.root = root;

            scrollArea = new GUIScrollArea(ScrollBarType.ShowIfDoesntFit, ScrollBarType.NeverShow);
            layout.AddElement(scrollArea);

            SetSize(width, height);
        }

        public Action<string> OnEntrySelected;

        public void SetSize(int width, int height)
        {
            this.width = width;
            this.height = height;

            scrollArea.SetWidth(width);
            scrollArea.SetHeight(height);

            Rebuild();
        }

        public void SetFields(AnimFieldInfo[] fields)
        {
            this.fieldInfos.Clear();
            this.fieldInfos.AddRange(fields);

            Rebuild();
        }

        public void AddField(AnimFieldInfo field)
        {
            bool exists = fieldInfos.Exists(x =>
            {
                return x.path == field.path;
            });

            if (!exists)
            {
                fieldInfos.Add(field);
                Rebuild();
            }
        }

        public void SetDisplayValues(GUIAnimFieldPathValue[] values)
        {
            for(int i = 0; i < values.Length; i++)
                Debug.Log(i + ". " + values[i].value);

            for (int i = 0; i < fields.Length; i++)
            {
                string path = fields[i].Path;

                for (int j = 0; j < values.Length; j++)
                {
                    if(path == values[j].path)
                        fields[i].SetValue(values[j].value);
                }
            }
        }

        public void SetSelection(string[] paths)
        {
            Action<GUIAnimFieldEntry> updateSelection = field =>
            {
                bool foundSelected = false;
                for (int j = 0; j < paths.Length; j++)
                {
                    if (field.Path == paths[j])
                    {
                        field.SetSelection(true);
                        foundSelected = true;
                        break;
                    }
                }

                if (!foundSelected)
                    field.SetSelection(false);
            };

            for (int i = 0; i < fields.Length; i++)
            {
                updateSelection(fields[i]);

                // Check children (only one level allowed)
                GUIAnimFieldEntry[] children = fields[i].GetChildren();
                if (children == null)
                    continue;

                for (int j = 0; j < children.Length; j++)
                    updateSelection(children[j]);
            }
        }

        private void Rebuild()
        {
            scrollArea.Layout.Clear();
            fields = null;

            if (fieldInfos == null || root == null)
                return;

            layouts = new GUIAnimFieldLayouts();
            GUIPanel rootPanel = scrollArea.Layout.AddPanel();
            GUIPanel mainPanel = rootPanel.AddPanel();
            GUIPanel underlayPanel = rootPanel.AddPanel(1);
            GUIPanel overlayPanel = rootPanel.AddPanel(-1);
            GUIPanel backgroundPanel = rootPanel.AddPanel(2);

            layouts.main = mainPanel.AddLayoutY();
            layouts.underlay = underlayPanel.AddLayoutY();
            layouts.overlay = overlayPanel.AddLayoutY();
            layouts.background = backgroundPanel.AddLayoutY();

            GUIButton catchAll = new GUIButton("", EditorStyles.Blank);
            catchAll.Bounds = new Rect2I(0, 0, width, height);
            catchAll.OnClick += () => OnEntrySelected(null);

            underlayPanel.AddElement(catchAll);

            layouts.main.AddSpace(5);
            layouts.underlay.AddSpace(5);
            layouts.overlay.AddSpace(5);
            layouts.background.AddSpace(3); // Minor hack: Background starts heigher to get it to center better

            fields = new GUIAnimFieldEntry[fieldInfos.Count];
            for (int i = 0; i < fieldInfos.Count; i++)
            {
                if (string.IsNullOrEmpty(fieldInfos[i].path))
                    continue;

                bool entryIsMissing;
                if (fieldInfos[i].isUserCurve)
                {
                    string pathSuffix;
                    SerializableProperty property = Animation.FindProperty(root, fieldInfos[i].path, out pathSuffix);
                    entryIsMissing = property == null;
                }
                else
                    entryIsMissing = false;

                if (!entryIsMissing)
                {
                    switch (fieldInfos[i].type)
                    {
                        case SerializableProperty.FieldType.Vector2:
                            fields[i] = new GUIAnimVec2Entry(layouts, fieldInfos[i].path);
                            break;
                        case SerializableProperty.FieldType.Vector3:
                            fields[i] = new GUIAnimVec3Entry(layouts, fieldInfos[i].path);
                            break;
                        case SerializableProperty.FieldType.Vector4:
                            fields[i] = new GUIAnimVec4Entry(layouts, fieldInfos[i].path);
                            break;
                        case SerializableProperty.FieldType.Color:
                            fields[i] = new GUIAnimColorEntry(layouts, fieldInfos[i].path);
                            break;
                        case SerializableProperty.FieldType.Bool:
                        case SerializableProperty.FieldType.Int:
                        case SerializableProperty.FieldType.Float:
                            fields[i] = new GUIAnimSimpleEntry(layouts, fieldInfos[i].path);
                            break;
                    }
                }
                else
                {
                    fields[i] = new GUIAnimMissingEntry(layouts, fieldInfos[i].path);
                }

                if (fields[i] != null)
                    fields[i].OnEntrySelected += OnEntrySelected;
            }

            layouts.main.AddSpace(5);
            layouts.underlay.AddSpace(5);
            layouts.overlay.AddSpace(5);
            layouts.background.AddSpace(5);

            layouts.main.AddFlexibleSpace();
            layouts.underlay.AddFlexibleSpace();
            layouts.overlay.AddFlexibleSpace();
            layouts.background.AddFlexibleSpace();
        }
    }

    internal class GUIAnimFieldLayouts
    {
        public GUILayout main;
        public GUILayout underlay;
        public GUILayout overlay;
        public GUILayout background;
    }

    internal struct GUIAnimFieldPathValue
    {
        public string path;
        public object value;
    }

    internal abstract class GUIAnimFieldEntry
    {
        private const int MAX_PATH_LENGTH = 30;
        protected const int INDENT_AMOUNT = 10;

        protected string path;
        private GUIButton selectionBtn;
        private GUITexture backgroundTexture;

        private int entryHeight;

        public Action<string> OnEntrySelected;

        public string Path { get { return path; } }

        public GUIAnimFieldEntry(GUIAnimFieldLayouts layouts, string path, bool child)
        {
            this.path = path;

            GUILayoutX toggleLayout = layouts.main.AddLayoutX();
            toggleLayout.AddSpace(child ? 30 : 15);

            selectionBtn = new GUIButton(GetDisplayName(path, child), EditorStyles.Label, GUIOption.FlexibleWidth());
            selectionBtn.OnClick += () =>
            {
                OnEntrySelected?.Invoke(path);
            };

            toggleLayout.AddElement(selectionBtn);

            entryHeight = selectionBtn.Bounds.height;

            backgroundTexture = new GUITexture(Builtin.WhiteTexture, GUITextureScaleMode.StretchToFit, 
                GUIOption.FlexibleWidth());
            backgroundTexture.SetTint(Color.Transparent);
            backgroundTexture.SetHeight(entryHeight);

            layouts.background.AddElement(backgroundTexture);
        }

        public virtual void Toggle(bool on)
        {
            selectionBtn.Active = on;
            backgroundTexture.Active = on;
        }

        public void SetSelection(bool selected)
        {
            if(selected)
                backgroundTexture.SetTint(Color.DarkCyan);
            else
                backgroundTexture.SetTint(Color.Transparent);
        }

        public virtual void SetValue(object value) { }

        public virtual GUIAnimFieldEntry[] GetChildren()
        {
            return null;
        }

        protected int GetEntryHeight()
        {
            return entryHeight;
        }

        protected static string GetDisplayName(string path, bool shortName)
        {
            if (string.IsNullOrEmpty(path))
                return "";

            string soName;
            string compName;
            string propertyPath;

            string trimmedPath = path.Trim('/');
            GetNames(trimmedPath, shortName, out soName, out compName, out propertyPath);

            if (propertyPath == null)
                return "";

            if (shortName)
                return propertyPath;
            else
            {
                string truncatedPropPath;
                if (propertyPath.Length > MAX_PATH_LENGTH)
                    truncatedPropPath = "..." + propertyPath.Substring(propertyPath.Length - MAX_PATH_LENGTH);
                else
                    truncatedPropPath = propertyPath;

                if (soName != null)
                {
                    if (compName != null)
                        return soName + "(" + compName + ") - " + truncatedPropPath;
                    else
                        return soName + " - " + truncatedPropPath;
                }
                else
                {
                    if (compName != null)
                        return "(" + compName + ") - " + truncatedPropPath;
                    else
                        return truncatedPropPath;
                }
            }
        }

        protected static void GetNames(string path, bool shortName, out string soName, out string compName, out string propertyPath)
        {
            string[] entries = path.Split('/');

            // Find name of the last scene object in the path
            int pathIdx = 0;
            soName = null;
            for (; pathIdx < entries.Length; pathIdx++)
            {
                string entry = entries[pathIdx];

                if (string.IsNullOrEmpty(entry))
                    continue;

                // Not a scene object, break
                if (entry[0] != '!')
                {
                    if (pathIdx > 0)
                    {
                        string prevEntry = entries[pathIdx - 1];
                        soName = prevEntry.Substring(1, prevEntry.Length - 1);
                    }

                    break;
                }
            }

            if (pathIdx >= entries.Length)
            {
                compName = null;
                propertyPath = null;
                return;
            }

            // If path is referencing a component, find it
            {
                string entry = entries[pathIdx];
                if (entry[0] == ':')
                    compName = entry.Substring(1, entry.Length - 1);
                else
                    compName = null;
            }

            // Look for a field name
            if (compName != null)
            {
                pathIdx++;
                if (pathIdx >= entries.Length)
                {
                    propertyPath = null;
                    return;
                }
            }

            if (shortName)
            {
                if (pathIdx < entries.Length)
                    propertyPath = entries[entries.Length - 1];
                else
                    propertyPath = null;
            }
            else
            {
                StringBuilder pathBuilder = new StringBuilder();
                for (; pathIdx < entries.Length - 1; pathIdx++)
                    pathBuilder.Append(entries[pathIdx] + "/");

                if (pathIdx < entries.Length)
                    pathBuilder.Append(entries[pathIdx]);

                propertyPath = pathBuilder.ToString();
            }
        }
    }

    internal class GUIAnimSimpleEntry : GUIAnimFieldEntry
    {
        private GUILabel valueDisplay;
        private GUILayoutX underlayLayout;
        private GUILabel overlaySpacing;

        public GUIAnimSimpleEntry(GUIAnimFieldLayouts layouts, string path, bool child = false)
            : base(layouts, path, child)
        {
            valueDisplay = new GUILabel("", GUIOption.FixedHeight(GetEntryHeight()));
            underlayLayout = layouts.underlay.AddLayoutX();
            underlayLayout.AddFlexibleSpace();
            underlayLayout.AddElement(valueDisplay);
            underlayLayout.AddSpace(50);

            overlaySpacing = new GUILabel("", GUIOption.FixedHeight(GetEntryHeight()));
            layouts.overlay.AddElement(overlaySpacing);
        }

        public override void Toggle(bool on)
        {
            underlayLayout.Active = on;
            overlaySpacing.Active = on;

            base.Toggle(on);
        }

        public override void SetValue(object value)
        {
            if (value == null)
                return;

            string strValue = value.ToString();
            valueDisplay.SetContent(strValue);
        }
    }

    internal class GUIAnimComplexEntry : GUIAnimFieldEntry
    {
        private GUILayout foldoutLayout;
        private GUIToggle foldout;
        private GUILabel underlaySpacing;

        protected GUIAnimSimpleEntry[] children;

        public GUIAnimComplexEntry(GUIAnimFieldLayouts layouts, string path, string[] childEntries)
            : base(layouts, path, false)
        {
            foldout = new GUIToggle("", EditorStyles.Expand);
            foldout.OnToggled += Toggle;

            GUILabel spacer = new GUILabel("", GUIOption.FixedHeight(GetEntryHeight()));

            foldoutLayout = layouts.overlay.AddLayoutX();

            foldoutLayout.AddElement(foldout);
            foldoutLayout.AddElement(spacer);
            foldoutLayout.AddFlexibleSpace();

            underlaySpacing = new GUILabel("", GUIOption.FixedHeight(GetEntryHeight()));
            layouts.underlay.AddElement(underlaySpacing);

            children = new GUIAnimSimpleEntry[childEntries.Length];
            for (int i = 0; i < childEntries.Length; i++)
            {
                children[i] = new GUIAnimSimpleEntry(layouts, path + childEntries[i], true);
                children[i].OnEntrySelected += x => { OnEntrySelected?.Invoke(x); };
            }

            Toggle(false);
        }

        public override void Toggle(bool on)
        {
            foreach(var child in children)
                child.Toggle(on);
        }

        public override GUIAnimFieldEntry[] GetChildren()
        {
            return children;
        }
    }

    internal class GUIAnimVec2Entry : GUIAnimComplexEntry
    {
        public GUIAnimVec2Entry(GUIAnimFieldLayouts layouts, string path)
            : base(layouts, path,  new[] { ".x", ".y" })
        { }

        public override void SetValue(object value)
        {
            if (value == null)
                return;

            Vector2 vector = (Vector2)value;
            children[0].SetValue(vector.x);
            children[1].SetValue(vector.y);
        }
    }

    internal class GUIAnimVec3Entry : GUIAnimComplexEntry
    {
        public GUIAnimVec3Entry(GUIAnimFieldLayouts layouts, string path)
            : base(layouts, path, new[] { ".x", ".y", ".z" })
        { }

        public override void SetValue(object value)
        {
            if (value == null)
                return;

            Vector3 vector = (Vector3)value;
            children[0].SetValue(vector.x);
            children[1].SetValue(vector.y);
            children[2].SetValue(vector.z);
        }
    }

    internal class GUIAnimVec4Entry : GUIAnimComplexEntry
    {
        public GUIAnimVec4Entry(GUIAnimFieldLayouts layouts, string path)
            : base(layouts, path,  new[] { ".x", ".y", ".z", ".w" })
        { }

        public override void SetValue(object value)
        {
            if (value == null)
                return;

            Vector4 vector = (Vector4)value;
            children[0].SetValue(vector.x);
            children[1].SetValue(vector.y);
            children[2].SetValue(vector.z);
            children[3].SetValue(vector.w);
        }
    }

    internal class GUIAnimColorEntry : GUIAnimComplexEntry
    {
        public GUIAnimColorEntry(GUIAnimFieldLayouts layouts, string path)
            : base(layouts, path, new[] { ".r", ".g", ".b", ".a" })
        { }

        public override void SetValue(object value)
        {
            if (value == null)
                return;

            Color color = (Color)value;
            children[0].SetValue(color.r);
            children[1].SetValue(color.g);
            children[2].SetValue(color.b);
            children[3].SetValue(color.a);
        }
    }

    internal class GUIAnimMissingEntry : GUIAnimFieldEntry
    {
        private GUILabel missingLabel;
        private GUILayoutX underlayLayout;
        private GUILabel overlaySpacing;

        public GUIAnimMissingEntry(GUIAnimFieldLayouts layouts, string path)
            : base(layouts, path, false)
        {
            missingLabel = new GUILabel("Missing property!", GUIOption.FixedHeight(GetEntryHeight()));
            underlayLayout = layouts.underlay.AddLayoutX();
            underlayLayout.AddFlexibleSpace();
            underlayLayout.AddElement(missingLabel);
            underlayLayout.AddSpace(50);

            overlaySpacing = new GUILabel("", GUIOption.FixedHeight(GetEntryHeight()));
            layouts.overlay.AddElement(overlaySpacing);
        }

        public override void Toggle(bool on)
        {
            underlayLayout.Active = on;
            overlaySpacing.Active = on;

            base.Toggle(on);
        }
    }

    internal struct AnimFieldInfo
    {
        public AnimFieldInfo(string path, SerializableProperty.FieldType type, bool isUserCurve)
        {
            this.path = path;
            this.type = type;
            this.isUserCurve = isUserCurve;
        }

        public string path;
        public SerializableProperty.FieldType type;
        public bool isUserCurve;
    }

    /** @} */
}