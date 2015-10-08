﻿using System;
using System.Collections;
using System.Collections.Generic;
using BansheeEngine;

namespace BansheeEditor
{
    /// <summary>
    /// Base class for objects that display GUI for a modifyable dictionary of elements. Elements can be added, modified or
    /// removed.
    /// </summary>
    public abstract class GUIDictionaryFieldBase
    {
        private const int IndentAmount = 5;

        protected Dictionary<int, GUIDictionaryFieldRow> rows = new Dictionary<int, GUIDictionaryFieldRow>();
        protected GUIDictionaryFieldRow editRow;
        protected GUILayoutY guiLayout;
        protected GUILayoutX guiChildLayout;
        protected GUILayoutX guiTitleLayout;
        protected GUILayoutY guiContentLayout;
        protected bool isExpanded;
        protected int depth;
        protected LocString title;

        private int editRowIdx = -1;
        private object editKey;
        private object editValue;
        private object editOriginalKey;

        private State state;

        /// <summary>
        /// Constructs a new GUI dictionary.
        /// </summary>
        ///  <param name="title">Label to display on the dictionary GUI title.</param>
        /// <param name="layout">Layout to which to append the list GUI elements to.</param>
        /// <param name="depth">Determines at which depth to render the background. Useful when you have multiple
        ///                     nested containers whose backgrounds are overlaping. Also determines background style,
        ///                     depths divisible by two will use an alternate style.</param>
        protected GUIDictionaryFieldBase(LocString title, GUILayout layout, int depth = 0)
        {
            this.title = title;
            this.guiLayout = layout.AddLayoutY();
            this.depth = depth;
        }

        /// <summary>
        /// Completely rebuilds the dictionary GUI elements.
        /// </summary>
        protected void BuildGUI()
        {
            editKey = CreateKey();
            editValue = CreateValue();

            UpdateHeaderGUI(true);

            if (!IsNull())
            {
                // Hidden dependency: BuildGUI must be called after all elements are 
                // in the dictionary so we do it in two steps
                int numRows = GetNumRows();
                for (int i = 0; i < numRows; i++)
                {
                    GUIDictionaryFieldRow newRow = CreateRow();
                    rows.Add(i, newRow);
                }

                editRow = CreateRow();
                editRow.BuildGUI(this, guiContentLayout, numRows, depth + 1);
                editRow.Enabled = false;

                for (int i = 0; i < numRows; i++)
                    rows[i].BuildGUI(this, guiContentLayout, i, depth + 1);
            }
        }

        /// <summary>
        /// Rebuilds the GUI dictionary header if needed. 
        /// </summary>
        /// <param name="forceRebuild">Forces the header to be rebuilt.</param>
        protected void UpdateHeaderGUI(bool forceRebuild)
        {
            Action BuildEmptyGUI = () =>
            {
                guiTitleLayout = guiLayout.AddLayoutX();

                guiTitleLayout.AddElement(new GUILabel(title));
                guiTitleLayout.AddElement(new GUILabel("Empty", GUIOption.FixedWidth(100)));

                GUIContent createIcon = new GUIContent(EditorBuiltin.GetInspectorWindowIcon(InspectorWindowIcon.Create));
                GUIButton createBtn = new GUIButton(createIcon, GUIOption.FixedWidth(30));
                createBtn.OnClick += OnCreateButtonClicked;
                guiTitleLayout.AddElement(createBtn);
            };

            Action BuildFilledGUI = () =>
            {
                GUIToggle guiFoldout = new GUIToggle(title, EditorStyles.Foldout);
                guiFoldout.Value = isExpanded;
                guiFoldout.OnToggled += ToggleFoldout;

                GUIContent clearIcon = new GUIContent(EditorBuiltin.GetInspectorWindowIcon(InspectorWindowIcon.Clear));
                GUIButton guiClearBtn = new GUIButton(clearIcon, GUIOption.FixedWidth(30));
                guiClearBtn.OnClick += OnClearButtonClicked;

                GUIContent addIcon = new GUIContent(EditorBuiltin.GetInspectorWindowIcon(InspectorWindowIcon.Add));
                GUIButton guiAddBtn = new GUIButton(addIcon, GUIOption.FixedWidth(30));
                guiAddBtn.OnClick += OnAddButtonClicked;

                guiTitleLayout = guiLayout.AddLayoutX();
                guiTitleLayout.AddElement(guiFoldout);
                guiTitleLayout.AddElement(guiAddBtn);
                guiTitleLayout.AddElement(guiClearBtn);

                guiChildLayout = guiLayout.AddLayoutX();
                guiChildLayout.AddSpace(IndentAmount);

                GUIPanel guiContentPanel = guiChildLayout.AddPanel();
                GUILayoutX guiIndentLayoutX = guiContentPanel.AddLayoutX();
                guiIndentLayoutX.AddSpace(IndentAmount);
                GUILayoutY guiIndentLayoutY = guiIndentLayoutX.AddLayoutY();
                guiIndentLayoutY.AddSpace(IndentAmount);
                guiContentLayout = guiIndentLayoutY.AddLayoutY();
                guiIndentLayoutY.AddSpace(IndentAmount);
                guiIndentLayoutX.AddSpace(IndentAmount);
                guiChildLayout.AddSpace(IndentAmount);

                short backgroundDepth = (short)(Inspector.START_BACKGROUND_DEPTH - depth - 1);
                string bgPanelStyle = depth % 2 == 0
                    ? EditorStyles.InspectorContentBgAlternate
                    : EditorStyles.InspectorContentBg;

                GUIPanel backgroundPanel = guiContentPanel.AddPanel(backgroundDepth);
                GUITexture inspectorContentBg = new GUITexture(null, bgPanelStyle);
                backgroundPanel.AddElement(inspectorContentBg);

                ToggleFoldout(isExpanded);
            };

            if (forceRebuild)
            {
                if (state != State.None)
                    guiTitleLayout.Destroy();

                if (state == State.Filled)
                    guiChildLayout.Destroy();

                state = State.None;
            }

            if (state == State.None)
            {
                if (!IsNull())
                {
                    BuildFilledGUI();
                    state = State.Filled;
                }
                else
                {
                    BuildEmptyGUI();

                    state = State.Empty;
                }
            }
            else if (state == State.Empty)
            {
                if (!IsNull())
                {
                    guiTitleLayout.Destroy();
                    BuildFilledGUI();
                    state = State.Filled;
                }
            }
            else if (state == State.Filled)
            {
                if (IsNull())
                {
                    guiTitleLayout.Destroy();
                    guiChildLayout.Destroy();
                    BuildEmptyGUI();

                    state = State.Empty;
                }
            }
        }

