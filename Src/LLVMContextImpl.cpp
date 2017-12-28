/**
 * @file LLVMContextImpl.cpp
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

#include "LLVMContextImpl.hpp"
#include <Dilithium/LLVMContext.hpp>
#include <Dilithium/Util.hpp>

namespace Dilithium
{
	MDNode* MDAttachmentMap::Lookup(uint32_t id) const
	{
		for (auto const & att : attachments_)
		{
			if (att.first == id)
			{
				return att.second;
			}
		}
		return nullptr;
	}

	void MDAttachmentMap::GetAll(boost::container::small_vector_base<std::pair<uint32_t, MDNode*>>& result) const
	{
		result.insert(result.end(), attachments_.begin(), attachments_.end());

		// Sort the resulting array so it is stable.
		if (result.size() > 1)
		{
			std::sort(result.begin(), result.end());
		}
	}


	LLVMContextImpl::LLVMContextImpl(LLVMContext& context)
		: the_true_val(nullptr), the_false_val(nullptr),
			void_ty(context, Type::TID_Void),
			label_ty(context, Type::TID_Label),
			half_ty(context, Type::TID_Half),
			float_ty(context, Type::TID_Float),
			double_ty(context, Type::TID_Double),
			metadata_ty(context, Type::TID_Metadata),
			int1_ty(context, 1),
			int8_ty(context, 8),
			int16_ty(context, 16),
			int32_ty(context, 32),
			int64_ty(context, 64)
	{
		named_struct_types_unique_id = 0;
	}

	LLVMContextImpl::~LLVMContextImpl()
	{
		for (auto& v : int_constants)
		{
			delete v.second;
		}
		int_constants.clear();

		attrs_set.clear();
		attrs_lists.clear();
		attrs_set_nodes.clear();
	}
}
