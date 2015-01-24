#include "BsShapeMeshes3D.h"
#include "BsRect2.h"
#include "BsMesh.h"
#include "BsTime.h"
#include "BsVector2.h"
#include "BsQuaternion.h"
#include "BsSphere.h"
#include "BsMaterial.h"
#include "BsPass.h"
#include "BsCoreApplication.h"
#include "BsRenderQueue.h"
#include "BsException.h"
#include "BsCamera.h"
#include "BsBuiltinResources.h"
#include "BsVertexDataDesc.h"

namespace BansheeEngine
{
	const UINT32 ShapeMeshes3D::NUM_VERTICES_AA_LINE = 8;
	const UINT32 ShapeMeshes3D::NUM_INDICES_AA_LINE = 30;

	inline UINT8* writeVector3(UINT8* buffer, UINT32 stride, const Vector3& value)
	{
		*(Vector3*)buffer = value;
		return buffer + stride;
	}

	void ShapeMeshes3D::wireAABox(const AABox& box, const MeshDataPtr& meshData, UINT32 vertexOffset, UINT32 indexOffset)
	{
		UINT32* indexData = meshData->getIndices32();
		UINT8* positionData = meshData->getElementData(VES_POSITION);

		assert((vertexOffset + 8) <= meshData->getNumVertices());
		assert((indexOffset + 24) <= meshData->getNumIndices());	

		wireAABox(box, positionData, vertexOffset, meshData->getVertexDesc()->getVertexStride(), indexData, indexOffset);
	}

	void ShapeMeshes3D::solidAABox(const AABox& box, const MeshDataPtr& meshData, UINT32 vertexOffset, UINT32 indexOffset)
	{
		UINT32* indexData = meshData->getIndices32();
		UINT8* positionData = meshData->getElementData(VES_POSITION);
		UINT8* normalData = meshData->getElementData(VES_NORMAL);

		assert((vertexOffset + 24) <= meshData->getNumVertices());
		assert((indexOffset + 36) <= meshData->getNumIndices());

		solidAABox(box, positionData, normalData, vertexOffset, meshData->getVertexDesc()->getVertexStride(), indexData, indexOffset);
	}

	void ShapeMeshes3D::wireSphere(const Sphere& sphere, const MeshDataPtr& meshData, UINT32 vertexOffset, UINT32 indexOffset, UINT32 quality)
	{
		UINT32* indexData = meshData->getIndices32();
		UINT8* positionData = meshData->getElementData(VES_POSITION);

		UINT32 requiredNumVertices, requiredNumIndices;
		getNumElementsWireSphere(quality, requiredNumVertices, requiredNumIndices);

		assert((vertexOffset + requiredNumVertices) <= meshData->getNumVertices());
		assert((indexOffset + requiredNumIndices) <= meshData->getNumIndices());

		UINT32 verticesPerArc = (quality + 1) * 5;
		UINT32 indicesPerArc = (verticesPerArc - 1) * 2;

		wireDisc(sphere.getCenter(), sphere.getRadius(), Vector3::UNIT_X, meshData,
			vertexOffset, indexOffset, quality);

		wireDisc(sphere.getCenter(), sphere.getRadius(), Vector3::UNIT_Y, meshData,
			vertexOffset + verticesPerArc, indexOffset + indicesPerArc, quality);

		wireDisc(sphere.getCenter(), sphere.getRadius(), Vector3::UNIT_Z, meshData,
			vertexOffset + verticesPerArc * 2, indexOffset + indicesPerArc * 2, quality);
	}

	void ShapeMeshes3D::solidSphere(const Sphere& sphere, const MeshDataPtr& meshData, UINT32 vertexOffset, UINT32 indexOffset, UINT32 quality)
	{
		UINT32* indexData = meshData->getIndices32();
		UINT8* positionData = meshData->getElementData(VES_POSITION);
		UINT8* normalData = meshData->getElementData(VES_NORMAL);

		UINT32 requiredNumVertices, requiredNumIndices;
		getNumElementsSphere(quality, requiredNumVertices, requiredNumIndices);

		assert((vertexOffset + requiredNumVertices) <= meshData->getNumVertices());
		assert((indexOffset + requiredNumIndices) <= meshData->getNumIndices());

		solidSphere(sphere, positionData, normalData, vertexOffset, 
			meshData->getVertexDesc()->getVertexStride(), indexData, indexOffset, quality);
	}

	void ShapeMeshes3D::wireDisc(const Vector3& center, float radius, const Vector3& normal, const MeshDataPtr& meshData,
		UINT32 vertexOffset, UINT32 indexOffset, UINT32 quality)
	{
		wireArc(center, radius, normal, Degree(0), Degree(360), meshData, vertexOffset, indexOffset, quality);
	}

	void ShapeMeshes3D::solidDisc(const Vector3& center, float radius, const Vector3& normal, const MeshDataPtr& meshData,
		UINT32 vertexOffset, UINT32 indexOffset, UINT32 quality)
	{
		solidArc(center, radius, normal, Degree(0), Degree(360), meshData, vertexOffset, indexOffset, quality);
	}

	void ShapeMeshes3D::wireArc(const Vector3& center, float radius, const Vector3& normal, Degree startAngle, Degree amountAngle,
		const MeshDataPtr& meshData, UINT32 vertexOffset, UINT32 indexOffset, UINT32 quality)
	{
		UINT32* indexData = meshData->getIndices32();
		UINT8* positionData = meshData->getElementData(VES_POSITION);

		UINT32 requiredNumVertices, requiredNumIndices;
		getNumElementsWireArc(quality, requiredNumVertices, requiredNumIndices);

		assert((vertexOffset + requiredNumVertices) <= meshData->getNumVertices());
		assert((indexOffset + requiredNumIndices) <= meshData->getNumIndices());

		wireArc(center, radius, normal, startAngle, amountAngle, positionData, vertexOffset,
			meshData->getVertexDesc()->getVertexStride(), indexData, indexOffset, quality);
	}

