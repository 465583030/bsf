﻿using System;
using System.Collections.Generic;
using System.IO;
using BansheeEngine;

namespace BansheeEditor
{
    internal enum ProjectViewType
    {
        Grid64, Grid48, Grid32, List16
    }

    internal sealed class LibraryWindow : EditorWindow
    {
        internal enum MoveDirection
        {
            Up, Down, Left, Right
        }

        private const int DRAG_SCROLL_HEIGHT = 20;
        private const int DRAG_SCROLL_AMOUNT_PER_SECOND = 100;
        private const int FOLDER_BUTTON_WIDTH = 20;
        private const int FOLDER_SEPARATOR_WIDTH = 10;
        
        private bool hasContentFocus = false;
        private bool HasContentFocus { get { return HasFocus && hasContentFocus; } }

        private string searchQuery;
        private bool IsSearchActive { get { return !string.IsNullOrEmpty(searchQuery); } }

        private ProjectViewType viewType = ProjectViewType.Grid32;

        private bool requiresRefresh;
        private string currentDirectory = "";
        private List<string> selectionPaths = new List<string>();
        private int selectionAnchorStart = -1;
        private int selectionAnchorEnd = -1;
        private string pingPath = "";
        private string hoverHighlightPath = "";

        private LibraryGUIContent content;
        private GUIScrollArea contentScrollArea;
        private GUILayoutX searchBarLayout;
        private GUIButton optionsButton;
        private GUILayout folderBarLayout;
        private GUILayout folderListLayout;
        private GUITextField searchField;
        private GUITexture dragSelection;

        private ContextMenu entryContextMenu;
        private LibraryDropTarget dropTarget;

        private int autoScrollAmount;
        private bool isDraggingSelection;
        private Vector2I dragSelectionStart;
        private Vector2I dragSelectionEnd;

        private LibraryGUIEntry inProgressRenameElement;

        // Cut/Copy/Paste
        private List<string> copyPaths = new List<string>();
        private List<string> cutPaths = new List<string>();

        internal ProjectViewType ViewType
        {
            get { return viewType; }
            set { viewType = value; Refresh(); }
        }

        /// <summary>
        /// Returns a file or folder currently selected in the library window. If nothing is selected, returns the active
        /// folder. Returned path is relative to project library resources folder.
        /// </summary>
        public string SelectedEntry
        {
            get
            {
                if (selectionPaths.Count == 1)
                {
                    LibraryEntry entry = ProjectLibrary.GetEntry(selectionPaths[0]);
                    if (entry != null)
                        return entry.Path;
                }

                return currentDirectory;
            }
        }

        /// <summary>
        /// Returns a folder currently selected in the library window. If no folder is selected, returns the active
        /// folder. Returned path is relative to project library resources folder.
        /// </summary>
        public string SelectedFolder
        {
            get
            {
                DirectoryEntry selectedDirectory = null;
                if (selectionPaths.Count == 1)
                {
                    LibraryEntry entry = ProjectLibrary.GetEntry(selectionPaths[0]);
                    if (entry != null && entry.Type == LibraryEntryType.Directory)
                        selectedDirectory = (DirectoryEntry) entry;
                }

                if (selectedDirectory != null)
                    return selectedDirectory.Path;
                
                return currentDirectory;
            }
        }

        /// <summary>
        /// Returns the path to the folder currently displayed in the library window. Returned path is relative to project 
        /// library resources folder.
        /// </summary>
        public string CurrentFolder
        {
            get { return currentDirectory; }
        }

        // Note: I don't feel like I should be exposing this
        internal ContextMenu ContextMenu
        {
            get { return entryContextMenu; }
        }

        [MenuItem("Windows/Library", ButtonModifier.CtrlAlt, ButtonCode.L, 6000)]
        private static void OpenLibraryWindow()
        {
            OpenWindow<LibraryWindow>();
        }

