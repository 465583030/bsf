#pragma once

#include "BsGLSLGpuProgram.h"
#include "BsRenderAPI.h"
#include "BsException.h"
#include "BsGLSLParamParser.h"
#include "BsHardwareBufferManager.h"
#include "BsRenderStats.h"
#include "BsGpuParams.h"

namespace BansheeEngine 
{
	UINT32 GLSLGpuProgramCore::mVertexShaderCount = 0;
	UINT32 GLSLGpuProgramCore::mFragmentShaderCount = 0;
	UINT32 GLSLGpuProgramCore::mGeometryShaderCount = 0;
	UINT32 GLSLGpuProgramCore::mDomainShaderCount = 0;
	UINT32 GLSLGpuProgramCore::mHullShaderCount = 0;

	bool checkForGLSLError(const GLuint programObj, String& outErrorMsg)
	{
		StringStream stream;

		GLint linkCompileSuccess = 0;
		glGetProgramiv(programObj, GL_LINK_STATUS, &linkCompileSuccess);

		GLenum glErr;
		bool errorsFound = false;

		glErr = glGetError();
		while (glErr != GL_NO_ERROR)
		{
			const char* glerrStr = (const char*)gluErrorString(glErr);

			if (glerrStr)
			{
				if (errorsFound)
					stream << "\nPrevious errors: \n";

				stream << String(glerrStr) << std::endl;;
			}

			glErr = glGetError();
			errorsFound = true;
		}

		if ((errorsFound || !linkCompileSuccess) && programObj > 0)
		{
			GLint infologLength = 0;
			glGetProgramiv(programObj, GL_INFO_LOG_LENGTH, &infologLength);

			if (infologLength > 0)
			{
				GLint charsWritten = 0;

				GLchar* infoLog = (GLchar*)bs_alloc<ScratchAlloc>(sizeof(GLchar)* infologLength);

				glGetProgramInfoLog(programObj, infologLength, &charsWritten, infoLog);

				stream << "Compile and linker info log: \n";
				stream << String(infoLog);

				bs_free<ScratchAlloc>(infoLog);
			}
		}

		outErrorMsg = stream.str();

		return errorsFound || !linkCompileSuccess;
	}
	
	GLSLGpuProgramCore::GLSLGpuProgramCore(const String& source, const String& entryPoint, GpuProgramType gptype,
		GpuProgramProfile profile, bool isAdjacencyInfoRequired)
		:GpuProgramCore(source, entryPoint, gptype, profile, isAdjacencyInfoRequired),
		mProgramID(0), mGLHandle(0)
    { }

	GLSLGpuProgramCore::~GLSLGpuProgramCore()
    { 
		if (mIsCompiled && mGLHandle != 0)
		{
			glDeleteProgram(mGLHandle);
			mGLHandle = 0;
		}

		BS_INC_RENDER_STAT_CAT(ResDestroyed, RenderStatObject_GpuProgram);
	}

	void GLSLGpuProgramCore::initialize()
	{
		if (!isSupported())
		{
			mIsCompiled = false;
			mCompileError = "Specified program is not supported by the current render system.";

			GpuProgramCore::initialize();
			return;
		}

		GLenum shaderType = 0x0000;
		switch (mProperties.getType())
		{
		case GPT_VERTEX_PROGRAM:
			shaderType = GL_VERTEX_SHADER;
			mProgramID = ++mVertexShaderCount;
			break;
		case GPT_FRAGMENT_PROGRAM:
			shaderType = GL_FRAGMENT_SHADER;
			mProgramID = ++mFragmentShaderCount;
			break;
		case GPT_GEOMETRY_PROGRAM:
			shaderType = GL_GEOMETRY_SHADER;
			mProgramID = ++mGeometryShaderCount;
			break;
		case GPT_HULL_PROGRAM:
			shaderType = GL_TESS_CONTROL_SHADER;
			mProgramID = ++mDomainShaderCount;
			break;
		case GPT_DOMAIN_PROGRAM:
			shaderType = GL_TESS_EVALUATION_SHADER;
			mProgramID = ++mHullShaderCount;
			break;
		}

		// Add preprocessor extras and main source
		const String& source = mProperties.getSource();
		if (!source.empty())
		{
			Vector<GLchar*> lines;

			UINT32 lineLength = 0;
			for (UINT32 i = 0; i < source.size(); i++)
			{
				if (source[i] == '\n' || source[i] == '\r')
				{
					if (lineLength > 0)
					{
						assert(sizeof(source[i]) == sizeof(GLchar));

						bool isDefine = source[i - lineLength] == '#';

						GLchar* lineData = (GLchar*)stackAlloc(sizeof(GLchar) * (lineLength + 1 + (isDefine ? 1 : 0)));
						memcpy(lineData, &source[i - lineLength], sizeof(GLchar) * lineLength);

						if (isDefine) // Defines require a newline as well as a null terminator, otherwise it doesn't compile properly
						{
							lineData[lineLength] = '\n';
							lineData[lineLength + 1] = '\0';
						}
						else
							lineData[lineLength] = '\0';

						lines.push_back(lineData);
						lineLength = 0;
					}
				}
				else
				{
					lineLength++;
				}
			}

			if (lineLength > 0)
			{
				UINT32 end = (UINT32)source.size() - 1;
				assert(sizeof(source[end]) == sizeof(GLchar));

				GLchar* lineData = (GLchar*)stackAlloc(sizeof(GLchar) * (lineLength + 1));
				memcpy(lineData, &source[source.size() - lineLength], sizeof(GLchar) * lineLength);
				lineData[lineLength] = '\0';

				lines.push_back(lineData);
				lineLength = 0;
			}

			mGLHandle = glCreateShaderProgramv(shaderType, (GLsizei)lines.size(), (const GLchar**)lines.data());

			for (auto iter = lines.rbegin(); iter != lines.rend(); ++iter)
			{
				stackDeallocLast(*iter);
			}

			mCompileError = "";
			mIsCompiled = !checkForGLSLError(mGLHandle, mCompileError);
		}

		if (mIsCompiled)
		{
			GLSLParamParser paramParser;
			paramParser.buildUniformDescriptions(mGLHandle, *mParametersDesc);

			if (mProperties.getType() == GPT_VERTEX_PROGRAM)
			{
				List<VertexElement> elementList = paramParser.buildVertexDeclaration(mGLHandle);
				mVertexDeclaration = HardwareBufferCoreManager::instance().createVertexDeclaration(elementList);
			}
		}
		else
		{
			LOGWRN("Shader compilation/linking failed: " + mCompileError);
		}

		BS_INC_RENDER_STAT_CAT(ResCreated, RenderStatObject_GpuProgram);
		GpuProgramCore::initialize();
	}

	bool GLSLGpuProgramCore::isSupported() const
	{
		if (!isRequiredCapabilitiesSupported())
			return false;

		RenderAPICore* rs = BansheeEngine::RenderAPICore::instancePtr();
		return rs->getCapabilities()->isShaderProfileSupported("glsl");
	}
}

