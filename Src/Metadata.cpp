/**
 * @file Metadata.cpp
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

#include <Dilithium/Metadata.hpp>

#include <Dilithium/ErrorHandling.hpp>
#include <Dilithium/Util.hpp>

namespace Dilithium 
{
	MetadataAsValue* MetadataAsValue::Get(LLVMContext& context, Metadata* md)
	{
		DILITHIUM_UNUSED(context);
		DILITHIUM_UNUSED(md);
		DILITHIUM_NOT_IMPLEMENTED;
	}


	ValueAsMetadata* ValueAsMetadata::Get(Value* val)
	{
		DILITHIUM_UNUSED(val);
		DILITHIUM_NOT_IMPLEMENTED;
	}

	void ValueAsMetadata::HandleDeletion(Value* val)
	{
		DILITHIUM_UNUSED(val);
		DILITHIUM_NOT_IMPLEMENTED;
	}

	void ValueAsMetadata::HandleRAUW(Value* from, Value* to)
	{
		DILITHIUM_UNUSED(from);
		DILITHIUM_UNUSED(to);
		DILITHIUM_NOT_IMPLEMENTED;
	}


	MDString* MDString::Get(LLVMContext& context, std::string_view sv)
	{
		DILITHIUM_UNUSED(context);
		DILITHIUM_UNUSED(sv);
		DILITHIUM_NOT_IMPLEMENTED;
	}


	void TempMDNodeDeleter::operator()(MDNode* node) const
	{
		DILITHIUM_UNUSED(node);
		DILITHIUM_NOT_IMPLEMENTED;
	}


	MDTuple* MDNode::Get(LLVMContext& context, ArrayRef<Metadata*> mds)
	{
		DILITHIUM_UNUSED(context);
		DILITHIUM_UNUSED(mds);
		DILITHIUM_NOT_IMPLEMENTED;
	}

	MDTuple* MDNode::GetIfExists(LLVMContext& context, ArrayRef<Metadata*> mds)
	{
		DILITHIUM_UNUSED(context);
		DILITHIUM_UNUSED(mds);
		DILITHIUM_NOT_IMPLEMENTED;
	}

	MDTuple* MDNode::GetDistinct(LLVMContext& context, ArrayRef<Metadata*> mds)
	{
		DILITHIUM_UNUSED(context);
		DILITHIUM_UNUSED(mds);
		DILITHIUM_NOT_IMPLEMENTED;
	}

	void MDNode::ReplaceAllUsesWith(Metadata* md)
	{
		DILITHIUM_UNUSED(md);
		DILITHIUM_NOT_IMPLEMENTED;
	}


	void NamedMDNode::AddOperand(MDNode* mn)
	{
		DILITHIUM_UNUSED(mn);
		DILITHIUM_NOT_IMPLEMENTED;
	}
}
