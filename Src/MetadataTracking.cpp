/**
 * @file MetadataTracking.cpp
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
#include <Dilithium/Casting.hpp>
#include <Dilithium/Metadata.hpp>
#include <Dilithium/MetadataTracking.hpp>

namespace Dilithium
{
	bool MetadataTracking::Track(void* ref, Metadata& md, OwnerTy owner)
	{
		BOOST_ASSERT_MSG(ref, "Expected live reference");
		BOOST_ASSERT_MSG(owner || (*static_cast<Metadata **>(ref) == &md), "Reference without owner must be direct");
		auto r = ReplaceableMetadataImpl::Get(md);
		if (r)
		{
			r->AddRef(ref, owner);
			return true;
		}
		return false;
	}

	void MetadataTracking::Untrack(void* ref, Metadata& md)
	{
		BOOST_ASSERT_MSG(ref, "Expected live reference");
		auto r = ReplaceableMetadataImpl::Get(md);
		if (r)
		{
			r->DropRef(ref);
		}
	}

	bool MetadataTracking::Retrack(void* ref, Metadata& md, void* new_md)
	{
		BOOST_ASSERT_MSG(ref, "Expected live reference");
		BOOST_ASSERT_MSG(new_md, "Expected live reference");
		BOOST_ASSERT_MSG(ref != new_md, "Expected change");
		auto r = ReplaceableMetadataImpl::Get(md);
		if (r)
		{
			r->MoveRef(ref, new_md, md);
			return true;
		}
		return false;
	}

	bool MetadataTracking::IsReplaceable(Metadata const & md)
	{
		DILITHIUM_UNUSED(md);
		DILITHIUM_NOT_IMPLEMENTED;
	}
}
