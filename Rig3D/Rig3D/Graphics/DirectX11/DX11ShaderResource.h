#pragma once
#include "Rig3D/Graphics/Interface/IShaderResource.h"
#include <d3d11.h>
#include <vector>

#pragma warning(disable: 4251)

#ifdef _WINDLL
#define RIG3D __declspec(dllexport)
#else
#define RIG3D __declspec(dllimport)
#endif

namespace Rig3D
{
	class RIG3D DX11ShaderResource : public IShaderResource
	{
	public:
		DX11ShaderResource();
		~DX11ShaderResource();

		ID3D11ShaderResourceView**	GetShaderResourceViews();
		ID3D11SamplerState**		GetSamplerStates();
		ID3D11Buffer**				GetConstantBuffers();
		ID3D11Buffer**				GetInstanceBuffers();
		UINT*						GetInstanceBufferStrides();
		UINT*						GetInstanceBufferOffsets();
		ID3D11BlendState**			GetBlendStates();

		uint32_t GetShaderResourceViewCount() const;
		uint32_t GetSamplerStateCount() const;
		uint32_t GetConstantBufferCount() const;
		uint32_t GetInstanceBufferCount() const;
		uint32_t GetBlendStateCount() const;

		void SetShaderResourceViews(std::vector<ID3D11ShaderResourceView*>& shaderResourceViews);
		void SetSamplerStates(std::vector<ID3D11SamplerState*>& samplerStates);
		void SetConstantBuffers(std::vector<ID3D11Buffer*>& buffers);
		void SetInstanceBuffers(std::vector<ID3D11Buffer*>& buffers, std::vector<UINT>& strides, std::vector<UINT>& offsets);
		void SetBlendStates(std::vector<ID3D11BlendState*>& blendStates);

		void AddShaderResourceViews(std::vector<ID3D11ShaderResourceView*>& shaderResourceViews);
		void AddSamplerState(ID3D11SamplerState* samplerState);
		void AddBlendState(ID3D11BlendState* blendState);

		void VClearConstantBuffers() override;
		void VClearInstanceBuffers() override;
		void VClearShaderResourceViews() override;
		void VClearSamplerStates() override;
		void VClearBlendStates() override;

	private:
		std::vector<ID3D11ShaderResourceView*>	mShaderResourceViews;
		std::vector<ID3D11SamplerState*>		mSamplerStates;
		std::vector<ID3D11Buffer*>				mConstantBuffers;
		std::vector<ID3D11Buffer*>				mInstanceBuffers;
		std::vector<UINT>						mInstanceBufferStrides;
		std::vector<UINT>						mInstanceBufferOffsets;
		std::vector<ID3D11BlendState*>			mBlendStates;
	};
}