/**
 * @file DxilResource.hpp
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

#ifndef DILITHIUM_DXIL_RESOURCE_HPP
#define DILITHIUM_DXIL_RESOURCE_HPP

#pragma once

#include <Dilithium/dxc/HLSL/DxilResourceBase.hpp>

namespace Dilithium
{
	class DxilResource : public DxilResourceBase
	{
	public:
		DxilResource();

		uint32_t GetSampleCount() const
		{
			return sample_count_;
		}
		void SetSampleCount(uint32_t sample_count)
		{
			sample_count_ = sample_count;
		}

		bool IsGloballyCoherent() const
		{
			return globally_coherent_;
		}
		void SetGloballyCoherent(bool g)
		{
			globally_coherent_ = g;
		}
		bool HasCounter() const
		{
			return has_counter_;
		}
		void SetHasCounter(bool c)
		{
			has_counter_ = c;
		}

		bool IsReadOnly() const
		{
			return this->GetClass() == ResourceClass::SRV;
		}
		bool IsReadWrite() const
		{
			return this->GetClass() == ResourceClass::UAV;
		}
		void SetReadWrite(bool rw)
		{
			this->SetClass(rw ? ResourceClass::UAV : ResourceClass::SRV);
		}
		bool IsRasterizerOrderedView() const
		{
			return rov_;
		}
		void SetRasterizerOrderedView(bool rov)
		{
			rov_ = rov;
		}

	private:
		uint32_t sample_count_;
		bool globally_coherent_;
		bool has_counter_;
		bool rov_;
	};
}

#endif		// DILITHIUM_DXIL_RESOURCE_HPP
