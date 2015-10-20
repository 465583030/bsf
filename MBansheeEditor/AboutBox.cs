﻿using System;
using System.Collections.Generic;
using System.Linq;
using BansheeEngine;

namespace BansheeEditor
{
    /// <summary>
    /// Displays information about the engine, its creator and licenses.
    /// </summary>
    public class AboutBox : ModalWindow
    {
        private GUITextBox emailLabel;

        /// <summary>
        /// Opens the about box.
        /// </summary>
        [MenuItem("Help/About", 5000)]
        public static void Open()
        {
            new AboutBox();
        }

        /// <summary>
        /// Constructs the about box.
        /// </summary>
        protected AboutBox()
            : base(true)
        {
            Title = "About";
            Width = 400;
            Height = 350;
        }

        private void OnInitialize()
        {
            GUILabel title = new GUILabel(new LocEdString("Banshee Engine"), EditorStyles.TitleLabel);
            GUILabel subTitle = new GUILabel(new LocEdString("A modern open-source game development toolkit"), 
                EditorStyles.LabelCentered);
            GUILabel license = new GUILabel(new LocEdString(
                "This program is licensed under the GNU General Public License V3"), EditorStyles.LabelCentered);
            GUILabel copyright = new GUILabel(new LocEdString("Copyright (C) 2015 Marko Pintera. All rights reserved."), 
                EditorStyles.LabelCentered);
            GUILabel emailTitle = new GUILabel(new LocEdString("E-mail"), GUIOption.FixedWidth(150));
            emailLabel = new GUITextBox();
            GUILabel linkedInTitle = new GUILabel(new LocEdString("LinkedIn"), GUIOption.FixedWidth(150));
            GUIButton linkedInBtn = new GUIButton(new LocEdString("Profile"));

            GUIToggle contactFoldout = new GUIToggle(new LocEdString("Author contact"), EditorStyles.Foldout);
            GUIToggle thirdPartyFoldout = new GUIToggle(new LocEdString("Used third party libraries"), EditorStyles.Foldout);
            GUIToggle noticesFoldout = new GUIToggle(new LocEdString("Third party notices"), EditorStyles.Foldout);

            GUILabel freeTypeNotice = new GUILabel(new LocEdString(
                        "Portions of this software are copyright (C) 2015 The FreeType Project (www.freetype.org). " +
                        "All rights reserved."), EditorStyles.MultiLineLabelCentered, 
                        GUIOption.FlexibleHeight(), GUIOption.FixedWidth(380));

            GUILabel fbxSdkNotice = new GUILabel(new LocEdString(
                "This software contains Autodesk(R) FBX(R) code developed by Autodesk, Inc. Copyright 2013 Autodesk, Inc. " +
                "All rights, reserved. Such code is provided \"as is\" and Autodesk, Inc. disclaims any and all warranties, " +
                "whether express or implied, including without limitation the implied warranties of merchantability, " +
                "fitness for a particular purpose or non-infringement of third party rights. In no event shall Autodesk, " +
                "Inc. be liable for any direct, indirect, incidental, special, exemplary, or consequential damages " +
                "(including, but not limited to, procurement of substitute goods or services; loss of use, data, or " +
                "profits; or business interruption) however caused and on any theory of liability, whether in contract, " +
                "strict liability, or tort (including negligence or otherwise) arising in any way out of such code."),
                EditorStyles.MultiLineLabelCentered, GUIOption.FlexibleHeight(), GUIOption.FixedWidth(380));

            GUILayoutY mainLayout = GUI.AddLayoutY();
            mainLayout.AddSpace(10);
            mainLayout.AddElement(title);
            mainLayout.AddElement(subTitle);
            mainLayout.AddSpace(10);
            mainLayout.AddElement(license);
            mainLayout.AddElement(copyright);
            mainLayout.AddSpace(10);
            mainLayout.AddElement(contactFoldout);

            GUILayoutY contactLayout = mainLayout.AddLayoutY();
            GUILayout emailLayout = contactLayout.AddLayoutX();
            emailLayout.AddSpace(10);
            emailLayout.AddElement(emailTitle);
            emailLayout.AddElement(emailLabel);
            emailLayout.AddSpace(10);
            GUILayout linkedInLayout = contactLayout.AddLayoutX();
            linkedInLayout.AddSpace(10);
            linkedInLayout.AddElement(linkedInTitle);
            linkedInLayout.AddElement(linkedInBtn);
            linkedInLayout.AddSpace(10);

            mainLayout.AddSpace(5);
            mainLayout.AddElement(thirdPartyFoldout);
            GUILayoutY thirdPartyLayout = mainLayout.AddLayoutY();

            CreateThirdPartyGUI(thirdPartyLayout, "Autodesk FBX SDK",
                "http://usa.autodesk.com/adsk/servlet/pc/item?siteID=123112&id=10775847", "FBX_SDK_License.rtf");
            CreateThirdPartyGUI(thirdPartyLayout, "FreeImage", "http://freeimage.sourceforge.net/", "freeimage-license.txt");
            CreateThirdPartyGUI(thirdPartyLayout, "FreeType", "http://www.freetype.org/", "FTL.TXT");
            CreateThirdPartyGUI(thirdPartyLayout, "Mono", "http://www.mono-project.com/", "Mono.txt");
            CreateThirdPartyGUI(thirdPartyLayout, "NVIDIA Texture Tools",
                "https://github.com/castano/nvidia-texture-tools", "NVIDIATextureTools.txt");

            mainLayout.AddSpace(5);
            mainLayout.AddElement(noticesFoldout);
            GUILayout noticesLayout = mainLayout.AddLayoutY();
            noticesLayout.AddElement(freeTypeNotice);
            noticesLayout.AddSpace(10);
            noticesLayout.AddElement(fbxSdkNotice);
            mainLayout.AddFlexibleSpace();

            contactLayout.Active = false;
            contactFoldout.OnToggled += x => contactLayout.Active = x;

            thirdPartyLayout.Active = false;
            thirdPartyFoldout.OnToggled += x => thirdPartyLayout.Active = x;

            noticesLayout.Active = false;
            noticesFoldout.OnToggled += x => noticesLayout.Active = x;

            emailLabel.Text = "marko.pintera@gmail.com";
            linkedInBtn.OnClick += () => { System.Diagnostics.Process.Start("http://hr.linkedin.com/in/markopintera"); };
        }

        private void CreateThirdPartyGUI(GUILayoutY layout, string name, string webURL, string licenseFile)
        {
            GUILabel label = new GUILabel(new LocEdString(name), GUIOption.FixedWidth(150));
            GUIButton linkBtn = new GUIButton(new LocEdString("Website"), GUIOption.FixedWidth(50));
            GUIButton licenseBtn = new GUIButton(new LocEdString("License"), GUIOption.FixedWidth(50));

            string licensePath = "..\\..\\..\\License\\Third Party\\" + licenseFile;

            GUILayoutX horzLayout = layout.AddLayoutX();
            horzLayout.AddSpace(10);
            horzLayout.AddElement(label);
            horzLayout.AddSpace(10);
            horzLayout.AddElement(linkBtn);
            horzLayout.AddSpace(5);
            horzLayout.AddElement(licenseBtn);
            horzLayout.AddSpace(10);

            linkBtn.OnClick += () => { System.Diagnostics.Process.Start(webURL); };
            licenseBtn.OnClick += () => { System.Diagnostics.Process.Start(licensePath); };
        }

        private void OnEditorUpdate()
        {
            emailLabel.Text = "marko.pintera@gmail.com";
        }
    }
}
