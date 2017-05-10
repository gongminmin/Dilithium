/**
 * @file MetadataTracking.hpp
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

#ifndef _DILITHIUM_METADATA_TRACKING_HPP
#define _DILITHIUM_METADATA_TRACKING_HPP

#pragma once

#include <Dilithium/PointerUnion.hpp>
#include <type_traits>

namespace Dilithium
{
	class Metadata;
	class MetadataAsValue;

	class MetadataTracking
	{
	public:
		typedef PointerUnion<MetadataAsValue*, Metadata*> OwnerTy;

	public:
		static bool Track(Metadata*& md)
		{
			return Track(&md, *md, static_cast<Metadata*>(nullptr));
		}
		static bool Track(void* ref, Metadata& md, Metadata& owner)
		{
			return Track(ref, md, &owner);
		}
		static bool track(void* ref, Metadata& md, MetadataAsValue& owner)
		{
			return Track(ref, md, &owner);
		}

		static void Untrack(Metadata*& md)
		{
			Untrack(&md, *md);
		}
		static void Untrack(void* ref, Metadata& md);

		static bool Retrack(Metadata*& md, Metadata*& new_md)
		{
			return Retrack(&md, *md, &new_md);
		}
		static bool Retrack(void* ref, Metadata& md, void* new_md);

		static bool IsReplaceable(Metadata const & md);

	private:
		static bool Track(void* ref, Metadata& md, OwnerTy owner);
	};
}

#endif		// _DILITHIUM_METADATA_TRACKING_HPP
