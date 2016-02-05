#pragma once
#define _USE_MATH_DEFINES
#include <cmath>
#include <vector>
#include <math.h>

namespace Rig3D
{
	namespace Geometry
	{
		template <class Vertex, class Index>
		void Plane(std::vector<Vertex>& vertices, std::vector<Index>& indices, float width, float depth, uint32_t vertexWidth, uint32_t vertexDepth)
		{
			vertices.reserve(sizeof(Vertex) * vertexWidth * vertexDepth);
			indices.reserve(sizeof(Index) * (vertexWidth - 1) * (vertexDepth - 1) * 6);

			float widthStep = width / static_cast<float>(vertexWidth - 1);
			float depthStep = depth / static_cast<float>(vertexDepth - 1);

			float halfWidth = width * 0.5f;
			float halfDepth = depth * 0.5f;

			for (uint32_t z = 0; z < vertexDepth; z++)
			{
				for (uint32_t x = 0; x < vertexWidth; x++)
				{
					Vertex v0;
					v0.Position = { x * widthStep - halfWidth, 0.0f, z * depthStep - halfDepth };
					v0.Normal	= { 0.0f, 1.0f, 0.0f };
					v0.UV = { static_cast<float>(x) / (vertexWidth - 1), 1.0f - (static_cast<float>(z) / (vertexDepth - 1)) };
					vertices.push_back(v0);
				}
			}

			for (uint32_t z = 0; z < vertexDepth - 1; z++)
			{
				for (uint32_t x = 0; x < vertexWidth - 1; x++)
				{
					indices.push_back((z * vertexWidth) + x);
					indices.push_back(((z + 1) * vertexWidth) + x);
					indices.push_back(((z + 1) * vertexWidth) + x + 1);
					indices.push_back(((z + 1) * vertexWidth) + x + 1);
					indices.push_back((z * vertexWidth) + x + 1);
					indices.push_back((z * vertexWidth) + x);
				}
			}
		}

		template <class Vertex, class Index>
		static void Sphere(std::vector<Vertex>& vertices, std::vector<Index>& indices, uint32_t azimuthSubdivisions, uint32_t polarSubdivisions, float radius)
		{
			uint32_t index = 0;
			float pi = static_cast<float>(M_PI);

			for (uint32_t t = 0; t < polarSubdivisions; t++)
			{
				float theta0 = (static_cast<float>(t) / polarSubdivisions) * pi;
				float theta1 = (static_cast<float>(t + 1) / polarSubdivisions) * pi;

				for (uint32_t p = 0; p < azimuthSubdivisions; p++)
				{
					float phi0 = (static_cast<float>(p) / azimuthSubdivisions) * pi * 2.0f;
					float phi1 = (static_cast<float>(p + 1) / azimuthSubdivisions) * pi * 2.0f;


					float sinTheta0 = sin(theta0);
					float cosTheta0 = cos(theta0);
					float sinPhi0 = sin(phi0);
					float cosPhi0 = cos(phi0);

					float sinTheta1 = sin(theta1);
					float cosTheta1 = cos(theta1);
					float sinPhi1 = sin(phi1);
					float cosPhi1 = cos(phi1);

					vec3f n0 = { sinTheta0 * cosPhi0, cosTheta0, sinTheta0 * sinPhi0 };
					vec3f n1 = { sinTheta0 * cosPhi1, cosTheta0, sinTheta0 * sinPhi1 };
					vec3f n2 = { sinTheta1 * cosPhi1, cosTheta1, sinTheta1 * sinPhi1 };
					vec3f n3 = { sinTheta1 * cosPhi0, cosTheta1, sinTheta1 * sinPhi0 };

					Vertex v0, v1, v2, v3;
					v0.Position = radius * n0;
					v1.Position = radius * n1;
					v2.Position = radius * n2;
					v3.Position = radius * n3;

					v0.Normal = n0;
					v1.Normal = n1;
					v2.Normal = n2;
					v3.Normal = n3;

					v0.UV = { 0.5f + (atan2(n0.z, n0.x) / (2.0f * pi)), 0.5f - (asin(n0.y) / pi) };
					v0.UV = { 0.5f + (atan2(n1.z, n1.x) / (2.0f * pi)), 0.5f - (asin(n1.y) / pi) };
					v0.UV = { 0.5f + (atan2(n2.z, n2.x) / (2.0f * pi)), 0.5f - (asin(n2.y) / pi) };
					v0.UV = { 0.5f + (atan2(n3.z, n3.x) / (2.0f * pi)), 0.5f - (asin(n3.y) / pi) };

					if (t == 0)
					{
						vertices.push_back(v3);
						vertices.push_back(v2);
						vertices.push_back(v0);

						indices.push_back(index++);
						indices.push_back(index++);
						indices.push_back(index++);
					}
					else if (t + 1 == polarSubdivisions)
					{
						vertices.push_back(v1);
						vertices.push_back(v0);
						vertices.push_back(v2);

						indices.push_back(index++);
						indices.push_back(index++);
						indices.push_back(index++);
					}
					else
					{
						vertices.push_back(v3);
						vertices.push_back(v1);
						vertices.push_back(v0);

						vertices.push_back(v3);
						vertices.push_back(v2);
						vertices.push_back(v1);

						indices.push_back(index++);
						indices.push_back(index++);
						indices.push_back(index++);

						indices.push_back(index++);
						indices.push_back(index++);
						indices.push_back(index++);
					}
				}
			}
		}
	
