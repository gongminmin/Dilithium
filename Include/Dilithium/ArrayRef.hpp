/**
 * @file ArrayRef.hpp
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

#ifndef _DILITHIUM_ARRAY_REF_HPP
#define _DILITHIUM_ARRAY_REF_HPP

#include <Dilithium/Util.hpp>

#include <vector>

#include <boost/assert.hpp>
#include <boost/container/small_vector.hpp>

namespace Dilithium
{
	// ArrayRef - Represent a constant reference to an array (0 or more elements
	// consecutively in memory), i.e. a start pointer and a size.  It allows
	// various APIs to take consecutive elements easily and conveniently.
	//
	// This class does not own the underlying data, it is expected to be used in
	// situations where the data resides in some other buffer, whose lifetime
	// extends past that of the ArrayRef. For this reason, it is not in general
	// safe to store an ArrayRef.
	//
	// This is intended to be trivially copyable, so it should be passed by value.
	template <typename T>
	class ArrayRef
	{
	public:
		typedef T const * iterator;
		typedef T const * const_iterator;
		typedef size_t size_type;

		typedef std::reverse_iterator<iterator> reverse_iterator;

	public:
		ArrayRef()
			: data_(nullptr), size_(0)
		{
		}

		ArrayRef(ArrayRef const & rhs)
			: data_(rhs.data()), size_(rhs.size())
		{
		}

		DILITHIUM_IMPLICIT ArrayRef(T const & t)
			: data_(&t), size_(1)
		{
		}

		ArrayRef(T const * data, size_t size)
			: data_(data), size_(size)
		{
		}

		ArrayRef(T const * begin, T const * end)
			: data_(begin), size_(end - begin)
		{
		}

		template <typename U>
		DILITHIUM_IMPLICIT ArrayRef(boost::container::small_vector_base<T, U> const & v)
			: data_(v.data()), size_(v.size())
		{
		}

		template <typename A>
		DILITHIUM_IMPLICIT ArrayRef(std::vector<T, A> const & v)
			: data_(v.data()), size_(v.size())
		{
		}

		template <size_t N>
		DILITHIUM_IMPLICIT constexpr ArrayRef(T const (&arr)[N])
			: data_(arr), size_(N)
		{
		}

		ArrayRef(std::initializer_list<T> const & v)
			: data_(v.begin() == v.end() ? nullptr : v.begin()), size_(v.size())
		{
		}

		template <typename U>
		DILITHIUM_IMPLICIT ArrayRef(ArrayRef<U*> const & rhs,
			typename std::enable_if<std::is_convertible<U* const *, T const *>::value>::type* = 0)
			: data_(rhs.data()), size_(rhs.size())
		{
		}

		template <typename U, typename A>
		DILITHIUM_IMPLICIT ArrayRef(std::vector<U*, A> const & v,
			typename std::enable_if<std::is_convertible<U* const *, T const *>::value>::type* = 0)
			: data_(v.data()), size_(v.size())
		{
		}

		iterator begin() const
		{
			return data_;
		}
		iterator end() const
		{
			return data_ + size_;
		}

		reverse_iterator rbegin() const
		{
			return reverse_iterator(this->end());
		}
		reverse_iterator rend() const
		{
			return reverse_iterator(this->begin());
		}

		T const * data() const
		{
			return data_;
		}

		size_t size() const
		{
			return size_;
		}

		bool empty() const
		{
			return size_ == 0;
		}

		T const & front() const
		{
			BOOST_ASSERT(!this->empty());
			return data_[0];
		}

		T const & back() const
		{
			BOOST_ASSERT(!this->empty());
			return data_[size_ - 1];
		}

		template <typename Alloc>
		ArrayRef<T> Copy(Alloc& alloc)
		{
			T* buff = alloc.template allocate<T>(size_);
			std::uninitialized_copy(this->begin(), this->end(), buff);
			return ArrayRef<T>(buff, size_);
		}

		ArrayRef<T> Slice(uint32_t n) const
		{
			BOOST_ASSERT_MSG(n <= this->size(), "Invalid specifier");
			return ArrayRef<T>(this->data() + n, this->size() - n);
		}

		ArrayRef<T> Slice(uint32_t n, uint32_t m) const
		{
			BOOST_ASSERT_MSG(n + m <= this->size(), "Invalid specifier");
			return ArrayRef<T>(this->data() + n, m);
		}

		ArrayRef<T> DropBack(uint32_t n = 1) const
		{
			BOOST_ASSERT_MSG(this->size() >= n, "Dropping more elements than exist");
			return this->Slice(0, this->Size() - n);
		}

		T const & operator[](size_t index) const
		{
			BOOST_ASSERT_MSG(index < size_, "Invalid index!");
			return data_[index];
		}

		std::vector<T> ToVector() const
		{
			return std::vector<T>(data_, data_ + size_);
		}

	private:
		T const * data_;
		size_type size_;
	};

	template <typename T>
	inline bool operator==(ArrayRef<T> lhs, ArrayRef<T> rhs)
	{
		if (lhs.size() != rhs.size())
		{
			return false;
		}
		return std::equal(lhs.begin(), lhs.end(), rhs.begin());
	}

	template <typename T>
	inline bool operator!=(ArrayRef<T> lhs, ArrayRef<T> rhs)
	{
		return !(lhs == rhs);
	}
}

#endif		// _DILITHIUM_ARRAY_REF_HPP