        private void OnInitialize()
        {
            ProjectLibrary.OnEntryAdded += OnEntryChanged;
            ProjectLibrary.OnEntryRemoved += OnEntryChanged;

            GUILayoutY contentLayout = GUI.AddLayoutY();

            searchBarLayout = contentLayout.AddLayoutX();
            searchField = new GUITextField();
            searchField.OnChanged += OnSearchChanged;
            GUIButton clearSearchBtn = new GUIButton("C");
            clearSearchBtn.OnClick += ClearSearch;
            clearSearchBtn.SetWidth(40);
            optionsButton = new GUIButton("O");
            optionsButton.OnClick += OpenOptionsWindow;
            optionsButton.SetWidth(40);
            searchBarLayout.AddElement(searchField);
            searchBarLayout.AddElement(clearSearchBtn);
            searchBarLayout.AddElement(optionsButton);

            folderBarLayout = contentLayout.AddLayoutX();
            GUIButton homeButton = new GUIButton("H", GUIOption.FixedWidth(FOLDER_BUTTON_WIDTH));
            homeButton.OnClick += OnHomeClicked;
            GUIButton upButton = new GUIButton("U", GUIOption.FixedWidth(FOLDER_BUTTON_WIDTH));
            upButton.OnClick += OnUpClicked;

            folderBarLayout.AddElement(homeButton);
            folderBarLayout.AddElement(upButton);
            folderBarLayout.AddSpace(10);

            contentScrollArea = new GUIScrollArea(GUIOption.FlexibleWidth(), GUIOption.FlexibleHeight());
            contentLayout.AddElement(contentScrollArea);
            contentLayout.AddFlexibleSpace();

            entryContextMenu = LibraryMenu.CreateContextMenu(this);
            content = new LibraryGUIContent(this, contentScrollArea);
            
            Reset();

            dropTarget = new LibraryDropTarget(this);
            dropTarget.Bounds = contentScrollArea.Bounds;
            dropTarget.OnStart += OnDragStart;
            dropTarget.OnDrag += OnDragMove;
            dropTarget.OnLeave += OnDragLeave;
            dropTarget.OnDropResource += OnResourceDragDropped;
            dropTarget.OnDropSceneObject += OnSceneObjectDragDropped;
            dropTarget.OnEnd += OnDragEnd;

            Selection.OnSelectionChanged += OnSelectionChanged;
            Selection.OnResourcePing += OnPing;
        }

        private void OnDestroy()
        {
            Selection.OnSelectionChanged -= OnSelectionChanged;
            Selection.OnResourcePing -= OnPing;
        }

        private void OnEditorUpdate()
        {
            bool isRenameInProgress = inProgressRenameElement != null;

            if (HasContentFocus)
            {
                if (!isRenameInProgress)
                {
                    if (Input.IsButtonHeld(ButtonCode.LeftControl) || Input.IsButtonHeld(ButtonCode.RightControl))
                    {
                        if (Input.IsButtonUp(ButtonCode.C))
                        {
                            CopySelection();
                        }
                        else if (Input.IsButtonUp(ButtonCode.X))
                        {
                            CutSelection();
                        }
                        else if (Input.IsButtonUp(ButtonCode.D))
                        {
                            DuplicateSelection();
                        }
                        else if (Input.IsButtonUp(ButtonCode.V))
                        {
                            PasteToSelection();
                        }
                    }

                    if (Input.IsButtonDown(ButtonCode.Return))
                    {
                        if (selectionPaths.Count == 1)
                        {
                            LibraryEntry entry = ProjectLibrary.GetEntry(selectionPaths[0]);
                            if (entry != null && entry.Type == LibraryEntryType.Directory)
                            {
                                EnterDirectory(entry.Path);
                            }
                        }
                    }
                    else if (Input.IsButtonDown(ButtonCode.Back))
                    {
                        LibraryEntry entry = ProjectLibrary.GetEntry(currentDirectory);
                        if (entry != null && entry.Parent != null)
                        {
                            EnterDirectory(entry.Parent.Path);
                        }
                    }
                    else if (Input.IsButtonDown(ButtonCode.Up))
                    {
                        MoveSelection(MoveDirection.Up);
                    }
                    else if (Input.IsButtonDown(ButtonCode.Down))
                    {
                        MoveSelection(MoveDirection.Down);
                    }
                    else if (Input.IsButtonDown(ButtonCode.Left))
                    {
                        MoveSelection(MoveDirection.Left);
                    }
                    else if (Input.IsButtonDown(ButtonCode.Right))
                    {
                        MoveSelection(MoveDirection.Right);
                    }
                    else if (Input.IsButtonDown(ButtonCode.F2))
                    {
                        RenameSelection();
                    }
                    else if (Input.IsButtonDown(ButtonCode.Delete))
                    {
                        DeleteSelection();
                    }
                }
                else
                {
                    if (Input.IsButtonDown(ButtonCode.Return))
                    {
                        string newName = inProgressRenameElement.GetRenamedName();

                        string originalPath = inProgressRenameElement.path;
                        originalPath = originalPath.TrimEnd(Path.DirectorySeparatorChar, Path.AltDirectorySeparatorChar);

                        string newPath = Path.GetDirectoryName(originalPath);
                        newPath = Path.Combine(newPath, newName + Path.GetExtension(originalPath));

                        bool renameOK = true;
                        if (!PathEx.IsValidFileName(newName))
                        {
                            DialogBox.Open(new LocEdString("Error"), new LocEdString("The name you specified is not a valid file name. Try another."), DialogBox.Type.OK);
                            renameOK = false;
                        }

                        if (renameOK)
                        {
                            // Windows sees paths with dot at the end as if they didn't have it
                            // so remove the dot to ensure the project library does the same
                            string trimmedNewPath = newPath.TrimEnd('.');

                            if (originalPath != trimmedNewPath && ProjectLibrary.Exists(trimmedNewPath))
                            {
                                DialogBox.Open(new LocEdString("Error"), new LocEdString("File/folder with that name already exists in this folder."), DialogBox.Type.OK);
                                renameOK = false;
                            }
                        }

                        if (renameOK)
                        {
                            ProjectLibrary.Rename(originalPath, newPath);
                            StopRename();
                        }
                    }
                    else if (Input.IsButtonDown(ButtonCode.Escape))
                    {
                        StopRename();
                    }
                }
            }
            else
            {
                if (isRenameInProgress)
                    StopRename();
            }

            if (autoScrollAmount != 0)
            {
                Rect2I contentBounds = contentScrollArea.ContentBounds;
                float scrollPct = autoScrollAmount / (float)contentBounds.height;

                contentScrollArea.VerticalScroll += scrollPct * Time.FrameDelta;
            }

            if (requiresRefresh)
                Refresh();

            dropTarget.Update();
        }

