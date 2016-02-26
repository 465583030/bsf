//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "BsJoint.h"
#include "BsD6Joint.h"
#include "BsHingeJoint.h"

namespace BansheeEngine
{
	/** Helper class for dealing with D6Joint::Drive structure. */
	class BS_SCR_BE_EXPORT ScriptD6JointDrive : public ScriptObject<ScriptD6JointDrive>
	{
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "D6JointDrive")

	public:
		/** Converts managed limit to its native counterpart. */
		static D6Joint::Drive convert(MonoObject* object);

	private:
		ScriptD6JointDrive(MonoObject* instance);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		typedef void(__stdcall *GetNativeDataThunkDef) (MonoObject*, D6Joint::Drive*, MonoException**);

		static GetNativeDataThunkDef getNativeDataThunk;
	};

	/** Helper class for dealing with HingeJoint::Drive structure. */
	class BS_SCR_BE_EXPORT ScriptHingeJointDrive : public ScriptObject<ScriptHingeJointDrive>
	{
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "HingeJointDrive")

	public:
		/** Converts managed limit to its native counterpart. */
		static HingeJoint::Drive convert(MonoObject* object);

	private:
		ScriptHingeJointDrive(MonoObject* instance);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		typedef void(__stdcall *GetNativeDataThunkDef) (MonoObject*, HingeJoint::Drive*, MonoException**);

		static GetNativeDataThunkDef getNativeDataThunk;
	};

	/** Helper class for dealing with LimitLinearRange structure. */
	class BS_SCR_BE_EXPORT ScriptLimitLinearRange : public ScriptObject<ScriptLimitLinearRange>
	{
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "LimitLinearRange")

	public:
		/** Converts managed limit to its native counterpart. */
		static LimitLinearRange convert(MonoObject* object);

	private:
		ScriptLimitLinearRange(MonoObject* instance);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		typedef void(__stdcall *GetNativeDataThunkDef) (MonoObject*, LimitLinearRange*, MonoException**);

		static GetNativeDataThunkDef getNativeDataThunk;
	};

	/** Helper class for dealing with LimitLinear structure. */
	class BS_SCR_BE_EXPORT ScriptLimitLinear : public ScriptObject<ScriptLimitLinear>
	{
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "LimitLinear")

	public:
		/** Converts managed limit to its native counterpart. */
		static LimitLinear convert(MonoObject* object);

	private:
		ScriptLimitLinear(MonoObject* instance);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		typedef void(__stdcall *GetNativeDataThunkDef) (MonoObject*, LimitLinear*, MonoException**);

		static GetNativeDataThunkDef getNativeDataThunk;
	};

	/** Helper class for dealing with LimitAngularRange structure. */
	class BS_SCR_BE_EXPORT ScriptLimitAngularRange : public ScriptObject<ScriptLimitAngularRange>
	{
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "LimitAngularRange")

	public:
		/** Converts managed limit to its native counterpart. */
		static LimitAngularRange convert(MonoObject* object);

	private:
		ScriptLimitAngularRange(MonoObject* instance);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		typedef void(__stdcall *GetNativeDataThunkDef) (MonoObject*, LimitAngularRange*, MonoException**);

		static GetNativeDataThunkDef getNativeDataThunk;
	};

	/** Helper class for dealing with LimitConeRange structure. */
	class BS_SCR_BE_EXPORT ScriptLimitConeRange : public ScriptObject<ScriptLimitConeRange>
	{
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "LimitConeRange")

	public:
		/** Converts managed limit to its native counterpart. */
		static LimitConeRange convert(MonoObject* object);

	private:
		ScriptLimitConeRange(MonoObject* instance);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		typedef void(__stdcall *GetNativeDataThunkDef) (MonoObject*, LimitConeRange*, MonoException**);

		static GetNativeDataThunkDef getNativeDataThunk;
	};
}