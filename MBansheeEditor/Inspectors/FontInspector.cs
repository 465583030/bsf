﻿using System;
using System.Collections.Generic;
using BansheeEngine;

namespace BansheeEditor
{
    /// <summary>
    /// Renders an inspector for the <see cref="Font"/> resource.
    /// </summary>
    [CustomInspector(typeof(Font))]
    internal class FontInspector : Inspector
    {
        private GUIArrayField<int, FontSizeArrayRow> fontSizes;
        private GUIArrayField<CharRange, CharRangeArrayRow> charRanges;
        private GUIToggleField antialiasingField;
        private GUIIntField dpiField;
        private GUIButton reimportButton;

        private FontImportOptions importOptions;

        /// <inheritdoc/>
        protected internal override void Initialize()
        {
            if (InspectedObject != null)
            {
                importOptions = GetImportOptions();
                BuildGUI();
            }
        }

        /// <inheritdoc/>
        protected internal override void Refresh()
        {
            FontImportOptions newImportOptions = GetImportOptions();

            bool rebuildGUI = false;

            int[] newFontSizes = newImportOptions.FontSizes;
            if (newFontSizes == null)
                rebuildGUI |= fontSizes.Array != null;
            else
            {
                if (fontSizes.Array == null)
                    rebuildGUI = true;
                else
                    rebuildGUI |= newFontSizes.Length != fontSizes.Array.GetLength(0);
            }

            CharRange[] newCharRanges = newImportOptions.CharRanges;
            if (newCharRanges == null)
                rebuildGUI |= charRanges.Array != null;
            else
            {
                if (charRanges.Array == null)
                    rebuildGUI = true;
                else
                    rebuildGUI |= newCharRanges.Length != charRanges.Array.GetLength(0);
            }

            if (rebuildGUI)
                BuildGUI();

            fontSizes.Refresh();
            charRanges.Refresh();

            antialiasingField.Value = newImportOptions.Antialiasing;
            dpiField.Value = newImportOptions.DPI;
            importOptions = newImportOptions;
        }

        /// <summary>
        /// Recreates all the GUI elements used by this inspector.
        /// </summary>
        private void BuildGUI()
        {
            Layout.Clear();

            fontSizes = GUIArrayField<int, FontSizeArrayRow>.Create(
                new LocEdString("Font sizes"), importOptions.FontSizes, Layout);
            fontSizes.OnChanged += x => importOptions.FontSizes = x;

            charRanges = GUIArrayField<CharRange, CharRangeArrayRow>.Create(
                new LocEdString("Character ranges"), importOptions.CharRanges, Layout);
            charRanges.OnChanged += x => importOptions.CharRanges = x;

            antialiasingField = new GUIToggleField(new LocEdString("Antialiasing"));
            dpiField = new GUIIntField(new LocEdString("DPI"));

            reimportButton = new GUIButton(new LocEdString("Reimport"));
            reimportButton.OnClick += TriggerReimport;

            Layout.AddElement(antialiasingField);
            Layout.AddElement(dpiField);
            Layout.AddSpace(10);

            GUILayout reimportButtonLayout = Layout.AddLayoutX();
            reimportButtonLayout.AddFlexibleSpace();
            reimportButtonLayout.AddElement(reimportButton);
        }

        /// <summary>
        /// Retrieves import options for the texture we're currently inspecting.
        /// </summary>
        /// <returns>Font import options object.</returns>
        private FontImportOptions GetImportOptions()
        {
            Font font = InspectedObject as Font;
            FontImportOptions output = null;

            if (font != null)
            {
                LibraryEntry texEntry = ProjectLibrary.GetEntry(ProjectLibrary.GetPath(font));
                if (texEntry != null && texEntry.Type == LibraryEntryType.File)
                {
                    FileEntry texFileEntry = (FileEntry)texEntry;
                    output = texFileEntry.Options as FontImportOptions;
                }
            }

            if (output == null)
            {
                if (importOptions == null)
                    output = new FontImportOptions();
                else
                    output = importOptions;
            }

            return output;
        }

        /// <summary>
        /// Reimports the texture resource according to the currently set import options.
        /// </summary>
        private void TriggerReimport()
        {
            Texture2D texture = (Texture2D)InspectedObject;
            string resourcePath = ProjectLibrary.GetPath(texture);

            ProjectLibrary.Reimport(resourcePath, importOptions, true);
        }

        /// <summary>
        /// Row element used for displaying GUI for font size array elements.
        /// </summary>
        public class FontSizeArrayRow : GUIListFieldRow
        {
            private GUIIntField sizeField;

            /// <inheritdoc/>
            protected override GUILayoutX CreateGUI(GUILayoutY layout)
            {
                GUILayoutX titleLayout = layout.AddLayoutX();
                sizeField = new GUIIntField(new LocEdString(seqIndex + ". "));
                titleLayout.AddElement(sizeField);

                sizeField.OnChanged += SetValue;

                return titleLayout;
            }

            /// <inheritdoc/>
            internal protected override bool Refresh()
            {
                sizeField.Value = GetValue<int>();
                return false;
            }
        }

        /// <summary>
        /// Row element used for displaying GUI for character range array elements.
        /// </summary>
        public class CharRangeArrayRow : GUIListFieldRow
        {
            private GUIIntField rangeStartField;
            private GUIIntField rangeEndField;

            /// <inheritdoc/>
            protected override GUILayoutX CreateGUI(GUILayoutY layout)
            {
                GUILayoutX titleLayout = layout.AddLayoutX();

                rangeStartField = new GUIIntField(new LocEdString(seqIndex + ". Start"));
                rangeEndField = new GUIIntField(new LocEdString("End"));

                titleLayout.AddElement(rangeStartField);
                titleLayout.AddElement(rangeEndField);

                rangeStartField.OnChanged += x =>
                {
                    CharRange range = GetValue<CharRange>();
                    range.start = x;
                    SetValue(range);
                };

                rangeEndField.OnChanged += x =>
                {
                    CharRange range = GetValue<CharRange>();
                    range.end = x;
                    SetValue(range);
                };

                return titleLayout;
            }

            /// <inheritdoc/>
            internal protected override bool Refresh()
            {
                CharRange newValue = GetValue<CharRange>();
                rangeStartField.Value = newValue.start;
                rangeEndField.Value = newValue.end;

                return false;
            }
        }
    }
}
