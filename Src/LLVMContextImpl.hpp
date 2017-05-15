/**
 * @file LLVMContextImpl.hpp
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

#ifndef _DILITHIUM_LLVM_CONTEXT_IMPL_HPP
#define _DILITHIUM_LLVM_CONTEXT_IMPL_HPP

#include <Dilithium/Constants.hpp>
#include <Dilithium/DerivedType.hpp>
#include <Dilithium/TrackingMDRef.hpp>
#include "AttributeImpl.hpp"

#include <memory>
#include <unordered_map>
#include <unordered_set>

#include <boost/container/small_vector.hpp>


namespace Dilithium
{
	class ConstantInt;
	class ConstantFP;
	class LLVMContext;
	class Type;
	class Value;
	class ValueHandleBase;

	class MDAttachmentMap
	{
	public:
		bool empty() const
		{ 
			return attachments_.empty();
		}
		size_t size() const
		{
			return attachments_.size();
		}

		MDNode* Lookup(uint32_t id) const;

		void Set(uint32_t id, MDNode& md);

		void Erase(uint32_t id);

		void GetAll(boost::container::small_vector_base<std::pair<uint32_t, MDNode*>>& result) const;

		template <typename PredTy>
		void remove_if(PredTy should_remove)
		{
			attachments_.erase(std::remove_if(attachments_.begin(), attachments_.end(), should_remove), attachments_.end());
		}

	private:
		boost::container::small_vector<std::pair<uint32_t, TrackingMDNodeRef>, 2> attachments_;
	};

	// TODO: Consider merged with LLVMContext
	struct LLVMContextImpl
	{
		explicit LLVMContextImpl(LLVMContext& context);
		~LLVMContextImpl();

		std::unordered_map<uint64_t, std::unique_ptr<AttributeImpl>> attrs_set;
		std::unordered_map<uint64_t, std::unique_ptr<AttributeSetImpl>> attrs_lists;
		std::unordered_map<uint64_t, std::unique_ptr<AttributeSetNode>> attrs_set_nodes;

		ConstantInt* the_true_val;
		ConstantInt* the_false_val;

		Type void_ty, label_ty, half_ty, float_ty, double_ty, metadata_ty;
		IntegerType int1_ty, int8_ty, int16_ty, int32_ty, int64_ty;

		std::unordered_map<uint32_t, std::unique_ptr<IntegerType>> integer_types;
		std::unordered_map<uint64_t, std::unique_ptr<FunctionType>> function_types;
		std::unordered_map<uint64_t, std::unique_ptr<StructType>> anon_struct_types;
		std::unordered_map<std::string, std::unique_ptr<StructType>> named_struct_types;
		uint32_t named_struct_types_unique_id;

		std::unordered_map<std::pair<Type*, uint64_t>, std::unique_ptr<ArrayType>> array_types;
		std::unordered_map<std::pair<Type*, uint32_t>, std::unique_ptr<VectorType>> vector_types;
		std::unordered_map<Type*, std::unique_ptr<PointerType>> pointer_types;  // Pointers in addrress space = 0
		std::unordered_map<std::pair<Type*, uint32_t>, std::unique_ptr<PointerType>> as_pointer_types;

		// This map keeps track of all of the value handles that are watching a Value*
		std::unordered_map<Value*, ValueHandleBase*> value_handles;

		// Metadata string to ID mapping
		std::unordered_map<std::string, uint32_t> custom_md_kind_names;

		//DILITHIUM_NOT_IMPLEMENTED;
	};
}

#endif		// _DILITHIUM_LLVM_CONTEXT_IMPL_HPP
