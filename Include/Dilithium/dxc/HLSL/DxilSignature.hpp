/**
 * @file DxilSignature.hpp
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

#ifndef _DILITHIUM_DXIL_SIGNATURE_HPP
#define _DILITHIUM_DXIL_SIGNATURE_HPP

#pragma once

#include <Dilithium/dxc/HLSL/DxilConstants.hpp>
#include <Dilithium/dxc/HLSL/DxilSignatureElement.hpp>

namespace Dilithium
{
	class DxilSignature
	{
	public:
		DxilSignature(ShaderKind shader_kind, SignatureKind sig_kind);
		explicit DxilSignature(SigPointKind sig_point_kind);
		virtual ~DxilSignature();

		bool IsInput() const;
		bool IsOutput() const;

		virtual std::unique_ptr<DxilSignatureElement> CreateElement();

		uint32_t AppendElement(std::unique_ptr<DxilSignatureElement> se, bool set_id = true);

		DxilSignatureElement& Element(uint32_t idx)
		{
			return *elements_[idx].get();
		}
		DxilSignatureElement const & Element(uint32_t idx) const
		{
			return *elements_[idx].get();
		}
		std::vector<std::unique_ptr<DxilSignatureElement>> const & Elements() const
		{
			return elements_;
		}

	private:
		SigPointKind sig_point_kind_;
		std::vector<std::unique_ptr<DxilSignatureElement>> elements_;
	};
}

#endif		// _DILITHIUM_DXIL_SIGNATURE_HPP
