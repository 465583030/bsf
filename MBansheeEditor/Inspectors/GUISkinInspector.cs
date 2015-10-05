﻿using System;
using System.Collections.Generic;
using BansheeEngine;

namespace BansheeEditor
{
    /// <summary>
    /// Renders an inspector for the <see cref="GUISkin"/> resource.
    /// </summary>
    [CustomInspector(typeof(GUISkin))]
    public class GUISkinInspector : Inspector
    {
        private GUIDictionaryField<string, GUIElementStyle> valuesField = new GUIDictionaryField<string, GUIElementStyle>();

        private Dictionary<string, GUIElementStyle> styles = new Dictionary<string, GUIElementStyle>();

        /// <inheritdoc/>
        protected internal override void Initialize()
        {
            BuildGUI();
        }

        /// <inheritdoc/>
        protected internal override bool Refresh()
        {
            return valuesField.Refresh(); ;
        }

        /// <summary>
        /// Recreates all the GUI elements used by this inspector.
        /// </summary>
        private void BuildGUI()
        {
            layout.Clear();
            styles.Clear();

            GUISkin guiSkin = referencedObject as GUISkin;
            if (guiSkin == null)
                return;

            string[] styleNames = guiSkin.StyleNames;
            foreach (var styleName in styleNames)
                styles[styleName] = guiSkin.GetStyle(styleName);

            valuesField.Update<GUIElementStyleEntry>(new LocEdString("Styles"), styles, layout);

            valuesField.OnChanged += x =>
            {
                foreach (var KVP in x)
                {
                    GUIElementStyle oldValue;
                    if (styles.TryGetValue(KVP.Key, out oldValue))
                    {
                        if (oldValue != KVP.Value)
                            guiSkin.SetStyle(KVP.Key, KVP.Value);
                    }
                    else
                        guiSkin.SetStyle(KVP.Key, KVP.Value);
                }

                foreach (var KVP in styles)
                {
                    if (!x.ContainsKey(KVP.Key))
                        guiSkin.RemoveStyle(KVP.Key);
                }

                BuildGUI();
                Refresh();
            };

            valuesField.OnValueChanged += x =>
            {
                guiSkin.SetStyle(x, styles[x]);
            };

            layout.AddSpace(10);
        }

        /// <summary>
        /// Row element used for displaying GUI for GUI element style dictionary elements.
        /// </summary>
        public class GUIElementStyleEntry : GUIDictionaryFieldRow
        {
            private GUITextField keyField;
            private GUIElementStyleGUI valueField;

            /// <inheritdoc/>
            protected override GUILayoutX CreateKeyGUI(GUILayoutY layout)
            {
                string key = GetKey<string>();

                GUILayoutX titleLayout = layout.AddLayoutX();
                keyField = new GUITextField(new LocEdString((string)key));
                titleLayout.AddElement(keyField);

                keyField.OnChanged += SetKey;

                return titleLayout;
            }

            /// <inheritdoc/>
            protected override void CreateValueGUI(GUILayoutY layout)
            {
                GUIElementStyle value = GetValue<GUIElementStyle>();

                valueField = new GUIElementStyleGUI(value, layout);
            }

            /// <inheritdoc/>
            internal protected override bool Refresh(out bool rebuildGUI)
            {
                rebuildGUI = false;

                string newKey = GetKey<string>();
                if (keyField.Value != newKey)
                {
                    keyField.Value = newKey;
                    return true;
                }

                // Key cannot be changed so we don't check it here
                return valueField.Refresh();
            }
        }

        /// <summary>
        /// Creates GUI elements for editing/displaying <see cref="GUIElementStyle"/>
        /// </summary>
        private class GUIElementStyleGUI
        {
            private GUIResourceField fontField;
            private GUIIntField fontSizeField;
            private GUIEnumField horzAlignField;
            private GUIEnumField vertAlignField;
            private GUIEnumField imagePositionField;
            private GUIToggleField wordWrapField;

            private GUIElementStateStyleGUI normalGUI;
            private GUIElementStateStyleGUI hoverGUI;
            private GUIElementStateStyleGUI activeGUI;
            private GUIElementStateStyleGUI focusedGUI;
            private GUIElementStateStyleGUI normalOnGUI;
            private GUIElementStateStyleGUI hoverOnGUI;
            private GUIElementStateStyleGUI activeOnGUI;
            private GUIElementStateStyleGUI focusedOnGUI;

