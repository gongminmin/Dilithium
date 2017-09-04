/**
 * @file DxilSignatureElement.hpp
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

#ifndef _DILITHIUM_DXIL_SIGNATURE_ELEMENT_HPP
#define _DILITHIUM_DXIL_SIGNATURE_ELEMENT_HPP

#pragma once

#include <Dilithium/dxc/HLSL/DxilCompType.hpp>
#include <Dilithium/dxc/HLSL/DxilConstants.hpp>
#include <Dilithium/dxc/HLSL/DxilSemantic.hpp>

namespace Dilithium
{
	class DxilSignatureElement
	{
	public:
		static uint32_t constexpr UNDEFINED_ID = UINT_MAX;

	public:
		explicit DxilSignatureElement(SigPointKind kind);
		virtual ~DxilSignatureElement();

		void Initialize(std::string_view name, DxilCompType const & elem_type, InterpolationMode interp_mode,
			uint32_t rows, uint32_t cols,
			int start_row = DxilSemantic::UNDEFINED_ROW, int start_col = DxilSemantic::UNDEFINED_COL,
			uint32_t id = UNDEFINED_ID, std::vector<uint32_t> const & index_vec = std::vector<uint32_t>());

		uint32_t GetId() const
		{
			return id_;
		}
		void SetId(uint32_t id)
		{
			id_ = id;
		}

		uint32_t GetRows() const
		{
			return rows_;
		}
		void SetRows(uint32_t rows)
		{
			rows_ = rows;
		}
		uint32_t GetCols() const
		{
			return cols_;
		}
		void SetCols(uint32_t cols)
		{
			cols_ = cols;
		}
		InterpolationMode const * GetInterpolationMode() const
		{
			return &interp_mode_;
		}
		DxilCompType GetCompType() const
		{
			return comp_type_;
		}
		uint32_t GetOutputStream() const
		{
			return output_stream_;
		}
		void SetOutputStream(uint32_t stream)
		{
			output_stream_ = stream;
		}

		DxilSemantic const * GetSemantic() const
		{
			return semantic_;
		}
		void SetKind(SemanticKind kind);
		SemanticKind GetKind() const
		{
			return semantic_->GetKind();
		}

		int GetStartRow() const
		{
			return start_row_;
		}
		void SetStartRow(int start_row)
		{
			start_row_ = start_row;
		}
		int GetStartCol() const
		{
			return start_col_;
		}
		void SetStartCol(int start_col)
		{
			start_col_ = start_col;
		}

	private:
		SigPointKind sig_point_kind_;
		DxilSemantic const * semantic_;
		uint32_t id_;
		std::string name_;
		std::string_view semantic_name_;
		uint32_t semantic_start_index_;
		DxilCompType comp_type_;
		InterpolationMode interp_mode_;
		std::vector<uint32_t> semantic_index_;
		uint32_t rows_;
		uint32_t cols_;
		int start_row_;
		int start_col_;
		uint32_t output_stream_;
	};
}

#endif		// _DILITHIUM_DXIL_SIGNATURE_ELEMENT_HPP
