/**
 * @file LLVMModule.cpp
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
#include <Dilithium/LLVMModule.hpp>

#include <Dilithium/GVMaterializer.hpp>
#include <Dilithium/LLVMContext.hpp>

#include <Dilithium/dxc/HLSL/DxilModule.hpp>

namespace Dilithium
{
	LLVMModule::LLVMModule(std::string const & name, std::shared_ptr<LLVMContext> const & context)
		: context_(context), name_(name), data_layout_("")
	{
	}

	LLVMModule::~LLVMModule()
	{
		//this->ResetDxilModule();
		this->DropAllReferences();
		function_list_.clear();
		named_md_list_.clear();

		//DILITHIUM_NOT_IMPLEMENTED;
	}

	void LLVMModule::SetDataLayout(std::string_view desc)
	{
		data_layout_.Reset(std::string(desc));
	}

	void LLVMModule::SetDataLayout(DataLayout const & dl)
	{
		data_layout_ = dl;
	}

	uint32_t LLVMModule::MdKindId(std::string_view name) const
	{
		return context_->MdKindId(name);
	}

	void LLVMModule::MdKindNames(boost::container::small_vector_base<std::string_view>& result) const
	{
		return context_->MdKindNames(result);
	}

	NamedMDNode* LLVMModule::GetNamedMetadata(std::string_view name) const
	{
		auto iter = named_md_sym_tab_.find(std::string(name));
		if (iter != named_md_sym_tab_.end())
		{
			return iter->second;
		}
		else
		{
			return nullptr;
		}
	}

	NamedMDNode* LLVMModule::GetOrInsertNamedMetadata(std::string_view name)
	{
		auto& nmd_ptr = named_md_sym_tab_[std::string(name)];
		if (!nmd_ptr)
		{
			auto nmd = std::make_unique<NamedMDNode>(name);
			nmd->Parent(this);
			nmd_ptr = nmd.get();
			named_md_list_.push_back(std::move(nmd));
		}
		return nmd_ptr;
	}

	void LLVMModule::Materializer(std::shared_ptr<GVMaterializer> const & gvm)
	{
		materializer_ = gvm;
	}

	void LLVMModule::MaterializeAllPermanently()
	{
		if (materializer_)
		{
			materializer_->MaterializeModule(this);
			materializer_.reset();
		}
	}

	void LLVMModule::DropAllReferences()
	{
		for (auto& func : *this)
		{
			func->DropAllReferences();
		}
	}

	bool LLVMModule::HasDxilModule() const
	{
		return dxil_module_.get() != nullptr;
	}

	void LLVMModule::SetDxilModule(std::unique_ptr<DxilModule> value)
	{
		dxil_module_ = std::move(value);
	}

	DxilModule& LLVMModule::GetDxilModule()
	{
		return *dxil_module_;
	}

	DxilModule& LLVMModule::GetOrCreateDxilModule(bool skip_init)
	{
		if (!this->HasDxilModule())
		{
			auto mod = std::make_unique<DxilModule>(this);
			if (!skip_init)
			{
				mod->LoadDxilMetadata();
			}
			this->SetDxilModule(std::move(mod));
		}
		return this->GetDxilModule();
	}

	void LLVMModule::ResetDxilModule()
	{
		if (this->HasDxilModule())
		{
			dxil_module_.reset();
		}
	}
}
