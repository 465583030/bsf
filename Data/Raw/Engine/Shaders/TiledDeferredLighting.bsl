#include "$ENGINE$\GBuffer.bslinc"
#include "$ENGINE$\LightingCommon.bslinc"

Parameters =
{
	Sampler2D 	gGBufferASamp : alias("gGBufferATex");
	Sampler2D 	gGBufferBSamp : alias("gGBufferBTex");
	Sampler2D 	gDepthBufferSamp : alias("gDepthBufferTex");
	
	Texture2D 	gGBufferATex : auto("GBufferA");
	Texture2D	gGBufferBTex : auto("GBufferB");
	Texture2D 	gDepthBufferTex : auto("GBufferDepth");
};

Technique 
  : inherits("GBuffer")
  : inherits("LightingCommon") =
{
	Language = "HLSL11";
	
	Pass =
	{
		Compute = 
		{
			// Arbitrary limit, increase if needed
            #define MAX_SPOT_LIGHTS 512
            #define MAX_RADIAL_LIGHTS 512		

			SamplerState 	gGBufferASamp : register(s0);
			SamplerState 	gGBufferBSamp : register(s1);
			SamplerState 	gDepthBufferSamp : register(s2);
	
			Texture2D 		gGBufferATex : register(t0);
			Texture2D		gGBufferBTex : register(t1);
			Texture2D 		gDepthBufferTex : register(t2);
			
			SurfaceData decodeGBuffer(float4 GBufferAData, float4 GBufferBData, float deviceZ)
			{
				SurfaceData output;
				
				output.albedo.xyz = GBufferAData.xyz;
				output.albedo.w = 1.0f;
				output.worldNormal = GBufferBData * float4(2, 2, 2, 1) - float4(1, 1, 1, 0);
				output.worldNormal.xyz = normalize(output.worldNormal.xyz);
				output.depth = convertFromDeviceZ(deviceZ);
				
				return output;
			}			
			
			SurfaceData getGBufferData(float2 uv)
			{
				float4 GBufferAData = gGBufferATex.SampleLevel(gGBufferASamp, uv, 0);
				float4 GBufferBData = gGBufferBTex.SampleLevel(gGBufferBSamp, uv, 0);
				float deviceZ = gDepthBufferTex.SampleLevel(gDepthBufferSamp, uv, 0).r;
				
				return decodeGBuffer(GBufferAData, GBufferBData, deviceZ);
			}
			
			SurfaceData getGBufferData(uint2 pixelPos)
			{
				float4 GBufferAData = gGBufferATex.Load(int3(pixelPos, 0));
				float4 GBufferBData = gGBufferBTex.Load(int3(pixelPos, 0));
				float deviceZ = gDepthBufferTex.Load(int3(pixelPos, 0)).r;
				
				return decodeGBuffer(GBufferAData, GBufferBData, deviceZ);
			}	
			
			StructuredBuffer<LightData> gDirLights : register(t3);
			StructuredBuffer<LightData> gPointLights : register(t4);
			StructuredBuffer<LightData> gSpotLights  : register(t5);				
		
			RWTexture2D<float4>	gOutput : register(u0);
		
			cbuffer Params : register(b0)
			{
				// x - directional, y - point, z - spot
				uint3 gNumLightsPerType;
			}
			
			groupshared uint sTileMinZ;
			groupshared uint sTileMaxZ;

            groupshared uint sNumRadialLights;
            groupshared uint sNumSpotLights;

            groupshared uint sRadialLightIndices[MAX_RADIAL_LIGHTS];
            groupshared uint sSpotLightIndices[MAX_SPOT_LIGHTS];

			[numthreads(TILE_SIZE, TILE_SIZE, 1)]
			void main(
				uint3 groupId : SV_GroupID,
				uint3 groupThreadId : SV_GroupThreadID,
				uint3 dispatchThreadId : SV_DispatchThreadID,
				uint threadIndex : SV_GroupIndex)
			{
				uint2 pixelPos = dispatchThreadId.xy + gViewportRectangle.xy;
				
				float deviceZ = gDepthBufferTex.Load(int3(pixelPos, 0)).r;
				float depth = convertFromDeviceZ(deviceZ);

				// Set initial values
				if(threadIndex == 0)
				{
					sTileMinZ = 0x7F7FFFFF;
					sTileMaxZ = 0;
					sNumRadialLights = 0;
					sNumSpotLights = 0;
				}
				
				GroupMemoryBarrierWithGroupSync();
				
				// Determine minimum and maximum depth values
				InterlockedMin(sTileMinZ, asuint(-depth));
				InterlockedMax(sTileMaxZ, asuint(-depth));

				GroupMemoryBarrierWithGroupSync();
				
			    float minTileZ = asfloat(sTileMinZ);
				float maxTileZ = asfloat(sTileMaxZ);
				
				// Create a frustum for the current tile
				// First determine a scale of the tile compared to the viewport
				float2 tileScale = gViewportRectangle.zw / float2(TILE_SIZE, TILE_SIZE);

				// Now we need to use that scale to scale down the frustum.
				// Assume a projection matrix:
				// A, 0, C, 0
				// 0, B, D, 0
				// 0, 0, Q, QN
				// 0, 0, -1, 0
				//
				// Where A is = 2*n / (r - l)
				// and C = (r + l) / (r - l)
				// 
				// Q & QN are used for Z value which we don't need to scale. B & D are equivalent for the
				// Y value, we'll only consider the X values (A & C) from now on.
				//
				// Both and A and C are inversely proportional to the size of the frustum (r - l). Larger scale mean that
				// tiles are that much smaller than the viewport. This means as our scale increases, (r - l) decreases,
				// which means A & C as a whole increase. Therefore:
				// A' = A * tileScale.x
				// C' = C * tileScale.x
				
				// Aside from scaling, we also need to offset the frustum to the center of the tile.
				// For this we calculate the bias value which we add to the C & D factors (which control
				// the offset in the projection matrix).
				float2 tileBias = tileScale - 1 - groupId.xy * 2;

				// This will yield a bias ranging from [-(tileScale - 1), tileScale - 1], with only odd
				// numbers (except for tile scale of 1). e.g.
				// tileScale = 1
				//  - bias[0] = 0
				
				// tileScale = 2
				//  - bias[0] = 1
				//  - bias[1] = -1
				
				// tileScale = 4
				// - bias[0] = 3
				// - bias[1] = 1
				// - bias[2] = -1
				// - bias[3] = -3

				// etc.
				
				// We use only odd numbers as that ensures we get only the frustums centered on tiles,
				// and not those overlapping two tiles (centered on their boundary).
				
				float At = gMatProj[0][0] * tileScale.x;
				float Ctt = gMatProj[0][2] * tileScale.x - tileBias.x;
				
				float Bt = gMatProj[1][1] * tileScale.y;
				float Dtt = gMatProj[1][2] * tileScale.y + tileBias.y;
				
				// Extract left/right/top/bottom frustum planes from scaled projection matrix
				// Note: Do this on the CPU? Since they're shared among all entries in a tile. Plus they don't change across frames.
				float4 frustumPlanes[6];
				frustumPlanes[0] = float4(At, 0.0f, gMatProj[3][2] + Ctt, 0.0f);
				frustumPlanes[1] = float4(-At, 0.0f, gMatProj[3][2] - Ctt, 0.0f);
				frustumPlanes[2] = float4(0.0f, -Bt, gMatProj[3][2] - Dtt, 0.0f);
				frustumPlanes[3] = float4(0.0f, Bt, gMatProj[3][2] + Dtt, 0.0f);
				
				// Normalize
                [unroll]
                for (uint i = 0; i < 4; ++i) 
					frustumPlanes[i] *= rcp(length(frustumPlanes[i].xyz));
				
				// Generate near/far frustum planes
				// Note: d gets negated in plane equation, this is why its in opposite direction than it intuitively should be
				frustumPlanes[4] = float4(0.0f, 0.0f, -1.0f, -minTileZ); 
				frustumPlanes[5] = float4(0.0f, 0.0f, 1.0f, maxTileZ);
				
				// Generate world position
				float2 screenUv = ((float2)(gViewportRectangle.xy + pixelPos) + 0.5f) / (float2)gViewportRectangle.zw;
				float2 clipSpacePos = (screenUv - gClipToUVScaleOffset.zw) / gClipToUVScaleOffset.xy;
			
				// x, y are now in clip space, z, w are in view space
				// We multiply them by a special inverse view-projection matrix, that had the projection entries that effect
				// z, w eliminated (since they are already in view space)
				// Note: Multiply by depth should be avoided if using ortographic projection
				float4 mixedSpacePos = float4(clipSpacePos.xy * -depth, depth, 1);
				float4 worldPosition4D = mul(gMatScreenToWorld, mixedSpacePos);
				float3 worldPosition = worldPosition4D.xyz / worldPosition4D.w;
				
                // Find lights overlapping the tile
                for (uint i = threadIndex; i < gNumLightsPerType.y && i < MAX_RADIAL_LIGHTS; i += TILE_SIZE)
                {
                    float4 lightPosition = mul(gMatView, float4(gPointLights[i].position, 1.0f));
                    float lightRadius = gPointLights[i].radius;
                    
		            // Note: The cull method can have false positives. In case of large light bounds and small tiles, it
                    // can end up being quite a lot. Consider adding an extra heuristic to check a separating plane.
                    bool lightInTile = true;
				
                    // First check side planes as this will cull majority of the lights
                    [unroll]
                    for (uint j = 0; j < 4; ++j)
                    {
                        float dist = dot(frustumPlanes[j], lightPosition);
                        lightInTile = lightInTile && (dist >= -lightRadius);
                    }

                    // Make sure to do an actual branch, since it's quite likely an entire warp will have the same value
                    [branch]
                    if (lightInTile)
                    {
                        bool inDepthRange = true;
				
			            // Check near/far planes
                        [unroll]
                        for (uint j = 4; j < 6; ++j)
                        {
                            float dist = dot(frustumPlanes[j], lightPosition);
                            inDepthRange = inDepthRange && (dist >= -lightRadius);
                        }
                        
                        // In tile, add to branch
                        [branch]
                        if (inDepthRange)
                        {
                            uint idx;
                            InterlockedAdd(sNumRadialLights, 1U, idx);
                            sRadialLightIndices[idx] = i;
                        }
                    }
                }

                for (uint i = threadIndex; i < gNumLightsPerType.z && i < MAX_SPOT_LIGHTS; i += TILE_SIZE)
                {
                    float4 lightPosition = mul(gMatView, float4(gSpotLights[i].position, 1.0f));
                    float lightRadius = gSpotLights[i].radius;
                    
		            // Note: The cull method can have false positives. In case of large light bounds and small tiles, it
                    // can end up being quite a lot. Consider adding an extra heuristic to check a separating plane.
                    bool lightInTile = true;
				
                    // First check side planes as this will cull majority of the lights
                    [unroll]
                    for (uint j = 0; j < 4; ++j)
                    {
                        float dist = dot(frustumPlanes[j], lightPosition);
                        lightInTile = lightInTile && (dist >= -lightRadius);
                    }

                    // Make sure to do an actual branch, since it's quite likely an entire warp will have the same value
                    [branch]
                    if (lightInTile)
                    {
                        bool inDepthRange = true;
				
			            // Check near/far planes
                        [unroll]
                        for (uint j = 4; j < 6; ++j)
                        {
                            float dist = dot(frustumPlanes[j], lightPosition);
                            inDepthRange = inDepthRange && (dist >= -lightRadius);
                        }
                        
                        // In tile, add to branch
                        [branch]
                        if (inDepthRange)
                        {
                            uint idx;
                            InterlockedAdd(sNumSpotLights, 1U, idx);
                            sSpotLightIndices[idx] = i;
                        }
                    }
                }

                GroupMemoryBarrierWithGroupSync();

				// Note: This unnecessarily samples depth again
				SurfaceData surfaceData = getGBufferData(pixelPos);
				
				float3 lightAccumulator = 0;
				float alpha = 0.0f;
				if(surfaceData.worldNormal.w > 0.0f)
				{
					for(uint i = 0; i < gNumLightsPerType.x; ++i)
						lightAccumulator += getDirLightContibution(surfaceData, gDirLights[i]);
					
                    for (uint i = 0; i < sNumRadialLights; ++i)
                    {
                        uint lightIdx = sRadialLightIndices[i];
                        lightAccumulator += getPointLightContribution(worldPosition, surfaceData, gPointLights[lightIdx]);
                    }

					for(uint i = 0; i < sNumSpotLights; ++i)
                    {
                        uint lightIdx = sSpotLightIndices[i];
                        lightAccumulator += getSpotLightContribution(worldPosition, surfaceData, gSpotLights[lightIdx]);
                    }

					alpha = 1.0f;
				}
				
				float3 diffuse = surfaceData.albedo.xyz / PI; // TODO - Add better lighting model later
				uint2 viewportMax = gViewportRectangle.xy + gViewportRectangle.zw;

				// Ignore pixels out of valid range
				if (all(dispatchThreadId.xy < viewportMax)) 
					gOutput[pixelPos] = float4(gOutput[pixelPos].xyz + diffuse * lightAccumulator, alpha);
			}
		};
	};
};