		template <class Vertex, class Index>
		static void Cube(std::vector<Vertex>& vertices, std::vector<Index>& indices, uint32_t subdivisions)
		{
			vertices.reserve(sizeof(Vertex) * subdivisions * subdivisions * 6);
			indices.reserve(sizeof(Index) * (subdivisions - 1) * (subdivisions - 1) * 36);

			float halfExtents = 0.5f;

			float step = 1.0f / static_cast<float>(subdivisions - 1);

			uint32_t faceOffset = 0;

			// Top
			for (uint32_t z = 0; z < subdivisions; z++)
			{
				for (uint32_t x = 0; x < subdivisions; x++)
				{
					Vertex v0;
					v0.Position = { x * step - halfExtents, halfExtents, z * step - halfExtents };
					v0.Normal = { 0.0f, 1.0f, 0.0f };
					v0.UV = { static_cast<float>(x) / (subdivisions - 1), 1.0f - (static_cast<float>(z) / (subdivisions - 1)) };
					vertices.push_back(v0);
				}
			}

			for (uint32_t z = 0; z < subdivisions - 1; z++)
			{
				for (uint32_t x = 0; x < subdivisions - 1; x++)
				{
					indices.push_back((z * subdivisions) + x);
					indices.push_back((z * subdivisions) + x + 1);
					indices.push_back(((z + 1) * subdivisions) + x + 1);
					indices.push_back(((z + 1) * subdivisions) + x + 1);
					indices.push_back(((z + 1) * subdivisions) + x);
					indices.push_back((z * subdivisions) + x);
				}
			}
		
			faceOffset += 4;

			// Bottom
			for (uint32_t z = 0; z < subdivisions; z++)
			{
				for (uint32_t x = 0; x < subdivisions; x++)
				{
					Vertex v0;
					v0.Position = { x * step - halfExtents, -halfExtents, z * step - halfExtents };
					v0.Normal = { 0.0f, -1.0f, 0.0f };
					v0.UV = { static_cast<float>(x) / (subdivisions - 1), 1.0f - (static_cast<float>(z) / (subdivisions - 1)) };
					vertices.push_back(v0);
				}
			}

			for (uint32_t z = 0; z < subdivisions - 1; z++)
			{
				for (uint32_t x = 0; x < subdivisions - 1; x++)
				{
					indices.push_back((z * subdivisions) + x + faceOffset);
					indices.push_back(((z + 1) * subdivisions) + x + 1 + faceOffset);
					indices.push_back(((z + 1) * subdivisions) + x + faceOffset);
					indices.push_back((z * subdivisions) + x + 1 + faceOffset);
					indices.push_back(((z + 1) * subdivisions) + x + 1 + faceOffset);
					indices.push_back((z * subdivisions) + x + faceOffset);
				}
			}
			
			faceOffset += 4;

			// Right
			for (uint32_t y = 0; y < subdivisions; y++)
			{
				for (uint32_t z = 0; z < subdivisions; z++)
				{
					Vertex v0;
					v0.Position = { halfExtents, y * step - halfExtents, z * step - halfExtents };
					v0.Normal = { 1.0f, 0.0f, 0.0f };
					v0.UV = { static_cast<float>(y) / (subdivisions - 1), 1.0f - (static_cast<float>(z) / (subdivisions - 1)) };
					vertices.push_back(v0);
				}
			}

			for (uint32_t y = 0; y < subdivisions - 1; y++)
			{
				for (uint32_t z = 0; z < subdivisions - 1; z++)
				{
					indices.push_back((z * subdivisions) + y + faceOffset);
					indices.push_back(((z + 1) * subdivisions) + y + faceOffset);
					indices.push_back(((z + 1) * subdivisions) + y + 1 + faceOffset);
					indices.push_back(((z + 1) * subdivisions) + y + 1 + faceOffset);
					indices.push_back((z * subdivisions) + y + 1 + faceOffset);
					indices.push_back((z * subdivisions) + y + faceOffset);
				}
			}

			faceOffset += 4;

			// Left
			for (uint32_t y = 0; y < subdivisions; y++)
			{
				for (uint32_t z = 0; z < subdivisions; z++)
				{
					Vertex v0;
					v0.Position = { -halfExtents, y * step - halfExtents, z * step - halfExtents };
					v0.Normal = { -1.0f, 0.0f, 0.0f };
					v0.UV = { static_cast<float>(y) / (subdivisions - 1), 1.0f - (static_cast<float>(z) / (subdivisions - 1)) };
					vertices.push_back(v0);
				}
			}

			for (uint32_t y = 0; y < subdivisions - 1; y++)
			{
				for (uint32_t z = 0; z < subdivisions - 1; z++)
				{
					indices.push_back((z * subdivisions) + y + faceOffset);
					indices.push_back(((z + 1) * subdivisions) + y + 1 + faceOffset);
					indices.push_back((z * subdivisions) + y + 1 + faceOffset);
					indices.push_back(((z + 1) * subdivisions) + y + faceOffset);
					indices.push_back(((z + 1) * subdivisions) + y + 1 + faceOffset);
					indices.push_back((z * subdivisions) + y + faceOffset);
				}
			}

			faceOffset += 4;

			// Front
			for (uint32_t y = 0; y < subdivisions; y++)
			{
				for (uint32_t x = 0; x < subdivisions; x++)
				{
					Vertex v0;
					v0.Position = { x * step - halfExtents, y * step - halfExtents, -halfExtents};
					v0.Normal = { 0.0f, 0.0f, -1.0f };
					v0.UV = { static_cast<float>(x) / (subdivisions - 1), 1.0f - (static_cast<float>(y) / (subdivisions - 1)) };
					vertices.push_back(v0);
				}
			}

			for (uint32_t y = 0; y < subdivisions - 1; y++)
			{
				for (uint32_t x = 0; x < subdivisions - 1; x++)
				{
					indices.push_back((y * subdivisions) + x + faceOffset);
					indices.push_back(((y + 1) * subdivisions) + x + faceOffset);
					indices.push_back(((y + 1) * subdivisions) + x + 1 + faceOffset);
					indices.push_back(((y + 1) * subdivisions) + x + 1 + faceOffset);
					indices.push_back((y * subdivisions) + x + 1 + faceOffset);
					indices.push_back((y * subdivisions) + x + faceOffset);
				}
			}

			faceOffset += 4;

			// Back
			for (uint32_t y = 0; y < subdivisions; y++)
			{
				for (uint32_t x = 0; x < subdivisions; x++)
				{
					Vertex v0;
					v0.Position = { x * step - halfExtents, y * step - halfExtents, halfExtents };
					v0.Normal = { 0.0f, 0.0f, 1.0f };
					v0.UV = { static_cast<float>(x) / (subdivisions - 1), 1.0f - (static_cast<float>(y) / (subdivisions - 1)) };
					vertices.push_back(v0);
				}
			}

			for (uint32_t y = 0; y < subdivisions - 1; y++)
			{
				for (uint32_t x = 0; x < subdivisions - 1; x++)
				{
					indices.push_back((y * subdivisions) + x + faceOffset);
					indices.push_back(((y + 1) * subdivisions) + x + 1 + faceOffset);
					indices.push_back((y * subdivisions) + x + 1 + faceOffset);
					indices.push_back(((y + 1) * subdivisions) + x + faceOffset);
					indices.push_back(((y + 1) * subdivisions) + x + 1 + faceOffset);
					indices.push_back((y * subdivisions) + x + faceOffset);
				}
			}
		}
	