        /// <summary>
        /// Rebuilds GUI for all existing dictionary rows.
        /// </summary>
        private void BuildRows()
        {
            foreach (var KVP in rows)
                KVP.Value.Destroy();

            editRow.Destroy();

            if (!IsNull())
            {
                editRow.BuildGUI(this, guiContentLayout, rows.Count, depth + 1);
                editRow.Enabled = false;

                foreach (var KVP in rows)
                    KVP.Value.BuildGUI(this, guiContentLayout, KVP.Key, depth + 1);
            }
            else
            {
                rows.Clear();
            }
        }

        /// <summary>
        /// Returns the layout that is used for positioning the elements in the title bar.
        /// </summary>
        /// <returns>Horizontal layout for positioning the title bar elements.</returns>
        public GUILayoutX GetTitleLayout()
        {
            return guiTitleLayout;
        }

        /// <summary>
        /// Refreshes contents of all dictionary rows and checks if anything was modified.
        /// </summary>
        public void Refresh()
        {
            for (int i = 0; i < rows.Count; i++)
            {
                if (rows[i].Refresh())
                    rows[i].BuildGUI(this, guiContentLayout, i, depth + 1);
            }

            if (editRow != null && editRow.Enabled)
            {
                if (editRow.Refresh())
                    editRow.BuildGUI(this, guiContentLayout, rows.Count, depth + 1);
            }

        }

        /// <summary>
        /// Destroys the GUI elements.
        /// </summary>
        public void Destroy()
        {
            if (guiLayout != null)
            {
                guiLayout.Destroy();
                guiLayout = null;
            }

            guiLayout = null;
            guiTitleLayout = null;
            guiChildLayout = null;

            for (int i = 0; i < rows.Count; i++)
                rows[i].Destroy();

            rows.Clear();

            if (editRow != null)
                editRow.Destroy();

            editRow = null;
        }

        /// <summary>
        /// Checks is the specified row index the temporary edit row.
        /// </summary>
        /// <param name="rowIdx">Sequential index of the row to check.</param>
        /// <returns>True if the index is of an edit row.</returns>
        private bool IsTemporaryRow(int rowIdx)
        {
            return rowIdx == rows.Count;
        }

        /// <summary>
        /// Checks is any row being currently edited.
        /// </summary>
        /// <returns>True if a row is being edited, false otherwise.</returns>
        private bool IsEditInProgress()
        {
            return editRowIdx != -1;
        }

        /// <summary>
        /// Returns the number of rows in the dictionary.
        /// </summary>
        /// <returns>Number of rows in the dictionary.</returns>
        protected abstract int GetNumRows();

        /// <summary>
        /// Checks is the dictionary instance not assigned.
        /// </summary>
        /// <returns>True if there is not a dictionary instance.</returns>
        protected abstract bool IsNull();

        /// <summary>
        /// Gets a value of an element at the specified index in the list. Also handles temporary edit fields.
        /// </summary>
        /// <param name="rowIdx">Sequential index of the row to set the value for.</param>
        /// <returns>Value of the list element at the specified key.</returns>
        protected internal virtual object GetValueInternal(int rowIdx)
        {
            if (rowIdx == editRowIdx || IsTemporaryRow(rowIdx))
                return editValue;
            else
                return GetValue(GetKey(rowIdx));
        }

