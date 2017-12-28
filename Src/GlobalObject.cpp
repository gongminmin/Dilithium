/**
 * @file GlobalObject.cpp
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
#include <Dilithium/GlobalObject.hpp>
#include <Dilithium/DerivedType.hpp>

namespace Dilithium 
{
	GlobalObject::GlobalObject(PointerType* ty, ValueTy vty, uint32_t num_ops, uint32_t num_uses, LinkageTypes linkage,
		std::string_view name)
		: GlobalValue(ty, vty, num_ops, num_uses, linkage, name)
	{
		this->GlobalValueSubClassData(0);
	}

	uint32_t GlobalObject::GetAlignment() const
	{
		uint32_t data = this->GlobalValueSubClassData();
		uint32_t alignment_data = data & ALIGNMENT_MASK;
		return (1U << alignment_data) >> 1;
	}

	void GlobalObject::SetAlignment(uint32_t align)
	{
		BOOST_ASSERT_MSG((align & (align - 1)) == 0, "Alignment is not a power of 2!");
		BOOST_ASSERT_MSG(align <= MAX_ALIGNMENT, "Alignment is greater than MAX_ALIGNMENT!");
		uint32_t alignment_data = Log2_32(align) + 1;
		uint32_t old_data = this->GlobalValueSubClassData();
		this->GlobalValueSubClassData((old_data & ~ALIGNMENT_MASK) | alignment_data);
		BOOST_ASSERT_MSG(this->GetAlignment() == align, "Alignment representation error!");
	}

	uint32_t GlobalObject::GlobalObjectSubClassData() const
	{
		uint32_t value_data = this->GlobalValueSubClassData();
		return value_data >> ALIGNMENT_BITS;
	}

	void GlobalObject::GlobalObjectSubClassData(uint32_t val)
	{
		uint32_t old_data = this->GlobalValueSubClassData();
		this->GlobalValueSubClassData((old_data & ALIGNMENT_MASK) | (val << ALIGNMENT_BITS));
		BOOST_ASSERT_MSG(this->GlobalObjectSubClassData() == val, "Representation error");
	}

	void GlobalObject::SetSection(std::string_view sec)
	{
		section_ = std::string(sec);
	}
}