		template <class Vertex, class Index>
		static void Cone(std::vector<Vertex>& vertices, std::vector<Index>& indices, uint32_t subdivisions, float height, float angle)
		{
			vertices.reserve(sizeof(Vertex) * (subdivisions + 1) * 2);
			indices.reserve(sizeof(Index) * (subdivisions * 2) * 2);

			float radius = tan(angle * 0.5f) * height;

			float step = (2.0f * PI) / subdivisions;
			float ny0 = (PI * 0.5f) - angle / (PI * 0.5f);

			Vertex c0;
			c0.Position = { 0.0f, height, 0.0f };
			c0.Normal = { 0.0f, 1.0f, 0.0f };
			c0.UV = { 0.5f, 0.5f };

			vertices.push_back(c0);
			indices.push_back(0);

			Vertex v0;
			v0.Position = { 0.0f, 0.0f, radius  };
			v0.Normal = { 0.0f, 0.0f, 1.0f };
			v0.UV = { 0.0f, 1.0f };
	
			vertices.push_back(v0);

			for (uint32_t i = 1; i < subdivisions; i++)
			{
				indices.push_back(i);

				float a = i * step;
				float s = sin(a);
				float c = cos(a);

				float x0 = radius * s;
				float z0 = radius * cos(i * step);

				v0.Position = { radius * s, 0.0f, radius * c };
				v0.Normal = { s, 0.0f, c };
				v0.UV = { static_cast<float>(i) / subdivisions, 1.0f };

				vertices.push_back(v0);
				indices.push_back(i + 1);
				indices.push_back(0);
			}

			indices.push_back(subdivisions);
			indices.push_back(1);

			Vertex c1;
			c1.Position = { 0.0f, 0.0f, 0.0f };
			c1.Normal = { 0.0f, 1.0f, 0.0f };
			c1.UV = { 0.5f, 0.5f };

			vertices.push_back(c1);
			uint32_t bi = vertices.size() - 1;
			indices.push_back(bi);

			v0.Position = { 0.0f, 0.0f, radius };
			v0.Normal = { 0.0f, 0.0f, 1.0f };
			v0.UV = { 0.0f, 1.0f };

			vertices.push_back(v0);

			for (uint32_t i = 1; i < subdivisions; i++)
			{
				indices.push_back(i + subdivisions);

				float a = i * step;
				float s = sin(a);
				float c = cos(a);

				float x0 = radius * s;
				float z0 = radius * cos(i * step);

				v0.Position = { radius * s, 0.0f, radius * c };
				v0.Normal = { s, 0.0f, c };
				v0.UV = { static_cast<float>(i) / subdivisions, 1.0f };

				vertices.push_back(v0);
				indices.push_back(i + subdivisions + 1);
				indices.push_back(bi);
			}

			indices.push_back(subdivisions * 2);
			indices.push_back(subdivisions);
		}

		template <class Vertex, class Index>
		static void NDSQuad(std::vector<Vertex>& vertices, std::vector<Index>& indices)
		{
			vertices.reserve(sizeof(Vertex) * 4);
			indices.reserve(sizeof(Index) * 6);

			Vertex v0, v1, v2, v3;
			v0.Position = { -1.0f, +1.0f, 0.0f };
			v1.Position = { +1.0f, +1.0f, 0.0f };
			v2.Position = { +1.0f, -1.0f, 0.0f };
			v3.Position = { -1.0f, -1.0f, 0.0f };

			v0.UV = { 0.0f, 0.0f };
			v1.UV = { 1.0f, 0.0f };
			v2.UV = { 1.0f, 1.0f };
			v3.UV = { 0.0f, 1.0f };

			vertices.push_back(v0);
			vertices.push_back(v1);
			vertices.push_back(v2);
			vertices.push_back(v3);

			indices.push_back(0);
			indices.push_back(1);
			indices.push_back(2);
			indices.push_back(2);
			indices.push_back(3);
			indices.push_back(0);
		}
}
}