﻿using BansheeEngine;
using System;
using System.Collections.Generic;
using System.IO;

namespace BansheeEditor
{
    /// <summary>
    /// Displays a list of log messages.
    /// </summary>
    [DefaultSize(600, 300)]
    public class ConsoleWindow : EditorWindow
    {
        /// <summary>
        /// Filter type that determines what kind of messages are shown in the console.
        /// </summary>
        [Flags]
        private enum EntryFilter
        {
            Info = 0x01, Warning = 0x02, Error = 0x04, All = Info | Warning | Error
        }

        private const int TITLE_HEIGHT = 21;
        private const int ENTRY_HEIGHT = 39;
        private const int SEPARATOR_WIDTH = 3;
        private const float DETAILS_PANE_SIZE_PCT = 0.7f;
        private static readonly Color SEPARATOR_COLOR = new Color(33.0f/255.0f, 33.0f/255.0f, 33.0f/255.0f);
        private static int sSelectedElementIdx = -1;

        private GUIListView<ConsoleGUIEntry, ConsoleEntryData> listView;
        private List<ConsoleEntryData> entries = new List<ConsoleEntryData>();
        private List<ConsoleEntryData> filteredEntries = new List<ConsoleEntryData>();
        private EntryFilter filter = EntryFilter.All;
        private GUITexture detailsSeparator;
        private GUIScrollArea detailsArea;

        /// <summary>
        /// Returns the height of the list area.
        /// </summary>
        private int ListHeight
        {
            get { return Height - TITLE_HEIGHT; }
        }

        /// <summary>
        /// Opens the console window.
        /// </summary>
        [MenuItem("Windows/Console", ButtonModifier.CtrlAlt, ButtonCode.C, 6000)]
        private static void OpenConsoleWindow()
        {
            OpenWindow<ConsoleWindow>();
        }

        /// <inheritdoc/>
        protected override LocString GetDisplayName()
        {
            return new LocEdString("Console");
        }

        private void OnInitialize()
        {
            GUILayoutY layout = GUI.AddLayoutY();
            GUILayoutX titleLayout = layout.AddLayoutX();

            GUIToggle infoBtn = new GUIToggle(new GUIContent(EditorBuiltin.GetLogIcon(LogIcon.Info, 16)), EditorStyles.Button);
            GUIToggle warningBtn = new GUIToggle(new GUIContent(EditorBuiltin.GetLogIcon(LogIcon.Warning, 16)), EditorStyles.Button);
            GUIToggle errorBtn = new GUIToggle(new GUIContent(EditorBuiltin.GetLogIcon(LogIcon.Error, 16)), EditorStyles.Button);

            GUIToggle detailsBtn = new GUIToggle(new LocEdString("Show details"), EditorStyles.Button);
            GUIButton clearBtn = new GUIButton(new LocEdString("Clear"));

            titleLayout.AddElement(infoBtn);
            titleLayout.AddElement(warningBtn);
            titleLayout.AddElement(errorBtn);
            titleLayout.AddFlexibleSpace();
            titleLayout.AddElement(detailsBtn);
            titleLayout.AddElement(clearBtn);

            infoBtn.Value = filter.HasFlag(EntryFilter.Info);
            warningBtn.Value = filter.HasFlag(EntryFilter.Warning);
            errorBtn.Value = filter.HasFlag(EntryFilter.Error);

            infoBtn.OnToggled += x =>
            {
                if (x) 
                    SetFilter(filter | EntryFilter.Info);
                else 
                    SetFilter(filter & ~EntryFilter.Info);
            };

            warningBtn.OnToggled += x =>
            {
                if (x)
                    SetFilter(filter | EntryFilter.Warning);
                else
                    SetFilter(filter & ~EntryFilter.Warning);
            };

            errorBtn.OnToggled += x =>
            {
                if (x)
                    SetFilter(filter | EntryFilter.Error);
                else
                    SetFilter(filter & ~EntryFilter.Error);
            };

            detailsBtn.OnToggled += ToggleDetailsPanel;
            clearBtn.OnClick += Clear;

            GUILayoutX mainLayout = layout.AddLayoutX();

            listView = new GUIListView<ConsoleGUIEntry, ConsoleEntryData>(Width, ListHeight, ENTRY_HEIGHT, mainLayout);

            detailsSeparator = new GUITexture(Builtin.WhiteTexture, GUIOption.FixedWidth(SEPARATOR_WIDTH));
            detailsArea = new GUIScrollArea(ScrollBarType.ShowIfDoesntFit, ScrollBarType.NeverShow);
            mainLayout.AddElement(detailsSeparator);
            mainLayout.AddElement(detailsArea);
            detailsSeparator.Active = false;
            detailsArea.Active = false;

            detailsSeparator.SetTint(SEPARATOR_COLOR);

            Debug.OnAdded += OnEntryAdded;
        }

