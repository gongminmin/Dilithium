/**
 * @file Attributes.cpp
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

#include <Dilithium/Attributes.hpp>

#include <Dilithium/ErrorHandling.hpp>
#include <Dilithium/Util.hpp>

namespace Dilithium 
{
	AttributeSet AttributeSet::Get(LLVMContext& context, ArrayRef<AttributeSet> attrs)
	{
		DILITHIUM_UNUSED(context);
		DILITHIUM_UNUSED(attrs);
		DILITHIUM_NOT_IMPLEMENTED;
	}
	
	AttributeSet AttributeSet::Get(LLVMContext& context, uint32_t index, ArrayRef<Attribute::AttrKind> kind)
	{
		DILITHIUM_UNUSED(context);
		DILITHIUM_UNUSED(index);
		DILITHIUM_UNUSED(kind);
		DILITHIUM_NOT_IMPLEMENTED;
	}

	AttributeSet AttributeSet::Get(LLVMContext& context, uint32_t index, AttrBuilder const & ab)
	{
		DILITHIUM_UNUSED(context);
		DILITHIUM_UNUSED(index);
		DILITHIUM_UNUSED(ab);
		DILITHIUM_NOT_IMPLEMENTED;
	}


	AttrBuilder& AttrBuilder::AddAttribute(Attribute::AttrKind val)
	{
		DILITHIUM_UNUSED(val);
		DILITHIUM_NOT_IMPLEMENTED;
	}

	AttrBuilder& AttrBuilder::AddAttribute(Attribute attr)
	{
		DILITHIUM_UNUSED(attr);
		DILITHIUM_NOT_IMPLEMENTED;
	}

	AttrBuilder& AttrBuilder::AddAttribute(std::string_view attr, std::string_view val)
	{
		DILITHIUM_UNUSED(attr);
		DILITHIUM_UNUSED(val);
		DILITHIUM_NOT_IMPLEMENTED;
	}

	AttrBuilder& AttrBuilder::AddAlignmentAttr(uint32_t align)
	{
		DILITHIUM_UNUSED(align);
		DILITHIUM_NOT_IMPLEMENTED;
	}

	AttrBuilder& AttrBuilder::AddStackAlignmentAttr(uint32_t align)
	{
		DILITHIUM_UNUSED(align);
		DILITHIUM_NOT_IMPLEMENTED;
	}

	AttrBuilder& AttrBuilder::AddDereferenceableAttr(uint64_t bytes)
	{
		DILITHIUM_UNUSED(bytes);
		DILITHIUM_NOT_IMPLEMENTED;
	}

	AttrBuilder& AttrBuilder::AddDereferenceableOrNullAttr(uint64_t bytes)
	{
		DILITHIUM_UNUSED(bytes);
		DILITHIUM_NOT_IMPLEMENTED;
	}

	AttrBuilder& AttrBuilder::AddRawValue(uint64_t val)
	{
		DILITHIUM_UNUSED(val);
		DILITHIUM_NOT_IMPLEMENTED;
	}
}
