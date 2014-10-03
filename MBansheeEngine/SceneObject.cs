﻿using System;
using System.Runtime.CompilerServices;

namespace BansheeEngine
{
    // TODO - Unfinished
    public sealed class SceneObject : GameObject
    {
        public SceneObject parent
        {
            set { Internal_SetParent(mCachedPtr, value); }
            get { return Internal_GetParent(mCachedPtr); }
        }

        public Vector3 position
        {
            set { throw new NotImplementedException(); }
            get { throw new NotImplementedException(); }
        }

        public Vector3 localPosition
        {
            set { throw new NotImplementedException(); }
            get { throw new NotImplementedException(); }
        }

        public Quaternion rotation
        {
            set { throw new NotImplementedException(); }
            get { throw new NotImplementedException(); }
        }

        public Quaternion localRotation
        {
            set { throw new NotImplementedException(); }
            get { throw new NotImplementedException(); }
        }

        public Vector3 scale
        {
            set { throw new NotImplementedException(); }
            get { throw new NotImplementedException(); }
        }

        // For internal use
        private SceneObject()
        {
            
        }

        public SceneObject(string name)
        {
            Internal_CreateInstance(this, name);
        }

        public T AddComponent<T>() where T : Component
        {
            return (T)Component.Internal_AddComponent(this, typeof (T));
        }

        public T GetComponent<T>() where T : Component
        {
            return (T)Component.Internal_GetComponent(this, typeof(T));
        }

        public Component[] GetComponents()
        {
            return Component.Internal_GetComponents(this);
        }

        public void RemoveComponent<T>() where T : Component
        {
            Component.Internal_RemoveComponent(this, typeof(T));
        }

        public int GetNumChildren()
        {
            return Internal_GetNumChildren(mCachedPtr);
        }

        public SceneObject GetChild(int idx)
        {
            return Internal_GetChild(mCachedPtr, idx);
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_CreateInstance(SceneObject instance, string name);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetParent(IntPtr nativeInstance, SceneObject parent);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern SceneObject Internal_GetParent(IntPtr nativeInstance);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern int Internal_GetNumChildren(IntPtr nativeInstance);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern SceneObject Internal_GetChild(IntPtr nativeInstance, int idx);
    }
}
