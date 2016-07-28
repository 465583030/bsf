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
        private List<string> paths = new List<string>();

        private GUIAnimFieldEntry[] fields;
        private GUIAnimFieldLayouts layouts;

        public GUIAnimFieldDisplay(GUILayout layout, int width, int height, SceneObject root)
        {
            this.root = root;

            scrollArea = new GUIScrollArea();
            layout.AddElement(scrollArea);

            SetSize(width, height);
        }

        public Action<string, bool> OnSelectionChanged;

        public void SetSize(int width, int height)
        {
            this.width = width;
            this.height = height;

            scrollArea.SetWidth(width);
            scrollArea.SetHeight(height);

            Rebuild();
        }

        public void SetFields(string[] paths)
        {
            this.paths.Clear();
            this.paths.AddRange(paths);

            Rebuild();
        }

        public void AddField(string path)
        {
            if (!paths.Contains(path))
            {
                paths.Add(path);
                Rebuild();
            }
        }

        public void SetDisplayValues(GUIAnimFieldPathValue[] values)
        {
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

        private SerializableProperty FindProperty(string path)
        {
            if (string.IsNullOrEmpty(path) || root == null)
                return null;

            string[] entries = path.Split('/');

            // Find scene object referenced by the path
            SceneObject so = null;
            int pathIdx = 0;
            for (; pathIdx < entries.Length; pathIdx++)
            {
                string entry = entries[pathIdx];

                if (string.IsNullOrEmpty(entry))
                    continue;

                // Not a scene object, break
                if (entry[0] != '!')
                    break;

                if (so == null)
                    so = root;
                else
                {
                    string childName = entry.Substring(1, entry.Length - 1);
                    so = so.FindChild(childName);

                    if (so == null)
                        break;
                }
            }

            // Cannot find scene object with specified hierarchy & name
            if (so == null)
                return null;

            if (pathIdx >= entries.Length)
                return null;

            // If path is referencing a component, find it
            Component component = null;
            {
                string entry = entries[pathIdx];
                if (entry[0] == ':')
                {
                    string componentName = entry.Substring(1, entry.Length - 1);

                    Component[] components = so.GetComponents();
                    component = Array.Find(components, x => x.GetType().Name == componentName);

                    // Cannot find component with specified type
                    if (component == null)
                        return null;
                }
            }

            // Look for a field within a component
            if (component != null)
            {
                pathIdx++;
                if (pathIdx >= entries.Length)
                    return null;

                SerializableObject componentObj = new SerializableObject(component);

                StringBuilder pathBuilder = new StringBuilder();
                for(; pathIdx < entries.Length; pathIdx++)
                    pathBuilder.Append(entries[pathIdx] + "/");

                return componentObj.FindProperty(pathBuilder.ToString());
            }
            else // Field is one of the builtin ones on the SceneObject itself
            {
                if ((pathIdx + 1) < entries.Length)
                    return null;

                string entry = entries[pathIdx];
                if (entry == "Position")
                {
                    SerializableProperty property = new SerializableProperty(
                        SerializableProperty.FieldType.Vector3,
                        typeof(Vector3),
                        () => so.LocalPosition, 
                        (x) => so.LocalPosition = (Vector3)x);

                    return property;
                }
                else if (entry == "Rotation")
                {
                    SerializableProperty property = new SerializableProperty(
                        SerializableProperty.FieldType.Vector3,
                        typeof(Vector3),
                        () => so.LocalRotation.ToEuler(), 
                        (x) => so.LocalRotation = Quaternion.FromEuler((Vector3)x));

                    return property;
                }
                else if (entry == "Scale")
                {
                    SerializableProperty property = new SerializableProperty(
                        SerializableProperty.FieldType.Vector3,
                        typeof(Vector3),
                        () => so.LocalScale, 
                        (x) => so.LocalScale = (Vector3)x);

                    return property;
                }

                return null;
            }
        }

        private void Rebuild()
        {
            scrollArea.Layout.Clear();
            fields = null;

            if (paths == null || root == null)
                return;

            layouts = new GUIAnimFieldLayouts();
            GUIPanel rootPanel = scrollArea.Layout.AddPanel();
            GUIPanel mainPanel = rootPanel.AddPanel();
            GUIPanel underlayPanel = rootPanel.AddPanel(1);
            GUIPanel overlayPanel = rootPanel.AddPanel(-1);

            layouts.main = mainPanel.AddLayoutY();
            layouts.underlay = underlayPanel.AddLayoutY();
            layouts.overlay = overlayPanel.AddLayoutY();

            fields = new GUIAnimFieldEntry[paths.Count];
            for (int i = 0; i < paths.Count; i++)
            {
                SerializableProperty property = FindProperty(paths[i]);
                if (property != null)
                {
                    switch (property.Type)
                    {
                        case SerializableProperty.FieldType.Vector2:
                            fields[i] = new GUIAnimVec2Entry(layouts, paths[i]);
                            break;
                        case SerializableProperty.FieldType.Vector3:
                            fields[i] = new GUIAnimVec3Entry(layouts, paths[i]);
                            break;
                        case SerializableProperty.FieldType.Vector4:
                            fields[i] = new GUIAnimVec4Entry(layouts, paths[i]);
                            break;
                        case SerializableProperty.FieldType.Color:
                            fields[i] = new GUIAnimColorEntry(layouts, paths[i]);
                            break;
                        case SerializableProperty.FieldType.Bool:
                        case SerializableProperty.FieldType.Int:
                        case SerializableProperty.FieldType.Float:
                            fields[i] = new GUIAnimSimpleEntry(layouts, paths[i]);
                            break;
                    }

                    if (fields[i] != null)
                        fields[i].OnSelectionChanged += OnSelectionChanged;
                }
                else
                {
                    fields[i] = new GUIAnimMissingEntry(layouts, paths[i]);
                }
            }

            layouts.main.AddFlexibleSpace();
            layouts.underlay.AddFlexibleSpace();
            layouts.overlay.AddFlexibleSpace();
        }
    }

    internal class GUIAnimFieldLayouts
    {
        public GUILayout main;
        public GUILayout underlay;
        public GUILayout overlay;
    }

    internal struct GUIAnimFieldPathValue
    {
        public string path;
        public object value;
    }

    internal abstract class GUIAnimFieldEntry
    {
        private const int MAX_PATH_LENGTH = 20;
        protected const int INDENT_AMOUNT = 10;

        protected string path;
        private GUIToggle toggle;

        private int entryHeight;

        public Action<string, bool> OnSelectionChanged;

        public string Path { get { return path; } }

        public GUIAnimFieldEntry(GUIAnimFieldLayouts layouts, string path)
        {
            this.path = path;

            GUILayoutX toggleLayout = layouts.main.AddLayoutX();
            toggleLayout.AddSpace(15);

            toggle = new GUIToggle(GetDisplayName(path), EditorStyles.SelectableLabel, GUIOption.FlexibleWidth());
            toggle.OnToggled += x => { OnSelectionChanged?.Invoke(path, x); };

            toggleLayout.AddElement(toggle);

            entryHeight = toggle.Bounds.height;
        }

        public virtual void Toggle(bool on)
        {
            toggle.Active = on;
        }

        public virtual void SetValue(object value) { }

        protected int GetEntryHeight()
        {
            return entryHeight;
        }

        protected static string GetDisplayName(string path)
        {
            if (string.IsNullOrEmpty(path))
                return "";

            string soName;
            string compName;
            string propertyPath;

            GetNames(path, out soName, out compName, out propertyPath);

            if (soName == null || propertyPath == null)
                return "";

            string truncatedPropPath;
            if (propertyPath.Length > MAX_PATH_LENGTH)
                truncatedPropPath = "..." + propertyPath.Substring(propertyPath.Length - MAX_PATH_LENGTH);
            else
                truncatedPropPath = propertyPath;

            if (compName != null)
                return soName + "(" + compName + ")." + truncatedPropPath;
            else
                return soName + "." + truncatedPropPath;
        }

        protected static void GetNames(string path, out string soName, out string compName, out string propertyPath)
        {
            string[] entries = path.Split('/');

            // Find name of the last scene object in the path
            int pathIdx = 0;
            for (; pathIdx < entries.Length; pathIdx++)
            {
                string entry = entries[pathIdx];

                if (string.IsNullOrEmpty(entry))
                    continue;

                // Not a scene object, break
                if (entry[0] != '!')
                    break;
            }

            if (pathIdx == 0)
            {
                soName = null;
                compName = null;
                propertyPath = null;

                return;
            }

            soName = entries[pathIdx - 1].Substring(1, entries[pathIdx - 1].Length - 1);

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

            StringBuilder pathBuilder = new StringBuilder();
            for (; pathIdx < entries.Length; pathIdx++)
                pathBuilder.Append(entries[pathIdx] + "/");

            propertyPath = pathBuilder.ToString();
        }
    }

    internal class GUIAnimSimpleEntry : GUIAnimFieldEntry
    {
        private GUILabel valueDisplay;
        private GUILayoutX underlayLayout;
        private GUILabel overlaySpacing;

        public GUIAnimSimpleEntry(GUIAnimFieldLayouts layouts, string path)
            : base(layouts, path)
        {
            valueDisplay = new GUILabel("", GUIOption.FixedHeight(GetEntryHeight()));
            underlayLayout = layouts.underlay.AddLayoutX();
            underlayLayout.AddFlexibleSpace();
            underlayLayout.AddElement(valueDisplay);
            underlayLayout.AddSpace(50);

            overlaySpacing = new GUILabel("", GUIOption.FixedHeight(GetEntryHeight()));
            layouts.overlay.AddElement(overlaySpacing);

            // TODO - Alternating backgrounds
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
            : base(layouts, path)
        {
            foldout = new GUIToggle("", EditorStyles.Expand);
            foldout.OnToggled += Toggle;
            foldoutLayout = layouts.overlay.AddLayoutX();

            foldoutLayout.AddElement(foldout);
            foldoutLayout.AddFlexibleSpace();

            underlaySpacing = new GUILabel("", GUIOption.FixedHeight(GetEntryHeight()));
            layouts.underlay.AddElement(underlaySpacing);

            children = new GUIAnimSimpleEntry[childEntries.Length];
            for (int i = 0; i < childEntries.Length; i++)
                children[i] = new GUIAnimSimpleEntry(layouts, path + childEntries[i]);

            Toggle(false);
        }

        public override void Toggle(bool on)
        {
            foreach(var child in children)
                child.Toggle(on);
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
            : base(layouts, path)
        {
            missingLabel = new GUILabel("Missing property!", GUIOption.FixedHeight(GetEntryHeight()));
            underlayLayout = layouts.underlay.AddLayoutX();
            underlayLayout.AddFlexibleSpace();
            underlayLayout.AddElement(missingLabel);
            underlayLayout.AddSpace(50);

            overlaySpacing = new GUILabel("", GUIOption.FixedHeight(GetEntryHeight()));
            layouts.overlay.AddElement(overlaySpacing);

            // TODO - Alternating backgrounds
        }

        public override void Toggle(bool on)
        {
            underlayLayout.Active = on;
            overlaySpacing.Active = on;

            base.Toggle(on);
        }
    }

    /** @} */
}