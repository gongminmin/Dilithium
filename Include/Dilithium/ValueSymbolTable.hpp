/**
 * @file ValueSymbolTable.hpp
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

#ifndef _DILITHIUM_VALUE_SYMBOL_TABLE_HPP
#define _DILITHIUM_VALUE_SYMBOL_TABLE_HPP

#pragma once

#include <Dilithium/CXX17/string_view.hpp>
#include <Dilithium/Value.hpp>

#include <unordered_map>

#include <boost/core/noncopyable.hpp>

namespace Dilithium
{
	class ValueSymbolTable
	{
		friend class Value;
		friend class BasicBlock;
		friend class Function;
		template <typename NodeType>
		friend void AddToSymbolTableList(NodeType*, typename NodeType::ParentType*);
		template <typename NodeType>
		friend void RemoveFromSymbolTableList(NodeType*);

	public:
		typedef std::unordered_map<uint64_t, Value*> ValueMap;
		typedef ValueMap::iterator iterator;
		typedef ValueMap::const_iterator const_iterator;

	public:
		ValueSymbolTable()
			: vmap_(0), last_unique_(0)
		{
		}
		~ValueSymbolTable();

		bool empty() const
		{
			return vmap_.empty();
		}
		size_t size() const
		{
			return vmap_.size();
		}

		iterator begin()
		{
			return vmap_.begin();
		}
		const_iterator begin() const
		{
			return vmap_.begin();
		}
		iterator end()
		{
			return vmap_.end();
		}
		const_iterator end() const
		{
			return vmap_.end();
		}

	private:
		void ReinsertValue(Value* val);
		std::string CreateValueName(std::string_view name, Value* val);
		void RemoveValueName(uint64_t name_hash);

	private:
		ValueMap vmap_;
		mutable uint32_t last_unique_;
	};
}

#endif		// _DILITHIUM_VALUE_SYMBOL_TABLE_HPP
