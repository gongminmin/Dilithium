/**
 * @file DxilShaderModel.hpp
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

#ifndef _DILITHIUM_DXIL_SHADER_MODEL_HPP
#define _DILITHIUM_DXIL_SHADER_MODEL_HPP

#pragma once

#include <Dilithium/CXX17/string_view.hpp>
#include <Dilithium/dxc/HLSL/DxilConstants.hpp>

#include <string>

namespace Dilithium
{
	class DxilShaderModel
	{
		DxilShaderModel() = delete;

	public:
		DxilShaderModel(ShaderKind kind, uint32_t major, uint32_t minor, std::string_view name,
			uint32_t num_input_regs, uint32_t num_output_regs,
			bool supports_uavs, bool supports_typed_uavs, uint32_t num_uav_regs);

		bool IsPS() const
		{
			return kind_ == ShaderKind::Pixel;
		}
		bool IsVS() const
		{
			return kind_ == ShaderKind::Vertex;
		}
		bool IsGS() const
		{
			return kind_ == ShaderKind::Geometry;
		}
		bool IsHS() const
		{
			return kind_ == ShaderKind::Hull;
		}
		bool IsDS() const
		{
			return kind_ == ShaderKind::Domain;
		}
		bool IsCS() const
		{
			return kind_ == ShaderKind::Compute;
		}
		bool IsValid() const;

		ShaderKind GetKind() const
		{
			return kind_;
		}

		uint32_t GetMajor() const
		{
			return major_;
		}
		uint32_t GetMinor() const
		{
			return minor_;
		}
		bool IsSM50Plus() const
		{
			return major_ >= 5;
		}
		bool IsSM51Plus() const
		{
			return (major_ > 5) || ((major_ == 5) && (minor_ >= 1));
		}

		std::string_view GetName() const
		{
			return name_;
		}
		std::string GetKindName() const;

		uint32_t GetNumTempRegs() const;
		uint32_t GetNumInputRegs() const
		{
			return num_input_regs_;
		}
		uint32_t GetNumOutputRegs() const
		{
			return num_output_regs_;
		}
		uint32_t GetCBufferSize() const;
		uint32_t SupportsUAV() const
		{
			return supports_uavs_;
		}
		uint32_t SupportsTypedUAVs() const
		{
			return supports_typed_uavs_;
		}
		uint32_t GetUAVRegLimit() const
		{
			return num_uav_regs_;
		}

		static uint32_t Count();

		static DxilShaderModel const * Get(uint32_t Idx);
		static DxilShaderModel const * Get(ShaderKind Kind, uint32_t major, uint32_t minor);
		static DxilShaderModel const * GetByName(std::string_view name);

	private:
		static DxilShaderModel const * GetInvalid();

	private:
		ShaderKind kind_;
		uint32_t major_;
		uint32_t minor_;
		std::string_view name_;
		uint32_t num_input_regs_;
		uint32_t num_output_regs_;
		bool supports_uavs_;
		bool supports_typed_uavs_;
		uint32_t num_uav_regs_;
	};
}

#endif		// _DILITHIUM_DXIL_MODULE_HPP
