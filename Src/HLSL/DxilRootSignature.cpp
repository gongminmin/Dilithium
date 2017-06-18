/**
 * @file DxilRootSignature.cpp
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
#include <Dilithium/dxc/HLSL/DxilRootSignature.hpp>

namespace
{
	using namespace Dilithium;

	template <typename T>
	void DeleteRootSignatureTemplate(T const & root_signature_desc)
	{
		for (uint32_t i = 0; i < root_signature_desc.num_parameters; ++ i)
		{
			auto const & param = root_signature_desc.parameters[i];
			if (param.parameter_type == DxilRootParameterType::DescriptorTable)
			{
				delete[] param.descriptor_table.descriptor_ranges;
			}
		}

		delete[] root_signature_desc.parameters;
		delete[] root_signature_desc.static_samplers;
	}

	void DeleteRootSignature(DxilVersionedRootSignatureDesc const * root_signature)
	{
		if (root_signature != nullptr)
		{
			switch (root_signature->version)
			{
			case DxilRootSignatureVersion::Version_1_0:
				DeleteRootSignatureTemplate<DxilRootSignatureDesc>(root_signature->desc_1_0);
				break;

			case DxilRootSignatureVersion::Version_1_1:
			default:
				BOOST_ASSERT_MSG(root_signature->version == DxilRootSignatureVersion::Version_1_1, "Invalid version");
				DeleteRootSignatureTemplate<DxilRootSignatureDesc1>(root_signature->desc_1_1);
				break;
			}

			delete root_signature;
		}
	}
}

namespace Dilithium
{
	DxilRootSignatureHandle::DxilRootSignatureHandle(DxilRootSignatureHandle&& rhs)
	{
		desc_ = std::move(rhs.desc_);
		serialized_ = std::move(rhs.serialized_);
	}

	void DxilRootSignatureHandle::Clear()
	{
		DeleteRootSignature(desc_);
		desc_ = nullptr;
		serialized_.clear();
	}

	uint8_t const * DxilRootSignatureHandle::GetSerializedBytes() const
	{
		return serialized_.data();
	}

	uint32_t DxilRootSignatureHandle::GetSerializedSize() const
	{
		return static_cast<uint32_t>(serialized_.size());
	}

	void DxilRootSignatureHandle::LoadSerialized(uint8_t const * data, uint32_t length)
	{
		BOOST_ASSERT(this->IsEmpty());
		serialized_.assign(data, data + length);
	}
}