	void ShapeMeshes3D::solidArc(const Vector3& center, float radius, const Vector3& normal, Degree startAngle, Degree amountAngle,
		const MeshDataPtr& meshData, UINT32 vertexOffset, UINT32 indexOffset, UINT32 quality)
	{
		UINT32* indexData = meshData->getIndices32();
		UINT8* positionData = meshData->getElementData(VES_POSITION);
		UINT8* normalData = meshData->getElementData(VES_NORMAL);

		UINT32 requiredNumVertices, requiredNumIndices;
		getNumElementsArc(quality, requiredNumVertices, requiredNumIndices);

		assert((vertexOffset + requiredNumVertices) <= meshData->getNumVertices());
		assert((indexOffset + requiredNumIndices) <= meshData->getNumIndices());

		solidArc(center, radius, normal, startAngle, amountAngle, positionData, normalData, vertexOffset,
			meshData->getVertexDesc()->getVertexStride(), indexData, indexOffset, quality);
	}

	void ShapeMeshes3D::wireFrustum(const Vector3& position, float aspect, Degree FOV, float near, float far,
		const MeshDataPtr& meshData, UINT32 vertexOffset, UINT32 indexOffset)
	{
		UINT32* indexData = meshData->getIndices32();
		UINT8* positionData = meshData->getElementData(VES_POSITION);

		assert((vertexOffset + 8) <= meshData->getNumVertices());
		assert((indexOffset + 24) <= meshData->getNumIndices());

		wireFrustum(position, aspect, FOV, near, far, positionData, vertexOffset, meshData->getVertexDesc()->getVertexStride(), indexData, indexOffset);
	}

	void ShapeMeshes3D::solidCone(const Vector3& base, const Vector3& normal, float height, float radius,
		const MeshDataPtr& meshData, UINT32 vertexOffset, UINT32 indexOffset, UINT32 quality)
	{
		UINT32* indexData = meshData->getIndices32();
		UINT8* positionData = meshData->getElementData(VES_POSITION);
		UINT8* normalData = meshData->getElementData(VES_NORMAL);

		UINT32 requiredNumVertices, requiredNumIndices;
		getNumElementsCone(quality, requiredNumVertices, requiredNumIndices);

		assert((vertexOffset + requiredNumVertices) <= meshData->getNumVertices());
		assert((indexOffset + requiredNumIndices) <= meshData->getNumIndices());

		solidCone(base, normal, height, radius, positionData, normalData, vertexOffset,
			meshData->getVertexDesc()->getVertexStride(), indexData, indexOffset, quality);
	}

	void ShapeMeshes3D::solidQuad(const Rect3& area, const MeshDataPtr& meshData, UINT32 vertexOffset, UINT32 indexOffset)
	{
		UINT32* indexData = meshData->getIndices32();
		UINT8* positionData = meshData->getElementData(VES_POSITION);
		UINT8* normalData = meshData->getElementData(VES_NORMAL);

		assert((vertexOffset + 8) <= meshData->getNumVertices());
		assert((indexOffset + 12) <= meshData->getNumIndices());

		solidQuad(area, positionData, normalData, vertexOffset,
			meshData->getVertexDesc()->getVertexStride(), indexData, indexOffset);
	}

	void ShapeMeshes3D::pixelLine(const Vector3& a, const Vector3& b, const MeshDataPtr& meshData, UINT32 vertexOffset, UINT32 indexOffset)
	{
		UINT32* indexData = meshData->getIndices32();
		UINT8* positionData = meshData->getElementData(VES_POSITION);

		assert((vertexOffset + 2) <= meshData->getNumVertices());
		assert((indexOffset + 2) <= meshData->getNumIndices());

		pixelLine(a, b, positionData, vertexOffset, meshData->getVertexDesc()->getVertexStride(), indexData, indexOffset);
	}

	void ShapeMeshes3D::antialiasedLine(const Vector3& a, const Vector3& b, const Vector3& up, float width, float borderWidth, 
		const Color& color, const MeshDataPtr& meshData, UINT32 vertexOffset, UINT32 indexOffset)
	{
		UINT32* indexData = meshData->getIndices32();
		UINT8* positionData = meshData->getElementData(VES_POSITION);
		UINT8* colorData = meshData->getElementData(VES_COLOR);

		assert((vertexOffset + NUM_VERTICES_AA_LINE) <= meshData->getNumVertices());
		assert((indexOffset + NUM_INDICES_AA_LINE) <= meshData->getNumIndices());

		antialiasedLine(a, b, up, width, borderWidth, color, positionData, colorData, vertexOffset, meshData->getVertexDesc()->getVertexStride(), indexData, indexOffset);
	}

	void ShapeMeshes3D::pixelLineList(const Vector<Vector3>& linePoints, const MeshDataPtr& meshData, UINT32 vertexOffset, UINT32 indexOffset)
	{
		assert(linePoints.size() % 2 == 0);

		assert((vertexOffset + linePoints.size() * 2) <= meshData->getNumVertices());
		assert((indexOffset + linePoints.size() * 2) <= meshData->getNumIndices());

		UINT32 curVertOffset = vertexOffset;
		UINT32 curIdxOffset = indexOffset;

		UINT32* indexData = meshData->getIndices32();
		UINT8* positionData = meshData->getElementData(VES_POSITION);

		UINT32 numPoints = (UINT32)linePoints.size();
		for (UINT32 i = 0; i < numPoints; i += 2)
		{
			pixelLine(linePoints[i], linePoints[i + 1], positionData, curVertOffset, meshData->getVertexDesc()->getVertexStride(), indexData, curIdxOffset);

			curVertOffset += 2;
			curIdxOffset += 2;
		}
	}

