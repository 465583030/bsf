#pragma once

#include "BsCorePrerequisites.h"
#include "BsGpuResourceData.h"
#include "BsVertexBuffer.h"
#include "BsIndexBuffer.h"
#include "BsVertexDeclaration.h"
#include "BsDrawOps.h"
#include "BsSubMesh.h"
#include "BsBounds.h"

namespace BansheeEngine
{
	/**
	 * @brief	Iterator that allows you to easily populate or read vertex elements
	 *			in MeshData.
	 */
	template<class T>
	class VertexElemIter
	{
	public:
		VertexElemIter()
			:mData(nullptr), mEnd(nullptr), mByteStride(0), mNumElements(0)
		{

		}

		VertexElemIter(UINT8* data, UINT32 byteStride, UINT32 numElements)
			:mData(data), mByteStride(byteStride), mNumElements(numElements)
		{
			mEnd = mData + byteStride * numElements;
		}

		/**
		 * @brief	Adds a new value to the iterators current position and
		 *			advances the iterator.
		 */
		void addValue(const T& value)
		{
			setValue(value);
			moveNext();
		}

		/**
		 * @brief	Sets a new value at the iterators current position.
		 */
		void setValue(const T& value)
		{
			memcpy(mData, &value, sizeof(T));
		}

		/**
		 * @brief	Returns the value at the iterators current position.
		 */
		T& getValue()
		{
			return *((T*)mData);
		}

		/**
		 * @brief	Moves the iterator to the next position. Returns true
		 *			if there are more elements.
		 */
		bool moveNext()
		{
#ifdef BS_DEBUG_MODE
			if(mData >= mEnd)
			{
				BS_EXCEPT(InternalErrorException, "Vertex element iterator out of buffer bounds.");
			}
#endif

			mData += mByteStride;

			return mData < mEnd;
		}

		/**
		 * @brief	Returns the number of elements this iterator can iterate over.
		 */
		UINT32 getNumElements() const { return mNumElements; }

	private:
		UINT8* mData;
		UINT8* mEnd;
		UINT32 mByteStride;
		UINT32 mNumElements;
	};

	/**
	 * @brief	Used for initializing, updating and reading mesh data from Meshes.
	 */
	class BS_CORE_EXPORT MeshData : public GpuResourceData
	{
	public:
		/**
		 * @brief	Constructs a new object that can hold number of vertices described by the provided vertex data description. As well
		 *			as a number of indices of the provided type.
		 */
		MeshData(UINT32 numVertices, UINT32 numIndexes, const VertexDataDescPtr& vertexData, IndexBuffer::IndexType indexType = IndexBuffer::IT_32BIT);
		~MeshData();

		/**
		 * @brief	Copies data from "data" parameter into the internal buffer for the specified semantic.
		 *
		 * @param	semantic   		Semantic that allows the engine to connect the data to a shader input slot.
		 * @param	data			Vertex data, containing at least "size" bytes.
		 * @param	size			The size of the data. Must be the size of the vertex element type * number of vertices.
		 * @param	semanticIdx 	(optional) If there are multiple semantics with the same name, use different index to differentiate between them.
		 * @param	streamIdx   	(optional) Zero-based index of the stream. Each stream will internally be represented as a single vertex buffer.
		 */
		void setVertexData(VertexElementSemantic semantic, UINT8* data, UINT32 size, UINT32 semanticIdx = 0, UINT32 streamIdx = 0);

		/**
		 * @brief	Returns an iterator you can use for easily retrieving or setting Vector2 vertex elements. This is the preferred
		 * 			method of assigning or reading vertex data. 
		 * 			
		 * @note	If vertex data of this type/semantic/index/stream doesn't exist and exception will be thrown.
		 */
		VertexElemIter<Vector2> getVec2DataIter(VertexElementSemantic semantic, UINT32 semanticIdx = 0, UINT32 streamIdx = 0);

		/**
		 * @brief	Returns an iterator you can use for easily retrieving or setting Vector3 vertex elements. This is the preferred
		 * 			method of assigning or reading vertex data. 
		 * 			
		 * @note	If vertex data of this type/semantic/index/stream doesn't exist and exception will be thrown.
		 */
		VertexElemIter<Vector3> getVec3DataIter(VertexElementSemantic semantic, UINT32 semanticIdx = 0, UINT32 streamIdx = 0);

		/**
		 * @brief	Returns an iterator you can use for easily retrieving or setting Vector4 vertex elements. This is the preferred
		 * 			method of assigning or reading vertex data. 
		 * 			
		 * @note	If vertex data of this type/semantic/index/stream doesn't exist and exception will be thrown.
		 */
		VertexElemIter<Vector4> getVec4DataIter(VertexElementSemantic semantic, UINT32 semanticIdx = 0, UINT32 streamIdx = 0);

		/**
		 * @brief	Returns an iterator you can use for easily retrieving or setting DWORD vertex elements. This is the preferred
		 * 			method of assigning or reading vertex data. 
		 * 			
		 * @note	If vertex data of this type/semantic/index/stream doesn't exist and exception will be thrown.
		 */
		VertexElemIter<UINT32> getDWORDDataIter(VertexElementSemantic semantic, UINT32 semanticIdx = 0, UINT32 streamIdx = 0);

		/**
		 * @brief	Returns the total number of vertices this object can hold.
		 */
		UINT32 getNumVertices() const { return mNumVertices; }

