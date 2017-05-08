/**
 * @file LLVMContext.hpp
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

#ifndef _DILITHIUM_LLVM_CONTEXT_HPP
#define _DILITHIUM_LLVM_CONTEXT_HPP

#pragma once

#include <Dilithium/CXX17/string_view.hpp>

#include <memory>

#include <boost/container/small_vector.hpp>
#include <boost/core/noncopyable.hpp>

namespace Dilithium
{
	struct LLVMContextImpl;

	class LLVMContext : boost::noncopyable
	{
	public:
		LLVMContext();
		~LLVMContext();

		// Pinned metadata names, which always have the same value.  This is a
		// compile-time performance optimization, not a correctness optimization.
		enum
		{
			MD_Dbg = 0,					// "dbg"
			MD_Tbaa,					// "tbaa"
			MD_Prof,					// "prof"
			MD_FpMath,					// "fpmath"
			MD_Range,					// "range"
			MD_TbaaStruct,				// "tbaa.struct"
			MD_InvariantLoad,			// "invariant.load"
			MD_AliasScope,				// "alias.scope"
			MD_NoAlias,					// "noalias"
			MD_NonTemporal,				// "nontemporal"
			MD_MemParallelLoopAccess,	// "llvm.mem.parallel_loop_access"
			MD_NonNull,					// "nonnull"
			MD_Dereferenceable,			// "dereferenceable"
			MD_DereferenceableOrNull	// "dereferenceable_or_null"
		};

		uint32_t MdKindId(std::string_view name) const;
		void MdKindNames(boost::container::small_vector_base<std::string_view>& result) const;

		LLVMContextImpl& Impl()
		{
			return *impl_;
		}

	private:
		std::unique_ptr<LLVMContextImpl> impl_;
	};
}

#endif		// _DILITHIUM_LLVM_CONTEXT_HPP
