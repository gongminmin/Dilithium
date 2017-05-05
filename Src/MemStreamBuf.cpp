/**
 * @file MemStreamBuf.cpp
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

#include <Dilithium/MemStreamBuf.hpp>
#include <Dilithium/Util.hpp>

#include <cstddef>

#include <boost/assert.hpp>

namespace Dilithium
{
	MemStreamBuf::MemStreamBuf(void const * begin, void const * end)
		: begin_(static_cast<char_type const *>(begin)), end_(static_cast<char_type const *>(end)),
			current_(begin_)
	{
		BOOST_ASSERT(begin_ <= end_);
	}

	MemStreamBuf::int_type MemStreamBuf::uflow()
	{
		if (current_ == end_)
		{
			return traits_type::eof();
		}

		char_type const * c = current_;
		++ current_;
		return *c;
	}

	MemStreamBuf::int_type MemStreamBuf::underflow()
	{
		if (current_ == end_)
		{
			return traits_type::eof();
		}

		return *current_;
	}

	std::streamsize MemStreamBuf::xsgetn(char_type* s, std::streamsize count)
	{
		if (current_ + count >= end_)
		{
			count = end_ - current_;
		}
		memcpy(s, current_, static_cast<size_t>(count * sizeof(char_type)));
		current_ += count;
		return count;
	}

	MemStreamBuf::int_type MemStreamBuf::pbackfail(int_type ch)
	{
		if ((current_ == begin_) || ((ch != traits_type::eof()) && (ch != current_[-1])))
		{
			return traits_type::eof();
		}

		-- current_;
		return *current_;
	}

	std::streamsize MemStreamBuf::showmanyc()
	{
		BOOST_ASSERT(current_ <= end_);
		return end_ - current_;
	}

	MemStreamBuf::pos_type MemStreamBuf::seekoff(off_type off, std::ios_base::seekdir way,
		std::ios_base::openmode which)
	{
		BOOST_ASSERT(which == std::ios_base::in);
		DILITHIUM_UNUSED(which);

		switch (way)
		{
		case std::ios_base::beg:
			if (off <= end_ - begin_)
			{
				current_ = begin_ + off;
			}
			else
			{
				off = -1;
			}
			break;

		case std::ios_base::end:
			if (end_ - off >= begin_)
			{
				current_ = end_ - off;
				off = current_ - begin_;
			}
			else
			{
				off = -1;
			}
			break;

		case std::ios_base::cur:
		default:
			if (current_ + off <= end_)
			{
				current_ += off;
				off = current_ - begin_;
			}
			else
			{
				off = -1;
			}
			break;
		}

		return off;
	}

	MemStreamBuf::pos_type MemStreamBuf::seekpos(pos_type sp, std::ios_base::openmode which)
	{
		BOOST_ASSERT(which == std::ios_base::in);
		DILITHIUM_UNUSED(which);

		if (sp < end_ - begin_)
		{
			current_ = begin_ + static_cast<int>(sp);
		}
		else
		{
			sp = -1;
		}

		return sp;
	}
}
