#pragma once

namespace Rig3D
{
	class IRenderContext
	{
	public:
		IRenderContext()
		{
			
		}

		virtual ~IRenderContext()
		{
			
		}

		virtual void VClearRenderTargetViews() = 0;
		virtual void VClearRenderTextures() = 0;
		virtual void VClearShaderResourceViews() = 0;
		virtual void VClearDepthStencilViews() = 0;
		virtual void VClearDepthStencilResourceViews() = 0;
	};
}