        /// <summary>
        /// Sets a value of an element at the specified index in the list. Also handles temporary edit fields.
        /// </summary>
        /// <param name="rowIdx">Sequential index of the row to set the value for.</param>
        /// <param name="value">Value to assign to the element. Caller must ensure it is of valid type.</param>
        protected internal virtual void SetValueInternal(int rowIdx, object value)
        {
            if (rowIdx == editRowIdx || IsTemporaryRow(rowIdx))
                editValue = value;
            else
                SetValue(GetKey(rowIdx), value);
        }

        /// <summary>
        /// Changes the value of the key of the specified row.
        /// </summary>
        /// <param name="rowIdx">Sequential index of the row to set the key for.</param>
        /// <param name="key">Key to assign to the specified row.</param>
        protected internal void SetKey(int rowIdx, object key)
        {
            if (editRowIdx != rowIdx)
            {
                Debug.LogError("Cannot change a dictionary row that is not in edit state.");
                return;
            }

            editKey = key;
        }

        /// <summary>
        /// Gets a key for a row at the specified index. Handles the special case for the currently edited row.
        /// </summary>
        /// <param name="rowIdx">Sequential index of the row for which to retrieve the key.</param>
        /// <returns>Key for a row at the specified index.</returns>
        protected internal object GetKeyInternal(int rowIdx)
        {
            if (editRowIdx == rowIdx || IsTemporaryRow(rowIdx))
                return editKey;

            return GetKey(rowIdx);
        }

        /// <summary>
        /// Creates a new dictionary row GUI.
        /// </summary>
        /// <returns>Object containing the dictionary row GUI.</returns>
        protected abstract GUIDictionaryFieldRow CreateRow();

        /// <summary>
        /// Gets a key for a row at the specified index.
        /// </summary>
        /// <param name="rowIdx">Sequential index of the row for which to retrieve the key.</param>
        /// <returns>Key for a row at the specified index.</returns>
        protected internal abstract object GetKey(int rowIdx);

        /// <summary>
        /// Gets a value of an element at the specified index in the list.
        /// </summary>
        /// <param name="key">Key of the element whose value to retrieve.</param>
        /// <returns>Value of the dictionary entry for the specified key.</returns>
        protected internal abstract object GetValue(object key);

        /// <summary>
        /// Sets a value of an element at the specified index in the list.
        /// </summary>
        /// <param name="key">Key of the element whose value to set.</param>
        /// <param name="value">Value to assign to the element. Caller must ensure it is of valid type.</param>
        protected internal abstract void SetValue(object key, object value);

        /// <summary>
        /// Updates both key and value of an existing entry.
        /// </summary>
        /// <param name="oldKey">Original key of the entry.</param>
        /// <param name="newKey">New key of the entry.</param>
        /// <param name="value">New value of the entry.</param>
        protected internal abstract void EditEntry(object oldKey, object newKey, object value);

        /// <summary>
        /// Adds a new entry to the dictionary.
        /// </summary>
        /// <param name="key">Key of the entry to add.</param>
        /// <param name="value">Value of the entry to add.</param>
        protected internal abstract void AddEntry(object key, object value);

        /// <summary>
        /// Removes the specified entry from the dictionary.
        /// </summary>
        /// <param name="key">Key of the entry to remove.</param>
        protected internal abstract void RemoveEntry(object key);

        /// <summary>
        /// Creates a new empty key object of a valid type that can be used in the dictionary.
        /// </summary>
        /// <returns>New empty key object.</returns>
        protected internal abstract object CreateKey();

        /// <summary>
        /// Creates a new empty value object of a valid type that can be used in the dictionary.
        /// </summary>
        /// <returns>New empty value object.</returns>
        protected internal abstract object CreateValue();

        /// <summary>
        /// Checks does the element with the specified key exist in the dictionary.
        /// </summary>
        /// <param name="key">Key of the element to check for existence.</param>
        /// <returns>True if the key exists in the dictionary, false otherwise.</returns>
        protected internal abstract bool Contains(object key);

        /// <summary>
        /// Creates a brand new dictionary with zero elements in the place of the current dictionary.
        /// </summary>
        protected abstract void CreateDictionary();

        /// <summary>
        /// Deletes the current dictionary object.
        /// </summary>
        protected abstract void DeleteDictionary();

        /// <summary>
        /// Hides or shows the dictionary rows.
        /// </summary>
        /// <param name="expanded">True if the rows should be displayed, false otherwise.</param>
        private void ToggleFoldout(bool expanded)
        {
            isExpanded = expanded;

            if (guiChildLayout != null)
                guiChildLayout.Active = isExpanded && (rows.Count > 0 || IsEditInProgress());
        }

        /// <summary>
        /// Triggered when the user clicks on the create button on the title bar. Creates a brand new dictionary with zero
        /// elements in the place of the current dictionary.
        /// </summary>
        protected void OnCreateButtonClicked()
        {
            CreateDictionary();
            UpdateHeaderGUI(false);
            BuildRows();
        }

