/**
 * @file DxilSignature.cpp
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
#include <Dilithium/dxc/HLSL/DxilSignature.hpp>
#include <Dilithium/dxc/HLSL/DxilSigPoint.hpp>

namespace Dilithium
{
	DxilSignature::DxilSignature(ShaderKind shader_kind, SignatureKind sig_kind)
		: sig_point_kind_(DxilSigPoint::GetKind(shader_kind, sig_kind, false, false))
	{
	}

	DxilSignature::DxilSignature(SigPointKind sig_point_kind)
		: sig_point_kind_(sig_point_kind)
	{
	}

	DxilSignature::~DxilSignature()
	{
	}

	bool DxilSignature::IsInput() const
	{
		return DxilSigPoint::GetSigPoint(sig_point_kind_)->IsInput();
	}

	bool DxilSignature::IsOutput() const
	{
		return DxilSigPoint::GetSigPoint(sig_point_kind_)->IsOutput();
	}

	std::unique_ptr<DxilSignatureElement> DxilSignature::CreateElement()
	{
		return std::make_unique<DxilSignatureElement>(sig_point_kind_);
	}

	uint32_t DxilSignature::AppendElement(std::unique_ptr<DxilSignatureElement> se, bool set_id)
	{
		uint32_t id = static_cast<uint32_t>(elements_.size());
		if (set_id)
		{
			se->SetId(id);
		}
		elements_.emplace_back(std::move(se));
		return id;
	}
}
