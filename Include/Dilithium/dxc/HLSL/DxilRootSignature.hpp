/**
 * @file DxilRootSignature.hpp
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

#ifndef _DILITHIUM_DXIL_ROOT_SIGNATURE_HPP
#define _DILITHIUM_DXIL_ROOT_SIGNATURE_HPP

#pragma once

namespace Dilithium
{
	enum class DxilRootSignatureVersion
	{
		Version_1 = 1,
		Version_1_0 = 1,
		Version_1_1 = 2
	};

	enum class DxilRootParameterType
	{
		DescriptorTable = 0,
		Constants32Bit = 1,
		CBV = 2,
		SRV = 3,
		UAV = 4
	};

	enum class DxilShaderVisibility
	{
		All = 0,
		Vertex = 1,
		Hull = 2,
		Domain = 3,
		Geometry = 4,
		Pixel = 5
	};

	enum class DxilDescriptorRangeType : uint32_t
	{
		SRV = 0,
		UAV = 1,
		CBV = 2,
		Sampler = 3
	};

	enum class DxilRootSignatureFlags : uint32_t
	{
		None = 0,
		AllowInputAssemblerInputLayout = 0x1,
		DenyVertexShaderRootAccess = 0x2,
		DenyHullShaderRootAccess = 0x4,
		DenyDomainShaderRootAccess = 0x8,
		DenyGeometryShaderRootAccess = 0x10,
		DenyPixelShaderRootAccess = 0x20,
		AllowStreamOutput = 0x40,
		AllowLowTierReservedHwCbLimit = 0x80000000,
		ValidFlags = 0x8000007F
	};

	enum class DxilDescriptorRangeFlags : uint32_t
	{
		None = 0,
		DescriptorsVolatile = 0x1,
		DataVolatile = 0x2,
		DataStaticWhileSetAtExecute = 0x4,
		DataStatic = 0x8,
		ValidFlags = 0xF,
		ValidSamplerFlags = DescriptorsVolatile
	};

	enum class DxilRootDescriptorFlags : uint32_t
	{
		None = 0,
		DataVolatile = 0x2,
		DataStaticWhileSetAtExecute = 0x4,
		DataStatic = 0x8,
		ValidFlags = 0xE
	};

	enum class DxilFilter
	{
		MinMagMipPoint = 0,
		MinMagPointMipLinear = 0x1,
		MinPointMagLinearMipPoint = 0x4,
		MinPointMagMipLinear = 0x5,
		MinLinearMagMipPoint = 0x10,
		MinLinearMagPointMipLinear = 0x11,
		MinMagLinearMipPoint = 0x14,
		MinMagMipLinear = 0x15,
		Anisotropic = 0x55,
		ComparisonMinMagMipPoint = 0x80,
		ComparisonMinMagPointMipLinear = 0x81,
		ComparisonMinPointMagLinearMipPoint = 0x84,
		ComparisonMinPointMagMipLinear = 0x85,
		ComparisonMinLinearMagMipPoint = 0x90,
		ComparisonMinLinearMagPointMipLinear = 0x91,
		ComparisonMinMagLinearMipPoint = 0x94,
		ComparisonMinMagMipLinear = 0x95,
		ComparisonAnisotropic = 0xD5,
		MinimumMinMagMipPoint = 0x100,
		MinimumMinMagPointMipLinear = 0x101,
		MinimumMinPointMagLinearMipPoint = 0x104,
		MinimumMinPointMagMipLinear = 0x105,
		MinimumMinLinearMagMipPoint = 0x110,
		MinimumMinLinearMagPointMipLinear = 0x111,
		MinimumMinMagLinearMipPoint = 0x114,
		MinimumMinMagMipLinear = 0x115,
		MinimumAnisotropic = 0x155,
		MaximumMinMagMipPoint = 0x180,
		MaximumMinMagPointMipLinear = 0x181,
		MaximumMinPointMagLinearMipPoint = 0x184,
		MaximumMinPointMagMipLinear = 0x185,
		MaximumMinLinearMagMipPoint = 0x190,
		MaximumMinLinearMagPointMipLinear = 0x191,
		MaximumMinMagLinearMipPoint = 0x194,
		MaximumMinMagMipLinear = 0x195,
		MaximumAnisotropic = 0x1D5
	};

	enum class DxilTextureAddressMode
	{
		Wrap = 1,
		Mirror = 2,
		Clamp = 3,
		Border = 4,
		MirrorOnce = 5
	};
	
	enum class DxilComparisonFunc : uint32_t
	{
		Never = 1,
		Less = 2,
		Equal = 3,
		LessEqual = 4,
		Greater = 5,
		NotEqual = 6,
		GreaterEqual = 7,
		Always = 8
	};

	enum class DxilStaticBorderColor
	{
		TransparentBlack = 0,
		OpaqueBlack = 1,
		OpaqueWhite = 2
	};

	struct DxilDescriptorRange
	{
		DxilDescriptorRangeType range_type;
		uint32_t num_descriptors;
		uint32_t base_shader_register;
		uint32_t register_space;
		uint32_t offset_in_descriptors_from_table_start;
	};

	struct DxilDescriptorRange1
	{
		DxilDescriptorRangeType range_type;
		uint32_t num_descriptors;
		uint32_t base_shader_register;
		uint32_t register_space;
		DxilDescriptorRangeFlags flags;
		uint32_t offset_in_descriptors_from_table_start;
	};

	struct DxilRootConstants
	{
		uint32_t shader_register;
		uint32_t register_space;
		uint32_t num_32bit_values;
	};

	struct DxilRootDescriptor
	{
		uint32_t shader_register;
		uint32_t register_space;
	};

	struct DxilRootDescriptor1
	{
		uint32_t shader_register;
		uint32_t register_space;
		DxilRootDescriptorFlags flags;
	};

	struct DxilRootDescriptorTable
	{
		uint32_t num_descriptor_ranges;
		DxilDescriptorRange const * descriptor_ranges;
	};

	struct DxilRootDescriptorTable1
	{
		uint32_t num_descriptor_ranges;
		DxilDescriptorRange1 const * descriptor_ranges;
	};

	struct DxilStaticSamplerDesc
	{
		DxilFilter filter;
		DxilTextureAddressMode address_u;
		DxilTextureAddressMode address_v;
		DxilTextureAddressMode address_w;
		float mip_lod_bias;
		uint32_t max_anisotropy;
		DxilComparisonFunc comparison_func;
		DxilStaticBorderColor border_color;
		float min_lod;
		float max_lod;
		uint32_t shader_register;
		uint32_t register_space;
		DxilShaderVisibility shader_visibility;
	};

	struct DxilRootParameter
	{
		DxilRootParameterType parameter_type;
		union
		{
			DxilRootDescriptorTable descriptor_table;
			DxilRootConstants constants;
			DxilRootDescriptor descriptor;
		};
		DxilShaderVisibility shader_visibility;
	};

	struct DxilRootParameter1
	{
		DxilRootParameterType parameter_type;
		union
		{
			DxilRootDescriptorTable1 descriptor_table;
			DxilRootConstants constants;
			DxilRootDescriptor1 descriptor;
		};
		DxilShaderVisibility shader_visibility;
	};

	struct DxilRootSignatureDesc
	{
		uint32_t num_parameters;
		DxilRootParameter const * parameters;
		uint32_t num_static_samplers;
		DxilStaticSamplerDesc const * static_samplers;
		DxilRootSignatureFlags flags;
	};

	struct DxilRootSignatureDesc1
	{
		uint32_t num_parameters;
		DxilRootParameter1 const * parameters;
		uint32_t num_static_samplers;
		DxilStaticSamplerDesc const * static_samplers;
		DxilRootSignatureFlags flags;
	};

	struct DxilVersionedRootSignatureDesc
	{
		DxilRootSignatureVersion version;
		union
		{
			DxilRootSignatureDesc desc_1_0;
			DxilRootSignatureDesc1 desc_1_1;
		};
	};

	class DxilRootSignatureHandle
	{
	public:
		DxilRootSignatureHandle()
			: desc_(nullptr)
		{
		}
		DxilRootSignatureHandle(DxilRootSignatureHandle const & rhs) = delete;
		DxilRootSignatureHandle(DxilRootSignatureHandle&& rhs);
		~DxilRootSignatureHandle()
		{
			this->Clear();
		}
		
		bool IsEmpty() const
		{
			return (desc_ == nullptr) && serialized_.empty();
		}
		uint8_t const * GetSerializedBytes() const;
		uint32_t GetSerializedSize() const;

		void Clear();
		void LoadSerialized(uint8_t const * data, uint32_t length);

	private:
		DxilVersionedRootSignatureDesc const * desc_;
		std::vector<uint8_t> serialized_;
	};
}

#endif		// _DILITHIUM_DXIL_ROOT_SIGNATURE_HPP
