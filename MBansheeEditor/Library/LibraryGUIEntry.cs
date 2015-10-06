﻿using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using BansheeEngine;

namespace BansheeEditor
{
    /// <summary>
    /// Represents GUI for a single resource tile used in <see cref="LibraryGUIContent"/>.
    /// </summary>
    internal class LibraryGUIEntry
    {
        private const int MAX_LABEL_HEIGHT = 50;

        private static readonly Color PING_COLOR = Color.BansheeOrange;
        private static readonly Color SELECTION_COLOR = Color.DarkCyan;
        private static readonly Color HOVER_COLOR = new Color(Color.DarkCyan.r, Color.DarkCyan.g, Color.DarkCyan.b, 0.5f);
        private static readonly Color CUT_COLOR = new Color(1.0f, 1.0f, 1.0f, 0.5f);
        private const int SELECTION_EXTRA_WIDTH = 3;

        /// <summary>
        /// Possible visual states for the resource tile.
        /// </summary>
        enum UnderlayState // Note: Order of these is relevant
        {
            None, Hovered, Selected, Pinged
        }

        public int index;
        public string path;
        public GUITexture icon;
        public GUILabel label;
        public Rect2I bounds;

        private GUITexture underlay;
        private LibraryGUIContent owner;
        private UnderlayState underlayState;
        private GUITextBox renameTextBox;

        /// <summary>
        /// Constructs a new resource tile entry.
        /// </summary>
        /// <param name="owner">Content area this entry is part of.</param>
        /// <param name="parent">Parent layout to add this entry's GUI elements to.</param>
        /// <param name="entry">Project library entry this entry displays data for.</param>
        /// <param name="index">Sequential index of the entry in the conent area.</param>
        /// <param name="labelWidth">Width of the GUI labels that display the elements.</param>
        public LibraryGUIEntry(LibraryGUIContent owner, GUILayout parent, LibraryEntry entry, int index, int labelWidth)
        {
            GUILayout entryLayout;

            if (owner.GridLayout)
                entryLayout = parent.AddLayoutY();
            else
                entryLayout = parent.AddLayoutX();

            SpriteTexture iconTexture = GetIcon(entry);

            icon = new GUITexture(iconTexture, GUIImageScaleMode.ScaleToFit,
                true, GUIOption.FixedHeight(owner.TileSize), GUIOption.FixedWidth(owner.TileSize));

            label = null;

            if (owner.GridLayout)
            {
                label = new GUILabel(entry.Name, EditorStyles.MultiLineLabelCentered,
                    GUIOption.FixedWidth(labelWidth), GUIOption.FlexibleHeight(0, MAX_LABEL_HEIGHT));
            }
            else
            {
                label = new GUILabel(entry.Name);
            }

            entryLayout.AddElement(icon);
            entryLayout.AddElement(label);

            this.owner = owner;
            this.index = index;
            this.path = entry.Path;
            this.bounds = new Rect2I();
            this.underlay = null;
        }

        /// <summary>
        /// Positions the GUI elements. Must be called after construction, but only after all content area entries have
        /// been constructed so that entry's final bounds are known.
        /// </summary>
        public void Initialize()
        {
            bounds = icon.Bounds;
            Rect2I labelBounds = label.Bounds;

            bounds.x = MathEx.Min(bounds.x, labelBounds.x - SELECTION_EXTRA_WIDTH);
            bounds.y = MathEx.Min(bounds.y, labelBounds.y) - 5; // 5 - Just padding for better look
            bounds.width = MathEx.Max(bounds.x + bounds.width,
                labelBounds.x + labelBounds.width) - bounds.x + SELECTION_EXTRA_WIDTH;
            bounds.height = MathEx.Max(bounds.y + bounds.height,
                labelBounds.y + labelBounds.height) - bounds.y;

            string hoistedPath = path;

            GUIButton overlayBtn = new GUIButton("", EditorStyles.Blank);
            overlayBtn.Bounds = bounds;
            overlayBtn.OnClick += () => OnEntryClicked(hoistedPath);
            overlayBtn.OnDoubleClick += () => OnEntryDoubleClicked(hoistedPath);
            overlayBtn.SetContextMenu(owner.Window.ContextMenu);

            owner.Overlay.AddElement(overlayBtn);
        }

        /// <summary>
        /// Bounds of the entry relative to part content area.
        /// </summary>
        public Rect2I Bounds
        {
            get { return bounds; }
        }

        /// <summary>
        /// Changes the visual representation of the element as being cut.
        /// </summary>
        /// <param name="enable">True if mark as cut, false to reset to normal.</param>
        public void MarkAsCut(bool enable)
        {
            if (enable)
                icon.SetTint(CUT_COLOR);
            else
                icon.SetTint(Color.White);
        }

        /// <summary>
        /// Changes the visual representation of the element as being selected.
        /// </summary>
        /// <param name="enable">True if mark as selected, false to reset to normal.</param>
        public void MarkAsSelected(bool enable)
        {
            if ((int)underlayState > (int)UnderlayState.Selected)
                return;

            if (enable)
            {
                CreateUnderlay();
                underlay.SetTint(SELECTION_COLOR);
            }
            else
                ClearUnderlay();

            underlayState = UnderlayState.Selected;
        }

