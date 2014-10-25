#pragma once

#include "BsEditorPrerequisites.h"
#include "BsModule.h"
#include "BsDegree.h"

namespace BansheeEngine
{
	class BS_ED_EXPORT HandleManager : public Module<HandleManager>
	{
	public:
		HandleManager();
		virtual ~HandleManager();

		void handleInput(const Vector2I& inputPos, const Ray& inputRay, bool pressed);
		void update();

		HandleSliderManager& getSliderManager() const { return *mSliderManager; }
		HandleDrawManager& getDrawManager() const { return *mDrawManager; }

		float getHandleSize(const HCamera& camera, const Vector3& handlePos) const;

		void setDefaultHandleSize(float value) { mDefaultHandleSize = value; }
		void setMoveHandleSnapAmount(float value) { mMoveHandleSnapAmount = value; }
		void setRotateHandleSnapAmount(Degree value) { mRotateHandleSnapAmount = value; }
		void setScaleHandleSnapAmount(float value) { mScaleHandleSnapAmount = value; }

		bool isHandleActive() const;

	protected:
		HandleSliderManager* mSliderManager;
		HandleDrawManager* mDrawManager;

		float mDefaultHandleSize = 20.0f;
		float mMoveHandleSnapAmount = 0.1f;
		Degree mRotateHandleSnapAmount = Degree(20.0f);
		float mScaleHandleSnapAmount = 0.1f;

		virtual void refreshHandles() = 0;
		virtual void triggerHandles() = 0;
		virtual void queueDrawCommands() = 0;
	};
}