        protected override LocString GetDisplayName()
        {
            return new LocEdString("Library");
        }

        protected override void WindowResized(int width, int height)
        {
            base.WindowResized(width, height);

            Refresh();

            dropTarget.Bounds = contentScrollArea.Bounds;
        }

        private LibraryGUIEntry FindElementAt(Vector2I windowPos)
        {
            Vector2I scrollPos = WindowToScrollAreaCoords(windowPos);

            return content.FindElementAt(scrollPos);
        }

        private void ClearHoverHighlight()
        {
            content.MarkAsHovered(hoverHighlightPath, false);
            hoverHighlightPath = "";
        }

        public void Ping(string path)
        {
            content.MarkAsPinged(pingPath, false);
            pingPath = path;
            content.MarkAsPinged(pingPath, true);
        }

        public void Reset()
        {
            currentDirectory = ProjectLibrary.Root.Path;
            selectionAnchorStart = -1;
            selectionAnchorEnd = -1;
            selectionPaths.Clear();
            pingPath = "";
            hoverHighlightPath = "";

            Refresh();
        }

        internal void DeselectAll(bool onlyInternal = false)
        {
            SetSelection(new List<string>(), onlyInternal);
            selectionAnchorStart = -1;
            selectionAnchorEnd = -1;
        }

        internal void Select(string path)
        {
            LibraryGUIEntry entry;
            if (!content.TryGetEntry(path, out entry))
                return;

            bool ctrlDown = Input.IsButtonHeld(ButtonCode.LeftControl) || Input.IsButtonHeld(ButtonCode.RightControl);
            bool shiftDown = Input.IsButtonHeld(ButtonCode.LeftShift) || Input.IsButtonHeld(ButtonCode.RightShift);

            if (shiftDown)
            {
                if (selectionAnchorStart != -1 && selectionAnchorStart < content.Entries.Length)
                {
                    int start = Math.Min(entry.index, selectionAnchorStart);
                    int end = Math.Max(entry.index, selectionAnchorStart);

                    List<string> newSelection = new List<string>();
                    for(int i = start; i <= end; i++)
                        newSelection.Add(content.Entries[i].path);

                    SetSelection(newSelection);
                    selectionAnchorEnd = entry.index;
                }
                else
                {
                    SetSelection(new List<string>() {path});
                    selectionAnchorStart = entry.index;
                    selectionAnchorEnd = entry.index;
                }
            }
            else if (ctrlDown)
            {
                List<string> newSelection = new List<string>(selectionPaths);

                if (selectionPaths.Contains(path))
                {
                    newSelection.Remove(path);
                    if (newSelection.Count == 0)
                        DeselectAll();
                    else
                    {
                        if (selectionAnchorStart == entry.index)
                        {
                            LibraryGUIEntry newAnchorEntry;
                            if (!content.TryGetEntry(newSelection[0], out newAnchorEntry))
                                selectionAnchorStart = -1;
                            else
                                selectionAnchorStart = newAnchorEntry.index;
                        }

                        if (selectionAnchorEnd == entry.index)
                        {
                            LibraryGUIEntry newAnchorEntry;
                            if (!content.TryGetEntry(newSelection[newSelection.Count - 1], out newAnchorEntry))
                                selectionAnchorEnd = -1;
                            else
                                selectionAnchorEnd = newAnchorEntry.index;
                        }

                        SetSelection(newSelection);
                    }
                }
                else
                {
                    newSelection.Add(path);
                    SetSelection(newSelection);
                    selectionAnchorEnd = entry.index;
                }
            }
            else
            {
                SetSelection(new List<string>() {path});

                selectionAnchorStart = entry.index;
                selectionAnchorEnd = entry.index;
            }
        }

