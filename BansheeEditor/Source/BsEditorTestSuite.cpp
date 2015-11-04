#include "BsEditorTestSuite.h"
#include "BsSceneObject.h"
#include "BsCmdRecordSO.h"
#include "BsCmdDeleteSO.h"
#include "BsUndoRedo.h"
#include "BsRTTIType.h"
#include "BsGameObjectRTTI.h"
#include "BsBinarySerializer.h"
#include "BsMemorySerializer.h"
#include "BsBinaryDiff.h"
#include "BsPrefab.h"
#include "BsResources.h"
#include "BsPrefabDiff.h"
#include "BsFrameAlloc.h"
#include "BsFileSystem.h"

namespace BansheeEngine
{
	class TestComponentARTTI : public RTTIType<TestComponentA, Component, TestComponentARTTI>
	{
	private:
		HSceneObject& getRef1(TestComponentA* obj) { return obj->ref1; }
		void setRef1(TestComponentA* obj, HSceneObject& val) { obj->ref1 = val; }

		HComponent& getRef2(TestComponentA* obj) { return obj->ref2; }
		void setRef2(TestComponentA* obj, HComponent& val) { obj->ref2 = val; }

	public:
		TestComponentARTTI()
		{
			addReflectableField("ref1", 0, &TestComponentARTTI::getRef1, &TestComponentARTTI::setRef1);
			addReflectableField("ref2", 1, &TestComponentARTTI::getRef2, &TestComponentARTTI::setRef2);
		}

		virtual const String& getRTTIName() override
		{
			static String name = "TestComponentA";
			return name;
		}

		virtual UINT32 getRTTIId() override
		{
			return TID_TestComponentA;
		}

		virtual std::shared_ptr<IReflectable> newRTTIObject() override
		{
			return GameObjectRTTI::createGameObject<TestComponentA>();
		}
	};

	class TestComponentBRTTI : public RTTIType<TestComponentB, Component, TestComponentBRTTI>
	{
	private:
		HSceneObject& getRef1(TestComponentB* obj) { return obj->ref1; }
		void setRef1(TestComponentB* obj, HSceneObject& val) { obj->ref1 = val; }

		String& getVal1(TestComponentB* obj) { return obj->val1; }
		void setVal1(TestComponentB* obj, String& val) { obj->val1 = val; }

	public:
		TestComponentBRTTI()
		{
			addReflectableField("ref1", 0, &TestComponentBRTTI::getRef1, &TestComponentBRTTI::setRef1);
			addPlainField("val1", 1, &TestComponentBRTTI::getVal1, &TestComponentBRTTI::setVal1);
		}

		virtual const String& getRTTIName() override
		{
			static String name = "TestComponentB";
			return name;
		}

		virtual UINT32 getRTTIId() override
		{
			return TID_TestComponentB;
		}

		virtual std::shared_ptr<IReflectable> newRTTIObject() override
		{
			return GameObjectRTTI::createGameObject<TestComponentB>();
		}
	};

	TestComponentA::TestComponentA(const HSceneObject& parent)
		:Component(parent)
	{}

	RTTITypeBase* TestComponentA::getRTTIStatic()
	{
		return TestComponentARTTI::instance();
	}

	RTTITypeBase* TestComponentA::getRTTI() const
	{
		return TestComponentA::getRTTIStatic();
	}

	TestComponentB::TestComponentB(const HSceneObject& parent)
		:Component(parent)
	{}

	RTTITypeBase* TestComponentB::getRTTIStatic()
	{
		return TestComponentBRTTI::instance();
	}

	RTTITypeBase* TestComponentB::getRTTI() const
	{
		return TestComponentB::getRTTIStatic();
	}

	struct TestObjectB : IReflectable
	{
		UINT32 intA = 100;
		String strA = "100";

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class TestObjectBRTTI;
		static RTTITypeBase* getRTTIStatic();
		virtual RTTITypeBase* getRTTI() const override;
	};

