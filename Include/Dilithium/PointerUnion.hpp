/**
 * @file PointerUnion.hpp
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

#ifndef _DILITHIUM_POINTER_UNION_HPP
#define _DILITHIUM_POINTER_UNION_HPP

#pragma once

#include <boost/assert.hpp>

namespace Dilithium
{
	// From LLVM

	template <typename T>
	struct PointerUnionTypeSelectorReturn
	{
		typedef T Return;
	};

	template <typename T1, typename T2, typename RetEQ, typename RetNE>
	struct PointerUnionTypeSelector
	{
		typedef typename PointerUnionTypeSelectorReturn<RetNE>::Return Return;
	};

	template <typename T, typename RetEQ, typename RetNE>
	struct PointerUnionTypeSelector<T, T, RetEQ, RetNE>
	{
		typedef typename PointerUnionTypeSelectorReturn<RetEQ>::Return Return;
	};

	template <typename T1, typename T2, typename RetEQ, typename RetNE>
	struct PointerUnionTypeSelectorReturn<PointerUnionTypeSelector<T1, T2, RetEQ, RetNE>>
	{
		typedef typename PointerUnionTypeSelector<T1, T2, RetEQ, RetNE>::Return Return;
	};

	// PointerUnion - This implements a discriminated union of two pointer types,
	// and keeps the discriminator bit-mangled into the low bits of the pointer.
	// This allows the implementation to be extremely efficient in space, but
	// permits a very natural and type-safe API.
	//
	// Common use patterns would be something like this:
	//    PointerUnion<int*, float*> P;
	//    P = (int*)0;
	//    printf("%d %d", P.is<int*>(), P.is<float*>());  // prints "1 0"
	//    X = P.get<int*>();     // ok.
	//    Y = P.get<float*>();   // runtime assertion failure.
	//    Z = P.get<double*>();  // compile time failure.
	//    P = (float*)0;
	//    Y = P.get<float*>();   // ok.
	//    X = P.get<int*>();     // runtime assertion failure.
	template <typename PT1, typename PT2>
	class PointerUnion
	{
	private:
		// Note: In LLVM, those two variables are packed into a pointer
		void* ptr_;
		bool is_pt2_;

		struct IsPT1
		{
			static int constexpr Num = 0;
		};
		struct IsPT2
		{
			static int constexpr Num = 1;
		};
		template <typename T>
		struct UNION_DOESNT_CONTAIN_TYPE
		{
		};

	public:
		PointerUnion()
		{
		}

		// TODO: Try to make it explicit
		PointerUnion(PT1 V)
			: ptr_(V), is_pt2_(false)
		{
		}
		PointerUnion(PT2 V)
			: ptr_(V), is_pt2_(true)
		{
		}

		bool IsNull() const
		{
			return !ptr_;
		}
		explicit operator bool() const
		{
			return !this->IsNull();
		}

		template <typename T>
		int Is() const
		{
			typedef typename PointerUnionTypeSelector<PT1, T, IsPT1,
				PointerUnionTypeSelector<PT2, T, IsPT2, UNION_DOESNT_CONTAIN_TYPE<T>>>::Return Ty;
			return static_cast<int>(is_pt2_) == Ty::Num;
		}

		template <typename T>
		T Get() const
		{
			BOOST_ASSERT_MSG(this->Is<T>(), "Invalid accessor called");
			return static_cast<T>(const_cast<void*>(ptr_));
		}

		void Assign(std::nullptr_t)
		{
			ptr_ = nullptr;
			is_pt2_ = false;
		}
		void Assign(PT1 rhs)
		{
			ptr_ = rhs;
			is_pt2_ = false;
		}
		void Assign(PT2 rhs)
		{
			ptr_ = rhs;
			is_pt2_ = true;
		}

		void* OpaqueValue() const
		{
			return ptr_;
		}
	};

	template <typename PT1, typename PT2>
	static bool operator==(PointerUnion<PT1, PT2> const & lhs, PointerUnion<PT1, PT2> const & rhs)
	{
		return lhs.OpaqueValue() == rhs.OpaqueValue();
	}

	template <typename PT1, typename PT2>
	static bool operator!=(PointerUnion<PT1, PT2> const & lhs, PointerUnion<PT1, PT2> const & rhs)
	{
		return lhs.OpaqueValue() != rhs.OpaqueValue();
	}

	template<typename PT1, typename PT2>
	static bool operator<(PointerUnion<PT1, PT2> const & lhs, PointerUnion<PT1, PT2> const & rhs)
	{
		return lhs.OpaqueValue() < rhs.OpaqueValue();
	}
}

#endif		// _DILITHIUM_POINTER_UNION_HPP
