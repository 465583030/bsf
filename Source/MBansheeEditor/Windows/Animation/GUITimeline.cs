﻿//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using System;
using BansheeEngine;

namespace BansheeEditor
{
    /** @addtogroup AnimationEditor
     *  @{
     */

    // TODO DOC
    public class GUITimeline
    {
        private const float LARGE_TICK_HEIGHT_PCT = 0.4f;
        private const float SMALL_TICK_HEIGHT_PCT = 0.2f;
        private const int PADDING = 30;
        private const int TEXT_PADDING = 2;
        private const int MIN_TICK_DISTANCE = 25;

        private GUICanvas canvas;
        private int width;
        private int height;
        private float rangeStart = 0.0f;
        private float rangeEnd = 60.0f;
        private int fps = 1;

        public GUITimeline(GUILayout layout, int width, int height)
        {
            canvas = new GUICanvas();
            layout.AddElement(canvas);

            SetSize(width, height);
        }

        public void SetSize(int width, int height)
        {
            this.width = width;
            this.height = height;

            canvas.SetWidth(width);
            canvas.SetHeight(height);

            Rebuild();
        }

        public void SetRange(float start, float end)
        {
            rangeStart = start;
            rangeEnd = end;

            Rebuild();
        }

        public void SetFPS(int fps)
        {
            this.fps = fps;

            Rebuild();
        }

        private float CalcTickInterval(float length)
        {
            const int OPTIMAL_TICK_COUNT = 10;
            float[] validIntervals =
            {
                0.001f, 0.005f, 0.010f, 0.025f, 0.050f, 0.100f, 0.250f, 0.500f, // Hundreds of a second
                1.0f, 5.0f, 10.0f, 30.0f, // Seconds
                60.0f, 120.0f, 300.0f, 600.0f, 1800.0f, 3600.0f // Minutes
            };

            int bestIntervalIdx = 0;
            float bestDistance = float.MaxValue;

            for(int i = 0; i < validIntervals.Length; i++)
            {
                float tickCount = length/validIntervals[i];
                float distance = Math.Abs(tickCount - OPTIMAL_TICK_COUNT);
                if (distance < bestDistance)
                {
                    bestDistance = distance;
                    bestIntervalIdx = i;
                }
            }

            // If the draw area is too narrow, limit amount of ticks displayed so they aren't all clumped together
            int numTicks = MathEx.FloorToInt(length / validIntervals[bestIntervalIdx]) + 1;

            int drawableWidth = Math.Max(0, width - PADDING * 2);
            int spacePerTick = drawableWidth/numTicks;

            float bestInterval;
            if (spacePerTick < MIN_TICK_DISTANCE)
            {
                int maxTickCount = drawableWidth/MIN_TICK_DISTANCE;

                bool foundInterval = false;
                for (int i = bestIntervalIdx; i < validIntervals.Length; i++)
                {
                    float tickCount = length/validIntervals[i];
                    if (tickCount <= maxTickCount)
                    {
                        bestIntervalIdx = i;
                        foundInterval = true;
                        break;
                    }
                }

                // Haven't found a valid round interval, try more intervals
                if (!foundInterval)
                {
                    float currentInterval = validIntervals[validIntervals.Length - 1]*2;
                    while (true)
                    {
                        float tickCount = length/ currentInterval;
                        if (tickCount <= maxTickCount)
                        {
                            bestInterval = currentInterval;
                            break;
                        }

                        currentInterval *= 2;
                    }
                }
                else
                    bestInterval = validIntervals[bestIntervalIdx];
            }
            else
                bestInterval = validIntervals[bestIntervalIdx];

            return bestInterval;
        }

        private void DrawTime(int xPos, float seconds, bool minutes)
        {
            TimeSpan timeSpan = TimeSpan.FromSeconds(seconds);

            string timeString;
            if (minutes)
            {
                timeString = timeSpan.TotalMinutes + ":" + timeSpan.Seconds.ToString("D2");
            }
            else
            {
                int hundredths = timeSpan.Milliseconds / 10;
                timeString = timeSpan.TotalSeconds + "." + hundredths.ToString("D3");
            }

            Vector2I textBounds = GUIUtility.CalculateTextBounds(timeString, EditorBuiltin.DefaultFont,
                EditorStyles.DefaultFontSize);

            Vector2I textPosition = new Vector2I();
            textPosition.x = xPos - textBounds.x / 2;
            textPosition.y = TEXT_PADDING;

            canvas.DrawText(timeString, textPosition, EditorBuiltin.DefaultFont, Color.LightGray,
                EditorStyles.DefaultFontSize);
        }

