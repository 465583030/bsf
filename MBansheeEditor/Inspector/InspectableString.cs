﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using BansheeEngine;

namespace BansheeEditor
{
    /// <summary>
    /// Displays GUI for a serializable property containing a string.
    /// </summary>
    public class InspectableString : InspectableField
    {
        private string propertyValue;
        private GUITextField guiField;
        private bool isInitialized;

        /// <summary>
        /// Creates a new inspectable string GUI for the specified property.
        /// </summary>
        /// <param name="title">Name of the property, or some other value to set as the title.</param>
        /// <param name="depth">Determines how deep within the inspector nesting hierarchy is this field. Some fields may
        ///                     contain other fields, in which case you should increase this value by one.</param>
        /// <param name="layout">Parent layout that all the field elements will be added to.</param>
        /// <param name="property">Serializable property referencing the array whose contents to display.</param>
        public InspectableString(string title, int depth, InspectableFieldLayout layout, SerializableProperty property)
            : base(title, depth, layout, property)
        {

        }

        /// <summary>
        /// Initializes the GUI elements the first time <see cref="Update"/> gets called.
        /// </summary>
        /// <param name="layoutIndex">Index at which to insert the GUI elements.</param>
        private void Initialize(int layoutIndex)
        {
            if (property.Type == SerializableProperty.FieldType.String)
            {
                guiField = new GUITextField(new GUIContent(title));
                guiField.OnChanged += OnFieldValueChanged;

                layout.AddElement(layoutIndex, guiField);
            }

            isInitialized = true;
        }

        /// <inheritdoc/>
        protected override bool IsModified()
        {
            if (!isInitialized)
                return true;

            string newPropertyValue = property.GetValue<string>();
            if (propertyValue != newPropertyValue)
                return true;

            return base.IsModified();
        }

        /// <inheritdoc/>
        protected override void Update(int layoutIndex)
        {
            base.Update(layoutIndex);

            if (!isInitialized)
                Initialize(layoutIndex);

            propertyValue = property.GetValue<string>();
            if (guiField != null)
            {
                if (guiField.HasInputFocus())
                    return;

                guiField.Value = propertyValue;
            }
        }

        /// <summary>
        /// Triggered when the user inputs a new string.
        /// </summary>
        /// <param name="newValue">New value of the string field.</param>
        private void OnFieldValueChanged(string newValue)
        {
            property.SetValue(newValue);
        }
    }
}
