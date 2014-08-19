﻿using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using BansheeEngine;

namespace BansheeEditor
{
    public class InspectableArray : InspectableObjectBase
    {
        private class EntryRow
        {
            public GUILayoutX rowLayout;
            public GUILayoutY contentLayout;
            public GUIButton cloneBtn;
            public GUIButton deleteBtn;
            public GUIButton moveUpBtn;
            public GUIButton moveDownBtn;

            public EntryRow(GUILayout parentLayout, int seqIndex, InspectableArray parent)
            {
                rowLayout = parentLayout.AddLayoutX();
                contentLayout = rowLayout.AddLayoutY();
                cloneBtn = new GUIButton("C");
                deleteBtn = new GUIButton("X");
                moveUpBtn = new GUIButton("Up");
                moveDownBtn = new GUIButton("Down");

                cloneBtn.OnClick += () => parent.OnCloneButtonClicked(seqIndex);
                deleteBtn.OnClick += () => parent.OnDeleteButtonClicked(seqIndex);
                moveUpBtn.OnClick += () => parent.OnMoveUpButtonClicked(seqIndex);
                moveDownBtn.OnClick += () => parent.OnMoveDownButtonClicked(seqIndex);

                rowLayout.AddElement(cloneBtn);
                rowLayout.AddElement(deleteBtn);
                rowLayout.AddElement(moveUpBtn);
                rowLayout.AddElement(moveDownBtn);
            }

            public void Destroy()
            {
                rowLayout.Destroy();
                contentLayout.Destroy();
                cloneBtn.Destroy();
                deleteBtn.Destroy();
                moveUpBtn.Destroy();
                moveDownBtn.Destroy();
            }
        }

        private const int IndentAmount = 15;

        private object propertyValue; // TODO - This will unnecessarily hold references to the object
        private int numArrayElements;

        private List<EntryRow> rows = new List<EntryRow>();
        private GUIIntField guiSizeField;
        private GUILayoutX guiChildLayout;
        private bool isExpanded;

        private bool forceUpdate = true;

        public InspectableArray(string title, InspectableFieldLayout layout, SerializableProperty property)
            : base(title, layout, property)
        {

        }

        protected override bool IsModified()
        {
            if (forceUpdate)
                return true;

            object newPropertyValue = property.GetValue<object>();
            if (propertyValue != newPropertyValue)
                return true;

            if (newPropertyValue != null)
            {
                SerializableArray array = property.GetArray();
                if (array.GetLength() != numArrayElements)
                    return true;
            }

            return base.IsModified();
        }

