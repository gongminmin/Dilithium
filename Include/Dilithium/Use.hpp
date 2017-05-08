/**
 * @file Use.hpp
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

#ifndef _DILITHIUM_USE_HPP
#define _DILITHIUM_USE_HPP

#pragma once

#include <boost/core/noncopyable.hpp>

namespace Dilithium
{
	class User;
	class Value;

	class Use : boost::noncopyable
	{
		friend class Value;

	public:
		Use()
			: val_(nullptr)
		{
		}
		Use(Use&& rhs)
		{
			this->Set(rhs.val_);
		}
		~Use();

		Use& operator=(Use const & rhs);
		Use& operator=(Use&& rhs);

		Value* operator->()
		{
			return val_;
		}
		Value const * operator->() const
		{
			return val_;
		}

		// TODO: Try to make it explicit
		operator Value*() const
		{
			return val_;
		}
		Value* Get() const
		{
			return val_;
		}

		void Set(Value* val);

		User* GetUser() const;

		Use* GetNext() const
		{
			return next_;
		}

		void Swap(Use& rhs);

		static Use* InitTags(Use* beg, Use* end);

	private:
		Use const * GetImpliedUser() const;
		void AddToList(Use** node);
		void RemoveFromList();

	private:
		enum PrevPtrTag
		{
			ZeroDigitTag,
			OneDigitTag,
			StopTag,
			FullStopTag
		};

		Value* val_;
		Use* next_;
		Use** prev_ptr_;
		PrevPtrTag tag_;

		// DILITHIUM_NOT_IMPLEMENTED
	};
}

#endif		// _DILITHIUM_USER_HPP
