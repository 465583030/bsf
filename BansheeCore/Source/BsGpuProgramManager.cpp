#include "BsGpuProgramManager.h"
#include "BsRenderSystem.h"

namespace BansheeEngine 
{
	String sNullLang = "null";

	/**
	 * @brief	Null GPU program used in place of GPU programs we cannot create.
	 *			Null programs don't do anything.
	 */
	class NullProgramCore : public GpuProgramCore
	{
	public:
		NullProgramCore()
			:GpuProgramCore("", "", GPT_VERTEX_PROGRAM, GPP_NONE, nullptr)
		{ }

		~NullProgramCore() { }

		bool isSupported() const { return false; }
		const String& getLanguage() const { return sNullLang; }

	protected:
		void loadFromSource() {}

		void buildConstantDefinitions() const { }
	};

	/**
	 * @brief	Factory that creates null GPU programs. 
	 */
	class NullProgramFactory : public GpuProgramFactory
	{
	public:
		NullProgramFactory() {}
		~NullProgramFactory() {}

		const String& getLanguage() const 
		{ 
			return sNullLang;
		}

		SPtr<GpuProgramCore> create(const String& source, const String& entryPoint, GpuProgramType gptype, 
			GpuProgramProfile profile, bool requiresAdjacencyInformation)
		{
			return bs_shared_ptr<NullProgramCore>();
		}

		SPtr<GpuProgramCore> create(GpuProgramType type)
		{
			return bs_shared_ptr<NullProgramCore>();
		}
	};

	GpuProgramPtr GpuProgramManager::create(const String& source, const String& entryPoint, const String& language,
		GpuProgramType gptype, GpuProgramProfile profile, const Vector<HGpuProgInclude>* includes,
		bool requiresAdjacencyInformation)
	{
		GpuProgram* program = new (bs_alloc<GpuProgram>()) GpuProgram(source, entryPoint, language, gptype, profile, includes, requiresAdjacencyInformation);
		GpuProgramPtr ret = bs_core_ptr<GpuProgram, GenAlloc>(program);
		ret->_setThisPtr(ret);
		ret->initialize();

		return ret;
	}

	GpuProgramPtr GpuProgramManager::createEmpty(const String& language, GpuProgramType type)
	{
		GpuProgram* program = new (bs_alloc<GpuProgram>()) GpuProgram("", "", language, GPT_VERTEX_PROGRAM, GPP_VS_1_1, nullptr, false);
		GpuProgramPtr ret = bs_core_ptr<GpuProgram, GenAlloc>(program);
		ret->_setThisPtr(ret);

		return ret;
	}

	GpuProgramCoreManager::GpuProgramCoreManager()
	{
		mNullFactory = bs_new<NullProgramFactory>();
		addFactory(mNullFactory);
	}

	GpuProgramCoreManager::~GpuProgramCoreManager()
	{
		bs_delete((NullProgramFactory*)mNullFactory);
	}

	void GpuProgramCoreManager::addFactory(GpuProgramFactory* factory)
	{
		mFactories[factory->getLanguage()] = factory;
	}

	void GpuProgramCoreManager::removeFactory(GpuProgramFactory* factory)
    {
        FactoryMap::iterator it = mFactories.find(factory->getLanguage());
        if (it != mFactories.end() && it->second == factory)
        {
            mFactories.erase(it);
        }
    }

	GpuProgramFactory* GpuProgramCoreManager::getFactory(const String& language)
	{
		FactoryMap::iterator i = mFactories.find(language);

		if (i == mFactories.end())
			i = mFactories.find(sNullLang);

		return i->second;
	}

	bool GpuProgramCoreManager::isLanguageSupported(const String& lang)
	{
		FactoryMap::iterator i = mFactories.find(lang);

		return i != mFactories.end();
	}

	SPtr<GpuProgramCore> GpuProgramCoreManager::create(const String& source, const String& entryPoint, const String& language,
		GpuProgramType gptype, GpuProgramProfile profile, bool requiresAdjacencyInformation)
    {
		SPtr<GpuProgramCore> ret = createInternal(source, entryPoint, language, gptype, profile, requiresAdjacencyInformation);
		ret->initialize();

        return ret;
    }

	SPtr<GpuProgramCore> GpuProgramCoreManager::createInternal(const String& source, const String& entryPoint, const String& language,
		GpuProgramType gptype, GpuProgramProfile profile, bool requiresAdjacencyInformation)
	{
		GpuProgramFactory* factory = getFactory(language);
		SPtr<GpuProgramCore> ret = factory->create(source, entryPoint, gptype, profile, requiresAdjacencyInformation);

		return ret;
	}
}
