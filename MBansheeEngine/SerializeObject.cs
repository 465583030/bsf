﻿using System;

namespace BansheeEngine
{
    [AttributeUsage(AttributeTargets.Class | AttributeTargets.Struct)]
    public sealed class SerializeObject : Attribute
    {
    }
}
