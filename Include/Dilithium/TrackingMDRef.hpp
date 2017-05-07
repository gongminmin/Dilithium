/**
 * @file TrackingMDRef.hpp
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

#ifndef _DILITHIUM_TRACKING_MD_REF_HPP
#define _DILITHIUM_TRACKING_MD_REF_HPP

#pragma once

#include <Dilithium/Util.hpp>

namespace Dilithium
{
	class TrackingMDRef
	{
	public:
		TrackingMDRef()
			: metadata_(nullptr)
		{
		}
		explicit TrackingMDRef(Metadata* md)
			: metadata_(md)
		{
			DILITHIUM_NOT_IMPLEMENTED;
		}

		TrackingMDRef(TrackingMDRef&& rhs)
			: metadata_(rhs.metadata_)
		{
			DILITHIUM_NOT_IMPLEMENTED;
		}
		TrackingMDRef(TrackingMDRef const & rhs)
			: metadata_(rhs.metadata_)
		{
			DILITHIUM_NOT_IMPLEMENTED;
		}
		~TrackingMDRef()
		{
			//DILITHIUM_NOT_IMPLEMENTED;
		}

		TrackingMDRef& operator=(TrackingMDRef&& rhs);
		TrackingMDRef& operator=(TrackingMDRef const & rhs);

		Metadata* Get() const
		{
			return metadata_;
		}
		Metadata* operator->() const
		{
			return this->Get();
		}
		Metadata& operator*() const
		{
			return *this->Get();
		}

		void Reset();
		void Reset(Metadata* md);

	private:
		Metadata* metadata_;

		// DILITHIUM_NOT_IMPLEMENTED
	};
}

#endif		// _DILITHIUM_TRACKING_MD_REF_HPP
