/**
 * @file DxilSemantic.hpp
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

#ifndef _DILITHIUM_DXIL_SEMANTIC_HPP
#define _DILITHIUM_DXIL_SEMANTIC_HPP

#pragma once

#include <Dilithium/CXX17/string_view.hpp>

#include <Dilithium/dxc/HLSL/DxilConstants.hpp>
#include <Dilithium/dxc/HLSL/DxilShaderModel.hpp>

namespace Dilithium
{
	class DxilSemantic
	{
		friend class DxilShaderModel;
		friend class DxilSignatureElement;

	public:
		static int constexpr UNDEFINED_ROW = -1;
		static int constexpr UNDEFINED_COL = -1;

	public:
		DxilSemantic(SemanticKind kind, char const * name);

		static DxilSemantic const * GetByName(std::string_view name);
		static DxilSemantic const * GetByName(std::string_view name, SigPointKind sig_point_kind,
			uint32_t major_version = DxilShaderModel::HIGHEST_MAJOR, uint32_t minor_version = DxilShaderModel::HIGHEST_MINOR);
		static DxilSemantic const * Get(SemanticKind kind);
		static DxilSemantic const * Get(SemanticKind kind, SigPointKind sig_point_kind,
			uint32_t major_version = DxilShaderModel::HIGHEST_MAJOR, uint32_t minor_version = DxilShaderModel::HIGHEST_MINOR);
		static DxilSemantic const * GetInvalid();
		static DxilSemantic const * GetArbitrary();
		static bool HasSVPrefix(std::string_view name);
		static void DecomposeNameAndIndex(std::string_view full_name, std::string_view* name, uint32_t* index);

		SemanticKind GetKind() const;
		char const * GetName() const;
		bool IsArbitrary() const;
		bool IsInvalid() const;

	private:
		DxilSemantic() = delete;

	private:
		SemanticKind kind_;
		char const * name_;
	};
}

#endif		// _DILITHIUM_DXIL_SEMANTIC_HPP
