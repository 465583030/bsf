﻿using BansheeEngine;

namespace BansheeEditor
{
    public sealed class RotateHandle : DefaultHandle
    {
        private Quaternion delta;

        private HandleSliderDisc xAxis;
        private HandleSliderDisc yAxis;
        private HandleSliderDisc zAxis;

        private bool isDragged;
        private Quaternion dragStartRotation;

        public Quaternion Delta
        {
            get { return delta; }
        }

        internal override bool IsDragged()
        {
            return xAxis.State == HandleSlider.StateType.Active ||
                    yAxis.State == HandleSlider.StateType.Active ||
                    zAxis.State == HandleSlider.StateType.Active;
        }

        public RotateHandle()
        {
            xAxis = new HandleSliderDisc(this, Vector3.xAxis, 1.0f);
            yAxis = new HandleSliderDisc(this, Vector3.yAxis, 1.0f);
            zAxis = new HandleSliderDisc(this, Vector3.zAxis, 1.0f);
        }

        protected override void PreInput()
        {
            xAxis.Position = position;
            yAxis.Position = position;
            zAxis.Position = position;

            Quaternion handleRotation = isDragged ? dragStartRotation : Rotation;
            xAxis.Rotation = handleRotation;
            yAxis.Rotation = handleRotation;
            zAxis.Rotation = handleRotation;

            xAxis.SetCutoffPlane(GetXStartAngle(isDragged), true);
            yAxis.SetCutoffPlane(GetYStartAngle(isDragged), true);
            zAxis.SetCutoffPlane(GetZStartAngle(isDragged), true);
        }

        protected override void PostInput()
        {
            if (IsDragged())
            {
                if (!isDragged)
                {
                    isDragged = true;
                    dragStartRotation = Rotation;
                }
            }
            else
            {
                isDragged = false;
                dragStartRotation = Quaternion.identity;
            }

            Degree xValue = 0.0f;
            Degree yValue = 0.0f;
            Degree zValue = 0.0f;

            if (Handles.RotateHandleSnapActive)
            {
                xValue = Handles.SnapValue(xAxis.Delta, Handles.RotateSnapAmount);
                yValue = Handles.SnapValue(yAxis.Delta, Handles.RotateSnapAmount);
                zValue = Handles.SnapValue(zAxis.Delta, Handles.RotateSnapAmount);
            }
            else
            {
                xValue = xAxis.Delta;
                yValue = yAxis.Delta;
                zValue = zAxis.Delta;
            }

            delta = Quaternion.FromEuler(xValue, yValue, zValue);
        }

