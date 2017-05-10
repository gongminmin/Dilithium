/**
 * @file ValueHandle.cpp
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

#include <Dilithium/ValueHandle.hpp>

#include <Dilithium/ErrorHandling.hpp>
#include <Dilithium/Util.hpp>

namespace Dilithium
{
	ValueHandleBase::ValueHandleBase(HandleBaseKind kind)
		: kind_(kind), prev_(nullptr), next_(nullptr), val_(nullptr)
	{
	}

	ValueHandleBase::ValueHandleBase(HandleBaseKind kind, Value* val)
		: kind_(kind), prev_(nullptr), next_(nullptr), val_(val)
	{
		if (this->IsValid(val_))
		{
			this->AddToUseList();
		}
	}

	ValueHandleBase::ValueHandleBase(HandleBaseKind kind, ValueHandleBase const & rhs)
		: kind_(kind), prev_(nullptr), next_(nullptr), val_(rhs.val_)
	{
		if (this->IsValid(val_))
		{
			this->AddToExistingUseList(rhs.prev_);
		}
	}

	ValueHandleBase::~ValueHandleBase()
	{
		if (this->IsValid(val_))
		{
			this->RemoveFromUseList();
		}
	}

	void ValueHandleBase::Assign(Value* val)
	{
		if (val_ == val)
		{
			return;
		}
		if (this->IsValid(val_))
		{
			this->RemoveFromUseList();
		}
		val_ = val;
		if (this->IsValid(val_))
		{
			this->AddToUseList();
		}
	}

	ValueHandleBase& ValueHandleBase::operator=(ValueHandleBase const & rhs)
	{
		if (val_ == rhs.val_)
		{
			return *this;
		}

		if (this->IsValid(val_))
		{
			this->RemoveFromUseList();
		}
		val_ = rhs.val_;
		if (this->IsValid(val_))
		{
			this->AddToExistingUseList(rhs.prev_);
		}
		return *this;
	}

	void ValueHandleBase::ValueIsDeleted(Value* val)
	{
		DILITHIUM_UNUSED(val);
		DILITHIUM_NOT_IMPLEMENTED;
	}

	void ValueHandleBase::ValueIsRAUWd(Value* old_val, Value* new_val)
	{
		DILITHIUM_UNUSED(old_val);
		DILITHIUM_UNUSED(new_val);
		DILITHIUM_NOT_IMPLEMENTED;
	}

	void ValueHandleBase::AddToExistingUseList(ValueHandleBase** list)
	{
		DILITHIUM_UNUSED(list);
		DILITHIUM_NOT_IMPLEMENTED;
	}

	void ValueHandleBase::AddToExistingUseListAfter(ValueHandleBase* node)
	{
		DILITHIUM_UNUSED(node);
		DILITHIUM_NOT_IMPLEMENTED;
	}

	void ValueHandleBase::AddToUseList()
	{
		DILITHIUM_NOT_IMPLEMENTED;
	}

	void ValueHandleBase::RemoveFromUseList()
	{
		DILITHIUM_NOT_IMPLEMENTED;
	}
}
