/**
 * @file SmallString.hpp
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

#ifndef _DILITHIUM_SMALL_STRING_HPP
#define _DILITHIUM_SMALL_STRING_HPP

#pragma once

#include <Dilithium/CXX17/string_view.hpp>

#include <iosfwd>

#include <boost/container/small_vector.hpp>

namespace Dilithium
{
	template <uint32_t InternalLen>
	class SmallString : public boost::container::small_vector<char, InternalLen>
	{
	public:
		SmallString()
		{
		}
		SmallString(std::string_view sv)
			: boost::container::small_vector<char, InternalLen>(sv.begin(), sv.end())
		{
		}
		template <typename IterT>
		SmallString(IterT beg, IterT end)
			: boost::container::small_vector<char, InternalLen>(beg, end)
		{
		}

		void assign(size_t num_elems, char elem)
		{
			this->boost::container::small_vector_base<char>::assign(num_elems, elem);
		}
		template <typename IterT>
		void assign(IterT beg, IterT end)
		{
			this->clear();
			boost::container::small_vector_base<char>::insert(this->end(), beg, end);
		}
		void assign(std::string_view sv)
		{
			this->clear();
			boost::container::small_vector_base<char>::insert(this->end(), sv.begin(), sv.end());
		}
		void assign(boost::container::small_vector_base<char> const & rhs)
		{
			this->clear();
			boost::container::small_vector_base<char>::insert(this->end(), rhs.begin(), rhs.end());
		}

		template <typename IterT>
		void append(IterT beg, IterT end)
		{
			boost::container::small_vector_base<char>::insert(this->end(), beg, end);
		}
		void append(size_t num_elems, char elem)
		{
			boost::container::small_vector_base<char>::insert(this->end(), num_elems, elem);
		}
		void append(std::string_view sv)
		{
			boost::container::small_vector_base<char>::insert(this->end(), sv.begin(), sv.end());
		}
		void append(boost::container::small_vector_base<char> const & rhs)
		{
			boost::container::small_vector_base<char>::insert(this->end(), rhs.begin(), rhs.end());
		}

		bool equals(std::string_view rhs) const
		{
			return this->str().equals(rhs);
		}

		int compare(std::string_view rhs) const
		{
			return this->str().compare(rhs);
		}

		size_t find(char c, size_t from = 0) const
		{
			return this->str().find(c, from);
		}
		size_t find(std::string_view str, size_t from = 0) const
		{
			return this->str().find(str, from);
		}

		size_t rfind(char c, size_t from = std::string_view::npos) const
		{
			return this->str().rfind(c, from);
		}
		size_t rfind(std::string_view str) const
		{
			return this->str().rfind(str);
		}

		size_t find_first_of(char c, size_t from = 0) const
		{
			return this->str().find_first_of(c, from);
		}
		size_t find_first_of(std::string_view chars, size_t from = 0) const
		{
			return this->str().find_first_of(chars, from);
		}

		size_t find_first_not_of(char c, size_t from = 0) const
		{
			return this->str().find_first_not_of(c, from);
		}
		size_t find_first_not_of(std::string_view chars, size_t from = 0) const
		{
			return this->str().find_first_not_of(chars, from);
		}

		size_t find_last_of(char c, size_t from = std::string_view::npos) const
		{
			return this->str().find_last_of(c, from);
		}
		size_t find_last_of(std::string_view chars, size_t from = std::string_view::npos) const
		{
			return this->str().find_last_of(chars, from);
		}

		size_t count(char c) const
		{
			return this->str().count(c);
		}
		size_t count(std::string_view sv) const
		{
			return this->str().count(sv);
		}

		std::string_view substr(size_t start, size_t n = std::string_view::npos) const
		{
			return this->str().substr(start, n);
		}

		std::string_view str() const
		{
			return std::string_view(this->data(), this->size());
		}

		// TODO: Make this const, if it's safe...
		char const * c_str()
		{
			this->push_back(0);
			this->pop_back();
			return this->data();
		}

		// TODO: Make this explicit
		operator std::string_view() const
		{
			return this->str();
		}

		SmallString& operator+=(std::string_view rhs)
		{
			this->append(rhs.begin(), rhs.end());
			return *this;
		}
		SmallString& operator+=(char c)
		{
			this->push_back(c);
			return *this;
		}
	};

	inline std::ostream& operator<<(std::ostream& os, boost::container::small_vector_base<char> const & str)
	{
		os.write(str.data(), str.size());
		return os;
	}
}

#endif		// _DILITHIUM_SMALL_STRING_HPP