        private void OnEditorUpdate()
        {
            listView.Update();
        }

        private void OnDestroy()
        {
            Debug.OnAdded -= OnEntryAdded;
        }

        /// <inheritdoc/>
        protected override void WindowResized(int width, int height)
        {
            if (detailsArea.Active)
            {
                int listWidth = width - (int)(width * DETAILS_PANE_SIZE_PCT) - SEPARATOR_WIDTH;
                listView.SetSize(listWidth, height - TITLE_HEIGHT);
            }
            else
                listView.SetSize(width, height - TITLE_HEIGHT);

            base.WindowResized(width, height);
        }

        /// <summary>
        /// Triggered when a new entry is added in the debug log.
        /// </summary>
        /// <param name="type">Type of the message.</param>
        /// <param name="message">Message string.</param>
        private void OnEntryAdded(DebugMessageType type, string message)
        {
            // Check if compiler message, otherwise parse it normally
            LogEntryData logEntry = ScriptCodeManager.ParseCompilerMessage(message);
            if (logEntry == null)
                logEntry = Debug.ParseLogMessage(message);

            ConsoleEntryData newEntry = new ConsoleEntryData();
            newEntry.type = type;
            newEntry.callstack = logEntry.callstack;
            newEntry.message = logEntry.message;

            entries.Add(newEntry);
            
            if (DoesFilterMatch(type))
            {
                listView.AddEntry(newEntry);
                filteredEntries.Add(newEntry);
            }
        }

        /// <summary>
        /// Changes the filter that controls what type of messages are displayed in the console.
        /// </summary>
        /// <param name="filter">Flags that control which type of messages should be displayed.</param>
        private void SetFilter(EntryFilter filter)
        {
            if (this.filter == filter)
                return;

            this.filter = filter;

            listView.Clear();
            filteredEntries.Clear();
            foreach (var entry in entries)
            {
                if (DoesFilterMatch(entry.type))
                {
                    listView.AddEntry(entry);
                    filteredEntries.Add(entry);
                }
            }

            sSelectedElementIdx = -1;
        }

        /// <summary>
        /// Checks if the currently active entry filter matches the provided type (i.e. the entry with the type should be
        /// displayed).
        /// </summary>
        /// <param name="type">Type of the entry to check.</param>
        /// <returns>True if the entry with the specified type should be displayed in the console.</returns>
        private bool DoesFilterMatch(DebugMessageType type)
        {
            switch (type)
            {
                case DebugMessageType.Info:
                    return filter.HasFlag(EntryFilter.Info);
                case DebugMessageType.Warning:
                    return filter.HasFlag(EntryFilter.Warning);
                case DebugMessageType.Error:
                    return filter.HasFlag(EntryFilter.Error);
            }

            return false;
        }

        /// <summary>
        /// Removes all entries from the console.
        /// </summary>
        private void Clear()
        {
            listView.Clear();
            entries.Clear();
            filteredEntries.Clear();

            sSelectedElementIdx = -1;
            RefreshDetailsPanel();
        }