        internal void MoveSelection(MoveDirection dir)
        {
            string newPath = "";

            if (selectionPaths.Count == 0 || selectionAnchorEnd == -1)
            {
                // Nothing is selected so we arbitrarily select first or last element
                if (content.Entries.Length > 0)
                {
                    switch (dir)
                    {
                        case MoveDirection.Left:
                        case MoveDirection.Up:
                            newPath = content.Entries[content.Entries.Length - 1].path;
                            break;
                        case MoveDirection.Right:
                        case MoveDirection.Down:
                            newPath = content.Entries[0].path;
                            break;
                    }
                }
            }
            else
            {
                switch (dir)
                {
                    case MoveDirection.Left:
                        if (selectionAnchorEnd - 1 >= 0)
                            newPath = content.Entries[selectionAnchorEnd - 1].path;
                        break;
                    case MoveDirection.Up:
                        if (selectionAnchorEnd - content.ElementsPerRow >= 0)
                            newPath = content.Entries[selectionAnchorEnd - content.ElementsPerRow].path;
                        break;
                    case MoveDirection.Right:
                        if (selectionAnchorEnd + 1 < content.Entries.Length)
                            newPath = content.Entries[selectionAnchorEnd + 1].path;
                        break;
                    case MoveDirection.Down:
                        if (selectionAnchorEnd + content.ElementsPerRow < content.Entries.Length)
                            newPath = content.Entries[selectionAnchorEnd + content.ElementsPerRow].path;
                        break;
                }
            }

            if (!string.IsNullOrEmpty(newPath))
            {
                Select(newPath);
                ScrollToEntry(newPath);
            }
        }

        internal void SetSelection(List<string> paths, bool onlyInternal = false)
        {
            if (selectionPaths != null)
            {
                foreach (var path in selectionPaths)
                    content.MarkAsSelected(path, false);
            }

            selectionPaths = paths;

            if (selectionPaths != null)
            {
                foreach (var path in selectionPaths)
                    content.MarkAsSelected(path, true);
            }

            Ping("");
            StopRename();

            if (!onlyInternal)
            {
                if (selectionPaths != null)
                    Selection.resourcePaths = selectionPaths.ToArray();
                else
                    Selection.resourcePaths = new string[0];
            }
        }

        internal void EnterDirectory(string directory)
        {
            currentDirectory = directory;
            DeselectAll();

            Refresh();
        }

        internal void Cut(IEnumerable<string> sourcePaths)
        {
            foreach (var path in cutPaths)
                content.MarkAsCut(path, false);

            cutPaths.Clear();
            cutPaths.AddRange(sourcePaths);

            foreach (var path in cutPaths)
                content.MarkAsCut(path, true);

            copyPaths.Clear();
        }

        internal void Copy(IEnumerable<string> sourcePaths)
        {
            copyPaths.Clear();
            copyPaths.AddRange(sourcePaths);

            foreach (var path in cutPaths)
                content.MarkAsCut(path, false);

            cutPaths.Clear();
        }

        internal void Duplicate(IEnumerable<string> sourcePaths)
        {
            foreach (var source in sourcePaths)
            {
                if (Directory.Exists(source))
                    DirectoryEx.Copy(source, LibraryUtility.GetUniquePath(source));
                else if (File.Exists(source))
                    FileEx.Copy(source, LibraryUtility.GetUniquePath(source));

                ProjectLibrary.Refresh();
            }
        }

        internal void Paste(string destinationFolder)
        {
            if (copyPaths.Count > 0)
            {
                for (int i = 0; i < copyPaths.Count; i++)
                {
                    string destination = Path.Combine(destinationFolder, PathEx.GetTail(copyPaths[i]));

                    if (Directory.Exists(copyPaths[i]))
                        DirectoryEx.Copy(copyPaths[i], LibraryUtility.GetUniquePath(destination));
                    else if (File.Exists(copyPaths[i]))
                        FileEx.Copy(copyPaths[i], LibraryUtility.GetUniquePath(destination));
                }

                ProjectLibrary.Refresh();
            }
            else if (cutPaths.Count > 0)
            {
                for (int i = 0; i < cutPaths.Count; i++)
                {
                    string destination = Path.Combine(destinationFolder, PathEx.GetTail(cutPaths[i]));

                    if (Directory.Exists(cutPaths[i]))
                        DirectoryEx.Move(cutPaths[i], LibraryUtility.GetUniquePath(destination));
                    else if (File.Exists(cutPaths[i]))
                        FileEx.Move(cutPaths[i], LibraryUtility.GetUniquePath(destination));
                }

                cutPaths.Clear();
                ProjectLibrary.Refresh();
            }
        }

