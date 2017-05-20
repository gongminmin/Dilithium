/**
 * @file DxilSigPoint.hpp
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

#ifndef _DILITHIUM_DXIL_SIG_POINT_HPP
#define _DILITHIUM_DXIL_SIG_POINT_HPP

#pragma once

#include <Dilithium/CXX17/string_view.hpp>
#include <Dilithium/dxc/HLSL/DxilConstants.hpp>

namespace Dilithium
{
	class DxilSigPoint
	{
	public:
		DxilSigPoint(SigPointKind spk, std::string_view name, SigPointKind rspk, ShaderKind shk, SignatureKind sigk, PackingKind pk);

		bool IsInput() const
		{
			return signature_kind_ == SignatureKind::Input;
		}
		bool IsOutput() const
		{
			return signature_kind_ == SignatureKind::Output;
		}
		bool IsPatchConstant() const
		{
			return signature_kind_ == SignatureKind::PatchConstant;
		}

		SigPointKind GetKind() const
		{
			return kind_;
		}
		std::string_view GetName() const
		{
			return name_;
		}
		ShaderKind GetShaderKind() const
		{
			return shader_kind_;
		}
		SigPointKind GetRelatedKind() const
		{
			return related_kind_;
		}
		SignatureKind GetSignatureKind() const
		{
			return signature_kind_;
		}
		SignatureKind GetSignatureKindWithFallback() const;
		PackingKind GetPackingKind() const
		{
			return packing_kind_;
		}
		bool NeedsInterpMode() const
		{
			return packing_kind_ == PackingKind::Vertex;
		}

		static DxilSigPoint const * GetSigPoint(SigPointKind kind);
		static SigPointKind GetKind(ShaderKind shader_kind, SignatureKind sig_kind,
			bool is_patch_constant_function, bool is_special_input);
		static SemanticInterpretationKind GetInterpretation(SemanticKind sk, SigPointKind kind,
			uint32_t major_version, uint32_t minor_version);
		static SigPointKind RecoverKind(SemanticKind sk, SigPointKind kind);

	private:
		SigPointKind kind_;
		SigPointKind related_kind_;
		ShaderKind shader_kind_;
		SignatureKind signature_kind_;
		std::string_view name_;
		PackingKind packing_kind_;
	};
}

#endif		// _DILITHIUM_DXIL_SIG_POINT_HPP
