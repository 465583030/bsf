#pragma once

#include "BsPrerequisites.h"
#include "BsColor.h"
#include "BsAABox.h"

namespace BansheeEngine
{
	/**
	 * @brief	Helper class for easily creating common 2D shapes.
	 */
	class BS_EXPORT ShapeMeshes2D
	{
	public:
		/**
		 * @brief	Fills the mesh data with vertices representing a quad (2 triangles).
		 *
		 * @param	area			Area in which to draw the quad.
		 * @param	meshData		Mesh data that will be populated.
		 * @param	vertexOffset	Offset in number of vertices from the start of the buffer to start writing at.
		 * @param	indexOffset 	Offset in number of indices from the start of the buffer to start writing at.
		 * 							
		 * @note	Provided MeshData must have some specific elements at least:
		 * 			  Vector2 VES_POSITION
		 * 			  32bit index buffer
		 * 			  Enough space for 4 vertices and 6 indices
		 *
		 *			Primitives are output in the form of a triangle list.
		 */
		static void solidQuad(const Rect2& area, const MeshDataPtr& meshData, UINT32 vertexOffset, UINT32 indexOffset);

		/**
		 * @brief	Fills the mesh data with vertices representing a per-pixel line.
		 *
		 * @param	a				Start point of the line.
		 * @param	b				End point of the line.
		 * @param	meshData		Mesh data that will be populated.
		 * @param	vertexOffset	Offset in number of vertices from the start of the buffer to start writing at.
		 * @param	indexOffset 	Offset in number of indices from the start of the buffer to start writing at.
		 * 							
		 * @note	Provided MeshData must have some specific elements at least:
		 * 			  Vector2 VES_POSITION
		 * 			  32bit index buffer
		 * 			  Enough space for 2 vertices and 2 indices
		 *
		 *			Primitives are output in the form of a line list.
		 */
		static void pixelLine(const Vector2& a, const Vector2& b, const MeshDataPtr& meshData, UINT32 vertexOffset, UINT32 indexOffset);

		/**
		 * @brief	Fills the mesh data with vertices representing an anti-aliased line of specific width. Antialiasing is done using alpha blending.
		 *
		 * @param	a				Start point of the line.
		 * @param	b				End point of the line.
		 * @param	width			Width of the line.
		 * @param	borderWidth		Width of the anti-aliased border.
		 * @param	color			Color of the line.
		 * @param	meshData		Mesh data that will be populated by this method.
		 * @param	vertexOffset	Offset in number of vertices from the start of the buffer to start writing at.
		 * @param	indexOffset 	Offset in number of indices from the start of the buffer to start writing at.
		 * 							
		 * @note	Provided MeshData must have some specific elements at least:
		 * 			  Vector2 VES_POSITION
		 * 			  UINT32  VES_COLOR
		 * 			  32bit index buffer
		 *			  Enough space for 8 vertices and 30 indices
		 *
		 *			Primitives are output in the form of a triangle list.
		 */
		static void antialiasedLine(const Vector2& a, const Vector2& b, float width, float borderWidth, const Color& color, const MeshDataPtr& meshData, UINT32 vertexOffset, UINT32 indexOffset);

		/**
		 * @brief	Fills the mesh data with vertices representing per-pixel lines.
		 *
		 * @param	linePoints		A list of start and end points for the lines. Must be a multiple of 2.
		 * @param	meshData		Mesh data that will be populated.
		 * @param	vertexOffset	Offset in number of vertices from the start of the buffer to start writing at.
		 * @param	indexOffset 	Offset in number of indices from the start of the buffer to start writing at.
		 * 							
		 * @note	Provided MeshData must have some specific elements at least:
		 * 			  Vector2  VES_POSITION
		 * 			  32bit index buffer
		 * 			  Enough space for (numLines * 2) vertices and (numLines * 2) indices
		 *
		 *			Primitives are output in the form of a line list.
		 */
		static void pixelLineList(const Vector<Vector2>& linePoints, const MeshDataPtr& meshData, UINT32 vertexOffset, UINT32 indexOffset);

		/**
		 * @brief	Fills the mesh data with vertices representing anti-aliased lines of specific width. Antialiasing is done using alpha blending.
		 *
		 * @param	linePoints		A list of start and end points for the lines. Must be a multiple of 2.
		 * @param	width			Width of the line.
		 * @param	borderWidth		Width of the anti-aliased border.
		 * @param	color			Color of the line.
		 * @param	meshData		Mesh data that will be populated by this method.
		 * @param	vertexOffset	Offset in number of vertices from the start of the buffer to start writing at.
		 * @param	indexOffset 	Offset in number of indices from the start of the buffer to start writing at.
		 * 							
		 * @note	Provided MeshData must have some specific elements at least:
		 * 			  Vector2 VES_POSITION
		 * 			  UINT32  VES_COLOR
		 * 			  32bit index buffer
		 *			  Enough space for (numLines * 8) vertices and (numLines * 30) indices
		 *
		 *			Primitives are output in the form of a triangle list.
		 */
		static void antialiasedLineList(const Vector<Vector2>& linePoints, float width, float borderWidth, const Color& color, const MeshDataPtr& meshData, UINT32 vertexOffset, UINT32 indexOffset);

