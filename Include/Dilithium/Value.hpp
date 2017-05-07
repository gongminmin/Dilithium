/**
 * @file Value.hpp
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

#ifndef _DILITHIUM_VALUE_HPP
#define _DILITHIUM_VALUE_HPP

#pragma once

#include <Dilithium/Use.hpp>

#include <functional>

#include <boost/core/noncopyable.hpp>
#include <boost/range/iterator_range.hpp>

namespace Dilithium
{
	class LLVMContext;
	class Type;
	class User;

	class Value : boost::noncopyable
	{
	public:
		static uint32_t constexpr MAX_ALIGNMENT_EXPONENT = 29;
		static uint32_t constexpr MAX_ALIGNMENT = 1U << MAX_ALIGNMENT_EXPONENT;

		enum ValueTy
		{
#define HANDLE_VALUE(Name) Name##Val,
#include "Value.inc"

			// Markers:
#define HANDLE_CONSTANT_MARKER(Marker, Constant) Marker = Constant##Val,
#include "Value.inc"
		};

	private:
		template <typename UseT> // UseT == 'Use' or 'const Use'
		class use_iterator_impl : public std::iterator<std::forward_iterator_tag, UseT*>
		{
			friend class Value;

			UseT* use_;

			explicit use_iterator_impl(UseT* use)
				: use_(use)
			{
			}

		public:
			use_iterator_impl() : use_()
			{
			}

			bool operator==(use_iterator_impl const & rhs) const
			{
				return use_ == rhs.use_;
			}
			bool operator!=(use_iterator_impl const & rhs) const
			{
				return use_ != rhs.use_;
			}

			use_iterator_impl& operator++()
			{
				BOOST_ASSERT_MSG(use_, "Cannot increment end iterator!");
				use_ = use_->GetNext();
				return *this;
			}
			use_iterator_impl operator++(int)
			{
				auto tmp = *this;
				++ *this;
				return tmp;
			}

			UseT& operator*() const
			{
				BOOST_ASSERT_MSG(use_, "Cannot dereference end iterator!");
				return *use_;
			}

			UseT* operator->() const
			{
				return use_;
			}

			operator use_iterator_impl<UseT const>() const
			{
				return use_iterator_impl<UseT const>(use_);
			}
		};

		template <typename UserT> // UserT == 'User' or 'User const'
		class user_iterator_impl : public std::iterator<std::forward_iterator_tag, UserT*>
		{
			friend class Value;

			use_iterator_impl<Use> use_iter_;

			explicit user_iterator_impl(Use* use)
				: use_iter_(use)
			{
			}

		public:
			user_iterator_impl()
			{
			}

			bool operator==(user_iterator_impl const & rhs) const
			{
				return use_iter_ == rhs.use_iter_;
			}
			bool operator!=(user_iterator_impl const & rhs) const
			{
				return use_iter_ != rhs.use_iter_;
			}

			bool AtEnd() const
			{ 
				return *this == user_iterator_impl();
			}

			user_iterator_impl &operator++()
			{
				++ use_iter_;
				return *this;
			}
			user_iterator_impl operator++(int)
			{
				auto tmp = *this;
				++ *this;
				return tmp;
			}

			UserT* operator*() const
			{
				return use_iter_->GetUser();
			}

			UserT* operator->() const
			{
				return operator*();
			}

			operator user_iterator_impl<UserT const>() const
			{
				return user_iterator_impl<UserT const>(*use_iter_);
			}

			Use& GetUse() const
			{
				return *use_iter_;
			}
		};

	public:
		Type* GetType() const
		{
			return type_;
		}

		LLVMContext& Context() const;

		bool HasName() const
		{
			return !name_.empty();
		}
		uint64_t NameHash() const
		{
			return name_hash_;
		}

		std::string_view Name() const;
		void Name(std::string_view name);

		bool UseEmpty() const
		{
			return use_list_ == nullptr;
		}

		typedef use_iterator_impl<Use> use_iterator;
		typedef use_iterator_impl<const Use> const_use_iterator;
		use_iterator UseBegin()
		{
			return use_iterator(use_list_);
		}
		const_use_iterator UseBegin() const
		{
			return const_use_iterator(use_list_);
		}
		use_iterator UseEnd()
		{
			return use_iterator();
		}
		const_use_iterator UseEnd() const
		{
			return const_use_iterator();
		}
		boost::iterator_range<use_iterator> Uses()
		{
			return boost::iterator_range<use_iterator>(this->UseBegin(), this->UseEnd());
		}
		boost::iterator_range<const_use_iterator> Uses() const
		{
			return boost::iterator_range<const_use_iterator>(this->UseBegin(), this->UseEnd());
		}

		bool UserEmpty() const
		{
			return use_list_ == nullptr;
		}

		typedef user_iterator_impl<User> user_iterator;
		typedef user_iterator_impl<const User> const_user_iterator;
		user_iterator UserBegin()
		{
			return user_iterator(use_list_);
		}
		const_user_iterator UserBegin() const
		{
			return const_user_iterator(use_list_);
		}
		user_iterator UserEnd()
		{
			return user_iterator();
		}
		const_user_iterator UserEnd() const
		{
			return const_user_iterator();
		}
		User* UserBack()
		{
			return *this->UserBegin();
		}
		User const * UserBack() const
		{
			return *this->UserBegin();
		}
		boost::iterator_range<user_iterator> Users()
		{
			return boost::iterator_range<user_iterator>(this->UserBegin(), this->UserEnd());
		}
		boost::iterator_range<const_user_iterator> Users() const
		{
			return boost::iterator_range<const_user_iterator>(this->UserBegin(), this->UserEnd());
		}

		void AddUse(Use& use)
		{
			use.AddToList(&use_list_);
		}

		uint32_t GetValueId() const
		{
			return subclass_id_;
		}

		void SortUseList(std::function<bool(Use const & lhs, Use const & rhs)> cmp);

	protected:
		std::string name_;
		uint64_t name_hash_;

	private:
		Type* type_;
		Use* use_list_;

		uint8_t const subclass_id_;
		uint8_t has_value_handle_ : 1;

		// DILITHIUM_NOT_IMPLEMENTED
	};
}

#endif		// _DILITHIUM_VALUE_HPP
