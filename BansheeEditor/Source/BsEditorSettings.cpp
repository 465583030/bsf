#include "BsEditorSettings.h"

namespace BansheeEngine
{
	EditorSettings::EditorSettings()
		:mMoveSnapActive(false), mRotateSnapActive(false), mMoveSnap(0.1f), mRotationSnap(20.0f), 
		mGridSize(256), mGridAxisSpacing(1.0f), mGridMajorAxisSpacing(10), mGridAxisMarkerSpacing(25), 
		mHandleSize(0.15f), mHash(0), mActiveSceneTool(1 /* Move */), mActiveCoordinateMode(0), mActivePivotMode(0)
	{ }
}