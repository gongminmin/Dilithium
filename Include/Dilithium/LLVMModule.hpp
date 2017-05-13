/**
 * @file LLVMModule.hpp
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

#ifndef _DILITHIUM_LLVM_MODULE_HPP
#define _DILITHIUM_LLVM_MODULE_HPP

#pragma once

#include <Dilithium/CXX17/string_view.hpp>
#include <Dilithium/DataLayout.hpp>
#include <Dilithium/Function.hpp>
#include <Dilithium/ValueSymbolTable.hpp>

#include <list>
#include <memory>
#include <string>

#include <boost/core/noncopyable.hpp>
#include <boost/range/iterator_range.hpp>

namespace Dilithium
{
	class GVMaterializer;
	class LLVMContext;
	class NamedMDNode;

	class LLVMModule : boost::noncopyable
	{
	public:
		typedef std::list<std::unique_ptr<Function>> FunctionListType;

		typedef FunctionListType::iterator                           iterator;
		typedef FunctionListType::const_iterator               const_iterator;

		typedef FunctionListType::reverse_iterator             reverse_iterator;
		typedef FunctionListType::const_reverse_iterator const_reverse_iterator;

	public:
		LLVMModule(std::string const & name, std::shared_ptr<LLVMContext> const & context);
		~LLVMModule();

		void SetDataLayout(std::string_view desc);
		void SetDataLayout(DataLayout const & dl);

		void SetTargetTriple(std::string_view sv)
		{
			target_triple_ = sv.to_string();
		}

		uint32_t MDKindID(std::string_view name) const;

		NamedMDNode* GetOrInsertNamedMetadata(std::string_view name);

		void Materializer(std::shared_ptr<GVMaterializer> const & gvm);
		void MaterializeAllPermanently();

		ValueSymbolTable const * GetValueSymbolTable() const
		{
			return &val_sym_tab_;
		}
		ValueSymbolTable* GetValueSymbolTable()
		{
			return &val_sym_tab_;
		}

		iterator begin()
		{
			return function_list_.begin();
		}
		const_iterator begin() const
		{
			return function_list_.begin();
		}
		iterator end()
		{
			return function_list_.end();
		}
		const_iterator end() const
		{
			return function_list_.end();
		}
		reverse_iterator rbegin()
		{
			return function_list_.rbegin();
		}
		const_reverse_iterator rbegin() const
		{
			return function_list_.rbegin();
		}
		reverse_iterator rend()
		{
			return function_list_.rend();
		}
		const_reverse_iterator rend() const
		{
			return function_list_.rend();
		}
		size_t size() const
		{
			return function_list_.size();
		}
		bool empty() const
		{
			return function_list_.empty();
		}

		boost::iterator_range<iterator> Functions()
		{
			return boost::iterator_range<iterator>(begin(), end());
		}
		boost::iterator_range<const_iterator> Functions() const
		{
			return boost::iterator_range<const_iterator>(begin(), end());
		}

	private:
		std::shared_ptr<LLVMContext> context_;
		FunctionListType function_list_;
		ValueSymbolTable val_sym_tab_;
		std::string name_;
		std::shared_ptr<GVMaterializer> materializer_;
		std::string target_triple_;
		DataLayout data_layout_;
	};
}

#endif		// _DILITHIUM_LLVM_MODULE_HPP