        private void ScrollToEntry(string path)
        {
            LibraryGUIEntry entryGUI;
            if (!content.TryGetEntry(path, out entryGUI))
                return;

            Rect2I entryBounds = entryGUI.Bounds;

            Rect2I contentBounds = contentScrollArea.Layout.Bounds;
            Rect2I windowEntryBounds = entryBounds;
            windowEntryBounds.x += contentBounds.x;
            windowEntryBounds.y += contentBounds.y;

            Rect2I scrollAreaBounds = contentScrollArea.Bounds;
            bool requiresScroll = windowEntryBounds.y < scrollAreaBounds.y ||
                                  (windowEntryBounds.y + windowEntryBounds.height) > (scrollAreaBounds.y + scrollAreaBounds.height);

            if (!requiresScroll)
                return;

            int scrollableSize = contentBounds.height - scrollAreaBounds.height;
            float percent = (((entryBounds.y + entryBounds.height * 0.5f) - scrollAreaBounds.height * 0.5f) / (float)scrollableSize);

            percent = MathEx.Clamp01(percent);
            contentScrollArea.VerticalScroll = percent;
        }

        private void Refresh()
        {
            requiresRefresh = false;

            LibraryEntry[] entriesToDisplay = new LibraryEntry[0];
            if (IsSearchActive)
            {
                entriesToDisplay = ProjectLibrary.Search("*" + searchQuery + "*");
            }
            else
            {
                DirectoryEntry entry = ProjectLibrary.GetEntry(currentDirectory) as DirectoryEntry;
                if (entry == null)
                {
                    currentDirectory = ProjectLibrary.Root.Path;
                    entry = ProjectLibrary.GetEntry(currentDirectory) as DirectoryEntry;
                }

                if(entry != null)
                    entriesToDisplay = entry.Children;
            }

            inProgressRenameElement = null;

            RefreshDirectoryBar();

            SortEntries(entriesToDisplay);
            content.Refresh(viewType, entriesToDisplay);

            if (entriesToDisplay.Length == 0)
                return;

            foreach (var path in cutPaths)
                content.MarkAsCut(path, true);

            foreach (var path in selectionPaths)
                content.MarkAsSelected(path, true);

            content.MarkAsPinged(pingPath, true);

            Rect2I contentBounds = content.Bounds;
            Rect2I minimalBounds = GetScrollAreaBounds();
            contentBounds.height = Math.Max(contentBounds.height, minimalBounds.height);

            GUIButton catchAll = new GUIButton("", EditorStyles.Blank);
            catchAll.Bounds = contentBounds;
            catchAll.OnClick += OnCatchAllClicked;
            catchAll.SetContextMenu(entryContextMenu);

            content.Underlay.AddElement(catchAll);

            Rect2I focusBounds = contentBounds; // Contents + Folder bar
            Rect2I scrollBounds = contentScrollArea.Bounds;
            focusBounds.x += scrollBounds.x;
            focusBounds.y += scrollBounds.y;

            Rect2I folderBarBounds = folderListLayout.Bounds;
            focusBounds.y -= folderBarBounds.height;
            focusBounds.height += folderBarBounds.height;

            GUIButton focusCatcher = new GUIButton("", EditorStyles.Blank);
            focusCatcher.OnFocusChanged += OnContentsFocusChanged;
            focusCatcher.Bounds = focusBounds;

            GUIPanel focusPanel = GUI.AddPanel(3);
            focusPanel.AddElement(focusCatcher);

            UpdateDragSelection(dragSelectionEnd);
        }

        private Vector2I WindowToScrollAreaCoords(Vector2I windowPos)
        {
            Rect2I scrollBounds = contentScrollArea.Layout.Bounds;
            Vector2I scrollPos = windowPos;
            scrollPos.x -= scrollBounds.x;
            scrollPos.y -= scrollBounds.y;

            return scrollPos;
        }

        private void StartDragSelection(Vector2I windowPos)
        {
            isDraggingSelection = true;
            dragSelectionStart = WindowToScrollAreaCoords(windowPos);
            dragSelectionEnd = dragSelectionStart;
        }

