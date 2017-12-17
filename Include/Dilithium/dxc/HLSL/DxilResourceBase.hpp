/**
 * @file DxilResourceBase.hpp
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

#ifndef _DILITHIUM_DXIL_RESOURCE_BASE_HPP
#define _DILITHIUM_DXIL_RESOURCE_BASE_HPP

#pragma once

#include <Dilithium/dxc/HLSL/DxilConstants.hpp>

namespace Dilithium
{
	class DxilResourceBase
	{
	public:
		explicit DxilResourceBase(ResourceClass c);
		virtual ~DxilResourceBase()
		{
		}

		ResourceClass GetClass() const
		{
			return class_;
		}
		ResourceKind GetKind() const
		{
			return kind_;
		}
		uint32_t GetID() const
		{
			return id_;
		}
		uint32_t GetSpaceID() const
		{
			return space_id_;
		}
		uint32_t GetLowerBound() const
		{
			return lower_bound_;
		}
		uint32_t GetUpperBound() const
		{
			return range_size_ != UINT_MAX ? lower_bound_ + range_size_ - 1 : UINT_MAX;
		}
		uint32_t GetRangeSize() const
		{
			return range_size_;
		}
		Constant* GetGlobalSymbol() const
		{
			return symbol_;
		}
		std::string const & GetGlobalName() const
		{
			return name_;
		}

		void SetKind(ResourceKind resource_kind);
		void SetSpaceID(uint32_t space_id)
		{
			space_id_ = space_id;
		}
		void SetLowerBound(uint32_t lb)
		{
			lower_bound_ = lb;
		}
		void SetRangeSize(uint32_t range_size)
		{
			range_size_ = range_size;
		}
		void SetGlobalSymbol(Constant* gv)
		{
			symbol_ = gv;
		}
		void SetGlobalName(std::string const & name)
		{
			name_ = name;
		}
		void SetHandle(Value* handle)
		{
			handle_ = handle;
		}

		// TODO: check whether we can make this a protected method.
		void SetID(uint32_t id)
		{
			id_ = id;
		}

		char const * GetResClassName() const;
		char const * GetResDimName() const;
		char const * GetResIDPrefix() const;
		char const * GetResBindPrefix() const;

	protected:
		void SetClass(ResourceClass c)
		{
			class_ = c;
		}

	private:
		ResourceClass class_;	// Resource class (SRV, UAV, CBuffer, Sampler)
		ResourceKind kind_;		// Detail resource kind (texture2D...)
		uint32_t id_;			// Unique ID within the class
		uint32_t space_id_;		// Root signature space
		uint32_t lower_bound_;	// Range lower bound
		uint32_t range_size_;	// Range size in entries
		Constant* symbol_;		// Global variable
		std::string name_;		// Unmangled name of the global variable.
		Value* handle_;			// Cached resource handle for SM5.0- (and maybe SM5.1).
	};
}

#endif		// _DILITHIUM_DXIL_RESOURCE_BASE_HPP
