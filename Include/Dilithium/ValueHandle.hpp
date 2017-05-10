/**
 * @file ValueHandle.hpp
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

#ifndef _DILITHIUM_VALUE_HANDLE_HPP
#define _DILITHIUM_VALUE_HANDLE_HPP

#pragma once

#include <Dilithium/Value.hpp>

#include <boost/core/noncopyable.hpp>

namespace Dilithium
{
	template<typename From>
	struct simplify_type;

	template <typename T>
	inline T* TombstonePointer()
	{
		uintptr_t val = static_cast<uintptr_t>(-2);
		return reinterpret_cast<T*>(val);
	}

	class ValueHandleBase
	{
		friend class Value;

		ValueHandleBase(ValueHandleBase const &) = delete;

	protected:
		enum HandleBaseKind
		{
			Assert,
			Callback,
			Tracking,
			Weak
		};

	public:
		explicit ValueHandleBase(HandleBaseKind kind);
		ValueHandleBase(HandleBaseKind kind, Value* val);
		ValueHandleBase(HandleBaseKind kind, ValueHandleBase const & rhs);
		~ValueHandleBase();

		void Assign(Value* val);

		ValueHandleBase& operator=(ValueHandleBase const & rhs);

		Value* operator->() const
		{
			return val_;
		}
		Value& operator*() const
		{
			return *val_;
		}

		// Callbacks made from Value.
		static void ValueIsDeleted(Value* val);
		static void ValueIsRAUWd(Value* old_val, Value* new_val);

	protected:
		Value* ValPtr() const
		{
			return val_;
		}

		static bool IsValid(Value* val)
		{
			return val && (val != TombstonePointer<Value>());
		}

	private:
		void AddToExistingUseList(ValueHandleBase** list);
		void AddToExistingUseListAfter(ValueHandleBase* node);

		void AddToUseList();
		void RemoveFromUseList();

	private:
		HandleBaseKind kind_;
		ValueHandleBase** prev_;
		ValueHandleBase* next_;

		Value* val_;

		// DILITHIUM_NOT_IMPLEMENTED
	};

	class WeakVH : public ValueHandleBase
	{
	public:
		WeakVH()
			: ValueHandleBase(Weak)
		{
		}
		explicit WeakVH(Value* val)
			: ValueHandleBase(Weak, val)
		{
		}
		WeakVH(WeakVH const & rhs)
			: ValueHandleBase(Weak, rhs)
		{
		}

		// TODO: Make this explicit
		operator Value*() const
		{
			return this->ValPtr();
		}
	};

	template <>
	struct simplify_type<WeakVH>
	{
		typedef Value* SimpleType;
		static SimpleType SimplifiedValue(WeakVH& wvh)
		{
			return wvh;
		}
	};
	template <>
	struct simplify_type<WeakVH const>
	{
		typedef Value /*const*/ * SimpleType;
		static SimpleType SimplifiedValue(WeakVH const & wvh)
		{
			return wvh;
		}
	};
}

#endif		// _DILITHIUM_VALUE_HANDLE_HPP
