/**
 * @file DxilResource.cpp
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
#include <Dilithium/Constant.hpp>
#include <Dilithium/DerivedType.hpp>
#include <Dilithium/dxc/HLSL/DxilResource.hpp>

namespace Dilithium
{
	DxilResource::DxilResource()
		: DxilResourceBase(ResourceClass::Invalid),
			sample_count_(0), element_stride_(0),
			globally_coherent_(false), has_counter_(false), rov_(false)
	{
	}

	Type* DxilResource::GetRetType() const
	{
		auto gv = this->GetGlobalSymbol();
		auto ty = gv->GetType()->PointerElementType();
		// For resource array, use element type.
		while (ty->IsArrayType())
		{
			ty = ty->ArrayElementType();
		}
		// Get the struct buffer type like this %class.StructuredBuffer = type {
		// %struct.mat }.
		auto st = cast<StructType>(ty);
		// Get the struct type inside struct buffer.
		return st->ElementType(0);
	}
}
