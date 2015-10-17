﻿using BansheeEngine;

namespace BansheeEditor
{
    /// <summary>
    /// Displays GUI for a serializable property containing a <see cref="GameObject"/> reference.
    /// </summary>
    public class InspectableGameObjectRef : InspectableField
    {
        private GUIGameObjectField guiField;
        private InspectableState state;

        /// <summary>
        /// Creates a new inspectable game object reference GUI for the specified property.
        /// </summary>
        /// <param name="title">Name of the property, or some other value to set as the title.</param>
        /// <param name="depth">Determines how deep within the inspector nesting hierarchy is this field. Some fields may
        ///                     contain other fields, in which case you should increase this value by one.</param>
        /// <param name="layout">Parent layout that all the field elements will be added to.</param>
        /// <param name="property">Serializable property referencing the array whose contents to display.</param>
        public InspectableGameObjectRef(string title, int depth, InspectableFieldLayout layout, SerializableProperty property)
            : base(title, depth, layout, property)
        {

        }

        /// <inheritoc/>
        protected internal override void Initialize(int layoutIndex)
        {
            if (property.Type == SerializableProperty.FieldType.GameObjectRef)
            {
                guiField = new GUIGameObjectField(property.InternalType, new GUIContent(title));
                guiField.OnChanged += OnFieldValueChanged;

                layout.AddElement(layoutIndex, guiField);
            }
        }

        /// <inheritdoc/>
        public override InspectableState Refresh(int layoutIndex)
        {
            if (guiField != null)
                guiField.Value = property.GetValue<GameObject>();

            InspectableState oldState = state;
            if (state.HasFlag(InspectableState.Modified))
                state = InspectableState.NotModified;

            return oldState;
        }

        /// <summary>
        /// Triggered when the user drops a new game object onto the field, or clears the current value.
        /// </summary>
        /// <param name="newValue">New game object to reference.</param>
        private void OnFieldValueChanged(GameObject newValue)
        {
            property.SetValue(newValue);
            state = InspectableState.Modified;
        }
    }
}
