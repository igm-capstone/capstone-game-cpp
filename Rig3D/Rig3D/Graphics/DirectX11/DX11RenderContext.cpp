#include "DX11RenderContext.h"

using namespace Rig3D;

DX11RenderContext::DX11RenderContext()
{
	
}


DX11RenderContext::~DX11RenderContext()
{
	for (ID3D11RenderTargetView* rtv : mRenderTargetViews)
	{
		if (rtv)
		{
			rtv->Release();
		}
	}

	mRenderTargetViews.clear();

	for (ID3D11Texture2D* texture2D : mRenderTextures)
	{
		if (texture2D)
		{
			texture2D->Release();
		}
	}

	mRenderTextures.clear();

	for (ID3D11ShaderResourceView* srv : mShaderResourceViews)
	{
		if (srv)
		{
			srv->Release();
		}
	}

	mShaderResourceViews.clear();

	for (ID3D11DepthStencilView* dsv : mDepthStencilViews)
	{
		if (dsv)
		{
			dsv->Release();
		}
	}

	mDepthStencilViews.clear();

	for (ID3D11ShaderResourceView* srv : mDepthStencilResourceViews)
	{
		if (srv)
		{
			srv->Release();
		}
	}

	mDepthStencilResourceViews.clear();
}

ID3D11RenderTargetView** DX11RenderContext::GetRenderTargetViews()
{
	return &mRenderTargetViews[0];
}

ID3D11Texture2D** DX11RenderContext::GetRenderTextures()
{
	return &mRenderTextures[0];
}

ID3D11ShaderResourceView** DX11RenderContext::GetShaderResourceViews()
{
	return &mShaderResourceViews[0];
}

ID3D11DepthStencilView** DX11RenderContext::GetDepthStencilViews()
{
	return &mDepthStencilViews[0];
}
ID3D11ShaderResourceView** DX11RenderContext::GetDepthStencilResourceViews()
{
	return &mDepthStencilResourceViews[0];
}

uint32_t  DX11RenderContext::GetRenderTargetViewCount() const
{
	return mRenderTargetViews.size();
}

uint32_t DX11RenderContext::GetRenderTextureCount() const
{
	return mRenderTextures.size();
}

uint32_t DX11RenderContext::GetShaderResourceViewCount() const
{
	return mShaderResourceViews.size();
}

uint32_t DX11RenderContext::GetDepthStencilViewCount() const
{
	return mDepthStencilViews.size();
}

uint32_t DX11RenderContext::GetDepthStencilResourceViewCount() const
{
	return mDepthStencilResourceViews.size();
}

void DX11RenderContext::SetRenderTargetViews(std::vector<ID3D11RenderTargetView*>& renderTargetViews)
{
	VClearRenderTargetViews();

	mRenderTargetViews = renderTargetViews;
}

void DX11RenderContext::SetRenderTextures(std::vector<ID3D11Texture2D*>& renderTextures)
{
	VClearRenderTextures();

	mRenderTextures = renderTextures;
}

void DX11RenderContext::SetShaderResourceViews(std::vector<ID3D11ShaderResourceView*>& shaderResourceViews)
{
	VClearShaderResourceViews();

	mShaderResourceViews = shaderResourceViews;
}

void DX11RenderContext::SetDepthStencilViews(std::vector<ID3D11DepthStencilView*>& depthStencilViews)
{
	VClearDepthStencilViews();

	mDepthStencilViews = depthStencilViews;
}

void DX11RenderContext::SetDepthStencilResourceViews(std::vector<ID3D11ShaderResourceView*>& depthStencilResourceViews)
{
	VClearDepthStencilResourceViews();

	mDepthStencilResourceViews = depthStencilResourceViews;
}

void DX11RenderContext::VClearRenderTargetViews()
{
	for (ID3D11RenderTargetView* rtv : mRenderTargetViews)
	{
		if (rtv)
		{
			rtv->Release();
		}
	}

	mRenderTargetViews.clear();
}

void DX11RenderContext::VClearRenderTextures()
{
	for (ID3D11Texture2D* texture2D : mRenderTextures)
	{
		if (texture2D)
		{
			texture2D->Release();
		}
	}

	mRenderTextures.clear();
}

void DX11RenderContext::VClearShaderResourceViews()
{
	for (ID3D11ShaderResourceView* srv : mShaderResourceViews)
	{
		if (srv)
		{
			srv->Release();
		}
	}

	mShaderResourceViews.clear();
}

void DX11RenderContext::VClearDepthStencilViews()
{
	for (ID3D11DepthStencilView* dsv : mDepthStencilViews)
	{
		if (dsv)
		{
			dsv->Release();
		}
	}

	mDepthStencilViews.clear();
}

void DX11RenderContext::VClearDepthStencilResourceViews()
{
	for (ID3D11ShaderResourceView* srv : mDepthStencilResourceViews)
	{
		if (srv)
		{
			srv->Release();
		}
	}

	mDepthStencilResourceViews.clear();
}