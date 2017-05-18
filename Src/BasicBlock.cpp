/**
 * @file BasicBlock.cpp
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
#include <Dilithium/BasicBlock.hpp>
#include <Dilithium/Type.hpp>
#include <Dilithium/SymbolTableList.hpp>
#include <Dilithium/ValueSymbolTable.hpp>

namespace Dilithium 
{
	BasicBlock::BasicBlock(LLVMContext& context, std::string_view name, Function* new_parent)
		: Value(Type::LabelType(context), Value::BasicBlockVal), parent_(nullptr)
	{
		new_parent->BasicBlockList().push_back(std::unique_ptr<BasicBlock>(this));
		AddToSymbolTableList(this, new_parent);

		this->Name(name);
	}

	BasicBlock::~BasicBlock()
	{
		RemoveFromSymbolTableList(this);

		// If the address of the block is taken and it is being deleted (e.g. because
		// it is dead), this means that there is either a dangling constant expr
		// hanging off the block, or an undefined use of the block (source code
		// expecting the address of a label to keep the block alive even though there
		// is no indirect branch).  Handle these cases by zapping the BlockAddress
		// nodes.  There are no other possible uses at this point.
		if (this->HasAddressTaken())
		{
			DILITHIUM_NOT_IMPLEMENTED;
		}

		BOOST_ASSERT_MSG(this->Parent() == nullptr, "BasicBlock still linked into the program!");
		this->DropAllReferences();
		inst_list_.clear();
	}

	BasicBlock* BasicBlock::Create(LLVMContext& context, std::string_view name, Function* parent)
	{
		return new BasicBlock(context, name, parent);
	}

	ValueSymbolTable* BasicBlock::GetValueSymbolTable()
	{
		Function* func = this->Parent();
		if (func)
		{
			return func->GetValueSymbolTable();
		}
		else
		{
			return nullptr;
		}
	}

	void BasicBlock::DropAllReferences()
	{
		for (auto iter = begin(), end_iter = end(); iter != end_iter; ++ iter)
		{
			(*iter)->DropAllReferences();
		}
	}

	void BasicBlock::ReplaceSuccessorsPhiUsesWith(BasicBlock* new_bb)
	{
		DILITHIUM_UNUSED(new_bb);
		DILITHIUM_NOT_IMPLEMENTED;
	}

	void BasicBlock::Parent(Function* new_parent)
	{
		ValueSymbolTable* old_st = parent_ ? parent_->GetValueSymbolTable() : nullptr;

		parent_ = new_parent;

		ValueSymbolTable* new_st = new_parent ? new_parent->GetValueSymbolTable() : nullptr;

		if ((old_st != new_st) && !inst_list_.empty())
		{
			if (old_st)
			{
				for (auto iter = inst_list_.begin(); iter != inst_list_.end(); ++ iter)
				{
					if ((*iter)->HasName())
					{
						old_st->RemoveValueName((*iter)->NameHash());
					}
				}
			}

			if (new_st)
			{
				for (auto iter = inst_list_.begin(); iter != inst_list_.end(); ++ iter)
				{
					if ((*iter)->HasName())
					{
						new_st->ReinsertValue(iter->get());
					}
				}
			}
		}
	}
}