	void ShapeMeshes3D::antialiasedLineList(const Vector<Vector3>& linePoints, const Vector3& up, float width, float borderWidth, 
		const Color& color, const MeshDataPtr& meshData, UINT32 vertexOffset, UINT32 indexOffset)
	{
		assert(linePoints.size() % 2 == 0);

		assert((vertexOffset + linePoints.size() * 4) <= meshData->getNumVertices());
		assert((indexOffset + linePoints.size() * 15) <= meshData->getNumIndices());

		UINT32 curVertOffset = vertexOffset;
		UINT32 curIdxOffset = indexOffset;

		UINT32* indexData = meshData->getIndices32();
		UINT8* positionData = meshData->getElementData(VES_POSITION);
		UINT8* colorData = meshData->getElementData(VES_COLOR);

		UINT32 numPoints = (UINT32)linePoints.size();
		for (UINT32 i = 0; i < numPoints; i += 2)
		{
			antialiasedLine(linePoints[i], linePoints[i + 1], up, width, borderWidth, color, positionData, colorData, curVertOffset, meshData->getVertexDesc()->getVertexStride(), indexData, curIdxOffset);

			curVertOffset += NUM_VERTICES_AA_LINE;
			curIdxOffset += NUM_INDICES_AA_LINE;
		}
	}

	/************************************************************************/
	/* 								ELEMENT COUNT                      		*/
	/************************************************************************/

	void ShapeMeshes3D::getNumElementsSphere(UINT32 quality, UINT32& numVertices, UINT32& numIndices)
	{
		numVertices = 20 * (4 * ((UINT32)std::pow(3, quality)));
		numIndices = numVertices;
	}

	void ShapeMeshes3D::getNumElementsWireSphere(UINT32 quality, UINT32& numVertices, UINT32& numIndices)
	{
		getNumElementsWireArc(quality, numVertices, numIndices);
		numVertices *= 3;
		numIndices *= 3;
	}

	void ShapeMeshes3D::getNumElementsArc(UINT32 quality, UINT32& numVertices, UINT32& numIndices)
	{
		numVertices = (quality + 1) * 5 + 1;
		numIndices = ((quality + 1) * 5 - 1) * 3;
	}

	void ShapeMeshes3D::getNumElementsWireArc(UINT32 quality, UINT32& numVertices, UINT32& numIndices)
	{
		numVertices = (quality + 1) * 5;
		numIndices = ((quality + 1) * 5 - 1) * 2;
	}

	void ShapeMeshes3D::getNumElementsDisc(UINT32 quality, UINT32& numVertices, UINT32& numIndices)
	{
		getNumElementsArc(quality, numVertices, numIndices);
	}

	void ShapeMeshes3D::getNumElementsWireDisc(UINT32 quality, UINT32& numVertices, UINT32& numIndices)
	{
		getNumElementsWireArc(quality, numVertices, numIndices);
	}

	void ShapeMeshes3D::getNumElementsCone(UINT32 quality, UINT32& numVertices, UINT32& numIndices)
	{
		numVertices = ((quality + 1) * 4 + 1) * 2;
		numIndices = ((quality + 1) * 4) * 6;
	}

	void ShapeMeshes3D::wireAABox(const AABox& box, UINT8* outVertices, UINT32 vertexOffset, UINT32 vertexStride, UINT32* outIndices, UINT32 indexOffset)
	{
		outVertices += vertexOffset * vertexStride;

		outVertices = writeVector3(outVertices, vertexStride, box.getCorner(AABox::NEAR_LEFT_BOTTOM));
		outVertices = writeVector3(outVertices, vertexStride, box.getCorner(AABox::NEAR_RIGHT_BOTTOM));
		outVertices = writeVector3(outVertices, vertexStride, box.getCorner(AABox::NEAR_RIGHT_TOP));
		outVertices = writeVector3(outVertices, vertexStride, box.getCorner(AABox::NEAR_LEFT_TOP));

		outVertices = writeVector3(outVertices, vertexStride, box.getCorner(AABox::FAR_RIGHT_BOTTOM));
		outVertices = writeVector3(outVertices, vertexStride, box.getCorner(AABox::FAR_LEFT_BOTTOM));
		outVertices = writeVector3(outVertices, vertexStride, box.getCorner(AABox::FAR_LEFT_TOP));
		outVertices = writeVector3(outVertices, vertexStride, box.getCorner(AABox::FAR_RIGHT_TOP));

		outIndices += indexOffset;

		// Front
		outIndices[0] = vertexOffset + 0;
		outIndices[1] = vertexOffset + 1;

		outIndices[2] = vertexOffset + 1;
		outIndices[3] = vertexOffset + 2;

		outIndices[4] = vertexOffset + 2;
		outIndices[5] = vertexOffset + 3;

		outIndices[6] = vertexOffset + 3;
		outIndices[7] = vertexOffset + 0;

		// Center
		outIndices[8] = vertexOffset + 0;
		outIndices[9] = vertexOffset + 5;

		outIndices[10] = vertexOffset + 1;
		outIndices[11] = vertexOffset + 4;

		outIndices[12] = vertexOffset + 2;
		outIndices[13] = vertexOffset + 7;

		outIndices[14] = vertexOffset + 3;
		outIndices[15] = vertexOffset + 6;

		// Back
		outIndices[16] = vertexOffset + 4;
		outIndices[17] = vertexOffset + 5;

		outIndices[18] = vertexOffset + 5;
		outIndices[19] = vertexOffset + 6;

		outIndices[20] = vertexOffset + 6;
		outIndices[21] = vertexOffset + 7;

		outIndices[22] = vertexOffset + 7;
		outIndices[23] = vertexOffset + 4;
	}