        /// <summary>
        /// Shows or hides the details panel.
        /// </summary>
        /// <param name="show">True to show, false to hide.</param>
        private void ToggleDetailsPanel(bool show)
        {
            detailsArea.Active = show;
            detailsSeparator.Active = show;

            if (show)
            {
                int listWidth = Width - (int)(Width * DETAILS_PANE_SIZE_PCT) - SEPARATOR_WIDTH;
                listView.SetSize(listWidth, ListHeight);

                RefreshDetailsPanel();
            }
            else
                listView.SetSize(Width, ListHeight);
        }

        /// <summary>
        /// Updates the contents of the details panel according to the currently selected element.
        /// </summary>
        private void RefreshDetailsPanel()
        {
            detailsArea.Layout.Clear();

            if (sSelectedElementIdx != -1)
            {
                GUILayoutX paddingX = detailsArea.Layout.AddLayoutX();
                paddingX.AddSpace(5);
                GUILayoutY paddingY = paddingX.AddLayoutY();
                paddingX.AddSpace(5);

                paddingY.AddSpace(5);
                GUILayoutY mainLayout = paddingY.AddLayoutY();
                paddingY.AddSpace(5);

                ConsoleEntryData entry = filteredEntries[sSelectedElementIdx];

                LocString message = new LocEdString(entry.message);
                GUILabel messageLabel = new GUILabel(message);
                mainLayout.AddElement(messageLabel);
                mainLayout.AddSpace(10);

                if (entry.callstack != null)
                {
                    foreach (var call in entry.callstack)
                    {
                        string fileName = Path.GetFileName(call.file);

                        string callMessage;
                        if (string.IsNullOrEmpty(call.method))
                            callMessage = "\tin " + fileName + ":" + call.line;
                        else
                            callMessage = "\t" + call.method + " in " + fileName + ":" + call.line;

                        GUIButton callBtn = new GUIButton(new LocEdString(callMessage));
                        mainLayout.AddElement(callBtn);

                        CallStackEntry hoistedCall = call;
                        callBtn.OnClick += () =>
                        {
                            CodeEditor.OpenFile(hoistedCall.file, hoistedCall.line);
                        };
                    }
                }
            }
            else
            {
                GUILayoutX centerX = detailsArea.Layout.AddLayoutX();
                centerX.AddFlexibleSpace();
                GUILayoutY centerY = centerX.AddLayoutY();
                centerX.AddFlexibleSpace();

                centerY.AddFlexibleSpace();
                GUILabel nothingSelectedLbl = new GUILabel(new LocEdString("(No entry selected)"));
                centerY.AddElement(nothingSelectedLbl);
                centerY.AddFlexibleSpace();
            }
        }

        /// <summary>
        /// Contains data for a single entry in the console.
        /// </summary>
        private class ConsoleEntryData : GUIListViewData
        {
            public DebugMessageType type;
            public string message;
            public CallStackEntry[] callstack;
        }

        /// <summary>
        /// Contains GUI elements used for displaying a single entry in the console.
        /// </summary>
        private class ConsoleGUIEntry : GUIListViewEntry<ConsoleEntryData>
        {
            private const int CALLER_LABEL_HEIGHT = 11;
            private const int PADDING = 3;
            private const int MESSAGE_HEIGHT = ENTRY_HEIGHT - CALLER_LABEL_HEIGHT - PADDING * 2;
            private static readonly Color BG_COLOR = Color.DarkGray;
            private static readonly Color SELECTION_COLOR = Color.DarkCyan;

            private GUIPanel overlay;
            private GUIPanel main;
            private GUIPanel underlay;

            private GUITexture icon;
            private GUILabel messageLabel;
            private GUILabel functionLabel;
            private GUITexture background;

            private int entryIdx;
            private string file;
            private int line;