        protected override void Draw()
        {
            //HandleDrawing.SetTransform(Matrix4.TRS(Position, Quaternion.identity, Vector3.one));
            HandleDrawing.SetTransform(Matrix4.TRS(Position, Rotation, Vector3.one));
            float handleSize = Handles.GetHandleSize(EditorApplication.SceneViewCamera, position);

            // Draw arcs
            if (xAxis.State == HandleSlider.StateType.Active)
                HandleDrawing.SetColor(Color.White);
            else if(xAxis.State == HandleSlider.StateType.Hover)
                HandleDrawing.SetColor(Color.BansheeOrange);
            else
                HandleDrawing.SetColor(Color.Red);

            //HandleDrawing.DrawWireArc(Vector3.zero, Rotation.Rotate(Vector3.xAxis), 1.0f, GetXStartAngle(false), -180.0f, handleSize);
            HandleDrawing.DrawWireArc(Vector3.zero, Vector3.xAxis, 1.0f, GetXStartAngle(false), -180.0f, handleSize);

            if (yAxis.State == HandleSlider.StateType.Active)
                HandleDrawing.SetColor(Color.White);
            else if (yAxis.State == HandleSlider.StateType.Hover)
                HandleDrawing.SetColor(Color.BansheeOrange);
            else
                HandleDrawing.SetColor(Color.Green);

            //HandleDrawing.DrawWireArc(Vector3.zero, Rotation.Rotate(Vector3.yAxis), 1.0f, GetYStartAngle(false), -180.0f, handleSize);
            HandleDrawing.DrawWireArc(Vector3.zero, Vector3.yAxis, 1.0f, GetYStartAngle(false), -180.0f, handleSize);

            if (zAxis.State == HandleSlider.StateType.Active)
                HandleDrawing.SetColor(Color.White);
            else if (zAxis.State == HandleSlider.StateType.Hover)
                HandleDrawing.SetColor(Color.BansheeOrange);
            else
                HandleDrawing.SetColor(Color.Blue);

            //HandleDrawing.DrawWireArc(Vector3.zero, Rotation.Rotate(Vector3.zAxis), 1.0f, GetZStartAngle(false), -180.0f, handleSize);
            HandleDrawing.DrawWireArc(Vector3.zero, Vector3.zAxis, 1.0f, GetZStartAngle(false), -180.0f, handleSize);

            // Draw active rotation pie
            Color gray = new Color(1.0f, 1.0f, 1.0f, 0.3f);
            HandleDrawing.SetColor(gray);
            HandleDrawing.SetTransform(Matrix4.TRS(Position, dragStartRotation, Vector3.one));

            if (xAxis.State == HandleSlider.StateType.Active)
                HandleDrawing.DrawArc(Vector3.zero, Vector3.xAxis, 1.0f, xAxis.StartAngle, xAxis.Delta, handleSize);
            else if (yAxis.State == HandleSlider.StateType.Active)
                HandleDrawing.DrawArc(Vector3.zero, Vector3.yAxis, 1.0f, yAxis.StartAngle, yAxis.Delta, handleSize);
            else if (zAxis.State == HandleSlider.StateType.Active)
                HandleDrawing.DrawArc(Vector3.zero, Vector3.zAxis, 1.0f, zAxis.StartAngle, zAxis.Delta, handleSize);

            // Draw free rotate handle
            HandleDrawing.SetTransform(Matrix4.TRS(Position, Quaternion.identity, Vector3.one));
            
            //// Rotate it so it always faces the camera, and move it forward a bit to always render in front
            Vector3 freeHandleNormal = EditorApplication.SceneViewCamera.SceneObject.Rotation.Rotate(Vector3.zAxis);
            Vector3 offset = freeHandleNormal * 0.1f;

            HandleDrawing.DrawWireDisc(offset, freeHandleNormal, 1.0f, handleSize);
        }

        private Degree GetXStartAngle(bool frozen)
        {
            Quaternion handleRotation = frozen ? dragStartRotation : Rotation;

            Vector3 xStartDir = Vector3.Cross(handleRotation.Inverse.Rotate(EditorApplication.SceneViewCamera.SceneObject.Forward), Vector3.xAxis);
            return PointOnCircleToAngle(Vector3.xAxis, xStartDir);
        }

        private Degree GetYStartAngle(bool frozen)
        {
            Quaternion handleRotation = frozen ? dragStartRotation : Rotation;

            Vector3 yStartDir = Vector3.Cross(handleRotation.Inverse.Rotate(EditorApplication.SceneViewCamera.SceneObject.Forward), Vector3.yAxis);
            return PointOnCircleToAngle(Vector3.yAxis, yStartDir);
        }

        private Degree GetZStartAngle(bool frozen)
        {
            Quaternion handleRotation = frozen ? dragStartRotation : Rotation;

            Vector3 zStartDir = Vector3.Cross(handleRotation.Inverse.Rotate(EditorApplication.SceneViewCamera.SceneObject.Forward), Vector3.zAxis);
            return PointOnCircleToAngle(Vector3.zAxis, zStartDir);
        }

        private Degree PointOnCircleToAngle(Vector3 up, Vector3 point)
        {
            Quaternion rot = Quaternion.FromToRotation(up, Vector3.yAxis);

            Matrix4 worldToPlane = Matrix4.TRS(Vector3.zero, rot, Vector3.one);
            point = worldToPlane.MultiplyAffine(point);

            return (MathEx.Atan2(-point.z, -point.x) + MathEx.Pi) * MathEx.Rad2Deg;
        }
    }
}
