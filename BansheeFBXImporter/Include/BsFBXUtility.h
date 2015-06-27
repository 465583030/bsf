#pragma once

#include "BsFBXPrerequisites.h"
#include "BsFBXImportData.h"

namespace BansheeEngine
{
	/**
	 * @brief	Various helper methods to use during FBX import.
	 */
	class FBXUtility
	{
	public:
		/**
		 * @brief	Calculates per-index normals based on the provided smoothing groups.
		 */
		static void normalsFromSmoothing(const Vector<Vector3>& positions, const Vector<int>& indices,
			const Vector<int>& smoothing, Vector<Vector3>& normals);

		/**
		 * @brief	Find vertices in the source mesh that have different attributes but have the same
		 *			indexes, and splits them into two or more vertexes. Mesh with split vertices
		 *			is output to "dest".
		 *
		 * @param	source	Source mesh to perform the split on. It's expected the position
		 *					values are per-vertex, and all other attributes are per-index.
		 * @param	dest	Output mesh with split vertices. Both vertex positions and
		 *					attributes are per-vertex.
		 */
		static void splitVertices(const FBXImportMesh& source, FBXImportMesh& dest);

		/**
		 * @brief	Flips the triangle window order for all the triangles in the mesh.
		 */
		static void flipWindingOrder(FBXImportMesh& input);

	private:
		/**
		 * @brief	Checks if vertex attributes at the specified indexes are similar enough, or
		 *			does the vertex require a split.
		 */
		static bool needsSplitAttributes(const FBXImportMesh& meshA, int idxA, const FBXImportMesh& meshB, int idxB);

		/**
		 * @brief	Copies vertex attributes from the source mesh at the specified index, 
		 *			to the destination mesh at the specified index.
		 */
		static void copyVertexAttributes(const FBXImportMesh& srcMesh, int srcIdx, FBXImportMesh& destMesh, int dstIdx);

		/**
		 * @brief	Adds a new vertex to the destination mesh and copies the vertex from the source mesh at the
		 *			vertex index, and vertex attributes from the source mesh at the source index.
		 */
		static void addVertex(const FBXImportMesh& srcMesh, int srcIdx, int srcVertex, FBXImportMesh& destMesh);
	};
}