        protected override void Update(int layoutIndex)
        {
            base.Update(layoutIndex);
            forceUpdate = false;

            if (property.Type != SerializableProperty.FieldType.Array || property.InternalType.GetArrayRank() != 1) // We don't support multirank arrays
                return;

            foreach (var row in rows)
                row.Destroy();

            rows.Clear();

            layout.DestroyElements();

            propertyValue = property.GetValue<object>();
            if (propertyValue == null)
            {
                guiChildLayout = null;
                GUILayoutX guiTitleLayout = layout.AddLayoutX(layoutIndex);

                guiTitleLayout.AddElement(new GUILabel(title));
                guiTitleLayout.AddElement(new GUILabel("Empty"));

                if (!property.IsValueType)
                {
                    GUIButton createBtn = new GUIButton("Create");
                    createBtn.OnClick += OnCreateButtonClicked;
                    guiTitleLayout.AddElement(createBtn);
                }

                numArrayElements = 0;
            }
            else
            {
                GUIFoldout guiFoldout = new GUIFoldout(title);
                guiFoldout.SetExpanded(isExpanded);
                guiFoldout.OnToggled += OnFoldoutToggled;
                guiSizeField = new GUIIntField();
                guiSizeField.SetRange(0, int.MaxValue);
                GUIButton guiResizeBtn = new GUIButton("Resize");
                guiResizeBtn.OnClick += OnResizeButtonClicked;
                GUIButton guiClearBtn = new GUIButton("Clear");
                guiClearBtn.OnClick += OnClearButtonClicked;

                GUILayoutX guiTitleLayout = layout.AddLayoutX(layoutIndex);
                guiTitleLayout.AddElement(guiFoldout);
                guiTitleLayout.AddElement(guiSizeField);
                guiTitleLayout.AddElement(guiResizeBtn);
                guiTitleLayout.AddElement(guiClearBtn);

                SerializableArray array = property.GetArray();
                numArrayElements = array.GetLength();
                guiSizeField.Value = numArrayElements;

                if (isExpanded)
                {
                    guiChildLayout = layout.AddLayoutX(layoutIndex);
                    guiChildLayout.AddSpace(IndentAmount);

                    GUILayoutY guiContentLayout = guiChildLayout.AddLayoutY();

                    for (int i = 0; i < numArrayElements; i++)
                    {
                        EntryRow newRow = new EntryRow(guiContentLayout, i, this);
                        rows.Add(newRow);

                        InspectableObjectBase childObj = CreateDefaultInspectable(i + ".", new InspectableFieldLayout(newRow.contentLayout), array.GetProperty(i));
                        AddChild(childObj);

                        childObj.Refresh(0);
                    }
                }
                else
                    guiChildLayout = null;
            }
        }

        private void OnFoldoutToggled(bool expanded)
        {
            isExpanded = expanded;
            forceUpdate = true;
        }

        private void OnResizeButtonClicked()
        {
            int size = guiSizeField.Value; // TODO - Support multi-rank arrays

            Array newArray = property.CreateArrayInstance(new int[] {size});
            Array array = property.GetValue<Array>();

            int maxSize = MathEx.Min(size, array.Length);

            for (int i = 0; i < maxSize; i++)
                newArray.SetValue(array.GetValue(i), i);

            property.SetValue(newArray);
        }

        private void OnDeleteButtonClicked(int index)
        {
            Array array = property.GetValue<Array>();

            int size = MathEx.Max(0, array.Length - 1);
            Array newArray = property.CreateArrayInstance(new int[] { size });

            int destIdx = 0;
            for (int i = 0; i < array.Length; i++)
            {
                if (i == index)
                    continue;

                newArray.SetValue(array.GetValue(i), destIdx);
                destIdx++;
            }

            property.SetValue(newArray);
        }

        private void OnCloneButtonClicked(int index)
        {
            SerializableArray array = property.GetArray();

            int size = array.GetLength() + 1;
            Array newArray = property.CreateArrayInstance(new int[] { size });

            object clonedEntry = null;
            for (int i = 0; i < array.GetLength(); i++)
            {
                object value = array.GetProperty(i).GetValue<object>();

                newArray.SetValue(value, i);

                if (i == index)
                {
                    clonedEntry = array.GetProperty(i).GetValueCopy<object>();
                }
            }

            newArray.SetValue(clonedEntry, size - 1);

            property.SetValue(newArray);
        }

        private void OnMoveUpButtonClicked(int index)
        {
            Array array = property.GetValue<Array>();

            if ((index - 1) >= 0)
            {
                object previousEntry = array.GetValue(index - 1);

                array.SetValue(array.GetValue(index), index - 1);
                array.SetValue(previousEntry, index);
            }
        }

        private void OnMoveDownButtonClicked(int index)
        {
            Array array = property.GetValue<Array>();

            if ((index + 1) < array.Length)
            {
                object nextEntry = array.GetValue(index + 1);

                array.SetValue(array.GetValue(index), index + 1);
                array.SetValue(nextEntry, index);
            }
        }

        private void OnCreateButtonClicked()
        {
            property.SetValue(property.CreateArrayInstance(new int[1] { 0 }));
        }

        private void OnClearButtonClicked()
        {
            property.SetValue<object>(null);
        }
    }
}
