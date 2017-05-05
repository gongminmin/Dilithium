/**
 * @file MemStreamBuf.hpp
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

#ifndef _DILITHIUM_MEM_STREAM_BUF_HPP
#define _DILITHIUM_MEM_STREAM_BUF_HPP

#pragma once

#include <streambuf>

#include <boost/core/noncopyable.hpp>

namespace Dilithium
{
	class MemStreamBuf : boost::noncopyable, public std::streambuf
	{
	public:
		MemStreamBuf(void const * begin, void const * end);

	protected:
		virtual int_type uflow() override;
		virtual int_type underflow() override;

		virtual std::streamsize xsgetn(char_type* s, std::streamsize count) override;

		virtual int_type pbackfail(int_type ch) override;
		virtual std::streamsize showmanyc() override;

		virtual pos_type seekoff(off_type off, std::ios_base::seekdir way, std::ios_base::openmode which) override;
		virtual pos_type seekpos(pos_type sp, std::ios_base::openmode which) override;

	private:
		char_type const * const begin_;
		char_type const * const end_;
		char_type const * current_;
	};
}
#endif		// _DILITHIUM_MEM_STREAM_BUF_HPP
