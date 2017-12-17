/**
 * @file DxilResourceBase.cpp
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
#include <Dilithium/dxc/HLSL/DxilResourceBase.hpp>

namespace Dilithium
{
	DxilResourceBase::DxilResourceBase(ResourceClass c)
		: class_(c), kind_(ResourceKind::Invalid)
	{
	}

	void DxilResourceBase::SetKind(ResourceKind resource_kind)
	{
		BOOST_ASSERT_MSG((resource_kind > ResourceKind::Invalid) && (resource_kind < ResourceKind::NumEntries), "Invalid resource type");
		kind_ = resource_kind;
	}

	char const * DxilResourceBase::GetResClassName() const
	{
		static const char *s_ResourceClassNames[] =
		{
			"texture", "UAV", "cbuffer", "sampler"
		};
		static_assert(std::size(s_ResourceClassNames) == static_cast<uint32_t>(ResourceClass::Invalid),
			"Wrong size of s_ResourceClassNames");

		return s_ResourceClassNames[static_cast<uint32_t>(class_)];
	}

	char const * DxilResourceBase::GetResDimName() const
	{
		static const char *s_ResourceDimNames[] =
		{
			"invalid", "1d",        "2d",      "2dMS",      "3d",
			"cube",    "1darray",   "2darray", "2darrayMS", "cubearray",
			"buf",     "rawbuf",    "structbuf", "cbuffer", "sampler",
			"tbuffer",
		};
		static_assert(std::size(s_ResourceDimNames) == static_cast<uint32_t>(ResourceKind::NumEntries),
			"Wrong size of s_ResourceDimNames");

		return s_ResourceDimNames[static_cast<uint32_t>(kind_)];
	}

	char const * DxilResourceBase::GetResIDPrefix() const
	{
		static const char *s_ResourceIDPrefixs[] =
		{
			"T", "U", "CB", "S"
		};
		static_assert(std::size(s_ResourceIDPrefixs) == static_cast<uint32_t>(ResourceClass::Invalid),
			"Wrong size of s_ResourceIDPrefixs");

		return s_ResourceIDPrefixs[static_cast<uint32_t>(class_)];
	}

	char const * DxilResourceBase::GetResBindPrefix() const
	{
		static const char *s_ResourceBindPrefixs[] =
		{
			"t", "u", "cb", "s"
		};
		static_assert(std::size(s_ResourceBindPrefixs) == static_cast<uint32_t>(ResourceClass::Invalid),
			"Wrong size of s_ResourceBindPrefixs");

		return s_ResourceBindPrefixs[static_cast<uint32_t>(class_)];
	}
}