	struct TestObjectA : IReflectable
	{
		TestObjectA()
		{
			arrStrA = { "10", "11", "12" };
			arrStrB = { "13", "14", "15" };
			arrStrC = { "16", "17", "18" };

			arrObjA = { TestObjectB(), TestObjectB(), TestObjectB() };
			arrObjB = { TestObjectB(), TestObjectB(), TestObjectB() };

			arrObjPtrA = { bs_shared_ptr_new<TestObjectB>(), bs_shared_ptr_new<TestObjectB>(), bs_shared_ptr_new<TestObjectB>() };
			arrObjPtrB = { bs_shared_ptr_new<TestObjectB>(), bs_shared_ptr_new<TestObjectB>(), bs_shared_ptr_new<TestObjectB>() };
		}

		UINT32 intA = 5;
		String strA = "5";
		String strB = "7";

		TestObjectB objA;
		TestObjectB objB;

		SPtr<TestObjectB> objPtrA = bs_shared_ptr_new<TestObjectB>();
		SPtr<TestObjectB> objPtrB = bs_shared_ptr_new<TestObjectB>();
		SPtr<TestObjectB> objPtrC = bs_shared_ptr_new<TestObjectB>();
		SPtr<TestObjectB> objPtrD = nullptr;

		Vector<String> arrStrA;
		Vector<String> arrStrB;
		Vector<String> arrStrC;

		Vector<TestObjectB> arrObjA;
		Vector<TestObjectB> arrObjB;

		Vector<SPtr<TestObjectB>> arrObjPtrA;
		Vector<SPtr<TestObjectB>> arrObjPtrB;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class TestObjectARTTI;
		static RTTITypeBase* getRTTIStatic();
		virtual RTTITypeBase* getRTTI() const override;
	};

	class TestObjectARTTI : public RTTIType < TestObjectA, IReflectable, TestObjectARTTI >
	{
	private:
		BS_PLAIN_MEMBER(intA);
		BS_PLAIN_MEMBER(strA);
		BS_PLAIN_MEMBER(strB);

		BS_REFL_MEMBER(objA);
		BS_REFL_MEMBER(objB);

		BS_REFLPTR_MEMBER(objPtrA);
		BS_REFLPTR_MEMBER(objPtrB);
		BS_REFLPTR_MEMBER(objPtrC);
		BS_REFLPTR_MEMBER(objPtrD);

		BS_PLAIN_MEMBER_VEC(arrStrA);
		BS_PLAIN_MEMBER_VEC(arrStrB);
		BS_PLAIN_MEMBER_VEC(arrStrC);

		BS_REFL_MEMBER_VEC(arrObjA);
		BS_REFL_MEMBER_VEC(arrObjB);

		BS_REFLPTR_MEMBER_VEC(arrObjPtrA);
		BS_REFLPTR_MEMBER_VEC(arrObjPtrB);

	public:
		TestObjectARTTI()
		{
			BS_ADD_PLAIN_FIELD(intA, 0);
			BS_ADD_PLAIN_FIELD(strA, 1);
			BS_ADD_PLAIN_FIELD(strB, 2);

			BS_ADD_REFL_FIELD(objA, 3);
			BS_ADD_REFL_FIELD(objB, 4);

			BS_ADD_REFLPTR_FIELD(objPtrA, 5);
			BS_ADD_REFLPTR_FIELD(objPtrB, 6);
			BS_ADD_REFLPTR_FIELD(objPtrC, 7);
			BS_ADD_REFLPTR_FIELD(objPtrD, 8);

			BS_ADD_PLAIN_FIELD_ARR(arrStrA, 9);
			BS_ADD_PLAIN_FIELD_ARR(arrStrB, 10);
			BS_ADD_PLAIN_FIELD_ARR(arrStrC, 11);

			BS_ADD_REFL_FIELD_ARR(arrObjA, 12);
			BS_ADD_REFL_FIELD_ARR(arrObjB, 13);

			BS_ADD_REFLPTR_FIELD_ARR(arrObjPtrA, 14);
			BS_ADD_REFLPTR_FIELD_ARR(arrObjPtrB, 15);
		}

		virtual const String& getRTTIName() override
		{
			static String name = "TestObjectA";
			return name;
		}

		virtual UINT32 getRTTIId() override
		{
			return TID_TestObjectA;
		}