	void ShapeMeshes3D::solidAABox(const AABox& box, UINT8* outVertices, UINT8* outNormals, UINT32 vertexOffset, UINT32 vertexStride,
		UINT32* outIndices, UINT32 indexOffset)
	{
		outVertices += (vertexOffset * vertexStride);
		Vector3* corners = (Vector3*)outVertices;

		// Front face
		outVertices = writeVector3(outVertices, vertexStride, box.getCorner(AABox::NEAR_LEFT_BOTTOM));
		outVertices = writeVector3(outVertices, vertexStride, box.getCorner(AABox::NEAR_RIGHT_BOTTOM));
		outVertices = writeVector3(outVertices, vertexStride, box.getCorner(AABox::NEAR_RIGHT_TOP));
		outVertices = writeVector3(outVertices, vertexStride, box.getCorner(AABox::NEAR_LEFT_TOP));

		// Back face
		outVertices = writeVector3(outVertices, vertexStride, box.getCorner(AABox::FAR_RIGHT_BOTTOM));
		outVertices = writeVector3(outVertices, vertexStride, box.getCorner(AABox::FAR_LEFT_BOTTOM));
		outVertices = writeVector3(outVertices, vertexStride, box.getCorner(AABox::FAR_LEFT_TOP));
		outVertices = writeVector3(outVertices, vertexStride, box.getCorner(AABox::FAR_RIGHT_TOP));

		// Left face
		outVertices = writeVector3(outVertices, vertexStride, box.getCorner(AABox::FAR_LEFT_BOTTOM));
		outVertices = writeVector3(outVertices, vertexStride, box.getCorner(AABox::NEAR_LEFT_BOTTOM));
		outVertices = writeVector3(outVertices, vertexStride, box.getCorner(AABox::NEAR_LEFT_TOP));
		outVertices = writeVector3(outVertices, vertexStride, box.getCorner(AABox::FAR_LEFT_TOP));

		// Right face
		outVertices = writeVector3(outVertices, vertexStride, box.getCorner(AABox::NEAR_RIGHT_BOTTOM));
		outVertices = writeVector3(outVertices, vertexStride, box.getCorner(AABox::FAR_RIGHT_BOTTOM));
		outVertices = writeVector3(outVertices, vertexStride, box.getCorner(AABox::FAR_RIGHT_TOP));
		outVertices = writeVector3(outVertices, vertexStride, box.getCorner(AABox::NEAR_RIGHT_TOP));

		// Top face
		outVertices = writeVector3(outVertices, vertexStride, box.getCorner(AABox::FAR_LEFT_TOP));
		outVertices = writeVector3(outVertices, vertexStride, box.getCorner(AABox::NEAR_LEFT_TOP));
		outVertices = writeVector3(outVertices, vertexStride, box.getCorner(AABox::NEAR_RIGHT_TOP));
		outVertices = writeVector3(outVertices, vertexStride, box.getCorner(AABox::FAR_RIGHT_TOP));

		// Bottom face
		outVertices = writeVector3(outVertices, vertexStride, box.getCorner(AABox::FAR_LEFT_BOTTOM));
		outVertices = writeVector3(outVertices, vertexStride, box.getCorner(AABox::FAR_RIGHT_BOTTOM));
		outVertices = writeVector3(outVertices, vertexStride, box.getCorner(AABox::NEAR_RIGHT_BOTTOM));
		outVertices = writeVector3(outVertices, vertexStride, box.getCorner(AABox::NEAR_LEFT_BOTTOM));

		static const Vector3 faceNormals[6] = 
		{
			Vector3(0, 0, 1),
			Vector3(0, 0, -1),
			Vector3(-1, 0, 0),
			Vector3(1, 0, 0),
			Vector3(0, 1, 0),
			Vector3(0, -1, 0)
		};

		outNormals += (vertexOffset * vertexStride);
		for (UINT32 face = 0; face < 6; face++)
		{
			outNormals = writeVector3(outNormals, vertexStride, faceNormals[face]);
			outNormals = writeVector3(outNormals, vertexStride, faceNormals[face]);
			outNormals = writeVector3(outNormals, vertexStride, faceNormals[face]);
			outNormals = writeVector3(outNormals, vertexStride, faceNormals[face]);
		}

		UINT32* indices = outIndices + indexOffset;
		for (UINT32 face = 0; face < 6; face++)
		{
			UINT32 faceVertOffset = vertexOffset + face * 4;

			indices[face * 6 + 0] = faceVertOffset + 2;
			indices[face * 6 + 1] = faceVertOffset + 1;
			indices[face * 6 + 2] = faceVertOffset + 0;
			indices[face * 6 + 3] = faceVertOffset + 0;
			indices[face * 6 + 4] = faceVertOffset + 3;
			indices[face * 6 + 5] = faceVertOffset + 2;
		}
	}