        private bool UpdateDragSelection(Vector2I windowPos)
        {
            if (!isDraggingSelection)
                return false;

            if (dragSelection == null)
            {
                dragSelection = new GUITexture(null, true, EditorStyles.SelectionArea);
                content.Overlay.AddElement(dragSelection);
            }

            dragSelectionEnd = WindowToScrollAreaCoords(windowPos);

            Rect2I selectionArea = CalculateSelectionArea();
            SelectInArea(selectionArea);
            dragSelection.Bounds = selectionArea;

            return true;
        }

        private bool EndDragSelection()
        {
            if (!isDraggingSelection)
                return false;

            if (dragSelection != null)
            {
                dragSelection.Destroy();
                dragSelection = null;
            }

            Rect2I selectionArea = CalculateSelectionArea();
            SelectInArea(selectionArea);

            isDraggingSelection = false;
            return false;
        }

        private Rect2I CalculateSelectionArea()
        {
            Rect2I selectionArea = new Rect2I();
            if (dragSelectionStart.x < dragSelectionEnd.x)
            {
                selectionArea.x = dragSelectionStart.x;
                selectionArea.width = dragSelectionEnd.x - dragSelectionStart.x;
            }
            else
            {
                selectionArea.x = dragSelectionEnd.x;
                selectionArea.width = dragSelectionStart.x - dragSelectionEnd.x;
            }

            if (dragSelectionStart.y < dragSelectionEnd.y)
            {
                selectionArea.y = dragSelectionStart.y;
                selectionArea.height = dragSelectionEnd.y - dragSelectionStart.y;
            }
            else
            {
                selectionArea.y = dragSelectionEnd.y;
                selectionArea.height = dragSelectionStart.y - dragSelectionEnd.y;
            }

            Rect2I maxBounds = contentScrollArea.Layout.Bounds;
            maxBounds.x = 0;
            maxBounds.y = 0;

            selectionArea.Clip(maxBounds);

            return selectionArea;
        }

        private void SelectInArea(Rect2I scrollBounds)
        {
            LibraryGUIEntry[] foundElements = content.FindElementsOverlapping(scrollBounds);

            if (foundElements.Length > 0)
            {
                selectionAnchorStart = foundElements[0].index;
                selectionAnchorEnd = foundElements[foundElements.Length - 1].index;
            }
            else
            {
                selectionAnchorStart = -1;
                selectionAnchorEnd = -1;
            }

            List<string> elementPaths = new List<string>();
            foreach (var elem in foundElements)
                elementPaths.Add(elem.path);

            SetSelection(elementPaths);
        }

        private void RefreshDirectoryBar()
        {
            if (folderListLayout != null)
            {
                folderListLayout.Destroy();
                folderListLayout = null;
            }

            folderListLayout = folderBarLayout.AddLayoutX();

            string[] folders = null;
            string[] fullPaths = null;

            if (IsSearchActive)
            {
                folders = new[] {searchQuery};
                fullPaths = new[] { searchQuery };
            }
            else
            {
                string currentDir = Path.Combine("Resources", currentDirectory);

                folders = currentDir.Split(new[] { Path.DirectorySeparatorChar, Path.AltDirectorySeparatorChar },
                    StringSplitOptions.RemoveEmptyEntries);

                fullPaths = new string[folders.Length];
                for (int i = 0; i < folders.Length; i++)
                {
                    if (i == 0)
                        fullPaths[i] = "";
                    else
                        fullPaths[i] = Path.Combine(fullPaths[i - 1], folders[i]);
                }
            }

            int availableWidth = folderBarLayout.Bounds.width - FOLDER_BUTTON_WIDTH * 2;
            int numFolders = 0;
            for (int i = folders.Length - 1; i >= 0; i--)
            {
                GUIButton folderButton = new GUIButton(folders[i]);

                if (!IsSearchActive)
                {
                    string fullPath = fullPaths[i];
                    folderButton.OnClick += () => OnFolderButtonClicked(fullPath);
                }

                GUIButton separator = new GUIButton("/", GUIOption.FixedWidth(FOLDER_SEPARATOR_WIDTH));

                folderListLayout.InsertElement(0, separator);
                folderListLayout.InsertElement(0, folderButton);
                numFolders++;

                Rect2I folderListBounds = folderListLayout.Bounds;
                if (folderListBounds.width > availableWidth)
                {
                    if (numFolders > 2)
                    {
                        separator.Destroy();
                        folderButton.Destroy();
                        break;
                    }
                }
            }
        }

        internal void CutSelection()
        {
            if (selectionPaths.Count > 0)
                Cut(selectionPaths);
        }

        internal void CopySelection()
        {
            if (selectionPaths.Count > 0)
                Copy(selectionPaths);
        }

        internal void DuplicateSelection()
        {
            if (selectionPaths.Count > 0)
                Duplicate(selectionPaths);
        }