        private void Rebuild()
        {
            canvas.Clear();

            // TODO - Enforce a minimum limit between ticks, so when width is too small they don't come too close together
            // TODO - Don't draw ticks below the frame rate

            // TODO - Text from invisible ticks should be displayed (otherwise it will just pop in was the tick is shown)

            // TODO - Draw small ticks (don't forget to handle offset properly)
            // TODO - Transition between interval sizes more lightly (dynamically change tick height?)

            // Constants
            const int TEXT_SPACING = 10;
            int maxTextWidth = GUIUtility.CalculateTextBounds("99:999", EditorBuiltin.DefaultFont, 
                EditorStyles.DefaultFontSize).x;
            int largeTickHeight = (int)(height * LARGE_TICK_HEIGHT_PCT);
            int drawableWidth = Math.Max(0, width - PADDING * 2);
            float rangeLength = rangeEnd - rangeStart;

            // Draw ticks
            float tickInterval = CalcTickInterval(rangeLength);
            int numTicks = MathEx.FloorToInt(rangeLength / tickInterval) + 1;

            bool displayAsMinutes = TimeSpan.FromSeconds(tickInterval).Minutes > 0;

            float offset = rangeStart % tickInterval - rangeStart;
            float t = offset;
            int lastTextPosition = -1000;
            for (int i = 0; i < numTicks; i++)
            {
                int xPos = (int)((t / rangeLength) * drawableWidth) + PADDING;

                // Draw tick
                Vector2I start = new Vector2I(xPos, height - largeTickHeight);
                Vector2I end = new Vector2I(xPos, height);

                canvas.DrawLine(start, end, Color.LightGray);

                // Draw text if it fits
                int diff = xPos - lastTextPosition;
                if (diff >= (maxTextWidth + TEXT_SPACING))
                {
                    DrawTime(xPos, rangeStart + t, displayAsMinutes);
                    lastTextPosition = xPos;
                }
                
                // Move to next tick
                t += tickInterval;
                t = Math.Min(t, rangeLength);
            }

            //float offsetLarge = MathEx.CeilToInt(rangeStart / largeTickInterval) * largeTickInterval - rangeStart;
            //float offsetSmall = MathEx.CeilToInt(rangeStart / smallTickInterval) * smallTickInterval - rangeStart;

            //int largeTickHeight = (int)(height * LARGE_TICK_HEIGHT_PCT);
            //int smallTickHeight = (int)(height * SMALL_TICK_HEIGHT_PCT);

            //bool drawSmallTicks = true; // TODO

            //float t = offsetSmall;
            //for (int i = 0; i < numVisibleTicks; i++)
            //{
            //    float distanceToLargeTick = MathEx.CeilToInt(t / largeTickInterval) * largeTickInterval - t;
            //    if (MathEx.ApproxEquals(distanceToLargeTick, 0.0f))
            //    {
            //        int xPos = (int)((t/rangeLength)* drawableWidth) + PADDING;

            //        Vector2I start = new Vector2I(xPos, height - largeTickHeight);
            //        Vector2I end = new Vector2I(xPos, height);

            //        canvas.DrawLine(start, end, Color.LightGray);

            //        TimeSpan intervalSpan = TimeSpan.FromSeconds(largeTickInterval);
            //        TimeSpan timeSpan = TimeSpan.FromSeconds(rangeStart + t);

            //        string timeString;
            //        if(intervalSpan.Minutes > 0)
            //            timeString = timeSpan.ToString(@"m\:ss");
            //        else
            //            timeString = timeSpan.ToString(@"ss\:fff");

            //        Vector2I textBounds = GUIUtility.CalculateTextBounds(timeString, EditorBuiltin.DefaultFont, 
            //            EditorStyles.DefaultFontSize);

            //        Vector2I textPosition = new Vector2I();
            //        textPosition.x = xPos - textBounds.x/2;
            //        textPosition.y = TEXT_PADDING;

            //        canvas.DrawText(timeString, textPosition, EditorBuiltin.DefaultFont, Color.LightGray, 
            //            EditorStyles.DefaultFontSize);
            //    }
            //    else
            //    {
            //        if (drawSmallTicks)
            //        {
            //            int xPos = (int)((t / rangeLength) * drawableWidth) + PADDING;

            //            Vector2I start = new Vector2I(xPos, height - smallTickHeight);
            //            Vector2I end = new Vector2I(xPos, height);

            //            canvas.DrawLine(start, end, Color.LightGray);
            //        }
            //    }

            //    t += smallTickInterval;
            //}
        }
    }

    /** @} */
}
