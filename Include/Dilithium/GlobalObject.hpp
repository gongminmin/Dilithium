/**
 * @file GlobalObject.hpp
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

#ifndef _DILITHIUM_GLOBAL_OBJECT_HPP
#define _DILITHIUM_GLOBAL_OBJECT_HPP

#pragma once

#include <Dilithium/GlobalValue.hpp>

namespace Dilithium
{
	class PointerType;

	class GlobalObject : public GlobalValue
	{
	public:
		uint32_t Alignment() const;
		void Alignment(uint32_t align);

		uint32_t GlobalObjectSubClassData() const;
		void GlobalObjectSubClassData(uint32_t val);

		void Section(std::string_view sec);

	protected:
		GlobalObject(PointerType* ty, ValueTy vty, uint32_t num_ops, uint32_t num_uses, LinkageTypes linkage, std::string_view name);

	protected:
		static uint32_t constexpr ALIGNMENT_BITS = 5;

	private:
		static uint32_t constexpr ALIGNMENT_MASK = (1U << ALIGNMENT_BITS) - 1;

		// DILITHIUM_NOT_IMPLEMENTED
	};
}

#endif		// _DILITHIUM_GLOBAL_OBJECT_HPP