        internal void PasteToSelection()
        {
            Paste(SelectedFolder);
        }

        internal void RenameSelection()
        {
            if (selectionPaths.Count == 0)
                return;

            if (selectionPaths.Count > 1)
            {
                DeselectAll();
                Select(selectionPaths[0]);
            }

            LibraryGUIEntry entry;
            if (content.TryGetEntry(selectionPaths[0], out entry))
            {
                entry.StartRename();
                inProgressRenameElement = entry;
            }
        }

        internal void DeleteSelection()
        {
            if (selectionPaths.Count == 0)
                return;

            DialogBox.Open(new LocEdString("Confirm deletion"), new LocEdString("Are you sure you want to delete the selected object(s)?"),
                DialogBox.Type.YesNo,
                type =>
                {
                    if (type == DialogBox.ResultType.Yes)
                    {
                        foreach (var path in selectionPaths)
                        {
                            ProjectLibrary.Delete(path);
                        }

                        DeselectAll();
                        Refresh();
                    }
                });
        }

        internal void StopRename()
        {
            if (inProgressRenameElement != null)
            {
                inProgressRenameElement.StopRename();
                inProgressRenameElement = null;
            }
        }

        private void ClearSearch()
        {
            searchField.Value = "";
            searchQuery = "";
            Refresh();
        }

        private void OpenOptionsWindow()
        {
            Vector2I openPosition;
            Rect2I buttonBounds = GUILayoutUtility.CalculateBounds(optionsButton, GUI);

            openPosition.x = buttonBounds.x + buttonBounds.width / 2;
            openPosition.y = buttonBounds.y + buttonBounds.height / 2;

            LibraryDropDown dropDown = DropDownWindow.Open<LibraryDropDown>(this, openPosition);
            dropDown.SetParent(this);
        }

        private Rect2I GetScrollAreaBounds()
        {
            Rect2I bounds = GUI.Bounds;
            Rect2I folderListBounds = folderListLayout.Bounds;
            Rect2I searchBarBounds = searchBarLayout.Bounds;

            bounds.y += folderListBounds.height + searchBarBounds.height;
            bounds.height -= folderListBounds.height + searchBarBounds.height;

            return bounds;
        }

        private void OnEntryChanged(string entry)
        {
            requiresRefresh = true;
        }

        private void OnDragStart(Vector2I windowPos)
        {
            LibraryGUIEntry underCursorElem = FindElementAt(windowPos);
            if (underCursorElem == null)
            {
                StartDragSelection(windowPos);
                return;
            }

            string resourceDir = ProjectLibrary.ResourceFolder;

            string[] dragPaths = null;
            if (selectionPaths.Count > 0)
            {
                foreach (var path in selectionPaths)
                {
                    if (path == underCursorElem.path)
                    {
                        dragPaths = new string[selectionPaths.Count];
                        for (int i = 0; i < selectionPaths.Count; i++)
                        {
                            dragPaths[i] = Path.Combine(resourceDir, selectionPaths[i]);
                        }

                        break;
                    }
                }
            }

            if (dragPaths == null)
                dragPaths = new[] { Path.Combine(resourceDir, underCursorElem.path) };

            ResourceDragDropData dragDropData = new ResourceDragDropData(dragPaths);
            DragDrop.StartDrag(dragDropData);
        }

        private void OnDragMove(Vector2I windowPos)
        {
            // Auto-scroll
            Rect2I scrollAreaBounds = contentScrollArea.Bounds;
            int scrollAreaTop = scrollAreaBounds.y;
            int scrollAreaBottom = scrollAreaBounds.y + scrollAreaBounds.height;

            if (windowPos.y > scrollAreaTop && windowPos.y <= (scrollAreaTop + DRAG_SCROLL_HEIGHT))
                autoScrollAmount = -DRAG_SCROLL_AMOUNT_PER_SECOND;
            else if (windowPos.y >= (scrollAreaBottom - DRAG_SCROLL_HEIGHT) && windowPos.y < scrollAreaBottom)
                autoScrollAmount = DRAG_SCROLL_AMOUNT_PER_SECOND;
            else
                autoScrollAmount = 0;

            // Selection box
            if (UpdateDragSelection(windowPos))
                return;

            // Drag and drop (hover element under cursor)
            LibraryGUIEntry underCursorElem = FindElementAt(windowPos);

            if (underCursorElem == null)
            {
                ClearHoverHighlight();
            }
            else
            {
                if (underCursorElem.path != hoverHighlightPath)
                {
                    ClearHoverHighlight();

                    hoverHighlightPath = underCursorElem.path;
                    underCursorElem.MarkAsHovered(true);
                }
            }
        }

