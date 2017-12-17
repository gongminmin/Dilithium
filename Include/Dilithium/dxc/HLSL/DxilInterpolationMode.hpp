/**
 * @file DxilInterpolationMode.hpp
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

#ifndef _DILITHIUM_DXIL_INTERPOLATION_MODE_HPP
#define _DILITHIUM_DXIL_INTERPOLATION_MODE_HPP

#pragma once

#include <Dilithium/dxc/HLSL/DxilConstants.hpp>

namespace Dilithium
{
	class DxilInterpolationMode
	{
	public:
		DxilInterpolationMode()
			: kind_(InterpolationMode::Undefined)
		{
		}
		explicit DxilInterpolationMode(InterpolationMode kind)
			: kind_(kind)
		{
		}
		explicit DxilInterpolationMode(uint64_t kind);
		DxilInterpolationMode(bool no_interpolation, bool linear, bool no_perspective, bool centroid, bool sample);

		DxilInterpolationMode& operator=(DxilInterpolationMode const & rhs);
		bool operator==(DxilInterpolationMode const & rhs) const;

		bool IsValid() const
		{
			return (kind_ >= InterpolationMode::Undefined) && (kind_ < InterpolationMode::Invalid);
		}
		bool IsUndefined() const
		{
			return kind_ == InterpolationMode::Undefined;
		}
		bool IsConstant() const
		{
			return kind_ == InterpolationMode::Constant;
		}
		bool IsLinear() const
		{
			return kind_ == InterpolationMode::Linear;
		}
		bool IsLinearCentroid() const
		{
			return kind_ == InterpolationMode::LinearCentroid;
		}
		bool IsLinearNoperspective() const
		{
			return kind_ == InterpolationMode::LinearNoperspective;
		}
		bool IsLinearNoperspectiveCentroid() const
		{
			return kind_ == InterpolationMode::LinearNoperspectiveCentroid;
		}
		bool IsLinearSample() const
		{
			return kind_ == InterpolationMode::LinearSample;
		}
		bool IsLinearNoperspectiveSample() const
		{
			return kind_ == InterpolationMode::LinearNoperspectiveSample;
		}

		bool IsAnyLinear() const;
		bool IsAnyNoPerspective() const;
		bool IsAnyCentroid() const;
		bool IsAnySample() const;

		InterpolationMode GetKind() const
		{
			return kind_;
		}
		char const * GetName() const;

	private:
		InterpolationMode kind_;
	};
}

#endif		// _DILITHIUM_DXIL_INTERPOLATION_MODE_HPP
