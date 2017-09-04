/**
 * @file DxilSignatureElement.cpp
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
#include <Dilithium/dxc/HLSL/DxilSignatureElement.hpp>
#include <Dilithium/dxc/HLSL/DxilSigPoint.hpp>

#include <climits>

namespace Dilithium
{
	DxilSignatureElement::DxilSignatureElement(SigPointKind kind)
		: sig_point_kind_(kind),
			id_(std::numeric_limits<uint32_t>::max())
	{
	}

	DxilSignatureElement::~DxilSignatureElement()
	{
	}

	void DxilSignatureElement::Initialize(std::string_view name, DxilCompType const & elem_type, InterpolationMode interp_mode,
		uint32_t rows, uint32_t cols,
		int start_row, int start_col,
		uint32_t id, std::vector<uint32_t> const & index_vec)
	{
		BOOST_ASSERT_MSG(semantic_ == nullptr, "An instance should be initiazed only once");

		id_ = id;
		name_ = std::string(name);
		DxilSemantic::DecomposeNameAndIndex(name, &semantic_name_, &semantic_start_index_);
		if (!index_vec.empty())
		{
			semantic_start_index_ = index_vec[0];
		}
		// Find semantic in the table.
		semantic_ = DxilSemantic::GetByName(semantic_name_, sig_point_kind_);
		comp_type_ = elem_type;
		interp_mode_ = interp_mode;
		semantic_index_ = index_vec;
		rows_ = rows;
		cols_ = cols;
		start_row_ = start_row;
		start_col_ = start_col;
		output_stream_ = 0;
	}

	uint32_t DxilSignatureElement::GetId() const
	{
		return id_;
	}

	void DxilSignatureElement::SetId(uint32_t id)
	{
		id_ = id;
	}

	void DxilSignatureElement::SetKind(SemanticKind kind)
	{
		// recover the original SigPointKind if necessary (for Shadow element).
		sig_point_kind_ = DxilSigPoint::RecoverKind(kind, sig_point_kind_);
		semantic_ = DxilSemantic::Get(kind, sig_point_kind_);
	}

	SemanticKind DxilSignatureElement::GetKind() const
	{
		return semantic_->GetKind();
	}
}
