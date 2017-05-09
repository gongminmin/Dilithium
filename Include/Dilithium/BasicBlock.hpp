/**
 * @file BasicBlock.hpp
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

#ifndef _DILITHIUM_BASIC_BLOCK_HPP
#define _DILITHIUM_BASIC_BLOCK_HPP

#pragma once

#include <Dilithium/Instruction.hpp>
#include <Dilithium/Value.hpp>

#include <list>
#include <memory>

namespace Dilithium
{
	class Function;
	class ValueSymbolTable;

	class BasicBlock : public Value
	{
	public:
		typedef std::list<std::unique_ptr<Instruction>> InstListType;
		typedef InstListType::iterator iterator;
		typedef InstListType::const_iterator const_iterator;
		typedef InstListType::reverse_iterator reverse_iterator;
		typedef InstListType::const_reverse_iterator const_reverse_iterator;

	public:
		~BasicBlock() override;

		static BasicBlock* Create(LLVMContext& context, std::string_view name, Function* parent);

		Function const * Parent() const
		{
			return parent_;
		}
		Function* Parent()
		{
			return parent_;
		}

		iterator begin()
		{
			return inst_list_.begin();
		}
		const_iterator begin() const
		{
			return inst_list_.begin();
		}
		iterator end()
		{
			return inst_list_.end();
		}
		const_iterator end() const
		{
			return inst_list_.end();
		}

		reverse_iterator rbegin()
		{
			return inst_list_.rbegin();
		}
		const_reverse_iterator rbegin() const
		{
			return inst_list_.rbegin();
		}
		reverse_iterator rend()
		{
			return inst_list_.rend();
		}
		const_reverse_iterator rend() const
		{
			return inst_list_.rend();
		}

		size_t size() const
		{
			return inst_list_.size();
		}
		bool empty() const
		{
			return inst_list_.empty();
		}
		Instruction const & front() const
		{
			return *inst_list_.front();
		}
		Instruction& front()
		{
			return *inst_list_.front();
		}
		Instruction const & back() const
		{
			return *inst_list_.back();
		}
		Instruction& back()
		{
			return *inst_list_.back();
		}

		InstListType const & InstList() const
		{
			return inst_list_;
		}
		InstListType& InstList()
		{
			return inst_list_;
		}

		ValueSymbolTable* GetValueSymbolTable();

		void ReplaceSuccessorsPhiUsesWith(BasicBlock* new_bb);

		static bool classof(Value const * v)
		{
			return v->GetValueId() == Value::BasicBlockVal;
		}

	private:
		InstListType inst_list_;
		Function* parent_;

		// DILITHIUM_NOT_IMPLEMENTED
	};
}

#endif		// _DILITHIUM_BASIC_BLOCK_HPP
