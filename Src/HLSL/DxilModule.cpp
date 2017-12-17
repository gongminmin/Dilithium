/**
 * @file DxilModule.cpp
 * @author Minmin Gong
 *
 * @section DESCRIPTION
 *
 * This source file is part of Dilithium
 * For the latest info, see https://github.com/gongminmin/Dilithium
 *
 * @section LICENSE
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2017 Minmin Gong. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files(the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions :
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <Dilithium/Dilithium.hpp>
#include <Dilithium/LLVMModule.hpp>
#include <Dilithium/dxc/HLSL/DxilModule.hpp>
#include <Dilithium/dxc/HLSL/DxilConstants.hpp>
#include <Dilithium/dxc/HLSL/DxilContainer.hpp>
#include <Dilithium/dxc/HLSL/DxilCBuffer.hpp>
#include <Dilithium/dxc/HLSL/DxilResource.hpp>
#include <Dilithium/dxc/HLSL/DxilSampler.hpp>
#include <Dilithium/dxc/HLSL/DxilShaderModel.hpp>
#include <Dilithium/dxc/HLSL/DxilTypeSystem.hpp>

namespace Dilithium
{
	DxilModule::ShaderFlags::ShaderFlags()
		: disable_optimizations_(false),
			disable_math_refactoring_(false),
			enable_double_precision_(false),
			force_early_depth_stencil_(false),
			enable_raw_and_structured_buffers_(false),
			enable_min_precision_(false),
			enable_double_extensions_(false),
			enable_msad_(false),
			all_resources_bound_(false),
			viewport_and_rt_array_index_(false),
			inner_coverage_(false),
			stencil_ref_(false),
			tiled_resources_(false),
			uav_load_additional_formats_(false),
			level_9_comparison_filtering_(false),
			cs_raw_and_structured_via_shader_4x_(false),
			all_64_uavs_(false),
			uavs_at_every_stage_(false),
			rovs_(false),
			wave_ops_(false),
			int64_ops_(false),
			align0_(0),
			align1_(0)
	{
	}

	uint32_t DxilModule::ShaderFlags::GetGlobalFlags() const
	{
		uint32_t flags = 0;
		flags |= disable_optimizations_ ? SF_DisableOptimizations : 0;
		flags |= disable_math_refactoring_ ? SF_DisableMathRefactoring : 0;
		flags |= enable_double_precision_ ? SF_EnableDoublePrecision : 0;
		flags |= force_early_depth_stencil_ ? SF_ForceEarlyDepthStencil : 0;
		flags |= enable_raw_and_structured_buffers_ ? SF_EnableRawAndStructuredBuffers : 0;
		flags |= enable_min_precision_ ? SF_EnableMinPrecision : 0;
		flags |= enable_double_extensions_ ? SF_EnableDoubleExtensions : 0;
		flags |= enable_msad_ ? SF_EnableMSAD : 0;
		flags |= all_resources_bound_ ? SF_AllResourcesBound : 0;
		return flags;
	}

	uint64_t DxilModule::ShaderFlags::GetFeatureInfo() const
	{
		uint64_t flags = 0;
		flags |= enable_double_precision_ ? DSFI_Doubles : 0;
		flags |= enable_min_precision_ ? DSFI_MininumPrecision : 0;
		flags |= enable_double_extensions_ ? DSFI_11_1_DoubleExtensions : 0;
		flags |= wave_ops_ ? DSFI_WaveOps : 0;
		flags |= int64_ops_ ? DSFI_Int64Ops : 0;
		flags |= rovs_ ? DSFI_ROVs : 0;
		flags |= viewport_and_rt_array_index_ ? DSFI_ViewportAndRTArrayIndexFromAnyShaderFeedingRasterizer : 0;
		flags |= inner_coverage_ ? DSFI_InnerCoverage : 0;
		flags |= stencil_ref_ ? DSFI_StencilRef : 0;
		flags |= tiled_resources_ ? DSFI_TiledResources : 0;
		flags |= enable_msad_ ? DSFI_11_1_ShaderExtensions : 0;
		flags |= cs_raw_and_structured_via_shader_4x_ ? DSFI_ComputeShadersPlusRawAndStructuredBuffersViaShader4X : 0;
		flags |= uavs_at_every_stage_ ? DSFI_UAVsAtEveryStage : 0;
		flags |= all_64_uavs_ ? DSFI_64UAVs : 0;
		flags |= level_9_comparison_filtering_ ? DSFI_Level9ComparisonFiltering : 0;
		flags |= uav_load_additional_formats_ ? DSFI_TypedUAVLoadAdditionalFormats : 0;
		return flags;
	}

	uint64_t DxilModule::ShaderFlags::GetShaderFlagsRawForCollection()
	{
		// This should be all the flags that can be set by DxilModule::CollectShaderFlags.
		ShaderFlags flags;
		flags.SetEnableDoublePrecision(true);
		flags.SetInt64Ops(true);
		flags.SetEnableMinPrecision(true);
		flags.SetEnableDoubleExtensions(true);
		flags.SetWaveOps(true);
		flags.SetTiledResources(true);
		flags.SetEnableMSAD(true);
		flags.SetUAVLoadAdditionalFormats(true);
		flags.SetStencilRef(true);
		flags.SetInnerCoverage(true);
		flags.SetViewportAndRTArrayIndex(true);
		flags.Set64UAVs(true);
		flags.SetUAVsAtEveryStage(true);
		flags.SetEnableRawAndStructuredBuffers(true);
		flags.SetCSRawAndStructuredViaShader4X(true);
		return flags.GetShaderFlagsRaw();
	}

	uint64_t DxilModule::ShaderFlags::GetShaderFlagsRaw() const
	{
		union Cast
		{
			Cast(DxilModule::ShaderFlags const & flags)
				: shader_flags(flags)
			{
			}

			DxilModule::ShaderFlags shader_flags;
			uint64_t raw_data;
		};
		static_assert(sizeof(uint64_t) == sizeof(DxilModule::ShaderFlags), "size must match to make sure no undefined bits when cast");

		Cast raw_cast(*this);
		return raw_cast.raw_data;
	}

	void DxilModule::ShaderFlags::SetShaderFlagsRaw(uint64_t data)
	{
		union Cast {
			Cast(uint64_t data) {
				rawData = data;
			}
			DxilModule::ShaderFlags shaderFlags;
			uint64_t  rawData;
		};

		Cast rawCast(data);
		*this = rawCast.shaderFlags;
	}



	DxilModule::DxilModule(LLVMModule* mod)
		: context_(mod->Context()), module_(mod),
			md_helper_(std::make_unique<DxilMDHelper>(mod, std::make_unique<DxilExtraPropertyHelper>(mod))),
			type_system_(std::make_unique<DxilTypeSystem>(mod)),
			sm_(nullptr)
	{
		BOOST_ASSERT(mod != nullptr);
	}

	DxilModule::~DxilModule()
	{
	}

	void DxilModule::LoadDxilMetadata()
	{
		BOOST_ASSERT_MSG(sm_ == nullptr, "Shader model must not change for the module");

		md_helper_->LoadDxilVersion(dxil_major_, dxil_minor_);

		DxilShaderModel const * sm;
		md_helper_->LoadDxilShaderModel(sm);
		sm_ = sm;
		md_helper_->ShaderModel(sm_);

		auto shader_kind = sm_->GetKind();
		input_signature_ = std::make_unique<DxilSignature>(shader_kind, SignatureKind::Input);
		output_signature_ = std::make_unique<DxilSignature>(shader_kind, SignatureKind::Output);
		patch_constant_signature_ = std::make_unique<DxilSignature>(shader_kind, SignatureKind::PatchConstant);
		root_signature_ = std::make_unique<DxilRootSignatureHandle>();

		auto entries = md_helper_->GetDxilEntryPoints();
		TIFBOOL(entries->NumOperands() == 1);

		MDOperand const * signatures;
		MDOperand const * resources;
		MDOperand const * properties;
		md_helper_->GetDxilEntryPoint(entries->Operand(0), entry_func_, entry_name_, signatures, resources, properties);

		md_helper_->LoadDxilSignatures(*signatures, *input_signature_,
			*output_signature_, *patch_constant_signature_);

		this->LoadDxilResources(*resources);
		this->LoadDxilShaderProperties(*properties);
		md_helper_->LoadDxilTypeSystem(*type_system_);
	}

	void DxilModule::LoadDxilResources(MDOperand const & operand)
	{
		if (operand.Get() == nullptr)
		{
			return;
		}

		MDTuple const * srvs;
		MDTuple const * uavs;
		MDTuple const * cbuffers;
		MDTuple const * samplers;
		md_helper_->GetDxilResources(operand, srvs, uavs, cbuffers, samplers);

		if (srvs != nullptr)
		{
			for (uint32_t i = 0; i < srvs->NumOperands(); ++ i)
			{
				auto srv = std::make_unique<DxilResource>();
				md_helper_->LoadDxilSRV(srvs->Operand(i), *srv);
				this->AddSRV(std::move(srv));
			}
		}

		if (uavs != nullptr)
		{
			for (uint32_t i = 0; i < uavs->NumOperands(); ++ i)
			{
				auto uav = std::make_unique<DxilResource>();
				md_helper_->LoadDxilUAV(uavs->Operand(i), *uav);
				this->AddUAV(std::move(uav));
			}
		}

		if (cbuffers != nullptr)
		{
			for (uint32_t i = 0; i < cbuffers->NumOperands(); ++ i)
			{
				auto cbuffer = std::make_unique<DxilCBuffer>();
				md_helper_->LoadDxilCBuffer(cbuffers->Operand(i), *cbuffer);
				this->AddCBuffer(std::move(cbuffer));
			}
		}

		if (samplers != nullptr)
		{
			for (uint32_t i = 0; i < samplers->NumOperands(); ++ i)
			{
				auto sampler = std::make_unique<DxilSampler>();
				md_helper_->LoadDxilSampler(samplers->Operand(i), *sampler);
				this->AddSampler(std::move(sampler));
			}
		}
	}

	void DxilModule::LoadDxilShaderProperties(MDOperand const & operand)
	{
		if (operand.Get() == nullptr)
		{
			return;
		}

		MDTuple const * tuple_md = dyn_cast<MDTuple>(operand.Get());
		TIFBOOL(tuple_md != nullptr);
		TIFBOOL((tuple_md->NumOperands() & 0x1) == 0);

		for (uint32_t i = 0; i < tuple_md->NumOperands(); i += 2)
		{
			uint32_t tag = DxilMDHelper::ConstMDToUInt32(tuple_md->Operand(i));
			MDOperand const & mdn = tuple_md->Operand(i + 1);
			TIFBOOL(mdn.Get() != nullptr);

			switch (tag)
			{
			case DxilMDHelper::DxilShaderFlagsTag:
				shader_flags_.SetShaderFlagsRaw(DxilMDHelper::ConstMDToUInt64(mdn));
				break;

			case DxilMDHelper::DxilNumThreadsTag:
				{
					MDNode* node = cast<MDNode>(mdn.Get());
					num_threads_[0] = DxilMDHelper::ConstMDToUInt32(node->Operand(0));
					num_threads_[1] = DxilMDHelper::ConstMDToUInt32(node->Operand(1));
					num_threads_[2] = DxilMDHelper::ConstMDToUInt32(node->Operand(2));
					break;
				}

			case DxilMDHelper::DxilGSStateTag:
				md_helper_->LoadDxilGSState(mdn, input_primitive_, max_vertex_count_, active_stream_mask_,
					stream_primitive_topology_, num_gs_instances_);
				break;

			case DxilMDHelper::DxilDSStateTag:
				md_helper_->LoadDxilDSState(mdn, tessellator_domain_, input_control_point_count_);
				break;

			case DxilMDHelper::DxilHSStateTag:
				md_helper_->LoadDxilHSState(mdn, patch_constant_func_, input_control_point_count_, output_control_point_count_,
					tessellator_domain_, tessellator_partitioning_, tessellator_output_primitive_, max_tessellation_factor_);
				break;

			case DxilMDHelper::DxilRootSignatureTag:
				md_helper_->LoadRootSignature(mdn, *root_signature_);
				break;

			default:
				DILITHIUM_UNREACHABLE("Unknown extended shader properties tag");
				break;
			}
		}
	}


	template <typename T>
	uint32_t DxilModule::AddResource(std::vector<std::unique_ptr<T>>& vec, std::unique_ptr<T> res)
	{
		uint32_t id = static_cast<uint32_t>(vec.size());
		BOOST_ASSERT(id < UINT_MAX);
		vec.emplace_back(std::move(res));
		return id;
	}

	uint32_t DxilModule::AddCBuffer(std::unique_ptr<DxilCBuffer> cb)
	{
		return this->AddResource<DxilCBuffer>(cbuffers_, std::move(cb));
	}

	uint32_t DxilModule::AddSampler(std::unique_ptr<DxilSampler> sampler)
	{
		return this->AddResource<DxilSampler>(samplers_, std::move(sampler));
	}

	uint32_t DxilModule::AddSRV(std::unique_ptr<DxilResource> srv)
	{
		return this->AddResource<DxilResource>(srvs_, std::move(srv));
	}

	uint32_t DxilModule::AddUAV(std::unique_ptr<DxilResource> uav)
	{
		return this->AddResource<DxilResource>(uavs_, std::move(uav));
	}

	DxilSignature& DxilModule::GetInputSignature()
	{
		return *input_signature_;
	}

	DxilSignature const & DxilModule::GetInputSignature() const
	{
		return *input_signature_;
	}

	DxilSignature& DxilModule::GetOutputSignature()
	{
		return *output_signature_;
	}

	DxilSignature const & DxilModule::GetOutputSignature() const
	{
		return *output_signature_;
	}

	DxilSignature& DxilModule::GetPatchConstantSignature()
	{
		return *patch_constant_signature_;
	}

	DxilSignature const & DxilModule::GetPatchConstantSignature() const
	{
		return *patch_constant_signature_;
	}

	DxilRootSignatureHandle const & DxilModule::GetRootSignature() const
	{
		return *root_signature_;
	}
}