        private void OnDragLeave()
        {
            ClearHoverHighlight();
            autoScrollAmount = 0;
        }

        private void OnResourceDragDropped(Vector2I windowPos, string[] paths)
        {
            ClearHoverHighlight();
            autoScrollAmount = 0;

            if (EndDragSelection())
                return;

            string resourceDir = ProjectLibrary.ResourceFolder;
            string destinationFolder = Path.Combine(resourceDir, currentDirectory);

            LibraryGUIEntry underCursorElement = FindElementAt(windowPos);
            if (underCursorElement != null)
            {
                LibraryEntry entry = ProjectLibrary.GetEntry(underCursorElement.path);
                if (entry != null && entry.Type == LibraryEntryType.Directory)
                    destinationFolder = Path.Combine(resourceDir, entry.Path);
            }

            if (paths != null)
            {
                foreach (var path in paths)
                {
                    if (path == null)
                        continue;

                    string absolutePath = path;
                    if (!Path.IsPathRooted(absolutePath))
                        absolutePath = Path.Combine(resourceDir, path);

                    if (string.IsNullOrEmpty(absolutePath))
                        continue;

                    if (PathEx.IsPartOf(destinationFolder, absolutePath) || PathEx.Compare(absolutePath, destinationFolder))
                        continue;

                    string pathTail = PathEx.GetTail(absolutePath);
                    string destination = Path.Combine(destinationFolder, pathTail);

                    bool doCopy = !ProjectLibrary.Exists(path);

                    if (Directory.Exists(path))
                    {
                        if (doCopy)
                            DirectoryEx.Copy(path, LibraryUtility.GetUniquePath(destination));
                        else
                            DirectoryEx.Move(path, LibraryUtility.GetUniquePath(destination));
                    }
                    else if (File.Exists(path))
                    {
                        if (doCopy)
                            FileEx.Copy(path, LibraryUtility.GetUniquePath(destination));
                        else
                            FileEx.Move(path, LibraryUtility.GetUniquePath(destination));
                    }

                    ProjectLibrary.Refresh();
                }
            }
        }

        private void OnSceneObjectDragDropped(Vector2I windowPos, SceneObject[] objects)
        {
            ClearHoverHighlight();
            autoScrollAmount = 0;

            if (EndDragSelection())
                return;

            string destinationFolder = currentDirectory;

            LibraryGUIEntry underCursorElement = FindElementAt(windowPos);
            if (underCursorElement != null)
            {
                LibraryEntry entry = ProjectLibrary.GetEntry(underCursorElement.path);
                if (entry != null && entry.Type == LibraryEntryType.Directory)
                    destinationFolder = entry.Path;
            }

            if (objects != null)
            {
                foreach (var so in objects)
                {
                    if (so == null)
                        continue;

                    Prefab newPrefab = new Prefab(so);

                    string destination = LibraryUtility.GetUniquePath(Path.Combine(destinationFolder, so.Name + ".prefab"));
                    ProjectLibrary.Create(newPrefab, destination);

                    ProjectLibrary.Refresh();
                }
            }
        }

        private void OnDragEnd(Vector2I windowPos)
        {
            EndDragSelection();
            autoScrollAmount = 0;
        }

        private void OnSelectionChanged(SceneObject[] sceneObjects, string[] resourcePaths)
        {
            if(sceneObjects.Length > 0)
                DeselectAll(true);
        }

        private void OnPing(string path)
        {
            Ping(path);
        }

        private void OnFolderButtonClicked(string path)
        {
            EnterDirectory(path);
        }

        private void OnContentsFocusChanged(bool focus)
        {
            hasContentFocus = focus;
        }

        private void OnCatchAllClicked()
        {
            DeselectAll();
        }

        private void OnHomeClicked()
        {
            currentDirectory = ProjectLibrary.Root.Path;
            Refresh();
        }

        private void OnUpClicked()
        {
            currentDirectory = currentDirectory.Trim(Path.DirectorySeparatorChar, Path.AltDirectorySeparatorChar);

            if (!string.IsNullOrEmpty(currentDirectory))
            {
                string parent = Path.GetDirectoryName(currentDirectory);

                currentDirectory = parent;
                Refresh();
            }
        }

        private void OnSearchChanged(string newValue)
        {
            searchQuery = newValue;
            Refresh();
        }

        private static void SortEntries(LibraryEntry[] input)
        {
            Array.Sort(input, (x, y) =>
            {
                if (x.Type == y.Type)
                    return x.Name.CompareTo(y.Name);
                else
                    return x.Type == LibraryEntryType.File ? 1 : -1;
            });
        }
    }
}
