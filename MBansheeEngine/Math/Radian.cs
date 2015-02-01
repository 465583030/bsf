﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;

namespace BansheeEngine
{
    [StructLayout(LayoutKind.Sequential), SerializeObject]
    public struct Radian
    {
		readonly float value;

		public Radian(float value = 0.0f)
	    {
	        this.value = value;
	    }

        public Radian(Degree d)
        {
            this.value = d.Radians;
        }

        public static implicit operator Radian(float value)
        {
            return new Radian(value);
        }

        public static implicit operator Radian(Degree d)
        {
            return new Radian(d.Radians);
        }

        public static explicit operator float(Radian d)
        {
            return d.value;
        }

        public float Degrees
        {
            get { return value*MathEx.Rad2Deg; }
        }

        public float Radians
        {
            get { return value; }
        }

        public static Radian operator +(Radian a)
	    {
	        return a;
	    }

        public static Radian operator +(Radian a, Radian b)
	    {
            return new Radian(a.value + b.value);
	    }

        public static Radian operator +(Radian a, Degree d) 
        {
            return new Radian(a.value + d.Radians); 
        }

        public static Radian operator -(Radian a)
	    {
            return new Radian(-a.value);
	    }

        public static Radian operator -(Radian a, Radian b)
	    {
            return new Radian(a.value - b.value);
	    }

        public static Radian operator -(Radian a, Degree d) 
        {
            return new Radian(a.value - d.Radians); 
        }

        public static Radian operator *(Radian a, float s)
        {
            return new Radian(a.value * s);
        }

        public static Radian operator *(Radian a, Radian b)
        {
            return new Radian(a.value * b.value);
        }

        public static Radian operator /(Radian a, float s)
        {
            return new Radian(a.value / s);
        }

        public static Radian operator /(Radian a, Radian b)
        {
            return new Radian(a.value / b.value);
        }

        public static bool operator <(Radian a, Radian b)
	    {
	        return a.value < b.value;
	    }

        public static bool operator >(Radian a, Radian b)
        {
            return a.value > b.value;
        }

        public static bool operator <=(Radian a, Radian b)
        {
            return a.value <= b.value;
        }

        public static bool operator >=(Radian a, Radian b)
        {
            return a.value >= b.value;
        }

        public static bool operator ==(Radian a, Radian b)
        {
            return a.value == b.value;
        }

        public static bool operator !=(Radian a, Radian b)
        {
            return a.value != b.value;
        }

        public override bool Equals(object other)
        {
            if (!(other is Radian))
                return false;

            Radian radian = (Radian)other;
            if (value.Equals(radian.value))
                return true;

            return false;
        }

        public override int GetHashCode()
        {
            return value.GetHashCode();
        }

        public override string ToString()
        {
            return value.ToString();
        }
    }
}
