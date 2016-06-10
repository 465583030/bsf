#include "$ENGINE$\PPBase.bslinc"

Parameters =
{
	float2		gInvTexSize;
	Sampler2D 	gInputSamp : alias("gInputTex");
	Texture2D 	gInputTex;
};

Technique =
{
	Language = "HLSL11";
	
	Pass =
	{
		Fragment =
		{
			cbuffer Input
			{
				float2 gInvTexSize;
			}		
		
			SamplerState gInputSamp;
			Texture2D gInputTex;

			float4 main(VStoFS input) : SV_Target0
			{
				float2 UV[4];

				// Blur using a 4x4 kernel. It's assumed current position is right in the middle of a 2x2 kernel (because the output
				// texture should be 1/2 the size of the output texture), and moving by one in each direction will sample areas
				// between a 2x2 kernel as well if bilinear filtering is enabled.
				UV[0] = input.uv0 + gInvTexSize * float2(-1, -1);
				UV[1] = input.uv0 + gInvTexSize * float2( 1, -1);
				UV[2] = input.uv0 + gInvTexSize * float2(-1,  1);
				UV[3] = input.uv0 + gInvTexSize * float2( 1,  1);

				float4 sample[4];

				for(uint i = 0; i < 4; i++)
					sample[i] = gInputTex.Sample(gInputSamp, UV[i]);

				return (sample[0] + sample[1] + sample[2] + sample[3]) * 0.25f;
			}	
		};
	};
};

Technique =
{
	Language = "GLSL";
	
	Pass =
	{
		Fragment =
		{
			// TODO
		};
	};
};