        /// <summary>
        /// Triggered when the user clicks on the add button on the title bar. Adds a new empty element to the dictionary.
        /// </summary>
        protected virtual void OnAddButtonClicked()
        {
            if (IsEditInProgress())
            {
                DialogBox.Open(
                    new LocEdString("Edit in progress."),
                    new LocEdString("You are editing the entry with key \"" + editKey + "\". You cannot add a row " +
                                    "before applying or discarding those changes. Do you wish to apply those changes first?"),
                    DialogBox.Type.YesNoCancel,
                    x =>
                    {
                        switch (x)
                        {
                            case DialogBox.ResultType.Yes:
                                if (ApplyChanges())
                                    StartAdd();
                                break;
                            case DialogBox.ResultType.No:
                                DiscardChanges();
                                StartAdd();
                                break;
                        }
                    });
            }
            else
            {
                if (!isExpanded)
                    ToggleFoldout(true);

                StartAdd();
            }
        }

        /// <summary>
        /// Triggered when the user clicks on the clear button on the title bar. Deletes the current dictionary object.
        /// </summary>
        protected void OnClearButtonClicked()
        {
            DeleteDictionary();
            UpdateHeaderGUI(false);
            BuildRows();
        }

        /// <summary>
        /// Triggered when the user clicks on the delete button next to a dictionary entry. Deletes an element in the 
        /// dictionary. 
        /// </summary>
        /// <param name="rowIdx">Sequential row index of the entry that was clicked.</param>
        protected internal virtual void OnDeleteButtonClicked(int rowIdx)
        {
            if (IsEditInProgress())
                DiscardChanges();
            else
            {
                // Remove the entry, but ensure the rows keep referencing the original keys (dictionaries have undefined
                // order so we need to compare old vs. new elements to determine if any changed).
                int oldNumRows = GetNumRows();
                Dictionary<object, int> oldKeys = new Dictionary<object, int>();
                for (int i = 0; i < oldNumRows; i++)
                    oldKeys.Add(GetKey(i), i);

                RemoveEntry(GetKey(rowIdx));

                int newNumRows = GetNumRows();
                Dictionary<object, int> newKeys = new Dictionary<object, int>();
                for (int i = 0; i < newNumRows; i++)
                    newKeys.Add(GetKey(i), i);

                foreach (var KVP in oldKeys)
                {
                    int newRowIdx;
                    if (newKeys.TryGetValue(KVP.Key, out newRowIdx))
                    {
                        if (KVP.Value != newRowIdx)
                        {
                            GUIDictionaryFieldRow temp = rows[KVP.Value];
                            rows[KVP.Value] = rows[newRowIdx];
                            rows[newRowIdx] = temp;
                        }
                    }
                }

                for (int i = newNumRows; i < oldNumRows; i++)
                {
                    rows[i].Destroy();
                    rows.Remove(i);
                }

                BuildRows();
            }
        }

        /// <summary>
        /// Triggered when the user clicks on the clone button next to a dictionary entry. Clones an element and
        /// adds the clone to the dictionary.
        /// </summary>
        /// <param name="rowIdx">Sequential row index of the entry that was clicked.</param>
        protected internal virtual void OnCloneButtonClicked(int rowIdx)
        {
            if (IsEditInProgress())
            {
                DialogBox.Open(
                    new LocEdString("Edit in progress."),
                    new LocEdString("You are editing the entry with key \"" + editKey + "\". You cannot clone a row " +
                        "before applying or discarding those changes. Do you wish to apply those changes first?"),
                    DialogBox.Type.YesNoCancel,
                    x =>
                    {
                        switch (x)
                        {
                            case DialogBox.ResultType.Yes:
                                if (ApplyChanges())
                                    StartClone(rowIdx);
                                break;
                            case DialogBox.ResultType.No:
                                DiscardChanges();
                                StartClone(rowIdx);
                                break;
                        }
                    });
            }
            else
                StartClone(rowIdx);
        }

        /// <summary>
        /// Triggered when user clicks the edit or apply (depending on state) button next to the dictionary entry. Starts
        /// edit mode for the element, if not already in edit mode. Applies edit mode changes if already in edit mode.
        /// </summary>
        /// <param name="rowIdx">Sequential row index of the entry that was clicked.</param>
        protected internal virtual void OnEditButtonClicked(int rowIdx)
        {
            if (editRowIdx == rowIdx)
                ApplyChanges();
            else
            {
                if (IsEditInProgress())
                {
                    DialogBox.Open(
                        new LocEdString("Edit in progress."),
                        new LocEdString("You are already editing the entry with key \"" + editKey + "\". You cannot edit " +
                            "another row before applying or discarding those changes. Do you wish to apply those changes first?"),
                        DialogBox.Type.YesNoCancel,
                        x =>
                        {
                            switch (x)
                            {
                                case DialogBox.ResultType.Yes:
                                    if (ApplyChanges())
                                        StartEdit(rowIdx);
                                    break;
                                case DialogBox.ResultType.No:
                                    DiscardChanges();
                                    StartEdit(rowIdx);
                                    break;
                            }
                        });
                }
                else
                    StartEdit(rowIdx);
            }
        }