		virtual std::shared_ptr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<TestObjectA>();
		}
	};

	class TestObjectBRTTI : public RTTIType < TestObjectB, IReflectable, TestObjectBRTTI >
	{
	private:
		BS_PLAIN_MEMBER(intA);
		BS_PLAIN_MEMBER(strA);

	public:
		TestObjectBRTTI()
		{
			BS_ADD_PLAIN_FIELD(intA, 0);
			BS_ADD_PLAIN_FIELD(strA, 1);
		}

		virtual const String& getRTTIName() override
		{
			static String name = "TestObjectB";
			return name;
		}

		virtual UINT32 getRTTIId() override
		{
			return TID_TestObjectB;
		}

		virtual std::shared_ptr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<TestObjectB>();
		}
	};

	RTTITypeBase* TestObjectB::getRTTIStatic()
	{
		return TestObjectBRTTI::instance();
	}

	RTTITypeBase* TestObjectB::getRTTI() const
	{
		return TestObjectB::getRTTIStatic();
	}

	RTTITypeBase* TestObjectA::getRTTIStatic()
	{
		return TestObjectARTTI::instance();
	}

	RTTITypeBase* TestObjectA::getRTTI() const
	{
		return TestObjectA::getRTTIStatic();
	}

	class TestComponentC : public Component
	{
	public:
		TestObjectA obj;

		/************************************************************************/
		/* 							COMPONENT OVERRIDES                    		*/
		/************************************************************************/

	protected:
		friend class SceneObject;

		TestComponentC(const HSceneObject& parent)
			:Component(parent)
		{}

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class TestComponentCRTTI;
		static RTTITypeBase* getRTTIStatic();
		virtual RTTITypeBase* getRTTI() const override;

	protected:
		TestComponentC() {} // Serialization only
	};

	class TestComponentD : public Component
	{
	public:
		TestObjectB obj;

		/************************************************************************/
		/* 							COMPONENT OVERRIDES                    		*/
		/************************************************************************/

	protected:
		friend class SceneObject;

		TestComponentD(const HSceneObject& parent)
			:Component(parent)
		{}

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class TestComponentDRTTI;
		static RTTITypeBase* getRTTIStatic();
		virtual RTTITypeBase* getRTTI() const override;

	protected:
		TestComponentD() {} // Serialization only
	};

	class TestComponentCRTTI : public RTTIType < TestComponentC, Component, TestComponentCRTTI >
	{
	private:
		BS_REFL_MEMBER(obj)

	public:
		TestComponentCRTTI()
		{
			BS_ADD_REFL_FIELD(obj, 0);
		}

		virtual const String& getRTTIName() override
		{
			static String name = "TestComponentC";
			return name;
		}

		virtual UINT32 getRTTIId() override
		{
			return TID_TestComponentC;
		}

		virtual std::shared_ptr<IReflectable> newRTTIObject() override
		{
			return GameObjectRTTI::createGameObject<TestComponentC>();
		}
	};

	class TestComponentDRTTI : public RTTIType < TestComponentD, Component, TestComponentDRTTI >
	{
	private:
		BS_REFL_MEMBER(obj)

	public:
		TestComponentDRTTI()
		{
			BS_ADD_REFL_FIELD(obj, 0);
		}

		virtual const String& getRTTIName() override
		{
			static String name = "TestComponentD";
			return name;
		}

		virtual UINT32 getRTTIId() override
		{
			return TID_TestComponentD;
		}

		virtual std::shared_ptr<IReflectable> newRTTIObject() override
		{
			return GameObjectRTTI::createGameObject<TestComponentD>();
		}
	};

	RTTITypeBase* TestComponentC::getRTTIStatic()
	{
		return TestComponentCRTTI::instance();
	}

	RTTITypeBase* TestComponentC::getRTTI() const
	{
		return TestComponentC::getRTTIStatic();
	}

	RTTITypeBase* TestComponentD::getRTTIStatic()
	{
		return TestComponentDRTTI::instance();
	}

	RTTITypeBase* TestComponentD::getRTTI() const
	{
		return TestComponentD::getRTTIStatic();
	}

	EditorTestSuite::EditorTestSuite()
	{
		BS_ADD_TEST(EditorTestSuite::SceneObjectRecord_UndoRedo);
		BS_ADD_TEST(EditorTestSuite::SceneObjectDelete_UndoRedo);
		BS_ADD_TEST(EditorTestSuite::BinaryDiff);
		BS_ADD_TEST(EditorTestSuite::TestPrefabDiff);
		BS_ADD_TEST(EditorTestSuite::TestFrameAlloc)
	}

	void EditorTestSuite::SceneObjectRecord_UndoRedo()
	{
		HSceneObject so0_0 = SceneObject::create("so0_0");
		HSceneObject so1_0 = SceneObject::create("so1_0");
		HSceneObject so1_1 = SceneObject::create("so1_1");
		HSceneObject so2_0 = SceneObject::create("so2_0");

		so1_0->setParent(so0_0);
		so1_1->setParent(so0_0);
		so2_0->setParent(so1_0);

		GameObjectHandle<TestComponentA> cmpA1_1 = so1_1->addComponent<TestComponentA>();
		GameObjectHandle<TestComponentB> cmpB1_1 = so1_1->addComponent<TestComponentB>();

		HSceneObject soExternal = SceneObject::create("soExternal");
		GameObjectHandle<TestComponentA> cmpExternal = soExternal->addComponent<TestComponentA>();

		cmpA1_1->ref1 = so2_0;
		cmpA1_1->ref2 = cmpB1_1;
		cmpB1_1->ref1 = soExternal;
		cmpB1_1->val1 = "InitialValue";
		cmpExternal->ref1 = so1_1;
		cmpExternal->ref2 = cmpA1_1;

		CmdRecordSO::execute(so0_0);
		cmpB1_1->val1 = "ModifiedValue";
		so0_0->setName("modified");
		UndoRedo::instance().undo();

		BS_TEST_ASSERT(!so0_0.isDestroyed());
		BS_TEST_ASSERT(!so1_0.isDestroyed());
		BS_TEST_ASSERT(!so1_1.isDestroyed());
		BS_TEST_ASSERT(!so2_0.isDestroyed());
		BS_TEST_ASSERT(!cmpA1_1.isDestroyed());
		BS_TEST_ASSERT(!cmpB1_1.isDestroyed());
		BS_TEST_ASSERT(!cmpA1_1->ref1.isDestroyed());
		BS_TEST_ASSERT(!cmpA1_1->ref2.isDestroyed());
		BS_TEST_ASSERT(!cmpB1_1->ref1.isDestroyed());
		BS_TEST_ASSERT(!cmpExternal->ref1.isDestroyed());
		BS_TEST_ASSERT(!cmpExternal->ref2.isDestroyed());
		BS_TEST_ASSERT(cmpB1_1->val1 == "InitialValue");
		BS_TEST_ASSERT(so0_0->getName() == "so0_0");

		so0_0->destroy();
		soExternal->destroy();
	}

	void EditorTestSuite::SceneObjectDelete_UndoRedo()
	{
		HSceneObject so0_0 = SceneObject::create("so0_0");
		HSceneObject so1_0 = SceneObject::create("so1_0");
		HSceneObject so1_1 = SceneObject::create("so1_1");
		HSceneObject so2_0 = SceneObject::create("so2_0");

		so1_0->setParent(so0_0);
		so1_1->setParent(so0_0);
		so2_0->setParent(so1_0);

		GameObjectHandle<TestComponentA> cmpA1_1 = so1_1->addComponent<TestComponentA>();
		GameObjectHandle<TestComponentB> cmpB1_1 = so1_1->addComponent<TestComponentB>();

		HSceneObject soExternal = SceneObject::create("soExternal");
		GameObjectHandle<TestComponentA> cmpExternal = soExternal->addComponent<TestComponentA>();

		cmpA1_1->ref1 = so2_0;
		cmpA1_1->ref2 = cmpB1_1;
		cmpB1_1->ref1 = soExternal;
		cmpB1_1->val1 = "InitialValue";
		cmpExternal->ref1 = so1_1;
		cmpExternal->ref2 = cmpA1_1;

		CmdDeleteSO::execute(so0_0);
		UndoRedo::instance().undo();

		BS_TEST_ASSERT(!so0_0.isDestroyed());
		BS_TEST_ASSERT(!so1_0.isDestroyed());
		BS_TEST_ASSERT(!so1_1.isDestroyed());
		BS_TEST_ASSERT(!so2_0.isDestroyed());
		BS_TEST_ASSERT(!cmpA1_1.isDestroyed());
		BS_TEST_ASSERT(!cmpB1_1.isDestroyed());
		BS_TEST_ASSERT(!cmpA1_1->ref1.isDestroyed());
		BS_TEST_ASSERT(!cmpA1_1->ref2.isDestroyed());
		BS_TEST_ASSERT(!cmpB1_1->ref1.isDestroyed());
		BS_TEST_ASSERT(!cmpExternal->ref1.isDestroyed());
		BS_TEST_ASSERT(!cmpExternal->ref2.isDestroyed());
		BS_TEST_ASSERT(cmpB1_1->val1 == "InitialValue");

		so0_0->destroy();
		soExternal->destroy();
	}

	void EditorTestSuite::BinaryDiff()
	{
		SPtr<TestObjectA> orgObj = bs_shared_ptr_new<TestObjectA>();
		SPtr<TestObjectA> newObj = bs_shared_ptr_new<TestObjectA>();

		newObj->intA = 995;
		newObj->strA = "potato";
		newObj->arrStrB = { "orange", "carrot" };
		newObj->arrStrC[2] = "banana";
		newObj->objB.intA = 9940;
		newObj->objPtrB->strA = "kiwi";
		newObj->objPtrC = nullptr;
		newObj->objPtrD = bs_shared_ptr_new<TestObjectB>();
		newObj->arrObjB[1].strA = "strawberry";
		newObj->arrObjPtrB[0]->intA = 99100;

		MemorySerializer ms;
		UINT32 orgDataLength = 0;
		UINT8* orgData = ms.encode(orgObj.get(), orgDataLength, &bs_alloc);

		UINT32 newDataLength = 0;
		UINT8* newData = ms.encode(newObj.get(), newDataLength, &bs_alloc);

		BinarySerializer bs;
		SPtr<SerializedObject> orgSerialized = bs._decodeIntermediate(orgData, orgDataLength);
		SPtr<SerializedObject> newSerialized = bs._decodeIntermediate(newData, newDataLength);

		IDiff& diffHandler = orgObj->getRTTI()->getDiffHandler();
		SPtr<SerializedObject> objDiff = diffHandler.generateDiff(orgSerialized, newSerialized);
		diffHandler.applyDiff(orgObj, objDiff);

		bs_free(orgData);
		bs_free(newData);

		BS_TEST_ASSERT(orgObj->intA == newObj->intA);
		BS_TEST_ASSERT(orgObj->strA == newObj->strA);
		BS_TEST_ASSERT(orgObj->strB == newObj->strB);

		BS_TEST_ASSERT(orgObj->objA.intA == newObj->objA.intA);
		BS_TEST_ASSERT(orgObj->objB.intA == newObj->objB.intA);
		
		BS_TEST_ASSERT(orgObj->objPtrA->strA == newObj->objPtrA->strA);
		BS_TEST_ASSERT(orgObj->objPtrB->strA == newObj->objPtrB->strA);
		BS_TEST_ASSERT(orgObj->objPtrD->strA == newObj->objPtrD->strA);
		BS_TEST_ASSERT(orgObj->objPtrC == newObj->objPtrC);

		BS_TEST_ASSERT(orgObj->arrStrA.size() == newObj->arrStrA.size());
		for (UINT32 i = 0; i < (UINT32)orgObj->arrStrA.size(); i++)
			BS_TEST_ASSERT(orgObj->arrStrA[i] == newObj->arrStrA[i]);

		BS_TEST_ASSERT(orgObj->arrStrB.size() == newObj->arrStrB.size());
		for (UINT32 i = 0; i < (UINT32)orgObj->arrStrB.size(); i++)
			BS_TEST_ASSERT(orgObj->arrStrB[i] == newObj->arrStrB[i]);

		BS_TEST_ASSERT(orgObj->arrStrC.size() == newObj->arrStrC.size());
		for (UINT32 i = 0; i < (UINT32)orgObj->arrStrC.size(); i++)
			BS_TEST_ASSERT(orgObj->arrStrC[i] == newObj->arrStrC[i]);

		BS_TEST_ASSERT(orgObj->arrObjA.size() == newObj->arrObjA.size());
		for (UINT32 i = 0; i < (UINT32)orgObj->arrObjA.size(); i++)
			BS_TEST_ASSERT(orgObj->arrObjA[i].strA == newObj->arrObjA[i].strA);

		BS_TEST_ASSERT(orgObj->arrObjB.size() == newObj->arrObjB.size());
		for (UINT32 i = 0; i < (UINT32)orgObj->arrObjB.size(); i++)
			BS_TEST_ASSERT(orgObj->arrObjB[i].strA == newObj->arrObjB[i].strA);

		BS_TEST_ASSERT(orgObj->arrObjPtrA.size() == newObj->arrObjPtrA.size());
		for (UINT32 i = 0; i < (UINT32)orgObj->arrObjPtrA.size(); i++)
			BS_TEST_ASSERT(orgObj->arrObjPtrA[i]->intA == newObj->arrObjPtrA[i]->intA);

		BS_TEST_ASSERT(orgObj->arrObjPtrB.size() == newObj->arrObjPtrB.size());
		for (UINT32 i = 0; i < (UINT32)orgObj->arrObjPtrB.size(); i++)
			BS_TEST_ASSERT(orgObj->arrObjPtrB[i]->intA == newObj->arrObjPtrB[i]->intA);
	}

	void EditorTestSuite::TestPrefabDiff()
	{
		HSceneObject root = SceneObject::create("root");
		HSceneObject so0 = SceneObject::create("so0");
		HSceneObject so1 = SceneObject::create("so1");
		HSceneObject so2 = SceneObject::create("so2");
		HSceneObject so0_0 = SceneObject::create("so0_0");
		HSceneObject so0_1 = SceneObject::create("so0_1");
		HSceneObject so1_0 = SceneObject::create("so1_0");
		HSceneObject so1_1 = SceneObject::create("so1_1");
		HSceneObject so1_2 = SceneObject::create("so1_2");
		HSceneObject so2_0 = SceneObject::create("so2_0");

		so0->setParent(root);
		so1->setParent(root);
		so2->setParent(root);

		so0_0->setParent(so0);
		so0_1->setParent(so0);

		so1_0->setParent(so1);
		so1_1->setParent(so1);
		so1_2->setParent(so1);

		so2_0->setParent(so2);

		GameObjectHandle<TestComponentC> cmp0 = so0->addComponent<TestComponentC>();
		GameObjectHandle<TestComponentC> cmp0_1_A = so0_1->addComponent<TestComponentC>();
		GameObjectHandle<TestComponentD> cmp0_1_B = so0_1->addComponent<TestComponentD>();

		GameObjectHandle<TestComponentD> cmp1 = so1->addComponent<TestComponentD>();
		GameObjectHandle<TestComponentD> cmp1_2 = so1_2->addComponent<TestComponentD>();

		GameObjectHandle<TestComponentD> cmp2 = so2->addComponent<TestComponentD>();

		Path prefabPath = Path::combine(FileSystem::getTempDirectoryPath(), "testprefab.asset");
		HPrefab prefab = Prefab::create(root);
		gResources().save(prefab, prefabPath, true);

		// Perform modifications
		GameObjectHandle<TestComponentC> cmp1_3;
		GameObjectHandle<TestComponentD> cmp3;
		HSceneObject so1_3, so2_1, so3;
		{
			cmp0->obj.strA = "banana";
			so0_0->destroy();
			cmp0_1_A->destroy();

			so1_3 = SceneObject::create("so1_2");
			so1_3->setParent(so1);
			cmp1_3 = so1_3->addComponent<TestComponentC>();
			cmp1_3->obj.intA = 999;

			so1_0->setName("apple");
			so1_1->destroy();
			cmp1_2->destroy();

			cmp1_2 = so1_2->addComponent<TestComponentD>();
			cmp1_2->obj.strA = "orange";

			so2_1 = SceneObject::create("so2_1");
			so2_1->setParent(so2);

			so2_0->addComponent<TestComponentD>();

			so3 = SceneObject::create("so3");
			so3->setParent(root);
			cmp3 = so3->addComponent<TestComponentD>();
		}

		SPtr<PrefabDiff> prefabDiff = PrefabDiff::create(prefab->_getRoot(), root);

		prefab = gResources().load<Prefab>(prefabPath);
		HSceneObject newRoot = prefab->instantiate();
		prefabDiff->apply(newRoot);

		// Compare and assert
		BS_TEST_ASSERT(root->getNumChildren() == newRoot->getNumChildren());
		HSceneObject nso0 = newRoot->getChild(0);

		GameObjectHandle<TestComponentC> ncmp0 = nso0->getComponent<TestComponentC>();
		BS_TEST_ASSERT(cmp0->obj.strA == ncmp0->obj.strA);

		BS_TEST_ASSERT(so0->getNumChildren() == nso0->getNumChildren());
		HSceneObject nso0_1 = nso0->getChild(0);
		GameObjectHandle<TestComponentC> ncmp0_1 = nso0_1->getComponent<TestComponentD>();
		BS_TEST_ASSERT(ncmp0_1 != nullptr);

		HSceneObject nso1 = newRoot->getChild(1);
		BS_TEST_ASSERT(so1->getNumChildren() == nso1->getNumChildren());

		HSceneObject nso1_0 = nso1->getChild(0);
		BS_TEST_ASSERT(so1_0->getName() == nso1_0->getName());

		HSceneObject nso1_2 = nso1->getChild(1);
		GameObjectHandle<TestComponentD> ncmp1_2 = nso1_2->getComponent<TestComponentD>();
		BS_TEST_ASSERT(cmp1_2->obj.strA == ncmp1_2->obj.strA);

		HSceneObject nso1_3 = nso1->getChild(2);
		GameObjectHandle<TestComponentC> ncmp1_3 = nso1_3->getComponent<TestComponentC>();
		BS_TEST_ASSERT(cmp1_3->obj.intA == ncmp1_3->obj.intA);

		HSceneObject nso2 = newRoot->getChild(2);
		BS_TEST_ASSERT(so2->getNumChildren() == nso2->getNumChildren());

		HSceneObject nso2_0 = nso2->getChild(0);
		GameObjectHandle<TestComponentD> ncmp2_0 = nso2_0->getComponent<TestComponentD>();
		BS_TEST_ASSERT(ncmp2_0 != nullptr);

		HSceneObject nso3 = newRoot->getChild(3);
		GameObjectHandle<TestComponentD> ncmp3 = nso3->getComponent<TestComponentD>();
		BS_TEST_ASSERT(ncmp3 != nullptr);

		root->destroy();
		newRoot->destroy();
	}

	void EditorTestSuite::TestFrameAlloc()
	{
		FrameAlloc alloc(128);
		alloc.markFrame();
		UINT8* a1 = alloc.alloc(5);
		UINT8* a2 = alloc.alloc(10);
		UINT8* a3 = alloc.alloc(130);
		UINT8* a4 = alloc.alloc(5);

		alloc.dealloc(a1);
		alloc.dealloc(a2);
		alloc.dealloc(a3);
		alloc.dealloc(a4);

		alloc.clear();

		alloc.markFrame();
		UINT8* a5 = alloc.alloc(5);
		UINT8* a6 = alloc.alloc(10);
		UINT8* a7 = alloc.alloc(130);
		UINT8* a8 = alloc.alloc(5);

		alloc.dealloc(a5);
		alloc.dealloc(a6);
		alloc.dealloc(a7);
		alloc.dealloc(a8);

		alloc.markFrame();
		UINT8* a9 = alloc.alloc(5);
		UINT8* a10 = alloc.alloc(10);
		UINT8* a11 = alloc.alloc(130);
		UINT8* a12 = alloc.alloc(5);

		alloc.dealloc(a9);
		alloc.dealloc(a10);
		alloc.dealloc(a11);
		alloc.dealloc(a12);

		alloc.clear();
		alloc.clear();

		UINT8* a13 = alloc.alloc(5);
		alloc.dealloc(a13);
		alloc.clear();
	}
}