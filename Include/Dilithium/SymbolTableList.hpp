/**
 * @file SymbolTableList.hpp
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

#ifndef _DILITHIUM_SYMBOL_TABLE_LIST_HPP
#define _DILITHIUM_SYMBOL_TABLE_LIST_HPP

namespace Dilithium
{
	template <typename NodeType>
	inline void AddToSymbolTableList(NodeType* ptr, typename NodeType::ParentType* parent)
	{
		if (parent)
		{
			ptr->Parent(parent);
			if (ptr->HasName())
			{
				auto symbol_tab = parent->GetValueSymbolTable();
				if (symbol_tab)
				{
					symbol_tab->ReinsertValue(ptr);
				}
			}
		}
	}

	template <typename NodeType>
	inline void RemoveFromSymbolTableList(NodeType* ptr)
	{
		auto parent = ptr->Parent();
		if (parent)
		{
			if (ptr->HasName())
			{
				auto symbol_tab = parent->GetValueSymbolTable();
				if (symbol_tab)
				{
					symbol_tab->RemoveValueName(ptr->NameHash());
				}
			}
			ptr->Parent(nullptr);
		}
	}
}

#endif		// _DILITHIUM_SYMBOL_TABLE_LIST_HPP