            private RectOffsetGUI borderGUI;
            private RectOffsetGUI marginsGUI;
            private RectOffsetGUI contentOffsetGUI;

            private GUIToggleField fixedWidthField;
            private GUIIntField widthField;
            private GUIIntField minWidthField;
            private GUIIntField maxWidthField;

            private GUIToggleField fixedHeightField;
            private GUIIntField heightField;
            private GUIIntField minHeightField;
            private GUIIntField maxHeightField;

            private GUIElementStyle style;

            /// <summary>
            /// Creates a new GUI element style GUI.
            /// </summary>
            /// <param name="style">Style to display in the GUI.</param>
            /// <param name="layout">Layout to append the GUI elements to.</param>
            public GUIElementStyleGUI(GUIElementStyle style, GUILayout layout)
            {
                this.style = style;

                fontField = new GUIResourceField(typeof (Font), new LocEdString("Font"));
                fontSizeField = new GUIIntField(new LocEdString("Font size"));
                horzAlignField = new GUIEnumField(typeof (TextHorzAlign), new LocEdString("Horizontal alignment"));
                vertAlignField = new GUIEnumField(typeof(TextVertAlign), new LocEdString("Vertical alignment"));
                imagePositionField = new GUIEnumField(typeof(GUIImagePosition), new LocEdString("Image position"));
                wordWrapField = new GUIToggleField(new LocEdString("Word wrap"));

                layout.AddElement(fontField);
                layout.AddElement(fontSizeField);
                layout.AddElement(horzAlignField);
                layout.AddElement(vertAlignField);
                layout.AddElement(imagePositionField);
                layout.AddElement(wordWrapField);

                normalGUI = new GUIElementStateStyleGUI(new LocEdString("Normal"), style.Normal, layout);
                hoverGUI = new GUIElementStateStyleGUI(new LocEdString("Hover"), style.Hover, layout);
                activeGUI = new GUIElementStateStyleGUI(new LocEdString("Active"), style.Active, layout);
                focusedGUI = new GUIElementStateStyleGUI(new LocEdString("Focused"), style.Focused, layout);
                normalOnGUI = new GUIElementStateStyleGUI(new LocEdString("NormalOn"), style.NormalOn, layout);
                hoverOnGUI = new GUIElementStateStyleGUI(new LocEdString("HoverOn"), style.HoverOn, layout);
                activeOnGUI = new GUIElementStateStyleGUI(new LocEdString("ActiveOn"), style.ActiveOn, layout);
                focusedOnGUI = new GUIElementStateStyleGUI(new LocEdString("FocusedOn"), style.FocusedOn, layout);

                borderGUI = new RectOffsetGUI(new LocEdString("Border"), style.Border, layout);
                marginsGUI = new RectOffsetGUI(new LocEdString("Margins"), style.Margins, layout);
                contentOffsetGUI = new RectOffsetGUI(new LocEdString("Content offset"), style.ContentOffset, layout);

                fixedWidthField = new GUIToggleField(new LocEdString("Fixed width"));
                widthField = new GUIIntField(new LocEdString("Width"));
                minWidthField = new GUIIntField(new LocEdString("Min. width"));
                maxWidthField = new GUIIntField(new LocEdString("Max. width"));

                fixedHeightField = new GUIToggleField(new LocEdString("Fixed height"));
                heightField = new GUIIntField(new LocEdString("Height"));
                minHeightField = new GUIIntField(new LocEdString("Min. height"));
                maxHeightField = new GUIIntField(new LocEdString("Max. height"));

                layout.AddElement(fixedWidthField);
                layout.AddElement(widthField);
                layout.AddElement(minWidthField);
                layout.AddElement(maxWidthField);

                layout.AddElement(fixedHeightField);
                layout.AddElement(heightField);
                layout.AddElement(minHeightField);
                layout.AddElement(maxHeightField);

                fontField.OnChanged += x => style.Font = (Font)x;
                fontSizeField.OnChanged += x => style.FontSize = x;
                horzAlignField.OnSelectionChanged += x => style.TextHorzAlign = (TextHorzAlign)x;
                vertAlignField.OnSelectionChanged += x => style.TextVertAlign = (TextVertAlign)x;
                imagePositionField.OnSelectionChanged += x => style.ImagePosition = (GUIImagePosition)x;
                wordWrapField.OnChanged += x => style.WordWrap = x;

                borderGUI.OnChanged += x => style.Border = x;
                marginsGUI.OnChanged += x => style.Margins = x;
                contentOffsetGUI.OnChanged += x => style.ContentOffset = x;

                fixedWidthField.OnChanged += x =>
                {
                    widthField.Enabled = x;
                    minWidthField.Enabled = !x;
                    maxWidthField.Enabled = !x;
                };

                widthField.OnChanged += x => style.Width = x;
                minWidthField.OnChanged += x => style.MinWidth = x;
                maxWidthField.OnChanged += x => style.MaxWidth = x;

                fixedHeightField.OnChanged += x =>
                {
                    heightField.Enabled = x;
                    minHeightField.Enabled = !x;
                    maxHeightField.Enabled = !x;
                };

                heightField.OnChanged += x => style.Height = x;
                minHeightField.OnChanged += x => style.MinHeight = x;
                maxHeightField.OnChanged += x => style.MaxHeight = x;
            }