        /// <summary>
        /// Starts an edit operation on a row with the specified key. Allows the user to change the key of the specified row.
        /// Caller must ensure no edit operation is already in progress.
        /// </summary>
        /// <param name="rowIdx">Sequential row index of the entry to edit.</param>
        private void StartEdit(int rowIdx)
        {
            object key = GetKey(rowIdx);

            editKey = SerializableUtility.Clone(key);
            editValue = SerializableUtility.Clone(GetValue(key));
            editOriginalKey = key;
            editRowIdx = rowIdx;

            rows[rowIdx].EditMode = true;
            guiChildLayout.Active = rows.Count > 0 && isExpanded;
        }

        /// <summary>
        /// Starts an add operation. Adds a new key/value pair and allows the user to set them up in a temporary row
        /// before inserting them into the dictionary. Caller must ensure no edit operation is already in progress.
        /// </summary>
        private void StartAdd()
        {
            editKey = CreateKey();
            editValue = CreateValue();
            editOriginalKey = null;
            editRowIdx = rows.Count;

            editRow.Enabled = true;
            editRow.EditMode = true;

            ToggleFoldout(isExpanded);
        }

        /// <summary>
        /// Starts a clone operation. Adds a new key/value pair by cloning an existing one. Allows the user to modify the 
        /// new pair in a temporary row before inserting them into the dictionary. Caller must ensure no edit operation is 
        /// already in progress.
        /// </summary>
        /// <param name="rowIdx">Sequential row index of the entry to clone.</param>
        private void StartClone(int rowIdx)
        {
            object key = GetKey(rowIdx);

            editKey = SerializableUtility.Clone(key);
            editValue = SerializableUtility.Clone(GetValue(key));
            editOriginalKey = null;
            editRowIdx = rows.Count;

            editRow.Enabled = true;
            editRow.EditMode = true;

            ToggleFoldout(isExpanded);
        }

        /// <summary>
        /// Attempts to apply any changes made to the currently edited row.
        /// </summary>
        /// <returns>True if the changes were successfully applied, false if the new key already exists in the dictionary.
        ///          </returns>
        private bool ApplyChanges()
        {
            if (!IsEditInProgress())
                return true;

            if (Contains(editKey) && (editOriginalKey == null || !editOriginalKey.Equals(editKey)))
            {
                DialogBox.Open(
                    new LocEdString("Key already exists."),
                    new LocEdString("Cannot add a key \"" + editKey + "\" to dictionary. Key already exists"),
                    DialogBox.Type.OK);

                return false;
            }
            else
            {
                if (IsTemporaryRow(editRowIdx))
                {
                    editRow.EditMode = false;
                    editRow.Enabled = false;
                }
                else
                {
                    rows[editRowIdx].EditMode = false;
                }

                // Add/remove the entry, but ensure the rows keep referencing the original keys (dictionaries have undefined
                // order so we need to compare old vs. new elements to determine if any changed).
                int oldNumRows = GetNumRows();
                Dictionary<object, int> oldKeys = new Dictionary<object, int>();
                for (int i = 0; i < oldNumRows; i++)
                    oldKeys.Add(GetKey(i), i);

                if (editOriginalKey != null) // Editing
                    EditEntry(editOriginalKey, editKey, editValue);
                else // Adding/Cloning
                    AddEntry(editKey, editValue);

                int newNumRows = GetNumRows();
                Dictionary<object, int> newKeys = new Dictionary<object, int>();
                for (int i = 0; i < newNumRows; i++)
                    newKeys.Add(GetKey(i), i);

                // Hidden dependency: BuildGUI must be called after all elements are 
                // in the dictionary so we do it in two steps
                for (int i = oldNumRows; i < newNumRows; i++)
                    rows[i] = CreateRow();

                foreach (var KVP in oldKeys)
                {
                    int newRowIdx;
                    if (newKeys.TryGetValue(KVP.Key, out newRowIdx))
                    {
                        if (KVP.Value != newRowIdx)
                        {
                            GUIDictionaryFieldRow temp = rows[KVP.Value];
                            rows[KVP.Value] = rows[newRowIdx];
                            rows[newRowIdx] = temp;
                        }
                    }
                }

                for (int i = newNumRows; i < oldNumRows; i++)
                {
                    rows[i].Destroy();
                    rows.Remove(i);
                }

                BuildRows();

                editKey = CreateKey();
                editValue = CreateValue();
                editOriginalKey = null;
                editRowIdx = -1;

                ToggleFoldout(isExpanded);
                return true;
            }
        }