	void ShapeMeshes3D::solidSphere(const Sphere& sphere, UINT8* outVertices, UINT8* outNormals, UINT32 vertexOffset, 
		UINT32 vertexStride, UINT32* outIndices, UINT32 indexOffset, UINT32 quality)
	{
		// Create icosahedron
		static const float x = 0.525731112119133606f;
		static const float z = 0.850650808352039932f;

		static const Vector3 vertices[12] = 
		{
			Vector3(-x,		0.0f,	z),
			Vector3(x,		0.0f,	z),
			Vector3(-x,		0.0f,	-z),
			Vector3(x,		0.0f,	-z),
			Vector3(0.0f,	z,		x),
			Vector3(0.0f,	z,		-x),
			Vector3(0.0f,	-z,		x),
			Vector3(0.0f,	-z,		-x),
			Vector3(z,		x,		0.0f),
			Vector3(-z,		x,		0.0f),
			Vector3(z,		-x,		0.0f),
			Vector3(-z,		-x,		0.0f)
		};

		static const UINT32 triangles[20][3] = 
		{
				{ 0, 4, 1 },  { 0, 9, 4 },	{ 9, 5, 4 },	{ 4, 5, 8 },
				{ 4, 8, 1 },  { 8, 10, 1 }, { 8, 3, 10 },	{ 5, 3, 8 },
				{ 5, 2, 3 },  { 2, 7, 3 },	{ 7, 10, 3 },	{ 7, 6, 10 },
				{ 7, 11, 6 }, { 11, 0, 6 }, { 0, 1, 6 },	{ 6, 1, 10 },
				{ 9, 0, 11 }, { 9, 11, 2 }, { 9, 2, 5 },	{ 7, 2, 11 }
		};

		// Tessellate it
		UINT32 curVertOffset = vertexOffset;
		for (int i = 0; i < 20; ++i) 
		{
			curVertOffset += subdivideTriangleOnSphere(sphere.getCenter(), sphere.getRadius(), quality,
				vertices[triangles[i][2]], vertices[triangles[i][1]], vertices[triangles[i][0]],
				outVertices, outNormals, curVertOffset, vertexStride);
		}

		// Create indices
		outIndices += indexOffset;

		UINT32 numIndices = 20 * (3 * ((UINT32)std::pow(4L, (long)quality)));
		for (UINT32 i = 0; i < numIndices; i += 3)
		{
			outIndices[i] = vertexOffset + i + 2;
			outIndices[i + 1] = vertexOffset + i + 1;
			outIndices[i + 2] = vertexOffset + i + 0;
		}
	}

	void ShapeMeshes3D::wireArc(const Vector3& center, float radius, const Vector3& normal, Degree startAngle, Degree amountAngle, 
		UINT8* outVertices, UINT32 vertexOffset, UINT32 vertexStride, UINT32* outIndices, UINT32 indexOffset, UINT32 quality)
	{
		UINT32 numVertices = (quality + 1) * 5;

		generateArcVertices(center, normal, radius, startAngle, amountAngle,
			numVertices, outVertices, vertexOffset, vertexStride);

		outIndices += indexOffset;
		UINT32 numLines = numVertices - 1;
		for (UINT32 i = 0; i < numLines; i++)
		{
			outIndices[i * 2 + 0] = vertexOffset + i;
			outIndices[i * 2 + 1] = vertexOffset + i + 1;
		}
	}

	void ShapeMeshes3D::solidArc(const Vector3& center, float radius, const Vector3& normal, Degree startAngle, Degree amountAngle, 
		UINT8* outVertices, UINT8* outNormals, UINT32 vertexOffset, UINT32 vertexStride, UINT32* outIndices, UINT32 indexOffset, UINT32 quality)
	{
		Vector3* centerVertex = (Vector3*)(outVertices + (vertexOffset * vertexStride));
		*centerVertex = center;

		UINT32 numArcVertices = (quality + 1) * 5;
		generateArcVertices(center, normal, radius, startAngle, amountAngle,
			numArcVertices, outVertices, vertexOffset + 1, vertexStride);

		UINT32 totalNumVertices = numArcVertices + 1;
		outNormals += vertexOffset * vertexStride;
		for (UINT32 i = 0; i < totalNumVertices; i++)
		{
			outNormals = writeVector3(outNormals, vertexStride, normal);
		}

		outIndices += indexOffset;
		UINT32 numTriangles = numArcVertices - 1;
		for (UINT32 i = 0; i < numTriangles; i++)
		{
			outIndices[i * 3 + 0] = vertexOffset + 0;
			outIndices[i * 3 + 1] = vertexOffset + i;
			outIndices[i * 3 + 2] = vertexOffset + i + 1;
		}
	}

	void ShapeMeshes3D::wireFrustum(const Vector3& position, float aspect, Degree FOV, float near, float far,
		UINT8* outVertices, UINT32 vertexOffset, UINT32 vertexStride, UINT32* outIndices, UINT32 indexOffset)
	{
		float fovTan = Math::tan(FOV * 0.5f);

		Vector3 nearPoint(0, 0, near);
		Vector3 nearWidth(near * fovTan * aspect, 0, 0);
		Vector3 nearHeight(0, (near * fovTan) / aspect, 0);

		Vector3 farPoint(0, 0, far);
		Vector3 farWidth(far * fovTan * aspect, 0, 0);
		Vector3 farHeight(0, (far * fovTan) / aspect, 0);

		Vector3 points[8] = 
		{
			nearPoint + nearWidth + nearHeight,
			nearPoint - nearWidth + nearHeight,
			nearPoint - nearWidth - nearHeight,
			nearPoint + nearWidth - nearHeight,
			farPoint + farWidth + farHeight,
			farPoint - farWidth + farHeight,
			farPoint - farWidth - farHeight,
			farPoint + farWidth - farHeight
		};

		outVertices += vertexOffset * vertexStride;

		for (UINT32 i = 0; i < 8; i++)
			outVertices = writeVector3(outVertices, vertexStride, position + points[i]);

		outIndices += indexOffset;

		// Front
		outIndices[0] = vertexOffset + 0; outIndices[1] = vertexOffset + 1;
		outIndices[2] = vertexOffset + 1; outIndices[3] = vertexOffset + 2;
		outIndices[4] = vertexOffset + 2; outIndices[5] = vertexOffset + 3;
		outIndices[6] = vertexOffset + 3; outIndices[7] = vertexOffset + 0;

		// Center
		outIndices[8] = vertexOffset + 0; outIndices[9] = vertexOffset + 4;
		outIndices[10] = vertexOffset + 1; outIndices[11] = vertexOffset + 5;
		outIndices[12] = vertexOffset + 2; outIndices[13] = vertexOffset + 6;
		outIndices[14] = vertexOffset + 3; outIndices[15] = vertexOffset + 7;

		// Back
		outIndices[16] = vertexOffset + 4; outIndices[17] = vertexOffset + 5;
		outIndices[18] = vertexOffset + 5; outIndices[19] = vertexOffset + 6;
		outIndices[20] = vertexOffset + 6; outIndices[21] = vertexOffset + 7;
		outIndices[22] = vertexOffset + 7; outIndices[23] = vertexOffset + 4;
	}