            /// <summary>
            /// Updates all GUI elements from the style if style changes.
            /// </summary>
            /// <returns>True if any changes were made.</returns>
            public bool Refresh()
            {
                bool anythingModified = false;

                if (fontField.Value != style.Font)
                {
                    fontField.Value = style.Font;
                    anythingModified = true;
                }

                if (fontSizeField.Value != style.FontSize)
                {
                    fontSizeField.Value = style.FontSize;
                    anythingModified = true;
                }

                if (horzAlignField.Value != (ulong)style.TextHorzAlign)
                {
                    horzAlignField.Value = (ulong)style.TextHorzAlign;
                    anythingModified = true;
                }

                if (vertAlignField.Value != (ulong)style.TextVertAlign)
                {
                    vertAlignField.Value = (ulong)style.TextVertAlign;
                    anythingModified = true;
                }

                if (imagePositionField.Value != (ulong)style.ImagePosition)
                {
                    imagePositionField.Value = (ulong)style.ImagePosition;
                    anythingModified = true;
                }

                if (wordWrapField.Value != style.WordWrap)
                {
                    wordWrapField.Value = style.WordWrap;
                    anythingModified = true;
                }

                anythingModified |= normalGUI.Refresh();
                anythingModified |= hoverGUI.Refresh();
                anythingModified |= activeGUI.Refresh();
                anythingModified |= focusedGUI.Refresh();
                anythingModified |= normalOnGUI.Refresh();
                anythingModified |= hoverOnGUI.Refresh();
                anythingModified |= activeOnGUI.Refresh();
                anythingModified |= focusedOnGUI.Refresh();

                anythingModified |= borderGUI.Refresh(style.Border);
                anythingModified |= marginsGUI.Refresh(style.Margins);
                anythingModified |= contentOffsetGUI.Refresh(style.ContentOffset);

                if (fixedWidthField.Value != style.FixedWidth)
                {
                    fixedWidthField.Value = style.FixedWidth;
                    anythingModified = true;
                }

                if (widthField.Value != style.Width)
                {
                    widthField.Value = style.Width;
                    anythingModified = true;
                }

                if (minWidthField.Value != style.MinWidth)
                {
                    minWidthField.Value = style.MinWidth;
                    anythingModified = true;
                }

                if (maxWidthField.Value != style.MaxWidth)
                {
                    maxWidthField.Value = style.MaxWidth;
                    anythingModified = true;
                }

                if (fixedHeightField.Value != style.FixedHeight)
                {
                    fixedHeightField.Value = style.FixedHeight;
                    anythingModified = true;
                }

                if (heightField.Value != style.Height)
                {
                    heightField.Value = style.Height;
                    anythingModified = true;
                }

                if (minHeightField.Value != style.MinHeight)
                {
                    minHeightField.Value = style.MinHeight;
                    anythingModified = true;
                }

                if (maxHeightField.Value != style.MaxHeight)
                {
                    maxHeightField.Value = style.MaxHeight;
                    anythingModified = true;
                }

                return anythingModified;
            }

            /// <summary>
            /// Creates GUI elements for editing/displaying <see cref="GUIElementStateStyle"/>
            /// </summary>
            public class GUIElementStateStyleGUI
            {
                private GUIToggleField foldout;
                private GUIResourceField textureField;
                private GUIColorField textColorField;

                private GUIElementStateStyle state;

