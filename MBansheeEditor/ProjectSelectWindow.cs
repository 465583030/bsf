﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using BansheeEngine;

namespace BansheeEditor
{
    internal sealed class ProjectSelectWindow : ModalWindow
    {
        public ProjectSelectWindow()
            :base(0, 0, 200, 200)
        {
            GUILabel label = GUI.main.AddLabel("Test test");
            label.SetContent("Test2");
        }
    }
}