/**
 * @file HLMatrixLowerPass.cpp
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

#include <Dilithium/DerivedType.hpp>
#include <Dilithium/dxc/HLSL/HLMatrixLowerHelper.hpp>

#include <boost/assert.hpp>

namespace Dilithium
{
	namespace HLMatrixLower
	{
		bool IsMatrixType(Type* ty)
		{
			auto* st = dyn_cast<StructType>(ty);
			if (st)
			{
				auto elt_ty = st->ElementType(0);
				if (!st->Name().find("class.matrix") == 0)
				{
					return false;
				}

				bool is_vec_array = elt_ty->IsArrayType() && elt_ty->ArrayElementType()->IsVectorType();
				return is_vec_array && (elt_ty->ArrayNumElements() <= 4);
			}
			return false;
		}

		Type* GetMatrixInfo(Type* ty, uint32_t& col, uint32_t& row)
		{
			BOOST_ASSERT_MSG(IsMatrixType(ty), "Not matrix type");

			auto st = cast<StructType>(ty);
			auto elt_ty = st->ElementType(0);
			auto col_ty = elt_ty->ArrayElementType();
			col = static_cast<uint32_t>(elt_ty->ArrayNumElements());
			row = col_ty->VectorNumElements();
			return col_ty->VectorElementType();
		}
	}
}