            /// <inheritdoc/>
            public override void BuildGUI()
            {
                main = Layout.AddPanel(0, 1, 1, GUIOption.FixedHeight(ENTRY_HEIGHT));
                overlay = main.AddPanel(-1, 0, 0, GUIOption.FixedHeight(ENTRY_HEIGHT));
                underlay = main.AddPanel(1, 0, 0, GUIOption.FixedHeight(ENTRY_HEIGHT));

                GUILayoutX mainLayout = main.AddLayoutX();
                GUILayoutY overlayLayout = overlay.AddLayoutY();
                GUILayoutY underlayLayout = underlay.AddLayoutY();

                icon = new GUITexture(null, GUIOption.FixedWidth(32), GUIOption.FixedHeight(32));
                messageLabel = new GUILabel(new LocEdString(""), EditorStyles.MultiLineLabel, GUIOption.FixedHeight(MESSAGE_HEIGHT));
                functionLabel = new GUILabel(new LocEdString(""), GUIOption.FixedHeight(CALLER_LABEL_HEIGHT));

                mainLayout.AddSpace(PADDING);
                GUILayoutY iconLayout = mainLayout.AddLayoutY();
                iconLayout.AddFlexibleSpace();
                iconLayout.AddElement(icon);
                iconLayout.AddFlexibleSpace();

                mainLayout.AddSpace(PADDING);
                GUILayoutY messageLayout = mainLayout.AddLayoutY();
                messageLayout.AddSpace(PADDING);
                messageLayout.AddElement(messageLabel);
                messageLayout.AddElement(functionLabel);
                messageLayout.AddSpace(PADDING);
                mainLayout.AddSpace(PADDING);

                background = new GUITexture(Builtin.WhiteTexture, GUIOption.FixedHeight(ENTRY_HEIGHT));
                underlayLayout.AddElement(background);

                GUIButton button = new GUIButton(new LocEdString(""), EditorStyles.Blank, GUIOption.FixedHeight(ENTRY_HEIGHT));
                overlayLayout.AddElement(button);

                button.OnClick += OnClicked;
                button.OnDoubleClick += OnDoubleClicked;
            }

            /// <inheritdoc/>
            public override void UpdateContents(int index, ConsoleEntryData data)
            {
                if (index != sSelectedElementIdx)
                {
                    if (index%2 != 0)
                    {
                        background.Visible = true;
                        background.SetTint(BG_COLOR);
                    }
                    else
                    {
                        background.Visible = false;
                    }
                }
                else
                {
                    background.Visible = true;
                    background.SetTint(SELECTION_COLOR);
                }

                switch (data.type)
                {
                    case DebugMessageType.Info:
                        icon.SetTexture(EditorBuiltin.GetLogIcon(LogIcon.Info, 32));
                        break;
                    case DebugMessageType.Warning:
                        icon.SetTexture(EditorBuiltin.GetLogIcon(LogIcon.Warning, 32));
                        break;
                    case DebugMessageType.Error:
                        icon.SetTexture(EditorBuiltin.GetLogIcon(LogIcon.Error, 32));
                        break;
                }

                messageLabel.SetContent(new LocEdString(data.message));

                string method = "";
                if (data.callstack != null && data.callstack.Length > 0)
                {
                    file = Path.GetFileName(data.callstack[0].file);
                    line = data.callstack[0].line;

                    if (string.IsNullOrEmpty(data.callstack[0].method))
                        method = "\tin " + file + ":" + line;
                    else
                        method = "\t" + data.callstack[0].method + " in " + file + ":" + line;
                }
                else
                {
                    file = "";
                    line = 0;
                }

                functionLabel.SetContent(new LocEdString(method));

                entryIdx = index;
            }

            /// <summary>
            /// Triggered when the entry is selected.
            /// </summary>
            private void OnClicked()
            {
                sSelectedElementIdx = entryIdx;

                ConsoleWindow window = GetWindow<ConsoleWindow>();
                window.RefreshDetailsPanel();

                RefreshEntries();
            }

            /// <summary>
            /// Triggered when the entry is double-clicked.
            /// </summary>
            private void OnDoubleClicked()
            {
                if(!string.IsNullOrEmpty(file))
                    CodeEditor.OpenFile(file, line);
            }
        }
    }
}