        /// <summary>
        /// Cancels any changes made on the currently edited row.
        /// </summary>
        private void DiscardChanges()
        {
            if (IsEditInProgress())
            {
                if (IsTemporaryRow(editRowIdx))
                {
                    editRow.EditMode = false;
                    editRow.Enabled = false;
                }
                else
                {
                    rows[editRowIdx].EditMode = false;
                }

                editKey = CreateKey();
                editValue = CreateValue();
                editOriginalKey = null;
                editRow.Enabled = false;
                editRowIdx = -1;

                ToggleFoldout(isExpanded);
            }
        }

        /// <summary>
        /// Possible states dictionary GUI can be in.
        /// </summary>
        private enum State
        {
            None,
            Empty,
            Filled
        }
    }

    /// <summary>
    /// Creates GUI elements that allow viewing and manipulation of a <see cref="Dictionary{TKey,TValue}"/>. When constructing the
    /// object user can provide a custom type that manages GUI for individual dictionary elements.
    /// </summary>
    /// <typeparam name="Key">Type of key used by the dictionary.</typeparam>
    /// <typeparam name="Value">Type of value stored in the dictionary.</typeparam>
    /// <typeparam name="RowType">Type of rows that are used to handle GUI for individual dictionary elements.</typeparam>
    public class GUIDictionaryField<Key, Value, RowType> : GUIDictionaryFieldBase where RowType : GUIDictionaryFieldRow, new()
    {
        public delegate int SortDictionaryDelegate(Key a, Key b);

        /// <summary>
        /// Triggered when the reference array has been changed. This does not include changes that only happen to its 
        /// internal elements.
        /// </summary>
        public Action<Dictionary<Key, Value>> OnChanged;

        /// <summary>
        /// Triggered when an element in the list has been added or changed.
        /// </summary>
        public Action<Key> OnValueChanged;

        /// <summary>
        /// Triggered when an element in the dictionary has been removed.
        /// </summary>
        public Action<Key> OnValueRemoved;

        /// <summary>
        /// Optional method that will be used for sorting the elements in the dictionary.
        /// </summary>
        public SortDictionaryDelegate SortMethod;

        /// <summary>
        /// Array object whose contents are displayed.
        /// </summary>
        public Dictionary<Key, Value> Dictionary { get { return dictionary; } }
        protected Dictionary<Key, Value> dictionary;

        private List<Key> orderedKeys = new List<Key>();

        /// <summary>
        /// Constructs a new dictionary GUI field.
        /// </summary>
        /// <param name="title">Label to display on the dictionary GUI title.</param>
        /// <param name="dictionary">Object containing the data. Can be null.</param>
        /// <param name="layout">Layout to which to append the list GUI elements to.</param>
        /// <param name="depth">Determines at which depth to render the background. Useful when you have multiple
        ///                     nested containers whose backgrounds are overlaping. Also determines background style,
        ///                     depths divisible by two will use an alternate style.</param>
        protected GUIDictionaryField(LocString title, Dictionary<Key, Value> dictionary, GUILayout layout, int depth = 0)
            : base(title, layout, depth)
        {
            this.dictionary = dictionary;
            UpdateKeys();
        }

        /// <summary>
        /// Creates a dictionary GUI field containing GUI elements required for displaying the provided dictionary.
        /// </summary>
        /// <typeparam name="RowType">Type of rows that are used to handle GUI for individual dictionary elements.</typeparam>
        /// <param name="title">Label to display on the list GUI title.</param>
        /// <param name="dictionary">Object containing the data. Can be null.</param>
        /// <param name="layout">Layout to which to append the list GUI elements to.</param>
        /// <param name="depth">Determines at which depth to render the background. Useful when you have multiple
        ///                     nested containers whose backgrounds are overlaping. Also determines background style,
        ///                     depths divisible by two will use an alternate style.</param>
        /// <returns>New instance of dictionary GUI field.</returns>
        public static GUIDictionaryField<Key, Value, RowType> Create(LocString title, Dictionary<Key, Value> dictionary, 
            GUILayout layout, int depth = 0)

        {
            GUIDictionaryField<Key, Value, RowType> guiDictionary = new GUIDictionaryField<Key, Value, RowType>(
                title, dictionary, layout, depth);

            guiDictionary.BuildGUI();
            return guiDictionary;
        }

        /// <summary>
        /// Updates the ordered set of keys used for mapping sequential indexes to keys. Should be called whenever a 
        /// dictionary key changes.
        /// </summary>
        private void UpdateKeys()
        {
            orderedKeys.Clear();

            if (dictionary != null)
            {
                foreach (var KVP in dictionary)
                    orderedKeys.Add(KVP.Key);

                if (SortMethod != null)
                    orderedKeys.Sort((x,y) => SortMethod(x, y));
                else
                    orderedKeys.Sort();
            }
        }

        /// <inheritdoc/>
        protected override GUIDictionaryFieldRow CreateRow()
        {
            return new RowType();
        }

