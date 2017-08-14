#include "$ENGINE$\PPBase.bslinc"
#include "$ENGINE$\SHCommon.bslinc"
#include "$ENGINE$\GBufferInput.bslinc"
#include "$ENGINE$\PerCameraData.bslinc"

technique IrradianceEvaluate
{
	mixin PPBase;
	mixin SHCommon;
	mixin GBufferInput;
	mixin PerCameraData;

	blend
	{
		target	
		{
			enabled = true;
			color = { one, one, add };
		};
	};	
	
	code
	{
		#if MSAA_COUNT > 1
			Texture2DMS<uint> gInputTex;
		#else
			Texture2D<uint> gInputTex;
		#endif
		
		struct Tetrahedron
		{
			uint4 indices;
			float3x4 transform;
		};
		
		struct TetrahedronFace
		{
			float3 corners[3];
			float3 normals[3];
			uint isQuadratic;
		};		
		
		StructuredBuffer<SHVector3RGB> gSHCoeffs;
		StructuredBuffer<Tetrahedron> gTetrahedra;
		StructuredBuffer<TetrahedronFace> gTetFaces;
		
		TextureCube gSkyIrradianceTex;
		SamplerState gSkyIrradianceSamp;

		cbuffer Params
		{
			float gSkyBrightness;
			uint gNumTetrahedra;
		}				
		
		float3 getSkyIndirectDiffuse(float3 dir)
		{
			return gSkyIrradianceTex.SampleLevel(gSkyIrradianceSamp, dir, 0).rgb * gSkyBrightness;
		}
		
		float evaluateLambert(SHVector3 coeffs)
		{
			// Multiply irradiance SH coefficients by cosine lobe (Lambert diffuse) and evaluate resulting SH
			// See: http://cseweb.ucsd.edu/~ravir/papers/invlamb/josa.pdf for derivation of the
			// cosine lobe factors
			float output = 0.0f;
			
			// Band 0 (factor 1.0)
			output += coeffs.v0[0];
			
			// Band 1 (factor 2/3)
			float f = (2.0f/3.0f);
			float4 f4 = float4(f, f, f, f);
			
			output += dot(coeffs.v0.gba, f4.rgb);
			
			// Band 2 (factor 1/4)
			f = (1.0f/4.0f);
			f4 = float4(f, f, f, f);
			
			output += dot(coeffs.v1, f4);
			output += coeffs.v2 * f;
						
			return output;
		}	

		float solveQuadratic(float A, float B, float C)
		{
			const float EPSILON = 0.00001f;
		
			if (abs(A) > EPSILON)
			{
				float p = B / (2 * A);
				float q = C / A;
				float D = p * p - q;

				return sqrt(D) - p;
			}
			else
			{
				if(abs(B) > EPSILON)
					return -C / B;
			
				return 0.0f;
			}
		}
		
		float solveCubic(float A, float B, float C)
		{
			const float EPSILON = 0.00001f;
			const float THIRD = 1.0f / 3.0f;
		
			float sqA = A * A;
			float p = THIRD * (-THIRD * sqA + B);
			float q = (0.5f) * ((2.0f / 27.0f) * A * sqA - THIRD * A * B + C);

			float cbp = p * p * p;
			float D = q * q + cbp;
			
			float t;
			if(D < 0.0f)
			{
				float phi = THIRD * acos(-q / sqrt(-cbp));
				t = (2 * sqrt(-p)) * cos(phi);
			}
			else
			{
				float sqrtD = sqrt(D);
				float u = pow(sqrtD + abs(q), 1.0f / 3.0f);

				
				if (q > 0.0f)
					t = -u + p / u;
				else
					t = u - p / u;
			}
			
			return t - THIRD * A;
		}
		
		float3 calcTriBarycentric(float3 p, float3 a, float3 b, float3 c)
		{
			float3 v0 = b - a;
			float3 v1 = c - a;
			float3 v2 = p - a;
			
			float d00 = dot(v0, v0);
			float d01 = dot(v0, v1);
			float d11 = dot(v1, v1);
			float d20 = dot(v2, v0);
			float d21 = dot(v2, v1);
			
			float denom = d00 * d11 - d01 * d01;
			float3 coords;
			coords.x = (d11 * d20 - d01 * d21) / denom;
			coords.y = (d00 * d21 - d01 * d20) / denom;
			coords.z = 1.0f - coords.x - coords.y;
			
			return coords;
		}
		
		float3 fsmain(VStoFS input
			#if MSAA_COUNT > 1
			, uint sampleIdx : SV_SampleIndex
			#endif
			) : SV_Target0
		{
			uint2 pixelPos = trunc(input.uv0);
		
			SurfaceData surfaceData;
			#if MSAA_COUNT > 1
				surfaceData = getGBufferData(pixelPos, sampleIdx);
			#else
				surfaceData = getGBufferData(pixelPos);
			#endif		
			
			float3 irradiance = 0;
			#if SKY_ONLY
				irradiance = gSkyIrradianceTex.SampleLevel(gSkyIrradianceSamp, surfaceData.worldNormal, 0).rgb * gSkyBrightness;
			#else
				uint volumeIdx;
				#if MSAA_COUNT > 1
					volumeIdx = gInputTex.Load(uint3(pixelPos, 0), sampleIdx).x;
				#else
					volumeIdx = gInputTex.Load(uint3(pixelPos, 0)).x;
				#endif
				
				if(volumeIdx == 0xFFFF) // Using 16-bit texture, so need to compare like this
					irradiance = gSkyIrradianceTex.SampleLevel(gSkyIrradianceSamp, surfaceData.worldNormal, 0).rgb * gSkyBrightness;
				else
				{
					Tetrahedron volume = gTetrahedra[volumeIdx];
					
					float3 P = NDCToWorld(input.screenPos, surfaceData.depth);
					
					float4 coords;
					[branch]
					if(volumeIdx >= gNumTetrahedra)
					{
						uint faceIdx = volumeIdx - gNumTetrahedra;
						TetrahedronFace face = gTetFaces[faceIdx];
					
						float3 factors = mul(volume.transform, float4(P, 1.0f));
						float A = factors.x;
						float B = factors.y;
						float C = factors.z;

						float t;
						if(face.isQuadratic > 0)
							t = solveQuadratic(A, B, C);
						else
							t = solveCubic(A, B, C);
							
						float3 triA = face.corners[0] + t * face.normals[0];
						float3 triB = face.corners[1] + t * face.normals[1];
						float3 triC = face.corners[2] + t * face.normals[2];
						
						float3 bary = calcTriBarycentric(P, triA, triB, triC);
						
						coords.x = bary.z;
						coords.yz = bary.xy;
						coords.w = 0.0f;
					}
					else
					{
						float3 factors = mul(volume.transform, float4(P, 1.0f));			
						coords = float4(factors, 1.0f - factors.x - factors.y - factors.z);
					}
					
					for(uint i = 0; i < 4; ++i)
					{
						if(coords[i] == 0.0f)
							continue;
					
						if(volume.indices[i] == 0)
							irradiance += float3(1.0f, 0, 0) * coords[i];
					
						if(volume.indices[i] == 1)
							irradiance += float3(0.0f, 1.0f, 0) * coords[i];
							
						if(volume.indices[i] == 2)
							irradiance += float3(1.0f, 1.0f, 1.0f) * coords[i];
							
						if(volume.indices[i] == 3)
							irradiance += float3(1.0f, 1.0f, 1.0f) * coords[i];
					
						if(volume.indices[i] == 4)
							irradiance += float3(0.0f, 1.0f, 1.0f) * coords[i];
							
						if(volume.indices[i] == 5)
							irradiance += float3(1.0f, 1.0f, 0.0f) * coords[i];
							
						if(volume.indices[i] == 6)
							irradiance += float3(1.0f, 1.0f, 1.0f) * coords[i];
							
						if(volume.indices[i] == 7)
							irradiance += float3(1.0f, 1.0f, 1.0f) * coords[i];
					}					
					
					//SHVector3RGB shCoeffs;
					//SHZero(shCoeffs);
					
					//for(uint i = 0; i < 4; ++i)
					//{
					//	if(coords[i] > 0.0f)
					//		SHMultiplyAdd(shCoeffs, gSHCoeffs[volume.indices[i]], coords[i]);
					//}
					
					//SHVector3 shBasis = SHBasis3(surfaceData.worldNormal);
					//SHMultiply(shCoeffs.R, shBasis);
					//SHMultiply(shCoeffs.G, shBasis);
					//SHMultiply(shCoeffs.B, shBasis);
					
					//irradiance.r = evaluate(shCoeffs.R);
					//irradiance.g = evaluate(shCoeffs.G);
					//irradiance.b = evaluate(shCoeffs.B);
					
					//irradiance *= float3(10.0f, 0.0f, 0.0f);
				}
			#endif // SKY_ONLY
			
			return irradiance * surfaceData.albedo.rgb;
		}	
	};
};