        /// <summary>
        /// Changes the visual representation of the element as being pinged.
        /// </summary>
        /// <param name="enable">True if mark as pinged, false to reset to normal.</param>
        public void MarkAsPinged(bool enable)
        {
            if ((int)underlayState > (int)UnderlayState.Pinged)
                return;

            if (enable)
            {
                CreateUnderlay();
                underlay.SetTint(PING_COLOR);
            }
            else
                ClearUnderlay();

            underlayState = UnderlayState.Pinged;
        }

        /// <summary>
        /// Changes the visual representation of the element as being hovered over.
        /// </summary>
        /// <param name="enable">True if mark as hovered, false to reset to normal.</param>
        public void MarkAsHovered(bool enable)
        {
            if ((int)underlayState > (int)UnderlayState.Hovered)
                return;

            if (enable)
            {
                CreateUnderlay();
                underlay.SetTint(HOVER_COLOR);
            }
            else
                ClearUnderlay();

            underlayState = UnderlayState.Hovered;
        }

        /// <summary>
        /// Starts a rename operation over the entry, displaying the rename input box.
        /// </summary>
        public void StartRename()
        {
            if (renameTextBox != null)
                return;

            renameTextBox = new GUITextBox(false);
            renameTextBox.Bounds = label.Bounds;
            owner.RenameOverlay.AddElement(renameTextBox);

            string name = Path.GetFileNameWithoutExtension(PathEx.GetTail(path));
            renameTextBox.Text = name;
            renameTextBox.Focus = true;

            label.Active = false;
        }

        /// <summary>
        /// Stops a rename operation over the entry, hiding the rename input box.
        /// </summary>
        public void StopRename()
        {
            if (renameTextBox != null)
            {
                renameTextBox.Destroy();
                renameTextBox = null;
            }

            label.Active = true;
        }

        /// <summary>
        /// Gets the new name of the entry. Only valid while a rename operation is in progress.
        /// </summary>
        /// <returns>New name of the entry currently entered in the rename input box.</returns>
        public string GetRenamedName()
        {
            if (renameTextBox != null)
                return renameTextBox.Text;

            return "";
        }

        /// <summary>
        /// Clears the underlay GUI element (e.g. ping, hover, select).
        /// </summary>
        private void ClearUnderlay()
        {
            if (underlay != null)
            {
                underlay.Destroy();
                underlay = null;
            }

            underlayState = UnderlayState.None;
        }

        /// <summary>
        /// Creates a GUI elements that may be used for underlay effects (e.g. ping, hover, select).
        /// </summary>
        private void CreateUnderlay()
        {
            if (underlay == null)
            {
                underlay = new GUITexture(Builtin.WhiteTexture);
                underlay.Bounds = Bounds;

                owner.Underlay.AddElement(underlay);
            }
        }

        /// <summary>
        /// Triggered when the user clicks on the entry.
        /// </summary>
        /// <param name="path">Project library path of the clicked entry.</param>
        private void OnEntryClicked(string path)
        {
            owner.Window.Select(path);
        }

        /// <summary>
        /// Triggered when the user double-clicked on the entry.
        /// </summary>
        /// <param name="path">Project library path of the double-clicked entry.</param>
        private void OnEntryDoubleClicked(string path)
        {
            LibraryEntry entry = ProjectLibrary.GetEntry(path);
            if (entry != null)
            {
                if (entry.Type == LibraryEntryType.Directory)
                    owner.Window.EnterDirectory(path);
                else
                {
                    FileEntry resEntry = (FileEntry)entry;
                    if (resEntry.ResType == ResourceType.Prefab)
                    {
                        EditorApplication.LoadScene(resEntry.Path);
                    }
                }
            }
        }

        /// <summary>
        /// Returns an icon that can be used for displaying a resource of the specified type.
        /// </summary>
        /// <param name="entry">Project library entry of the resource to retrieve icon for.</param>
        /// <returns>Icon to display for the specified entry.</returns>
        private static SpriteTexture GetIcon(LibraryEntry entry)
        {
            if (entry.Type == LibraryEntryType.Directory)
            {
                return EditorBuiltin.FolderIcon;
            }
            else
            {
                FileEntry fileEntry = (FileEntry)entry;
                switch (fileEntry.ResType)
                {
                    case ResourceType.Font:
                        return EditorBuiltin.FontIcon;
                    case ResourceType.Mesh:
                        return EditorBuiltin.MeshIcon;
                    case ResourceType.Texture:
                        return EditorBuiltin.TextureIcon;
                    case ResourceType.PlainText:
                        return EditorBuiltin.PlainTextIcon;
                    case ResourceType.ScriptCode:
                        return EditorBuiltin.ScriptCodeIcon;
                    case ResourceType.SpriteTexture:
                        return EditorBuiltin.SpriteTextureIcon;
                    case ResourceType.Shader:
                        return EditorBuiltin.ShaderIcon;
                    case ResourceType.Material:
                        return EditorBuiltin.MaterialIcon;
                    case ResourceType.Prefab:
                        return EditorBuiltin.PrefabIcon;
                }
            }

            return null;
        }
    }
}
