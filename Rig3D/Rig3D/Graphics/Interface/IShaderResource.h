#pragma once

#ifdef _WINDLL
#define RIG3D __declspec(dllexport)
#else
#define RIG3D __declspec(dllimport)
#endif

namespace Rig3D
{
	class RIG3D IShaderResource
	{
	public:
		IShaderResource()
		{
			
		}

		virtual ~IShaderResource()
		{
			
		}

		virtual void VClearConstantBuffers() = 0;
		virtual void VClearInstanceBuffers() = 0;
		virtual void VClearShaderResourceViews() = 0;
		virtual void VClearSamplerStates() = 0;
	};
}