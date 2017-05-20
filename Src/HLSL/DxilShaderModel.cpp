/**
 * @file DxilShaderModel.cpp
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
#include <Dilithium/dxc/HLSL/DxilShaderModel.hpp>

namespace
{
	using namespace Dilithium;

	const DxilShaderModel SHADER_MODELS[] =
	{
		//                                                     IR  OR   UAV?   TyUAV? UAV base
		DxilShaderModel(ShaderKind::Compute,  4, 0, "cs_4_0",  0,  0,   true,  false, 1),
		DxilShaderModel(ShaderKind::Compute,  4, 1, "cs_4_1",  0,  0,   true,  false, 1),
		DxilShaderModel(ShaderKind::Compute,  5, 0, "cs_5_0",  0,  0,   true,  true,  64),
		DxilShaderModel(ShaderKind::Compute,  5, 1, "cs_5_1",  0,  0,   true,  true,  UINT_MAX),
		DxilShaderModel(ShaderKind::Compute,  6, 0, "cs_6_0",  0,  0,   true,  true,  UINT_MAX),

		DxilShaderModel(ShaderKind::Domain,   5, 0, "ds_5_0",  32, 32,  true,  true,  64),
		DxilShaderModel(ShaderKind::Domain,   5, 1, "ds_5_1",  32, 32,  true,  true,  UINT_MAX),
		DxilShaderModel(ShaderKind::Domain,   6, 0, "ds_6_0",  32, 32,  true,  true,  UINT_MAX),

		DxilShaderModel(ShaderKind::Geometry, 4, 0, "gs_4_0",  16, 32,  false, false, 0),
		DxilShaderModel(ShaderKind::Geometry, 4, 1, "gs_4_1",  32, 32,  false, false, 0),
		DxilShaderModel(ShaderKind::Geometry, 5, 0, "gs_5_0",  32, 32,  true,  true,  64),
		DxilShaderModel(ShaderKind::Geometry, 5, 1, "gs_5_1",  32, 32,  true,  true,  UINT_MAX),
		DxilShaderModel(ShaderKind::Geometry, 6, 0, "gs_6_0",  32, 32,  true,  true,  UINT_MAX),

		DxilShaderModel(ShaderKind::Hull,     5, 0, "hs_5_0",  32, 32,  true,  true,  64),
		DxilShaderModel(ShaderKind::Hull,     5, 1, "hs_5_1",  32, 32,  true,  true,  UINT_MAX),
		DxilShaderModel(ShaderKind::Hull,     6, 0, "hs_6_0",  32, 32,  true,  true,  UINT_MAX),

		DxilShaderModel(ShaderKind::Pixel,    4, 0, "ps_4_0",  32, 8,   false, false, 0),
		DxilShaderModel(ShaderKind::Pixel,    4, 1, "ps_4_1",  32, 8,   false, false, 0),
		DxilShaderModel(ShaderKind::Pixel,    5, 0, "ps_5_0",  32, 8,   true,  true,  64),
		DxilShaderModel(ShaderKind::Pixel,    5, 1, "ps_5_1",  32, 8,   true,  true,  UINT_MAX),
		DxilShaderModel(ShaderKind::Pixel,    6, 0, "ps_6_0",  32, 8,   true,  true,  UINT_MAX),

		DxilShaderModel(ShaderKind::Vertex,   4, 0, "vs_4_0",  16, 16,  false, false, 0),
		DxilShaderModel(ShaderKind::Vertex,   4, 1, "vs_4_1",  32, 32,  false, false, 0),
		DxilShaderModel(ShaderKind::Vertex,   5, 0, "vs_5_0",  32, 32,  true,  true,  64),
		DxilShaderModel(ShaderKind::Vertex,   5, 1, "vs_5_1",  32, 32,  true,  true,  UINT_MAX),
		DxilShaderModel(ShaderKind::Vertex,   6, 0, "vs_6_0",  32, 32,  true,  true,  UINT_MAX),

		DxilShaderModel(ShaderKind::Invalid,  0, 0, "invalid", 0,  0,   false, false, 0),
	};
	uint32_t constexpr NUM_SHADER_MODELS = sizeof(SHADER_MODELS) / sizeof(SHADER_MODELS[0]);

	uint32_t constexpr MAX_NUM_TEMP_REG = 4096;
	uint32_t constexpr MAX_CBUFFER_SIZE = 4096;
}

namespace Dilithium
{
	DxilShaderModel::DxilShaderModel(ShaderKind kind, uint32_t major, uint32_t minor, std::string_view name,
		uint32_t num_input_regs, uint32_t num_output_regs,
		bool supports_uavs, bool supports_typed_uavs, uint32_t num_uav_regs)
		: kind_(kind), major_(major), minor_(minor), name_(name),
			num_input_regs_(num_input_regs), num_output_regs_(num_output_regs),
			supports_uavs_(supports_uavs), supports_typed_uavs_(supports_typed_uavs), num_uav_regs_(num_uav_regs)
	{
	}

	bool DxilShaderModel::IsValid() const
	{
		BOOST_ASSERT_MSG((kind_ >= ShaderKind::Pixel) || (kind_ <= ShaderKind::Invalid), "Invalid shader model");
		return kind_ != ShaderKind::Invalid;
	}

	std::string DxilShaderModel::GetKindName() const
	{
		return name_.substr(0, 2).to_string();
	}

	uint32_t DxilShaderModel::GetNumTempRegs() const
	{
		return MAX_NUM_TEMP_REG;
	}

	uint32_t DxilShaderModel::GetCBufferSize() const
	{
		return MAX_CBUFFER_SIZE;
	}

	uint32_t DxilShaderModel::Count()
	{
		return NUM_SHADER_MODELS - 1;
	}

	DxilShaderModel const * DxilShaderModel::Get(uint32_t idx)
	{
		return (idx < NUM_SHADER_MODELS - 1) ? &SHADER_MODELS[idx] : GetInvalid();
	}

	DxilShaderModel const * DxilShaderModel::Get(ShaderKind kind, uint32_t major, uint32_t minor)
	{
		for (uint32_t i = 0; i < NUM_SHADER_MODELS; ++ i)
		{
			auto const & sm = SHADER_MODELS[i];
			if ((sm.kind_ == kind) && (sm.major_ == major) && (sm.minor_ == minor))
			{
				return &sm;
			}
		}

		return GetInvalid();
	}

	DxilShaderModel const * DxilShaderModel::GetByName(std::string_view name)
	{
		// [ps|vs|gs|hs|ds|cs]_[major]_[minor]
		ShaderKind kind;
		switch (name[0])
		{
		case 'p':
			kind = ShaderKind::Pixel;
			break;
		case 'v':
			kind = ShaderKind::Vertex;
			break;
		case 'g':
			kind = ShaderKind::Geometry;
			break;
		case 'h':
			kind = ShaderKind::Hull;
			break;
		case 'd':
			kind = ShaderKind::Domain;
			break;
		case 'c':
			kind = ShaderKind::Compute;
			break;
		default:
			return GetInvalid();
		}

		uint32_t idx = 3;
		if (name[1] != 's' || name[2] != '_')
		{
			return GetInvalid();
		}

		uint32_t major;
		switch (name[idx])
		{
		case '4':
			major = 4;
			break;
		case '5':
			major = 5;
			break;
		case '6':
			major = 6;
			break;
		default:
			return GetInvalid();
		}
		++ idx;
		if (name[idx] != '_')
		{
			return GetInvalid();
		}
		++ idx;

		uint32_t minor;
		switch (name[idx])
		{
		case '0':
			minor = 0;
			break;
		case '1':
			minor = 1;
			break;
		default:
			return GetInvalid();
		}

		return Get(kind, major, minor);
	}

	DxilShaderModel const * DxilShaderModel::GetInvalid()
	{
		return &SHADER_MODELS[NUM_SHADER_MODELS - 1];
	}
}
