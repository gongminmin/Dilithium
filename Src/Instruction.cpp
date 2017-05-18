/**
 * @file Instruction.cpp
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
#include <Dilithium/Instruction.hpp>
#include <Dilithium/LLVMContext.hpp>
#include <Dilithium/Type.hpp>
#include <Dilithium/SymbolTableList.hpp>
#include <Dilithium/Value.hpp>
#include "LLVMContextImpl.hpp"

namespace
{
	template <typename T>
	class UniquePtrEqual
	{
	public:
		explicit UniquePtrEqual(T* ptr)
			: ptr_(ptr)
		{
		}

		bool operator()(std::unique_ptr<T> const & rhs)
		{
			return ptr_ == rhs.get();
		}

	private:
		T* ptr_;
	};
}

namespace Dilithium 
{
	Instruction::Instruction(Type* ty, uint32_t type, uint32_t num_ops, uint32_t num_uses, Instruction* insert_before)
		: User(ty, Value::InstructionVal + type, num_ops, num_uses),
			parent_(nullptr)
	{
		if (insert_before)
		{
			auto bb = insert_before->Parent();
			BOOST_ASSERT_MSG(bb, "Instruction to insert before is not in a basic block!");
			auto iter = std::find_if(bb->InstList().begin(), bb->InstList().end(), UniquePtrEqual<Instruction>(insert_before));
			-- iter;
			bb->InstList().insert(iter, std::unique_ptr<Instruction>(this));
			AddToSymbolTableList(this, bb);
		}
	}

	Instruction::Instruction(Type* ty, uint32_t type, uint32_t num_ops, uint32_t num_uses, BasicBlock* insert_at_end)
		: User(ty, Value::InstructionVal + type, num_ops, num_uses),
			parent_(nullptr)
	{
		BOOST_ASSERT_MSG(insert_at_end, "Basic block to append to may not be NULL!");
		insert_at_end->InstList().push_back(std::unique_ptr<Instruction>(this));
		AddToSymbolTableList(this, insert_at_end);
	}

	Instruction::~Instruction()
	{
		RemoveFromSymbolTableList(this);

		BOOST_ASSERT_MSG(!parent_, "Instruction still linked in the program!");
		if (this->HasMetadataHashEntry())
		{
			this->ClearMetadataHashEntries();
		}
	}

	void Instruction::InstructionSubclassData(uint16_t d)
	{
		BOOST_ASSERT_MSG((d & HasMetadataBit) == 0, "Out of range value put into field");
		this->SetValueSubclassData((this->GetSubclassDataFromValue() & HasMetadataBit) | d);
	}

	void Instruction::Parent(BasicBlock* parent)
	{
		parent_ = parent;
	}

	void Instruction::ClearMetadataHashEntries()
	{
		BOOST_ASSERT_MSG(this->HasMetadataHashEntry(), "Caller should check");
		this->Context().Impl().instruction_metadata.erase(this);
		this->HasMetadataHashEntry(false);
	}
}