Technique 
  : inherits("GBuffer")
  : inherits("LightingCommon") =
{
	Language = "GLSL";
	
	Pass =
	{
		Compute = 
		{
			layout (local_size_x = TILE_SIZE, local_size_y = TILE_SIZE) in;
		
			layout(binding = 0) uniform sampler2D gGBufferATex;
			layout(binding = 1) uniform sampler2D gGBufferBTex;
			layout(binding = 2) uniform sampler2D gDepthBufferTex;
			
			SurfaceData decodeGBuffer(vec4 GBufferAData, vec4 GBufferBData, float deviceZ)
			{
				SurfaceData surfaceData;
				
				surfaceData.albedo.xyz = GBufferAData.xyz;
				surfaceData.albedo.w = 1.0f;
				surfaceData.worldNormal = GBufferBData * vec4(2, 2, 2, 1) - vec4(1, 1, 1, 0);
				surfaceData.worldNormal.xyz = normalize(surfaceData.worldNormal.xyz);
				surfaceData.depth = convertFromDeviceZ(deviceZ);
				
				return surfaceData;
			}			
			
			SurfaceData getGBufferData(vec2 uv)
			{
				vec4 GBufferAData = textureLod(gGBufferATex, uv, 0);
				vec4 GBufferBData = textureLod(gGBufferBTex, uv, 0);
				float deviceZ = textureLod(gDepthBufferTex, uv, 0).r;
				
				return decodeGBuffer(GBufferAData, GBufferBData, deviceZ);
			}	
			
			SurfaceData getGBufferData(ivec2 pixelPos)
			{
				vec4 GBufferAData = texelFetch(gGBufferATex, pixelPos, 0);
				vec4 GBufferBData = texelFetch(gGBufferBTex, pixelPos, 0);
				float deviceZ = texelFetch(gDepthBufferTex, pixelPos, 0).r;
				
				return decodeGBuffer(GBufferAData, GBufferBData, deviceZ);
			}	
			
			layout(std430, binding = 3) buffer gDirLights
			{
				LightData[] gDirLightsData;
			};
			
			layout(std430, binding = 4) buffer gPointLights
			{
				LightData[] gPointLightsData;
			};
			
			layout(std430, binding = 5) buffer gSpotLights
			{
				LightData[] gSpotLightsData;
			};	
			
			layout(binding = 6, rgba16f) uniform image2D gOutput;
			
			layout(binding = 7, std140) uniform Params
			{
				// x - directional, y - point, z - spot
				uvec3 gNumLightsPerType;
			};
			
			void main()
			{
				ivec2 pixelPos = ivec2(gl_GlobalInvocationID.xy) + gViewportRectangle.xy;
				SurfaceData surfaceData = getGBufferData(pixelPos);

				float alpha = 0.0f;
				vec3 lightAccumulator = vec3(0, 0, 0);
				if(surfaceData.worldNormal.w > 0.0f)
				{
					vec2 screenUv = (vec2(gViewportRectangle.xy + pixelPos) + 0.5f) / vec2(gViewportRectangle.zw);
					vec2 clipSpacePos = (screenUv - gClipToUVScaleOffset.zw) / gClipToUVScaleOffset.xy;
				
					// x, y are now in clip space, z, w are in view space
					// We multiply them by a special inverse view-projection matrix, that had the projection entries that effect
					// z, w eliminated (since they are already in view space)
					// Note: Multiply by depth should be avoided if using ortographic projection
					vec4 mixedSpacePos = vec4(clipSpacePos.xy * -surfaceData.depth, surfaceData.depth, 1);
					vec4 worldPosition4D = gMatScreenToWorld * mixedSpacePos;
					vec3 worldPosition = worldPosition4D.xyz / worldPosition4D.w;
					
					for(uint i = 0; i < gNumLightsPerType.x; i++)
						lightAccumulator += getDirLightContibution(surfaceData, gDirLightsData[i]);
					
					for(uint i = 0; i < gNumLightsPerType.y; i++)
						lightAccumulator += getPointLightContribution(worldPosition, surfaceData, gPointLightsData[i]);
					
					for(uint i = 0; i < gNumLightsPerType.z; i++)
						lightAccumulator += getSpotLightContribution(worldPosition, surfaceData, gSpotLightsData[i]);
						
					alpha = 1.0f;
				}
				
				vec3 diffuse = surfaceData.albedo.xyz / PI; // TODO - Add better lighting model later
				
				uvec2 viewportMax = gViewportRectangle.xy + gViewportRectangle.zw;

				// Ignore pixels out of valid range
				if (all(lessThan(gl_GlobalInvocationID.xy, viewportMax))) 
				{
					vec4 existingValue = imageLoad(gOutput, pixelPos);
					imageStore(gOutput, pixelPos, vec4(diffuse * lightAccumulator + existingValue.xyz, alpha));
				}
			}
		};
	};
};