/**
 * @file Function.hpp
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

#ifndef _DILITHIUM_FUNCTION_HPP
#define _DILITHIUM_FUNCTION_HPP

#pragma once

#include <Dilithium/CXX17/string_view.hpp>
#include <Dilithium/Argument.hpp>
#include <Dilithium/Attributes.hpp>
#include <Dilithium/BasicBlock.hpp>
#include <Dilithium/CallingConv.hpp>
#include <Dilithium/GlobalObject.hpp>
#include <Dilithium/ValueSymbolTable.hpp>

#include <list>
#include <memory>

namespace Dilithium
{
	class FunctionType;
	class LLVMModule;

	class Function : public GlobalObject
	{
		template <typename NodeType>
		friend void AddToSymbolTableList(NodeType*, typename NodeType::ParentType*);
		template <typename NodeType>
		friend void RemoveFromSymbolTableList(NodeType*);

	public:
		typedef std::list<std::unique_ptr<Argument>> ArgumentListType;
		typedef ArgumentListType::iterator arg_iterator;
		typedef ArgumentListType::const_iterator const_arg_iterator;

		typedef std::list<std::unique_ptr<BasicBlock>> BasicBlockListType;
		typedef BasicBlockListType::iterator iterator;
		typedef BasicBlockListType::const_iterator const_iterator;

	public:
		static Function* Create(FunctionType* ty, LinkageTypes linkage, std::string_view name = "", LLVMModule* mod = nullptr);

		bool HasPersonalityFn() const;
		Constant* PersonalityFn() const;
		void PersonalityFn(Constant* c);

		bool IsVarArg() const;

		bool IsMaterializable() const;
		void IsMaterializable(bool m);

		CallingConv::ID GetCallingConv() const;
		void SetCallingConv(CallingConv::ID cc);

		AttributeSet GetAttributes() const
		{
			return attr_sets_;
		}
		void SetAttributes(AttributeSet const & attrs)
		{
			attr_sets_ = attrs;
		}

		ValueSymbolTable const * GetValueSymbolTable() const
		{
			return &sym_tab_;
		}
		ValueSymbolTable* GetValueSymbolTable()
		{
			return &sym_tab_;
		}
		arg_iterator ArgBegin();
		const_arg_iterator ArgBegin() const;
		arg_iterator ArgEnd();
		const_arg_iterator ArgEnd() const;

		bool HasPrefixData() const;
		Constant* PrefixData() const;
		void PrefixData(Constant* prefix_data);

		bool HasPrologueData() const;
		Constant* PrologueData() const;
		void PrologueData(Constant* prologue_data);

		static bool classof(Value const * v)
		{
			return v->GetValueId() == Value::FunctionVal;
		}

	private:
		void Parent(LLVMModule* parent);

	private:
		ValueSymbolTable sym_tab_;
		AttributeSet attr_sets_;

		// DILITHIUM_NOT_IMPLEMENTED
	};
}

#endif		// _DILITHIUM_FUNCTION_HPP
