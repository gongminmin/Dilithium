/**
 * @file ValueSymbolTable.cpp
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
#include <Dilithium/Hashing.hpp>
#include <Dilithium/Type.hpp>
#include <Dilithium/SmallString.hpp>
#include <Dilithium/ValueSymbolTable.hpp>

#include <iostream>

namespace Dilithium
{
	ValueSymbolTable::~ValueSymbolTable()
	{
#ifdef DILITHIUM_DEBUG
		for (auto iter = vmap_.begin(), end = vmap_.end(); iter != end; ++ iter)
		{
			std::clog << "Value still in symbol table! Type = '"
				<< *iter->second->GetType() << "' Name = '"
				<< iter->second->Name() << "'" << std::endl;
		}
		BOOST_ASSERT_MSG(vmap_.empty(), "Values remain in symbol table!");
#endif
	}

	void ValueSymbolTable::ReinsertValue(Value* val)
	{
		BOOST_ASSERT_MSG(val->HasName(), "Can't insert nameless Value into symbol table");

		auto iter = vmap_.find(val->NameHash());
		if (iter == vmap_.end())
		{
			vmap_.emplace(val->NameHash(), val);
		}
		else
		{
			SmallString<256> unique_name(val->Name().begin(), val->Name().end());

			size_t base_size = unique_name.size();
			for (;;)
			{
				unique_name.resize(base_size);
				++ last_unique_;
				unique_name.append('.' + std::to_string(last_unique_));

				uint64_t hash_val = boost::hash_value(unique_name.str());

				iter = vmap_.find(hash_val);
				if (iter == vmap_.end())
				{
					vmap_.emplace(hash_val, val);
					val->Name(unique_name);
					return;
				}
			}
		}
	}

	void ValueSymbolTable::RemoveValueName(uint64_t name_hash)
	{
		BOOST_ASSERT(vmap_.find(name_hash) != vmap_.end());
		vmap_.erase(name_hash);
	}

	std::string ValueSymbolTable::CreateValueName(std::string_view name, Value* val)
	{
		uint64_t hash_val = boost::hash_value(name);

		auto iter = vmap_.find(hash_val);
		if (iter == vmap_.end())
		{
			vmap_.emplace(hash_val, val);
			return std::string(name);
		}
		else
		{
			SmallString<256> unique_name(name.begin(), name.end());
			for (;;)
			{
				unique_name.resize(name.size());
				++ last_unique_;
				unique_name.append(std::to_string(last_unique_));

				hash_val = boost::hash_value(unique_name.str());

				iter = vmap_.find(hash_val);
				if (iter == vmap_.end())
				{
					vmap_.emplace(hash_val, val);
					return std::string(unique_name.str());
				}
			}
		}
	}
}