        /// <inheritdoc/>
        protected override int GetNumRows()
        {
            if (dictionary != null)
                return dictionary.Count;

            return 0;
        }

        /// <inheritdoc/>
        protected override bool IsNull()
        {
            return dictionary == null;
        }

        /// <inheritdoc/>
        protected internal override object GetKey(int rowIdx)
        {
            return orderedKeys[rowIdx];
        }

        /// <inheritdoc/>
        protected internal override object GetValue(object key)
        {
            return dictionary[(Key)key];
        }

        /// <inheritdoc/>
        protected internal override void SetValue(object key, object value)
        {
            dictionary[(Key)key] = (Value)value;

            if (OnValueChanged != null)
                OnValueChanged((Key)key);
        }

        /// <inheritdoc/>
        protected internal override bool Contains(object key)
        {
            return dictionary.ContainsKey((Key)key); ;
        }

        /// <inheritdoc/>
        protected internal override void EditEntry(object oldKey, object newKey, object value)
        {
            dictionary.Remove((Key)oldKey);
            dictionary[(Key)newKey] = (Value)value;

            if (OnValueRemoved != null)
                OnValueRemoved((Key)oldKey);

            if (OnValueChanged != null)
                OnValueChanged((Key)newKey);

            UpdateKeys();
        }

        /// <inheritdoc/>
        protected internal override void AddEntry(object key, object value)
        {
            dictionary[(Key)key] = (Value)value;

            if (OnValueChanged != null)
                OnValueChanged((Key)key);

            UpdateKeys();
        }

        /// <inheritdoc/>
        protected internal override void RemoveEntry(object key)
        {
            dictionary.Remove((Key) key);

            if (OnValueRemoved != null)
                OnValueRemoved((Key)key);

            UpdateKeys();
        }

        /// <inheritdoc/>
        protected internal override object CreateKey()
        {
            return SerializableUtility.Create<Key>();
        }

        /// <inheritdoc/>
        protected internal override object CreateValue()
        {
            return SerializableUtility.Create<Value>();
        }

        /// <inheritdoc/>
        protected override void CreateDictionary()
        {
            dictionary = new Dictionary<Key, Value>();

            if (OnChanged != null)
                OnChanged(dictionary);

            UpdateKeys();
        }

        /// <inheritdoc/>
        protected override void DeleteDictionary()
        {
            dictionary = null;

            if (OnChanged != null)
                OnChanged(dictionary);

            UpdateKeys();
        }
    }

    /// <summary>
    /// Contains GUI elements for a single entry in a dictionary.
    /// </summary>
    public abstract class GUIDictionaryFieldRow
    {
        private GUILayoutY rowLayout;
        private GUILayoutX keyRowLayout;
        private GUILayoutY keyLayout;
        private GUILayoutY valueLayout;
        private GUILayoutX titleLayout;
        private GUIButton deleteBtn;
        private GUIButton editBtn;
        private bool localTitleLayout;
        private bool enabled = true;
        private bool editMode = false;
        private GUIDictionaryFieldBase parent;

        protected int rowIdx;
        protected int depth;

        /// <summary>
        /// Determines is the row currently being displayed.
        /// </summary>
        internal bool Enabled
        {
            get { return enabled; }
            set { enabled = value; rowLayout.Active = value; }
        }

        /// <summary>
        /// Enables or disables the row's edit mode. This determines what type of buttons are shown on the row title bar.
        /// </summary>
        internal bool EditMode
        {
            set
            {
                if (value)
                {
                    GUIContent cancelIcon = new GUIContent(EditorBuiltin.GetInspectorWindowIcon(InspectorWindowIcon.Cancel));
                    GUIContent applyIcon = new GUIContent(EditorBuiltin.GetInspectorWindowIcon(InspectorWindowIcon.Apply));

                    deleteBtn.SetContent(cancelIcon);
                    editBtn.SetContent(applyIcon);
                }
                else
                {
                    GUIContent deleteIcon = new GUIContent(EditorBuiltin.GetInspectorWindowIcon(InspectorWindowIcon.Delete));
                    GUIContent editIcon = new GUIContent(EditorBuiltin.GetInspectorWindowIcon(InspectorWindowIcon.Edit));

                    deleteBtn.SetContent(deleteIcon);
                    editBtn.SetContent(editIcon);
                }

                editMode = value;
            }
        }

        /// <summary>
        /// Constructs a new dictionary row object.
        /// </summary>
        protected GUIDictionaryFieldRow()
        {

        }