	void ShapeMeshes3D::solidCone(const Vector3& base, const Vector3& normal, float height, float radius,
		UINT8* outVertices, UINT8* outNormals, UINT32 vertexOffset, UINT32 vertexStride, UINT32* outIndices, UINT32 indexOffset, UINT32 quality)
	{
		outVertices += vertexOffset * vertexStride;
		outNormals += vertexOffset * vertexStride;
		outIndices += indexOffset;

		// Generate base disc
		UINT32 numArcVertices = (quality + 1) * 4;

		generateArcVertices(base, normal, radius, Degree(0), Degree(360),
			numArcVertices + 1, outVertices, 0, vertexStride);

		outVertices += numArcVertices * vertexStride;
		outVertices = writeVector3(outVertices, vertexStride, base); // Write base vertex

		UINT32 baseIdx = numArcVertices;

		UINT32 totalNumBaseVertices = numArcVertices + 1;
		for (UINT32 i = 0; i < totalNumBaseVertices; i++)
		{
			outNormals = writeVector3(outNormals, vertexStride, -normal);
		}

		UINT32 numTriangles = numArcVertices;
		for (UINT32 i = 0; i < numTriangles - 1; i++)
		{
			outIndices[i * 3 + 0] = vertexOffset + baseIdx;
			outIndices[i * 3 + 1] = vertexOffset + i;
			outIndices[i * 3 + 2] = vertexOffset + i + 1;
		}

		{
			UINT32 i = numTriangles - 1;
			outIndices[i * 3 + 0] = vertexOffset + baseIdx;
			outIndices[i * 3 + 1] = vertexOffset + i;
			outIndices[i * 3 + 2] = vertexOffset + 0;
		}

		// Generate cone
		generateArcVertices(base, normal, radius, Degree(0), Degree(360),
			numArcVertices + 1, outVertices, 0, vertexStride);

		Vector3 topVertex = base + normal * height;

		UINT32 totalNumConeVertices = numArcVertices;
		for (UINT32 i = 0; i < totalNumConeVertices; i++)
		{
			int offsetA = i;
			int offsetB = (i + 1) % totalNumConeVertices;

			Vector3* a = (Vector3*)(outVertices + (offsetA * vertexStride));
			Vector3* b = (Vector3*)(outVertices + (offsetB * vertexStride));

			Vector3 triNormal = *a - topVertex;
			triNormal = triNormal.cross(*b - topVertex);
			triNormal.normalize();

			outNormals = writeVector3(outNormals, vertexStride, triNormal); // TODO - Smooth normals?
		}

		*(Vector3*)outNormals = normal;

		outVertices += numArcVertices * vertexStride;
		outVertices = writeVector3(outVertices, vertexStride, topVertex); // Write top vertex
		UINT32 topIdx = numArcVertices;

		outIndices += numTriangles * 3;
		UINT32 curVertOffset = vertexOffset + numArcVertices + 1;
		for (UINT32 i = 0; i < numTriangles - 1; i++)
		{
			outIndices[i * 3 + 0] = curVertOffset + topIdx;
			outIndices[i * 3 + 1] = curVertOffset + i + 1;
			outIndices[i * 3 + 2] = curVertOffset + i;
		}

		{
			UINT32 i = numTriangles - 1;
			outIndices[i * 3 + 0] = curVertOffset + topIdx;
			outIndices[i * 3 + 1] = curVertOffset + 0;
			outIndices[i * 3 + 2] = curVertOffset + i;
		}
	}

