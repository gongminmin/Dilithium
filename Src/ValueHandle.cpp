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

#include <Dilithium/Dilithium.hpp>
#include <Dilithium/LLVMContext.hpp>
#include <Dilithium/ValueHandle.hpp>
#include "LLVMContextImpl.hpp"

#include <iostream>

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
		BOOST_ASSERT_MSG(val->has_value_handle_, "Should only be called if ValueHandles present");

		auto& impl = val->Context().Impl();
		auto entry = impl.value_handles[val];
		BOOST_ASSERT_MSG(entry, "Value bit set but no entries exist");

		for (ValueHandleBase iter(Assert, *entry); entry; entry = iter.next_)
		{
			iter.RemoveFromUseList();
			iter.AddToExistingUseListAfter(entry);
			BOOST_ASSERT_MSG(entry->next_ == &iter, "Loop invariant broken.");

			switch (entry->kind_)
			{
			case Assert:
				break;
			case Tracking:
				entry->Assign(TombstonePointer<Value>());
				break;
			case Weak:
				entry->Assign(nullptr);
				break;
			case Callback:
				static_cast<CallbackVH*>(entry)->Deleted();
				break;
			}
		}

		if (val->has_value_handle_)
		{
#ifdef DILITHIUM_DEBUG
			std::clog << "While deleting: " << *val->GetType() << " %" << val->Name()
				<< std::endl;
			if (impl.value_handles[val]->kind_ == Assert)
			{
				DILITHIUM_UNREACHABLE("An asserting value handle still pointed to this value!");
			}

#endif
			DILITHIUM_UNREACHABLE("All references to V were not removed?");
		}
	}

	void ValueHandleBase::ValueIsRAUWd(Value* old_val, Value* new_val)
	{
		BOOST_ASSERT_MSG(old_val->has_value_handle_, "Should only be called if ValueHandles present");
		BOOST_ASSERT_MSG(old_val != new_val, "Changing value into itself!");
		BOOST_ASSERT_MSG(old_val->GetType() == new_val->GetType(), "replaceAllUses of value with new value of different type!");

		auto& impl = old_val->Context().Impl();
		auto entry = impl.value_handles[old_val];

		BOOST_ASSERT_MSG(entry, "Value bit set but no entries exist");

		for (ValueHandleBase iter(Assert, *entry); entry; entry = iter.next_)
		{
			iter.RemoveFromUseList();
			iter.AddToExistingUseListAfter(entry);
			BOOST_ASSERT_MSG(entry->next_ == &iter, "Loop invariant broken.");

			switch (entry->kind_)
			{
			case Assert:
				break;
			case Tracking:
			case Weak:
				entry->Assign(new_val);
				break;
			case Callback:
				static_cast<CallbackVH*>(entry)->AllUsesReplacedWith(new_val);
				break;
			}
		}

#ifdef DILITHIUM_DEBUG
		if (old_val->has_value_handle_)
		{
			for (entry = impl.value_handles[old_val]; entry; entry = entry->next_)
			{
				switch (entry->kind_)
				{
				case Tracking:
				case Weak:
					std::clog << "After RAUW from " << *old_val->GetType() << " %"
						<< old_val->Name() << " to " << *new_val->GetType() << " %"
						<< new_val->Name() << std::endl;
					DILITHIUM_UNREACHABLE("A tracking or weak value handle still pointed to the old value!\n");
				default:
					break;
				}
			}
		}
#endif
	}

	void ValueHandleBase::AddToExistingUseList(ValueHandleBase** node)
	{
		BOOST_ASSERT_MSG(node, "Handle list is null?");

		next_ = *node;
		*node = this;
		prev_ = node;
		if (next_)
		{
			next_->prev_ = &next_;
			BOOST_ASSERT_MSG(val_ == next_->val_, "Added to wrong list?");
		}
	}

	void ValueHandleBase::AddToExistingUseListAfter(ValueHandleBase* node)
	{
		BOOST_ASSERT_MSG(node, "Must insert after existing node");

		next_ = node->next_;
		prev_ = &node->next_;
		node->next_ = this;
		if (next_)
		{
			next_->prev_ = &next_;
		}
	}

	void ValueHandleBase::AddToUseList()
	{
		BOOST_ASSERT_MSG(val_, "Null pointer doesn't have a use list!");

		auto& impl = val_->Context().Impl();

		if (val_->has_value_handle_) {
			auto& entry = impl.value_handles[val_];
			BOOST_ASSERT_MSG(entry, "Value doesn't have any handles?");
			this->AddToExistingUseList(&entry);
		}
		else
		{
			auto& handles = impl.value_handles;

			auto& entry = handles[val_];
			BOOST_ASSERT_MSG(!entry, "Value really did already have handles?");
			this->AddToExistingUseList(&entry);
			val_->has_value_handle_ = true;

			for (auto iter = handles.begin(), end_iter = handles.end(); iter != end_iter; ++ iter)
			{
				BOOST_ASSERT_MSG(iter->second && iter->first == iter->second->val_, "List invariant broken!");
				iter->second->prev_ = &iter->second;
			}
		}
	}

	void ValueHandleBase::RemoveFromUseList()
	{
		BOOST_ASSERT_MSG(val_ && val_->has_value_handle_, "Pointer doesn't have a use list!");

		ValueHandleBase** prev_ptr = prev_;
		BOOST_ASSERT_MSG(*prev_ptr == this, "List invariant broken");

		*prev_ptr = next_;
		if (next_)
		{
			BOOST_ASSERT_MSG(next_->prev_ == &next_, "List invariant broken");
			next_->prev_ = prev_ptr;
		}
		else
		{
			auto& handles = val_->Context().Impl().value_handles;
			for (auto iter = handles.begin(), end_iter = handles.end(); iter != end_iter; ++ iter)
			{
				if (iter->second == *prev_ptr)
				{
					handles.erase(iter);
					val_->has_value_handle_ = false;
					break;
				}
			}
		}
	}
}
