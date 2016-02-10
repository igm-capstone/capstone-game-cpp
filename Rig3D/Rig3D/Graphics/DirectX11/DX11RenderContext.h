#pragma once
#include "Rig3D/Graphics/Interface/IRenderContext.h"
#include <d3d11.h>
#include <vector>

namespace Rig3D
{
	class DX11RenderContext : public IRenderContext
	{
	public:
		DX11RenderContext();
		~DX11RenderContext();

		ID3D11RenderTargetView**	GetRenderTargetViews();
		ID3D11Texture2D**			GetRenderTextures();
		ID3D11ShaderResourceView**  GetShaderResourceViews();
		ID3D11DepthStencilView**	GetDepthStencilViews();
		ID3D11ShaderResourceView**	GetDepthStencilResourceViews();

		uint32_t GetRenderTargetViewCount() const;
		uint32_t GetRenderTextureCount() const;
		uint32_t GetShaderResourceViewCount() const;
		uint32_t GetDepthStencilViewCount() const;
		uint32_t GetDepthStencilResourceViewCount() const;

		void SetRenderTargetViews(std::vector<ID3D11RenderTargetView*>& renderTargetViews);
		void SetRenderTextures(std::vector<ID3D11Texture2D*>& renderTextures);
		void SetShaderResourceViews(std::vector<ID3D11ShaderResourceView*>& depthStencilResourceViews);
		void SetDepthStencilViews(std::vector<ID3D11DepthStencilView*>& depthStencilViews);
		void SetDepthStencilResourceViews(std::vector<ID3D11ShaderResourceView*>& depthStencilResourceViews);

		void VClearRenderTargetViews() override;
		void VClearRenderTextures() override;
		void VClearShaderResourceViews() override;
		void VClearDepthStencilViews() override;
		void VClearDepthStencilResourceViews() override;

	private:
		std::vector<ID3D11RenderTargetView*>	mRenderTargetViews;
		std::vector<ID3D11Texture2D*>			mRenderTextures;
		std::vector<ID3D11ShaderResourceView*>	mShaderResourceViews;
		std::vector<ID3D11DepthStencilView*>	mDepthStencilViews;
		std::vector<ID3D11ShaderResourceView*>	mDepthStencilResourceViews;
	};
}