	void ShapeMeshes3D::solidQuad(const Rect3& area, UINT8* outVertices, UINT8* outNormals, UINT32 vertexOffset, UINT32 vertexStride, UINT32* outIndices, UINT32 indexOffset)
	{
		outVertices += (vertexOffset * vertexStride);

		Vector3 topLeft = area.getCenter() - area.getAxisHorz() * area.getExtentHorz() + area.getAxisVert() * area.getExtentVertical();
		Vector3 topRight = area.getCenter() + area.getAxisHorz() * area.getExtentHorz() + area.getAxisVert() * area.getExtentVertical();
		Vector3 botRight = area.getCenter() + area.getAxisHorz() * area.getExtentHorz() - area.getAxisVert() * area.getExtentVertical();
		Vector3 botLeft = area.getCenter() - area.getAxisHorz() * area.getExtentHorz() - area.getAxisVert() * area.getExtentVertical();

		outVertices = writeVector3(outVertices, vertexStride, topLeft);
		outVertices = writeVector3(outVertices, vertexStride, topRight);
		outVertices = writeVector3(outVertices, vertexStride, botRight);
		outVertices = writeVector3(outVertices, vertexStride, botLeft);

		outVertices = writeVector3(outVertices, vertexStride, topLeft);
		outVertices = writeVector3(outVertices, vertexStride, topRight);
		outVertices = writeVector3(outVertices, vertexStride, botRight);
		outVertices = writeVector3(outVertices, vertexStride, botLeft);

		Vector3 normal = area.getAxisHorz().cross(area.getAxisVert());
		Vector3 reverseNormal = -normal;

		outNormals += (vertexOffset * vertexStride);
		outNormals = writeVector3(outNormals, vertexStride, normal);
		outNormals = writeVector3(outNormals, vertexStride, normal);
		outNormals = writeVector3(outNormals, vertexStride, normal);
		outNormals = writeVector3(outNormals, vertexStride, normal);

		outNormals = writeVector3(outNormals, vertexStride, reverseNormal);
		outNormals = writeVector3(outNormals, vertexStride, reverseNormal);
		outNormals = writeVector3(outNormals, vertexStride, reverseNormal);
		outNormals = writeVector3(outNormals, vertexStride, reverseNormal);

		outIndices += indexOffset;
		outIndices[0] = vertexOffset;
		outIndices[1] = vertexOffset + 1;
		outIndices[2] = vertexOffset + 2;

		outIndices[3] = vertexOffset;
		outIndices[4] = vertexOffset + 2;
		outIndices[5] = vertexOffset + 3;

		outIndices[6] = vertexOffset;
		outIndices[7] = vertexOffset + 2;
		outIndices[8] = vertexOffset + 1;

		outIndices[9] = vertexOffset;
		outIndices[10] = vertexOffset + 3;
		outIndices[11] = vertexOffset + 2;
	}

	Vector3 ShapeMeshes3D::calcCenter(UINT8* vertices, UINT32 numVertices, UINT32 vertexStride)
	{
		Vector3 center = Vector3::ZERO;
		for(UINT32 i = 0; i < numVertices; i++)
		{
			Vector3* curVert = (Vector3*)vertices;
			center += *curVert;

			vertices += vertexStride;
		}

		center /= (float)numVertices;
		return center;
	}

	void ShapeMeshes3D::pixelLine(const Vector3& a, const Vector3& b, UINT8* outVertices,
		UINT32 vertexOffset, UINT32 vertexStride, UINT32* outIndices, UINT32 indexOffset)
	{
		outVertices += (vertexOffset * vertexStride);

		Vector3* vertices = (Vector3*)outVertices;
		(*vertices) = a;

		vertices = (Vector3*)(outVertices + vertexStride);
		(*vertices) = b;

		outIndices += indexOffset;
		outIndices[0] = vertexOffset + 0;
		outIndices[1] = vertexOffset + 1;
	}

	void ShapeMeshes3D::antialiasedLine(const Vector3& a, const Vector3& b, const Vector3& up, float width, float borderWidth, const Color& color, UINT8* outVertices, UINT8* outColors,
		UINT32 vertexOffset, UINT32 vertexStride, UINT32* outIndices, UINT32 indexOffset)
	{
		Vector3 dir = b - a;
		dir.normalize();

		Vector3 right = dir.cross(up);
		right.normalize();

		Vector<Vector3> points(4);

		float r = width * 0.5f;
		dir = dir * r;
		right = right * r;

		Vector3 v0 = a - dir - right;
		Vector3 v1 = a - dir + right;
		Vector3 v2 = b + dir + right;
		Vector3 v3 = b + dir - right;

		points[0] = v0;
		points[1] = v1;
		points[2] = v2;
		points[3] = v3;

		antialiasedPolygon(points, up, borderWidth, color, outVertices, outColors, vertexOffset, vertexStride, outIndices, indexOffset);
	}

	void ShapeMeshes3D::pixelSolidPolygon(const Vector<Vector3>& points, UINT8* outVertices,
		UINT32 vertexOffset, UINT32 vertexStride, UINT32* outIndices, UINT32 indexOffset)
	{
		outVertices += (vertexOffset * vertexStride);

		for (auto& point : points)
		{
			Vector3* vertices = (Vector3*)outVertices;
			(*vertices) = point;

			outVertices += vertexStride;
		}

		outIndices += indexOffset;
		INT32 numPoints = (INT32)points.size();
		UINT32 idxCnt = 0;
		for (int i = 2; i < numPoints; i++)
		{
			outIndices[idxCnt++] = vertexOffset;
			outIndices[idxCnt++] = vertexOffset + i - 1;
			outIndices[idxCnt++] = vertexOffset + i;
		}
	}

	void ShapeMeshes3D::pixelWirePolygon(const Vector<Vector3>& points, UINT8* outVertices,
		UINT32 vertexOffset, UINT32 vertexStride, UINT32* outIndices, UINT32 indexOffset)
	{
		INT32 numPoints = (INT32)points.size();
		UINT32 curVertOffset = vertexOffset;
		UINT32 curIdxOffset = indexOffset;
		for (INT32 i = 0, j = numPoints - 1; i < numPoints; j = i++)
		{
			pixelLine(points[j], points[i], outVertices, curVertOffset, vertexStride, outIndices, curIdxOffset);
			curVertOffset += 2;
			curIdxOffset += 2;
		}
	}