                /// <summary>
                /// Creates a new GUI element state style GUI.
                /// </summary>
                /// <param name="title">Text to display on the title bar.</param>
                /// <param name="state">State object to display in the GUI.</param>
                /// <param name="layout">Layout to append the GUI elements to.</param>
                public GUIElementStateStyleGUI(LocString title, GUIElementStateStyle state, GUILayout layout)
                {
                    this.state = state;

                    foldout = new GUIToggleField(title);
                    textureField = new GUIResourceField(typeof(SpriteTexture), new LocEdString("Texture"));
                    textColorField = new GUIColorField(new LocEdString("Text color"));

                    foldout.OnChanged += x =>
                    {
                        textureField.Enabled = x;
                        textColorField.Enabled = x;
                    };

                    layout.AddElement(foldout);
                    layout.AddElement(textureField);
                    layout.AddElement(textColorField);
                }

                /// <summary>
                /// Updates all GUI elements from the state if state changes.
                /// </summary>
                /// <returns>True if any changes were made.</returns>
                public bool Refresh()
                {
                    bool anythingModified = false;

                    if (textureField.Value != state.Texture)
                    {
                        state.Texture = (SpriteTexture)textureField.Value;
                        anythingModified = true;
                    }

                    if (textColorField.Value != state.TextColor)
                    {
                        state.TextColor = textColorField.Value;
                        anythingModified = true;
                    }

                    return anythingModified;
                }
            }

            /// <summary>
            /// Creates GUI elements for editing/displaying <see cref="RectOffset"/>
            /// </summary>
            public class RectOffsetGUI
            {
                private GUIIntField offsetLeftField;
                private GUIIntField offsetRightField;
                private GUIIntField offsetTopField;
                private GUIIntField offsetBottomField;

                /// <summary>
                /// Triggered when some value in the offset rectangle changes.
                /// </summary>
                public Action<RectOffset> OnChanged;

                /// <summary>
                /// Creates a new rectangle offset GUI.
                /// </summary>
                /// <param name="title">Text to display on the title bar.</param>
                /// <param name="offset">Rectangle offset object to display in the GUI.</param>
                /// <param name="layout">Layout to append the GUI elements to.</param>
                public RectOffsetGUI(LocString title, RectOffset offset, GUILayout layout)
                {
                    GUILayoutX rectLayout = layout.AddLayoutX();
                    rectLayout.AddElement(new GUILabel(title, GUIOption.FixedWidth(100)));
                    GUILayoutY rectContentLayout = rectLayout.AddLayoutY();

                    GUILayoutX rectTopRow = rectContentLayout.AddLayoutX();
                    GUILayoutX rectBotRow = rectContentLayout.AddLayoutX();

                    offsetLeftField = new GUIIntField(new LocEdString("Left"));
                    offsetRightField = new GUIIntField(new LocEdString("Right"));
                    offsetTopField = new GUIIntField(new LocEdString("Top"));
                    offsetBottomField = new GUIIntField(new LocEdString("Bottom"));

                    rectTopRow.AddElement(offsetLeftField);
                    rectTopRow.AddElement(offsetRightField);
                    rectBotRow.AddElement(offsetTopField);
                    rectBotRow.AddElement(offsetBottomField);

                    offsetLeftField.OnChanged += x =>
                    {
                        offset.left = x;

                        if(OnChanged != null)
                            OnChanged(offset);
                    };

                    offsetRightField.OnChanged += x =>
                    {
                        offset.right = x;

                        if (OnChanged != null)
                            OnChanged(offset);
                    };

                    offsetTopField.OnChanged += x =>
                    {
                        offset.top = x;

                        if (OnChanged != null)
                            OnChanged(offset);
                    };

                    offsetBottomField.OnChanged += x =>
                    {
                        offset.bottom = x;

                        if (OnChanged != null)
                            OnChanged(offset);
                    };
                }

                /// <summary>
                /// Updates all GUI elements from the offset if offset changes.
                /// </summary>
                /// <param name="offset">New (potentially modified) offset.</param>
                /// <returns>True if any changes were made.</returns>
                public bool Refresh(RectOffset offset)
                {
                    bool anythingModified = false;

                    if (offsetLeftField.Value != offset.left)
                    {
                        offsetLeftField.Value = offset.left;
                        anythingModified = true;
                    }

                    if (offsetRightField.Value != offset.right)
                    {
                        offsetRightField.Value = offset.right;
                        anythingModified = true;
                    }

                    if (offsetTopField.Value != offset.top)
                    {
                        offsetTopField.Value = offset.top;
                        anythingModified = true;
                    }

                    if (offsetBottomField.Value != offset.bottom)
                    {
                        offsetBottomField.Value = offset.bottom;
                        anythingModified = true;
                    }

                    return anythingModified;
                }
            }
        }
    }
}
