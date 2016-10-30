Parameters =
{
	mat4x4		matViewProj;
	Sampler2D 	mainTexSamp : alias("mainTexture");
	Texture2D 	mainTexture;
};

Technique =
{
	Language = "HLSL11";
	
	Pass =
	{
		Target = 
		{
			Blend = true;
			Color = { SRCA, SRCIA, ADD };
		};
		
		DepthWrite = false;
		CompareFunc = LTE;
		
		Vertex =
		{
			float4x4 matViewProj;

			void main(
				in float3 inPos : POSITION,
				in float2 uv : TEXCOORD0,
				in float4 color : COLOR0,
				out float4 oPosition : SV_Position,
				out float2 oUv : TEXCOORD0,
				out float4 oColor : COLOR0)
			{
				oPosition = mul(matViewProj, float4(inPos.xyz, 1));
				oUv = uv;
				oColor = color;
			}		
		};
		
		Fragment =
		{
			SamplerState mainTexSamp : register(s0);
			Texture2D mainTexture : register(t0);

			float4 main(
				in float4 inPos : SV_Position, 
				float2 uv : TEXCOORD0, 
				float4 color : COLOR0) : SV_Target
			{
				return color * mainTexture.Sample(mainTexSamp, uv);
			}		
		};
	};
	
	Pass =
	{
		Target = 
		{
			Blend = true;
			Color = { SRCA, SRCIA, ADD };
		};
		
		DepthWrite = false;
		CompareFunc = GT;
		
		Vertex =
		{
			float4x4 matViewProj;

			void main(
				in float3 inPos : POSITION,
				in float2 uv : TEXCOORD0,
				in float4 color : COLOR1,
				out float4 oPosition : SV_Position,
				out float2 oUv : TEXCOORD0,
				out float4 oColor : COLOR0)
			{
				oPosition = mul(matViewProj, float4(inPos.xyz, 1));
				oUv = uv;
				oColor = color;
			}		
		};
		
		Fragment =
		{
			SamplerState mainTexSamp : register(s0);
			Texture2D mainTexture : register(t0);

			float4 main(
				in float4 inPos : SV_Position, 
				float2 uv : TEXCOORD0, 
				float4 color : COLOR0) : SV_Target
			{
				return color * mainTexture.Sample(mainTexSamp, uv);
			}		
		};
	};	
};	

Technique =
{
	Language = "HLSL9";
	
	Pass =
	{
		Target = 
		{
			Blend = true;
			Color = { SRCA, SRCIA, ADD };
		};
		
		DepthWrite = false;
		CompareFunc = LTE;
		
		Vertex =
		{
			float4x4 matViewProj;

			void main(
				in float3 inPos : POSITION,
				in float4 color : COLOR0,
				in float2 uv : TEXCOORD0,
				out float4 oPosition : POSITION,
				out float4 oColor : COLOR0,
				out float2 oUv : TEXCOORD0)
			{
				oPosition = mul(matViewProj, float4(inPos.xyz, 1));
				oColor = color;
				oUv = uv;
			}		
		};
		
		Fragment =
		{
			sampler2D mainTexture;

			float4 main(float2 uv : TEXCOORD0, float4 color : COLOR0) : COLOR0
			{
				return color * tex2D(mainTexture, uv);
			}		
		};
	};
	
	Pass =
	{
		Target = 
		{
			Blend = true;
			Color = { SRCA, SRCIA, ADD };
		};
		
		DepthWrite = false;
		CompareFunc = GT;
		
		Vertex =
		{
			float4x4 matViewProj;

			void main(
				in float3 inPos : POSITION,
				in float4 color : COLOR1,
				in float2 uv : TEXCOORD0,
				out float4 oPosition : POSITION,
				out float4 oColor : COLOR0,
				out float2 oUv : TEXCOORD0)
			{
				oPosition = mul(matViewProj, float4(inPos.xyz, 1));
				oColor = color;
				oUv = uv;
			}
		};
		
		Fragment =
		{
			sampler2D mainTexture;

			float4 main(float2 uv : TEXCOORD0, float4 color : COLOR0) : COLOR0
			{
				return color * tex2D(mainTexture, uv);
			}		
		};
	};	
};	

Technique =
{
	Language = "GLSL";
	
	Pass =
	{
		Target = 
		{
			Blend = true;
			Color = { SRCA, SRCIA, ADD };
		};
		
		DepthWrite = false;
		CompareFunc = LTE;
		
		Vertex =
		{
			uniform mat4 matViewProj;

			in vec3 bs_position;
			in vec4 bs_color0;
			in vec2 bs_texcoord0;
			out vec4 color0;
			out vec2 texcoord0;

			out gl_PerVertex
			{
				vec4 gl_Position;
			};
			
			void main()
			{
				gl_Position = matViewProj * vec4(bs_position.xyz, 1);
				texcoord0 = bs_texcoord0;
				color0 = bs_color0;
			}		
		};
		
		Fragment =
		{
			uniform sampler2D mainTexture;

			in vec4 color0;
			in vec2 texcoord0;
			out vec4 fragColor;

			void main()
			{
				vec4 texColor = texture2D(mainTexture, texcoord0.st);
				fragColor = color0 * texColor;
			}		
		};
	};
	
	Pass =
	{
		Target = 
		{
			Blend = true;
			Color = { SRCA, SRCIA, ADD };
		};
		
		DepthWrite = false;
		CompareFunc = GT;
		
		Vertex =
		{
			uniform mat4 matViewProj;

			in vec3 bs_position;
			in vec4 bs_color1;
			in vec2 bs_texcoord0;
			out vec4 color0;
			out vec2 texcoord0;

			out gl_PerVertex
			{
				vec4 gl_Position;
			};
			
			void main()
			{
				gl_Position = matViewProj * vec4(bs_position.xyz, 1);
				texcoord0 = bs_texcoord0;
				color0 = bs_color1;
			}		
		};
		
		Fragment =
		{
			uniform sampler2D mainTexture;

			in vec4 color0;
			in vec2 texcoord0;
			out vec4 fragColor;

			void main()
			{
				vec4 texColor = texture2D(mainTexture, texcoord0.st);
				fragColor = color0 * texColor;
			}		
		};
	};	
};	