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
#include <Dilithium/dxc/HLSL/DxilInterpolationMode.hpp>
#include <Dilithium/dxc/HLSL/DxilSemantic.hpp>
#include <Dilithium/dxc/HLSL/DxilSigPoint.hpp>
#include <Dilithium/dxc/HLSL/DxilShaderModel.hpp>

namespace Dilithium
{
	class DxilSignatureElement
	{
	public:
		static uint32_t constexpr UNDEFINED_ID = UINT_MAX;

	public:
		explicit DxilSignatureElement(SigPointKind kind);
		virtual ~DxilSignatureElement();

		void Initialize(std::string_view name, DxilCompType const & elem_type, DxilInterpolationMode interp_mode,
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

		ShaderKind GetShaderKind() const;

		SigPointKind GetSigPointKind() const
		{
			return sig_point_kind_;
		}
		void SetSigPointKind(SigPointKind sig)
		{
			sig_point_kind_ = sig;
		}

		bool IsInput() const;
		bool IsOutput() const;
		bool IsPatchConstant() const;
		char const * GetName() const;

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
		DxilInterpolationMode const * GetInterpolationMode() const
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
		bool IsArbitrary() const
		{
			return semantic_->IsArbitrary();
		}
		bool IsDepth() const
		{
			return semantic_->GetKind() == SemanticKind::Depth;
		}
		bool IsDepthLE() const
		{
			return semantic_->GetKind() == SemanticKind::DepthLessEqual;
		}
		bool IsDepthGE() const
		{
			return semantic_->GetKind() == SemanticKind::DepthGreaterEqual;
		}
		bool IsAnyDepth() const
		{
			return this->IsDepth() || this->IsDepthLE() || this->IsDepthGE();
		}
		SemanticInterpretationKind GetInterpretation() const
		{
			return DxilSigPoint::GetInterpretation(semantic_->GetKind(), sig_point_kind_,
				DxilShaderModel::HIGHEST_MAJOR, DxilShaderModel::HIGHEST_MINOR);
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
		std::vector<uint32_t> const & GetSemanticIndexVec() const
		{
			return semantic_index_;
		}
		void SetSemanticIndexVec(std::vector<uint32_t> const & vec)
		{
			semantic_index_ = vec;
		}
		void AppendSemanticIndex(uint32_t sem_idx)
		{
			semantic_index_.push_back(sem_idx);
		}

	private:
		SigPointKind sig_point_kind_;
		DxilSemantic const * semantic_;
		uint32_t id_;
		std::string name_;
		std::string_view semantic_name_;
		uint32_t semantic_start_index_;
		DxilCompType comp_type_;
		DxilInterpolationMode interp_mode_;
		std::vector<uint32_t> semantic_index_;
		uint32_t rows_;
		uint32_t cols_;
		int start_row_;
		int start_col_;
		uint32_t output_stream_;
	};
}

#endif		// _DILITHIUM_DXIL_SIGNATURE_ELEMENT_HPP
