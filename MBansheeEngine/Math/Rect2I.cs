﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;

namespace BansheeEngine
{
    [StructLayout(LayoutKind.Sequential), SerializeObject]
    public struct Rect2I
    {
        public Rect2I(int x, int y, int width, int height)
        {
            this.x = x;
            this.y = y;
            this.width = width;
            this.height = height;
        }

		public static bool operator== (Rect2I lhs, Rect2I rhs)
		{
			return lhs.x == rhs.x && lhs.y == rhs.y && lhs.width == rhs.width && lhs.height == rhs.height;
		}

		public static bool operator!= (Rect2I lhs, Rect2I rhs)
		{
			return !(lhs == rhs);
		}

	    public bool Contains(Vector2I point)
	    {
		    if(point.x >= x && point.x < (x + width))
		    {
			    if(point.y >= y && point.y < (y + height))
				    return true;
		    }

		    return false;
	    }

        public override bool Equals(object other)
        {
            if (!(other is Rect2I))
                return false;

            Rect2I rect = (Rect2I)other;
            if (x.Equals(rect.x) && y.Equals(rect.y) && width.Equals(rect.width) && height.Equals(rect.height))
                return true;

            return false;
        }

        public override int GetHashCode()
        {
            return base.GetHashCode();
        }

        public override string ToString()
        {
            return String.Format("(x:{0} y:{1} width:{2} height:{3})", x, y, width, height);
        }

        public int x, y, width, height;
    }
}