		static const UINT32 NUM_VERTICES_AA_LINE;
		static const UINT32 NUM_INDICES_AA_LINE;
	protected:
		/**
		 * @brief	Fills the provided buffers with vertices representing a per-pixel line.
		 *
		 * @param	a				Start point of the line.
		 * @param	b				End point of the line.
		 * @param	outVertices		Output buffer that will store the vertex position data.
		 * @param	vertexOffset	Offset in number of vertices from the start of the buffer to start writing at.
		 * @param	vertexStride	Size of a single vertex, in bytes. (Same for both position and color buffer)
		 * @param	outIndices		Output buffer that will store the index data. Indices are 32bit.
		 * @param	indexOffset 	Offset in number of indices from the start of the buffer to start writing at.
		 */
		static void pixelLine(const Vector2& a, const Vector2& b, UINT8* outVertices,
			UINT32 vertexOffset, UINT32 vertexStride, UINT32* outIndices, UINT32 indexOffset);

		/**
		 * @brief	Fills the provided buffers with vertices representing an antialiased line with a custom width.
		 *
		 * @param	a				Start point of the line.
		 * @param	b				End point of the line.
		 * @param	width			Width of the line.
		 * @param	borderWidth		Width of the anti-aliased border.
		 * @param	color			Color of the line.
		 * @param	outVertices		Output buffer that will store the vertex position data.
		 * @param	outColors		Output buffer that will store the vertex color data.
		 * @param	vertexOffset	Offset in number of vertices from the start of the buffer to start writing at.
		 * @param	vertexStride	Size of a single vertex, in bytes. (Same for both position and color buffer)
		 * @param	outIndices		Output buffer that will store the index data. Indices are 32bit.
		 * @param	indexOffset 	Offset in number of indices from the start of the buffer to start writing at.
		 */
		static void antialiasedLine(const Vector2& a, const Vector2& b, float width, float borderWidth, const Color& color, UINT8* outVertices, UINT8* outColors,
			UINT32 vertexOffset, UINT32 vertexStride, UINT32* outIndices, UINT32 indexOffset);

		/**
		 * @brief	Fills the provided buffers with vertices representing an antialiased polygon.
		 *
		 * @param	points			Points defining the polygon. First point is assumed to be the start and end point.
		 * @param	borderWidth		Width of the anti-aliased border.
		 * @param	color			Color of the polygon.
		 * @param	outVertices		Output buffer that will store the vertex position data.
		 * @param	outColors		Output buffer that will store the vertex color data.
		 * @param	vertexOffset	Offset in number of vertices from the start of the buffer to start writing at.
		 * @param	vertexStride	Size of a single vertex, in bytes. (Same for both position and color buffer)
		 * @param	outIndices		Output buffer that will store the index data. Indices are 32bit.
		 * @param	indexOffset 	Offset in number of indices from the start of the buffer to start writing at.
		 */
		static void antialiasedPolygon(const Vector<Vector2>& points, float borderWidth, const Color& color, UINT8* outVertices, UINT8* outColors,
			UINT32 vertexOffset, UINT32 vertexStride, UINT32* outIndices, UINT32 indexOffset);

		/**
		 * @brief	Fills the provided buffers with position data and indices representing an inner 
		 *			area of a polygon (basically a normal non-antialiased polygon).
		 *
		 * @param	points			Points defining the polygon. First point is assumed to be the start and end point.
		 * @param	outVertices		Output buffer that will store the vertex position data.
		 * @param	vertexOffset	Offset in number of vertices from the start of the buffer to start writing at.
		 * @param	vertexStride	Size of a single vertex, in bytes. (Same for both position and color buffer)
		 * @param	outIndices		Output buffer that will store the index data. Indices are 32bit.
		 * @param	indexOffset 	Offset in number of indices from the start of the buffer to start writing at.
		 */
		static void pixelSolidPolygon(const Vector<Vector2>& points, UINT8* outVertices,
			UINT32 vertexOffset, UINT32 vertexStride, UINT32* outIndices, UINT32 indexOffset);

		/**
		 * @brief	Fills the provided buffers with vertices representing a pixel-wide polygon border.
		 *
		 * @param	points			Points defining the polygon. First point is assumed to be the start and end point.
		 * @param	outVertices		Output buffer that will store the vertex position data.
		 * @param	outColors		Output buffer that will store the vertex color data.
		 * @param	vertexOffset	Offset in number of vertices from the start of the buffer to start writing at.
		 * @param	vertexStride	Size of a single vertex, in bytes. (Same for both position and color buffer)
		 * @param	outIndices		Output buffer that will store the index data. Indices are 32bit.
		 * @param	indexOffset 	Offset in number of indices from the start of the buffer to start writing at.
		 */
		static void pixelWirePolygon(const Vector<Vector2>& points, UINT8* outVertices, UINT8* outColors,
			UINT32 vertexOffset, UINT32 vertexStride, UINT32* outIndices, UINT32 indexOffset);

	private:
		/**
		 * @brief	Converts an area with normalized ([0, 1] range) coordinates and returns
		 *			area in clip space coordinates.
		 */
		static Rect2 normalizedCoordToClipSpace(const Rect2& area);

		/**
		 * @brief	Converts a point with normalized ([0, 1] range) coordinates and returns
		 *			a point in clip space coordinates.
		 */
		static Vector2 normalizedCoordToClipSpace(const Vector2& pos);
	};
}