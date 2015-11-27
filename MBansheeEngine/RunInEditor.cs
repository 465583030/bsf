﻿using System;

namespace BansheeEngine
{
    /// <summary>
    /// Attribute that allows Component's code to be executed while game is not running in the editor. Normally the
    /// initialize/update/destroy methods will not be running while the game is paused or not running, but this overrides
    /// that behaviour.
    /// </summary>
    [AttributeUsage(AttributeTargets.Class)]
    public sealed class RunInEditor : Attribute
    {
    }
}