	void ShapeMeshes3D::antialiasedPolygon(const Vector<Vector3>& points, const Vector3& up, float borderWidth, const Color& color, UINT8* outVertices, UINT8* outColors,
		UINT32 vertexOffset, UINT32 vertexStride, UINT32* outIndices, UINT32 indexOffset)
	{
		UINT32 numCoords = (UINT32)points.size();

		outVertices += vertexOffset * vertexStride;
		outColors += vertexOffset * vertexStride;
		Vector<Vector3> tempNormals(numCoords);

		for (UINT32 i = 0, j = numCoords - 1; i < numCoords; j = i++)
		{
			const Vector3& v0 = points[j];
			const Vector3& v1 = points[i];

			Vector3 dir = v1 - v0;
			Vector3 right = dir.cross(up);
			right.normalize();

			tempNormals[j] = right;

			// Also start populating the vertex array
			Vector3* vertices = (Vector3*)outVertices;
			*vertices = v1;

			UINT32* colors = (UINT32*)outColors;
			*colors = color.getAsRGBA();

			outVertices += vertexStride;
			outColors += vertexStride;
		}

		Color transparentColor = color;
		transparentColor.a = 0.0f;

		for (UINT32 i = 0, j = numCoords - 1; i < numCoords; j = i++)
		{
			const Vector3& n0 = tempNormals[j];
			const Vector3& n1 = tempNormals[i];

			Vector3 avgNrm = (n0 + n1) * 0.5f;
			float magSqrd = avgNrm.squaredLength();

			if (magSqrd > 0.000001f)
			{
				float scale = 1.0f / magSqrd;
				if (scale > 10.0f)
					scale = 10.0f;

				avgNrm = avgNrm * scale;
			}

			Vector3 tempCoord = points[i] + avgNrm * borderWidth;

			// Move it to the vertex array
			Vector3* vertices = (Vector3*)outVertices;
			*vertices = tempCoord;

			UINT32* colors = (UINT32*)outColors;
			*colors = transparentColor.getAsRGBA();

			outVertices += vertexStride;
			outColors += vertexStride;
		}

		// Populate index buffer
		outIndices += indexOffset;

		UINT32 idxCnt = 0;
		for (UINT32 i = 0, j = numCoords - 1; i < numCoords; j = i++)
		{
			outIndices[idxCnt++] = vertexOffset + i;
			outIndices[idxCnt++] = vertexOffset + j;
			outIndices[idxCnt++] = vertexOffset + numCoords + j;

			outIndices[idxCnt++] = vertexOffset + numCoords + j;
			outIndices[idxCnt++] = vertexOffset + numCoords + i;
			outIndices[idxCnt++] = vertexOffset + i;
		}

		for (UINT32 i = 2; i < numCoords; ++i)
		{
			outIndices[idxCnt++] = vertexOffset + 0;
			outIndices[idxCnt++] = vertexOffset + i - 1;
			outIndices[idxCnt++] = vertexOffset + i;
		}
	}

	UINT32 ShapeMeshes3D::subdivideTriangleOnSphere(const Vector3& center, float radius, UINT32 numLevels,
		const Vector3& a, const Vector3& b, const Vector3& c,
		UINT8* outVertices, UINT8* outNormals, UINT32 vertexOffset, UINT32 vertexStride)
	{
		outVertices += (vertexOffset * vertexStride);
		outNormals += (vertexOffset * vertexStride);

		UINT32 numVertices = 0;

		if (numLevels > 0)
		{
			Vector3 sub1 = Vector3::normalize((a + b) * 0.5f);
			Vector3 sub2 = Vector3::normalize((b + c) * 0.5f);
			Vector3 sub3 = Vector3::normalize((c + a) * 0.5f);

			numLevels--;

			numVertices += subdivideTriangleOnSphere(center, radius, numLevels, a, sub1, sub3, outVertices, 
				outNormals, numVertices, vertexStride);
			numVertices += subdivideTriangleOnSphere(center, radius, numLevels, sub1, b, sub2, outVertices, 
				outNormals, numVertices, vertexStride);
			numVertices += subdivideTriangleOnSphere(center, radius, numLevels, sub1, sub2, sub3, outVertices, 
				outNormals, numVertices, vertexStride);
			numVertices += subdivideTriangleOnSphere(center, radius, numLevels, sub3, sub2, c, outVertices, 
				outNormals, numVertices, vertexStride);
		}
		else
		{
			*((Vector3*)outVertices) = center + a * radius;
			*((Vector3*)outNormals) = a;

			outVertices += vertexStride;
			outNormals += vertexStride;

			*((Vector3*)outVertices) = center + b * radius;
			*((Vector3*)outNormals) = b;

			outVertices += vertexStride;
			outNormals += vertexStride;

			*((Vector3*)outVertices) = center + c * radius;
			*((Vector3*)outNormals) = c;

			outVertices += vertexStride;
			outNormals += vertexStride;

			numVertices += 3;
		}

		return numVertices;
	}

	void ShapeMeshes3D::generateArcVertices(const Vector3& center, const Vector3& up, float radius, Degree startAngle, Degree angleAmount, UINT32 numVertices,
		UINT8* outVertices, UINT32 vertexOffset, UINT32 vertexStride)
	{
		assert(numVertices >= 2);

		startAngle += Degree(90.0f); // Offset so arc starts the same as trig functions

		Vector3 normalizedUp = Vector3::normalize(up);
		Vector3 right;

		if (Math::abs(normalizedUp.dot(Vector3::UNIT_Z)) <= 0.95f)
			right = Vector3::UNIT_Z.cross(normalizedUp);
		else
			right = Vector3::UNIT_Y.cross(normalizedUp);

		right.normalize();
		Quaternion moveToStart(-up, startAngle);
		Vector3 start = moveToStart.rotate(right);

		Quaternion increment(-up, angleAmount / (float)(numVertices - 1));

		outVertices += vertexOffset * vertexStride;
		Vector3 curDirection = start * radius;
		for (UINT32 i = 0; i < numVertices; i++)
		{
			outVertices = writeVector3(outVertices, vertexStride, center + curDirection);
			curDirection = increment.rotate(curDirection);
		}
	}
}