/**
 * @file DxilModule.hpp
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

#ifndef _DILITHIUM_DXIL_MODULE_HPP
#define _DILITHIUM_DXIL_MODULE_HPP

#pragma once

#include <Dilithium/dxc/HLSL/DxilMdHelper.hpp>
#include <Dilithium/dxc/HLSL/DxilSignature.hpp>
#include <Dilithium/dxc/HLSL/DxilRootSignature.hpp>

namespace Dilithium
{
	class LLVMModule;
	class Function;

	class DxilModule
	{
	public:
		// Shader properties.
		class ShaderFlags
		{
		public:
			ShaderFlags();

			uint32_t GetGlobalFlags() const;
			void SetDisableOptimizations(bool flag)
			{
				disable_optimizations_ = flag;
			}
			void SetDisableMathRefactoring(bool flag)
			{
				disable_math_refactoring_ = flag;
			}
			void SetEnableDoublePrecision(bool flag)
			{
				enable_double_precision_ = flag;
			}
			void SetForceEarlyDepthStencil(bool flag)
			{
				force_early_depth_stencil_ = flag;
			}
			void SetEnableRawAndStructuredBuffers(bool flag)
			{
				enable_raw_and_structured_buffers_ = flag;
			}
			void SetEnableMinPrecision(bool flag)
			{
				enable_min_precision_ = flag;
			}
			void SetEnableDoubleExtensions(bool flag)
			{
				enable_double_extensions_ = flag;
			}
			void SetEnableMSAD(bool flag)
			{
				enable_msad_ = flag;
			}
			void SetAllResourcesBound(bool flag)
			{
				all_resources_bound_ = flag;
			}

			uint64_t GetFeatureInfo() const;
			bool GetWaveOps() const
			{
				return wave_ops_;
			}
			void SetCSRawAndStructuredViaShader4X(bool flag)
			{
				cs_raw_and_structured_via_shader_4x_ = flag;
			}
			void SetROVs(bool flag)
			{
				rovs_ = flag;
			}
			void SetWaveOps(bool flag)
			{
				wave_ops_ = flag;
			}
			void SetInt64Ops(bool flag)
			{
				int64_ops_ = flag;
			}
			void SetTiledResources(bool flag)
			{
				tiled_resources_ = flag;
			}
			void SetStencilRef(bool flag)
			{
				stencil_ref_ = flag;
			}
			void SetInnerCoverage(bool flag)
			{
				inner_coverage_ = flag;
			}
			void SetViewportAndRTArrayIndex(bool flag)
			{
				viewport_and_rt_array_index_ = flag;
			}
			void SetUAVLoadAdditionalFormats(bool flag)
			{
				uav_load_additional_formats_ = flag;
			}
			void SetLevel9ComparisonFiltering(bool flag)
			{
				level_9_comparison_filtering_ = flag;
			}
			void Set64UAVs(bool flag)
			{
				all_64_uavs_ = flag;
			}
			void SetUAVsAtEveryStage(bool flag)
			{
				uavs_at_every_stage_ = flag;
			}

			static uint64_t GetShaderFlagsRawForCollection();	// Some flags are collected (eg use 64-bit),
																// some provided (eg allow refactoring)
			uint64_t GetShaderFlagsRaw() const;
			void SetShaderFlagsRaw(uint64_t data);

		private:
			uint32_t disable_optimizations_ : 1;				// D3D11_1_SB_GLOBAL_FLAG_SKIP_OPTIMIZATION
			uint32_t disable_math_refactoring_ : 1;				// ~D3D10_SB_GLOBAL_FLAG_REFACTORING_ALLOWED
			uint32_t enable_double_precision_ : 1;				// D3D11_SB_GLOBAL_FLAG_ENABLE_DOUBLE_PRECISION_FLOAT_OPS
			uint32_t force_early_depth_stencil_ : 1;			// D3D11_SB_GLOBAL_FLAG_FORCE_EARLY_DEPTH_STENCIL
			uint32_t enable_raw_and_structured_buffers_ : 1;	// D3D11_SB_GLOBAL_FLAG_ENABLE_RAW_AND_STRUCTURED_BUFFERS
			uint32_t enable_min_precision_ : 1;					// D3D11_1_SB_GLOBAL_FLAG_ENABLE_MINIMUM_PRECISION
			uint32_t enable_double_extensions_ : 1;				// D3D11_1_SB_GLOBAL_FLAG_ENABLE_DOUBLE_EXTENSIONS
			uint32_t enable_msad_ : 1;							// D3D11_1_SB_GLOBAL_FLAG_ENABLE_SHADER_EXTENSIONS
			uint32_t all_resources_bound_ : 1;					// D3D12_SB_GLOBAL_FLAG_ALL_RESOURCES_BOUND

			// SHADER_FEATURE_VIEWPORT_AND_RT_ARRAY_INDEX_FROM_ANY_SHADER_FEEDING_RASTERIZER
			uint32_t viewport_and_rt_array_index_ : 1;
			uint32_t inner_coverage_ : 1;					// SHADER_FEATURE_INNER_COVERAGE
			uint32_t stencil_ref_ : 1;						// SHADER_FEATURE_STENCIL_REF
			uint32_t tiled_resources_ : 1;					// SHADER_FEATURE_TILED_RESOURCES
			uint32_t uav_load_additional_formats_ : 1;		// SHADER_FEATURE_TYPED_UAV_LOAD_ADDITIONAL_FORMATS
			uint32_t level_9_comparison_filtering_ : 1;		// SHADER_FEATURE_LEVEL_9_COMPARISON_FILTERING
															// SHADER_FEATURE_11_1_SHADER_EXTENSIONS shared with EnableMSAD
			uint32_t all_64_uavs_ : 1;						// SHADER_FEATURE_64_UAVS
			uint32_t uavs_at_every_stage_ : 1;				// SHADER_FEATURE_UAVS_AT_EVERY_STAGE
			// SHADER_FEATURE_COMPUTE_SHADERS_PLUS_RAW_AND_STRUCTURED_BUFFERS_VIA_SHADER_4_X is specifically about shader model 4.x.
			uint32_t cs_raw_and_structured_via_shader_4x_ : 1;

			uint32_t rovs_ : 1;			// SHADER_FEATURE_ROVS
			uint32_t wave_ops_ : 1;		// SHADER_FEATURE_WAVE_OPS
			uint32_t int64_ops_ : 1;	// SHADER_FEATURE_INT64_OPS
			uint32_t align0_ : 11;		// align to 32 bit.
			uint32_t align1_;			// align to 64 bit.
		};

	public:
		explicit DxilModule(LLVMModule* mod);
		~DxilModule();

		uint32_t AddCBuffer(std::unique_ptr<DxilCBuffer> cb);
		uint32_t AddSampler(std::unique_ptr<DxilSampler> sampler);
		uint32_t AddSRV(std::unique_ptr<DxilResource> srv);
		uint32_t AddUAV(std::unique_ptr<DxilResource> uav);

		void LoadDxilMetadata();

	private:
		void LoadDxilResources(MDOperand const & operand);
		void LoadDxilShaderProperties(MDOperand const & operand);

		template <typename T>
		uint32_t AddResource(std::vector<std::unique_ptr<T>>& vec, std::unique_ptr<T> res);

	private:
		LLVMContext& context_;
		LLVMModule* module_;
		Function* entry_func_;
		Function* patch_constant_func_;
		std::string entry_name_;
		std::unique_ptr<DxilMDHelper> md_helper_;
		DxilShaderModel const * sm_;
		uint32_t dxil_major_;
		uint32_t dxil_minor_;

		std::unique_ptr<DxilTypeSystem> type_system_;

		std::unique_ptr<DxilSignature> input_signature_;
		std::unique_ptr<DxilSignature> output_signature_;
		std::unique_ptr<DxilSignature> patch_constant_signature_;
		std::unique_ptr<DxilRootSignatureHandle> root_signature_;

		std::vector<std::unique_ptr<DxilResource>> srvs_;
		std::vector<std::unique_ptr<DxilResource>> uavs_;
		std::vector<std::unique_ptr<DxilCBuffer>> cbuffers_;
		std::vector<std::unique_ptr<DxilSampler>> samplers_;

		ShaderFlags shader_flags_;

		// Compute shader
		uint32_t num_threads_[3];

		// Geometry shader
		InputPrimitive input_primitive_;
		uint32_t max_vertex_count_;
		uint32_t active_stream_mask_;
		PrimitiveTopology stream_primitive_topology_;
		uint32_t num_gs_instances_;

		// Hull and Domain shaders
		TessellatorDomain tessellator_domain_;
		uint32_t input_control_point_count_;

		// Hull shader
		uint32_t output_control_point_count_;
		TessellatorPartitioning tessellator_partitioning_;
		TessellatorOutputPrimitive tessellator_output_primitive_;
		float max_tessellation_factor_;
	};
}

#endif		// _DILITHIUM_DXIL_MODULE_HPP
