﻿//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using System;
using System.Runtime.CompilerServices;

namespace BansheeEngine
{
    /// <summary>
    /// Wrapper around the native D6Joint class.
    /// <see cref="D6Joint"/>
    /// </summary>
    internal class NativeD6Joint : NativeJoint
    {
        public Radian Twist
        {
            get { return new Radian(Internal_GetTwist(mCachedPtr)); }
        }

        public Radian SwingY
        {
            get { return new Radian(Internal_GetSwingY(mCachedPtr)); }
        }

        public Radian SwingZ
        {
            get { return new Radian(Internal_GetSwingZ(mCachedPtr)); }
        }

        public LimitLinear LimitLinear
        {
            set { Internal_SetLimitLinear(mCachedPtr, value); }
        }

        public LimitAngularRange LimitTwist
        {
            set { Internal_SetLimitTwist(mCachedPtr, value); }
        }

        public LimitConeRange LimitSwing
        {
            set { Internal_SetLimitSwing(mCachedPtr, value); }
        }

        public Vector3 DrivePosition
        {
            set { Internal_SetDrivePosition(mCachedPtr, ref value); }
        }

        public Quaternion DriveRotation
        {
            set { Internal_SetDriveRotation(mCachedPtr, ref value); }
        }

        public Vector3 DriveLinearVelocity
        {
            set { Internal_SetDriveLinearVelocity(mCachedPtr, ref value); }
        }

        public Vector3 DriveAngularVelocity
        {
            set { Internal_SetDriveAngularVelocity(mCachedPtr, ref value); }
        }

        public void SetMotion(D6JointAxis axis, D6JointMotion motion)
        {
            Internal_SetMotion(mCachedPtr, axis, motion);
        }

        public void SetDrive(D6JointDriveType type, D6JointDrive drive)
        {
            Internal_SetDrive(mCachedPtr, type, drive);
        }

        public NativeD6Joint()
        {
            Internal_CreateInstance(this);
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_CreateInstance(NativeD6Joint instance);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetMotion(IntPtr thisPtr, D6JointAxis axis, D6JointMotion motion);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern float Internal_GetTwist(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern float Internal_GetSwingY(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern float Internal_GetSwingZ(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetLimitLinear(IntPtr thisPtr, LimitLinear limit);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetLimitTwist(IntPtr thisPtr, LimitAngularRange limit);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetLimitSwing(IntPtr thisPtr, LimitConeRange limit);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetDrive(IntPtr thisPtr, D6JointDriveType type, D6JointDrive drive);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetDrivePosition(IntPtr thisPtr, ref Vector3 position);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetDriveRotation(IntPtr thisPtr, ref Quaternion rotation);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetDriveLinearVelocity(IntPtr thisPtr, ref Vector3 velocity);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetDriveAngularVelocity(IntPtr thisPtr, ref Vector3 velocity);
    }
}
