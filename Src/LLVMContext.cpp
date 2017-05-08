/**
 * @file LLVMContext.cpp
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

#include <Dilithium/LLVMContext.hpp>
#include <Dilithium/Util.hpp>
#include "LLVMContextImpl.hpp"

#include <boost/assert.hpp>

namespace Dilithium
{
	LLVMContext::LLVMContext()
		: impl_(std::make_unique<LLVMContextImpl>(*this))
	{
		// Create the fixed metadata kinds. This is done in the same order as the
		// MD_* enum values so that they correspond.

		uint32_t dbg_id = this->MdKindId("dbg");
		BOOST_ASSERT_MSG(dbg_id == MD_Dbg, "dbg kind id drifted");
		DILITHIUM_UNUSED(dbg_id);

		uint32_t tbaa_id = this->MdKindId("tbaa");
		BOOST_ASSERT_MSG(tbaa_id == MD_Tbaa, "tbaa kind id drifted");
		DILITHIUM_UNUSED(tbaa_id);

		uint32_t prof_id = this->MdKindId("prof");
		BOOST_ASSERT_MSG(prof_id == MD_Prof, "prof kind id drifted");
		DILITHIUM_UNUSED(prof_id);

		uint32_t fp_math_id = this->MdKindId("fpmath");
		BOOST_ASSERT_MSG(fp_math_id == MD_FpMath, "fpmath kind id drifted");
		DILITHIUM_UNUSED(fp_math_id);

		uint32_t range_id = this->MdKindId("range");
		BOOST_ASSERT_MSG(range_id == MD_Range, "range kind id drifted");
		DILITHIUM_UNUSED(range_id);

		uint32_t tbaa_struct_id = this->MdKindId("tbaa.struct");
		BOOST_ASSERT_MSG(tbaa_struct_id == MD_TbaaStruct, "tbaa.struct kind id drifted");
		DILITHIUM_UNUSED(tbaa_struct_id);

		uint32_t invariant_load_id = this->MdKindId("invariant.load");
		BOOST_ASSERT_MSG(invariant_load_id == MD_InvariantLoad, "invariant.load kind id drifted");
		DILITHIUM_UNUSED(invariant_load_id);

		uint32_t alias_scope_id = this->MdKindId("alias.scope");
		BOOST_ASSERT_MSG(alias_scope_id == MD_AliasScope, "alias.scope kind id drifted");
		DILITHIUM_UNUSED(alias_scope_id);

		uint32_t no_alias_id = this->MdKindId("noalias");
		BOOST_ASSERT_MSG(no_alias_id == MD_NoAlias, "noalias kind id drifted");
		DILITHIUM_UNUSED(no_alias_id);

		uint32_t non_temporal_id = this->MdKindId("nontemporal");
		BOOST_ASSERT_MSG(non_temporal_id == MD_NonTemporal, "nontemporal kind id drifted");
		DILITHIUM_UNUSED(non_temporal_id);

		uint32_t mem_parallel_loop_access_id = this->MdKindId("llvm.mem.parallel_loop_access");
		BOOST_ASSERT_MSG(mem_parallel_loop_access_id == MD_MemParallelLoopAccess, "mem_parallel_loop_access kind id drifted");
		DILITHIUM_UNUSED(mem_parallel_loop_access_id);

		uint32_t non_null_id = this->MdKindId("nonnull");
		BOOST_ASSERT_MSG(non_null_id == MD_NonNull, "nonnull kind id drifted");
		DILITHIUM_UNUSED(non_null_id);

		uint32_t dereferenceable_id = this->MdKindId("dereferenceable");
		BOOST_ASSERT_MSG(dereferenceable_id == MD_Dereferenceable, "dereferenceable kind id drifted");
		DILITHIUM_UNUSED(dereferenceable_id);

		uint32_t dereferenceable_or_null_id = this->MdKindId("dereferenceable_or_null");
		BOOST_ASSERT_MSG(dereferenceable_or_null_id == MD_DereferenceableOrNull, "dereferenceable_or_null kind id drifted");
		DILITHIUM_UNUSED(dereferenceable_or_null_id);
	}

	LLVMContext::~LLVMContext()
	{
	}

	uint32_t LLVMContext::MdKindId(std::string_view name) const
	{
		return impl_->custom_md_kind_names.emplace(name, static_cast<uint32_t>(impl_->custom_md_kind_names.size())).first->second;
	}

	void LLVMContext::MdKindNames(boost::container::small_vector_base<std::string_view>& names) const
	{
		names.resize(impl_->custom_md_kind_names.size());
		for (auto iter = impl_->custom_md_kind_names.begin(), end_iter = impl_->custom_md_kind_names.end(); iter != end_iter; ++ iter)
		{
			names[iter->second] = iter->first;
		}
	}
}