        /// <summary>
        /// (Re)creates all row GUI elements.
        /// </summary>
        /// <param name="parent">Parent array GUI object that the entry is contained in.</param>
        /// <param name="parentLayout">Parent layout that row GUI elements will be added to.</param>
        /// <param name="rowIdx">Sequential index of the row.</param>
        /// <param name="depth">Determines the depth at which the element is rendered.</param>
        internal void BuildGUI(GUIDictionaryFieldBase parent, GUILayout parentLayout, int rowIdx, int depth)
        {
            if (rowLayout != null)
                rowLayout.Destroy();

            this.parent = parent;
            this.rowIdx = rowIdx;
            this.depth = depth;

            rowLayout = parentLayout.AddLayoutY();
            keyRowLayout = rowLayout.AddLayoutX();
            keyLayout = keyRowLayout.AddLayoutY();
            valueLayout = rowLayout.AddLayoutY();

            GUILayoutX externalTitleLayout = CreateKeyGUI(keyLayout);
            CreateValueGUI(valueLayout);
            if (localTitleLayout || (titleLayout != null && titleLayout == externalTitleLayout))
                return;

            if (externalTitleLayout != null)
            {
                localTitleLayout = false;
                titleLayout = externalTitleLayout;
            }
            else
            {
                GUILayoutY buttonCenter = keyRowLayout.AddLayoutY();
                buttonCenter.AddFlexibleSpace();
                titleLayout = buttonCenter.AddLayoutX();
                buttonCenter.AddFlexibleSpace();

                localTitleLayout = true;
            }

            GUIContent cloneIcon = new GUIContent(EditorBuiltin.GetInspectorWindowIcon(InspectorWindowIcon.Clone));
            GUIContent deleteIcon = new GUIContent(EditorBuiltin.GetInspectorWindowIcon(InspectorWindowIcon.Delete));
            GUIContent editIcon = new GUIContent(EditorBuiltin.GetInspectorWindowIcon(InspectorWindowIcon.Edit));

            GUIButton cloneBtn = new GUIButton(cloneIcon, GUIOption.FixedWidth(30));
            deleteBtn = new GUIButton(deleteIcon, GUIOption.FixedWidth(30));
            editBtn = new GUIButton(editIcon, GUIOption.FixedWidth(30));

            cloneBtn.OnClick += () => parent.OnCloneButtonClicked(rowIdx);
            deleteBtn.OnClick += () => parent.OnDeleteButtonClicked(rowIdx);
            editBtn.OnClick += () => parent.OnEditButtonClicked(rowIdx);

            titleLayout.AddElement(cloneBtn);
            titleLayout.AddElement(deleteBtn);
            titleLayout.AddSpace(10);
            titleLayout.AddElement(editBtn);

            EditMode = editMode;

            Refresh();
        }

        /// <summary>
        /// Creates GUI elements specific to type in the key portion of a dictionary entry.
        /// </summary>
        /// <param name="layout">Layout to insert the row GUI elements to.</param>
        /// <returns>An optional title bar layout that the standard dictionary buttons will be appended to.</returns>
        protected abstract GUILayoutX CreateKeyGUI(GUILayoutY layout);

        /// <summary>
        /// Creates GUI elements specific to type in the key portion of a dictionary entry.
        /// </summary>
        /// <param name="layout">Layout to insert the row GUI elements to.</param>
        protected abstract void CreateValueGUI(GUILayoutY layout);

        /// <summary>
        /// Refreshes the GUI for the dictionary row and checks if anything was modified.
        /// </summary>
        /// <returns>Determines should the field's GUI elements be updated due to modifications.</returns>
        internal protected virtual bool Refresh()
        {
            return false;
        }

        /// <summary>
        /// Gets the key contained in this dictionary's row.
        /// </summary>
        /// <typeparam name="T">Type of the key. Must match the dictionary's element type.</typeparam>
        /// <returns>Key in this dictionary's row.</returns>
        protected T GetKey<T>()
        {
            return (T)parent.GetKeyInternal(rowIdx);
        }

        /// <summary>
        /// Sets the key for in this dictionary's row.
        /// </summary>
        /// <typeparam name="T">Type of the key. Must match the dictionary's element type.</typeparam>
        /// <param name="key">Key to assign to this row.</param>
        protected void SetKey<T>(T key)
        {
            parent.SetKey(rowIdx, key);
        }

        /// <summary>
        /// Gets the value contained in this dictionary's row.
        /// </summary>
        /// <typeparam name="T">Type of the value. Must match the dictionary's element type.</typeparam>
        /// <returns>Value in this dictionary's row.</returns>
        protected T GetValue<T>()
        {
            return (T)parent.GetValueInternal(rowIdx);
        }

        /// <summary>
        /// Sets the value contained in this dictionary's row.
        /// </summary>
        /// <typeparam name="T">Type of the value. Must match the dictionary's element type.</typeparam>
        /// <param name="value">Value to set.</param>
        protected void SetValue<T>(T value)
        {
            parent.SetValueInternal(rowIdx, value);
        }

        /// <summary>
        /// Destroys all row GUI elements.
        /// </summary>
        public void Destroy()
        {
            if (rowLayout != null)
            {
                rowLayout.Destroy();
                rowLayout = null;
            }

            keyRowLayout = null;
            keyLayout = null;
            valueLayout = null;
            titleLayout = null;
            deleteBtn = null;
            editBtn = null;
            localTitleLayout = false;
        }
    }
}