		/**
		 * @brief	Returns the total number of indices this object can hold.
		 */
		UINT32 getNumIndices() const;

		/**
		 * @brief	Returns a 16-bit pointer to the start of the internal index buffer.
		 */
		UINT16* getIndices16() const;

		/**
		 * @brief	Returns a 32-bit pointer to the start of the internal index buffer.
		 */
		UINT32* getIndices32() const;

		/**
		 * @brief	Returns the size of an index element in bytes.
		 */
		UINT32 getIndexElementSize() const;

		/**
		 * @brief	Returns the type of an index element.
		 */
		IndexBuffer::IndexType getIndexType() const { return mIndexType; }

		/**
		 * @brief	Returns the pointer to the first element of the specified type. If you want to
		 * 			iterate over all elements you need to call getVertexStride() to get the number
		 * 			of bytes you need to advance between each element.
		 *
		 * @param	semantic   		Semantic that allows the engine to connect the data to a shader input slot.
		 * @param	semanticIdx 	(optional) If there are multiple semantics with the same name, use different index to differentiate between them.
		 * @param	streamIdx   	(optional) Zero-based index of the stream. Each stream will internally be represented as a single vertex buffer.
		 *
		 * @return	null if it fails, else the element data.
		 */
		UINT8* getElementData(VertexElementSemantic semantic, UINT32 semanticIdx = 0, UINT32 streamIdx = 0) const;

		/**
		 * @brief	Returns an offset into the internal buffer where this element with the provided semantic starts. 
		 *			Offset is provided in number of bytes.
		 * 
		 * @param	semantic   		Semantic that allows the engine to connect the data to a shader input slot.
		 * @param	semanticIdx 	(optional) If there are multiple semantics with the same name, use different index to differentiate between them.
		 * @param	streamIdx   	(optional) Zero-based index of the stream. Each stream will internally be represented as a single vertex buffer.
		 */
		UINT32 getElementOffset(VertexElementSemantic semantic, UINT32 semanticIdx = 0, UINT32 streamIdx = 0) const;

		/**
		 * @brief	Returns an object that describes data contained in a single vertex.
		 */
		const VertexDataDescPtr& getVertexDesc() const { return mVertexData; }

		/**
		 * @brief	Combines a number of submeshes and their mesh data into one large mesh data buffer.
		 *
		 * @param	elements	Data containing vertices and indices referenced by the submeshes. Number of elements
		 *						must be the same as number of submeshes.
		 * @param	subMeshes	Submeshes representing vertex and index range to take from mesh data and combine. Number of
		 *						submeshes must match the number of provided MeshData elements.
		 * @param	[out] subMeshes		Outputs all combined sub-meshes with their new index and vertex offsets referencing
		 *								the newly created MeshData.
		 *
		 * @returns	Combined mesh data containing all vertices and indexes references by the provided sub-meshes.
		 */
		static MeshDataPtr combine(const Vector<MeshDataPtr>& elements, const Vector<Vector<SubMesh>>& allSubMeshes,
			Vector<SubMesh>& subMeshes);

	protected:
		/**
		 * @brief	Returns the size of the internal buffer in bytes.
		 */
		UINT32 getInternalBufferSize();

	private:
		/**
		 * @brief	Returns a pointer to the start of the index buffer.
		 */
		UINT8* getIndexData() const { return getData(); }

		/**
		 * @brief	Returns a pointer to the start of the specified vertex stream.
		 */
		UINT8* getStreamData(UINT32 streamIdx) const;

		/**
		 * @brief	Returns an offset in bytes to the start of the index buffer from the start of the
		 *			internal buffer.
		 */
		UINT32 getIndexBufferOffset() const;

		/**
		 * @brief	Returns an offset in bytes to the start of the stream from the start of the
		 *			internal buffer.
		 */
		UINT32 getStreamOffset(UINT32 streamIdx = 0) const;

		/**
		 * @brief	Returns the size of the index buffer in bytes.
		 */
		UINT32 getIndexBufferSize() const;

		/**
		 * @brief	Returns the size of the specified stream in bytes.
		 */
		UINT32 getStreamSize(UINT32 streamIdx) const;

		/**
		 * @brief	Returns the size of all the streams in bytes.
		 */
		UINT32 getStreamSize() const;

		/**
		 * @brief	Returns the data needed for iterating over the requested vertex element.
		 *
		 * @param	semantic   		Semantic of the element we are looking for.
		 * @param	semanticIdx 	If there are multiple semantics with the same name, use different index to differentiate between them.
		 * @param	streamIdx   	Zero-based index of the stream the element resides in.
		 * @param [out] data		Pointer to the start of this elements data.
		 * @param [out] stride		Number of bytes between vertex elements of this type.
		 */
		void getDataForIterator(VertexElementSemantic semantic, UINT32 semanticIdx, UINT32 streamIdx, UINT8*& data, UINT32& stride) const;

	private:
		friend class Mesh;
		friend class MeshHeap;

		UINT32 mDescBuilding;

		UINT32 mNumVertices;
		UINT32 mNumIndices;
		IndexBuffer::IndexType mIndexType;

		VertexDataDescPtr mVertexData;

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	private:
		MeshData(); // Serialization only

	public:
		friend class MeshDataRTTI;
		static RTTITypeBase* getRTTIStatic();
		virtual RTTITypeBase* getRTTI() const;
	};
}