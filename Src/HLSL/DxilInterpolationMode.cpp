/**
 * @file DxilInterpolationMode.cpp
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

#include <Dilithium/dxc/HLSL/DxilInterpolationMode.hpp>
#include <Dilithium/ErrorHandling.hpp>

#include <iterator>

namespace Dilithium
{
	DxilInterpolationMode::DxilInterpolationMode(uint64_t kind)
	{
		kind_ = static_cast<InterpolationMode>(kind);
		if (kind_ >= InterpolationMode::Invalid)
		{
			kind_ = InterpolationMode::Invalid;
		}
	}

	DxilInterpolationMode::DxilInterpolationMode(bool no_interpolation, bool linear, bool no_perspective, bool centroid, bool sample)
	{
		uint32_t mask = (no_interpolation ? 1 : 0) << 4;
		mask |= (linear ? 1 : 0) << 3;
		mask |= (no_perspective ? 1 : 0) << 2;
		mask |= (centroid ? 1 : 0) << 1;
		mask |= (sample ? 1 : 0);

		//                                                      no_interpolation, linear,  no_perspective, centroid, sample
		static InterpolationMode constexpr interp_mode_tab[] =
		{
			InterpolationMode::Undefined,                       //  0,   False,     False,     False,     False,     False
			InterpolationMode::LinearSample,                    //  1,   False,     False,     False,     False,     True
			InterpolationMode::LinearCentroid,                  //  2,   False,     False,     False,     True,      False
			InterpolationMode::LinearSample,                    //  3,   False,     False,     False,     True,      True
			InterpolationMode::LinearNoperspective,             //  4,   False,     False,     True,      False,     False
			InterpolationMode::LinearNoperspectiveSample,       //  5,   False,     False,     True,      False,     True
			InterpolationMode::LinearNoperspectiveCentroid,     //  6,   False,     False,     True,      True,      False
			InterpolationMode::LinearNoperspectiveSample,       //  7,   False,     False,     True,      True,      True
			InterpolationMode::Linear,                          //  8,   False,     True,      False,     False,     False
			InterpolationMode::LinearSample,                    //  9,   False,     True,      False,     False,     True
			InterpolationMode::LinearCentroid,                  // 10,   False,     True,      False,     True,      False
			InterpolationMode::LinearSample,                    // 11,   False,     True,      False,     True,      True
			InterpolationMode::LinearNoperspective,             // 12,   False,     True,      True,      False,     False
			InterpolationMode::LinearNoperspectiveSample,       // 13,   False,     True,      True,      False,     True
			InterpolationMode::LinearNoperspectiveCentroid,     // 14,   False,     True,      True,      True,      False
			InterpolationMode::LinearNoperspectiveSample,       // 15,   False,     True,      True,      True,      True
			InterpolationMode::Constant,                        // 16,   True,      False,     False,     False,     False
			InterpolationMode::Invalid,                         // 17,   True,      False,     False,     False,     True
			InterpolationMode::Invalid,                         // 18,   True,      False,     False,     True,      False
			InterpolationMode::Invalid,                         // 19,   True,      False,     False,     True,      True
			InterpolationMode::Invalid,                         // 20,   True,      False,     True,      False,     False
			InterpolationMode::Invalid,                         // 21,   True,      False,     True,      False,     True
			InterpolationMode::Invalid,                         // 22,   True,      False,     True,      True,      False
			InterpolationMode::Invalid,                         // 23,   True,      False,     True,      True,      True
			InterpolationMode::Invalid,                         // 24,   True,      True ,     False,     False,     False
			InterpolationMode::Invalid,                         // 25,   True,      True ,     False,     False,     True
			InterpolationMode::Invalid,                         // 26,   True,      True ,     False,     True,      False
			InterpolationMode::Invalid,                         // 27,   True,      True ,     False,     True,      True
			InterpolationMode::Invalid,                         // 28,   True,      True ,     True,      False,     False
			InterpolationMode::Invalid,                         // 29,   True,      True ,     True,      False,     True
			InterpolationMode::Invalid,                         // 30,   True,      True ,     True,      True,      False
			InterpolationMode::Invalid,                         // 31,   True,      True ,     True,      True,      True
		};

		// interp_mode_tab is generate from following code.
		/*kind_ = InterpolationMode::Invalid;	// Cases not set below are invalid.
		bool any_linear = linear | no_perspective | centroid | sample;
		if (no_interpolation)
		{
			if (!any_linear)
			{
				kind_ = InterpolationMode::Constant;
			}
		}
		else if (any_linear)
		{
			if (sample)
			{
				// warning case: sample overrides centroid.
				if (no_interpolation)
				{
					kind_ = InterpolationMode::LinearNoperspectiveSample;
				}
				else
				{
					kind_ = InterpolationMode::LinearSample;
				}
			}
			else
			{
				if (!no_perspective && !centroid)
				{
					kind_ = InterpolationMode::Linear;
				}
				else if (!no_perspective && centroid)
				{
					kind_ = InterpolationMode::LinearCentroid;
				}
				else if (no_perspective && !centroid)
				{
					kind_ = InterpolationMode::LinearNoperspective;
				}
				else if (no_perspective && centroid)
				{
					kind_ = InterpolationMode::LinearNoperspectiveCentroid;
				}
			}
		}
		else
		{
			kind_ = InterpolationMode::Undefined;
		}*/

		kind_ = interp_mode_tab[mask];
	}

	DxilInterpolationMode& DxilInterpolationMode::operator=(DxilInterpolationMode const & rhs)
	{
		if (this != &rhs)
		{
			kind_ = rhs.kind_;
		}

		return *this;
	}

	bool DxilInterpolationMode::operator==(DxilInterpolationMode const & rhs) const
	{
		return kind_ == rhs.kind_;
	}

	bool DxilInterpolationMode::IsAnyLinear() const
	{
		return (kind_ < InterpolationMode::Invalid) && (kind_ != InterpolationMode::Undefined) && (kind_ != InterpolationMode::Constant);
	}

	bool DxilInterpolationMode::IsAnyNoPerspective() const
	{
		return this->IsLinearNoperspective() || this->IsLinearNoperspectiveCentroid() || this->IsLinearNoperspectiveSample();
	}

	bool DxilInterpolationMode::IsAnyCentroid() const
	{
		return this->IsLinearCentroid() || this->IsLinearNoperspectiveCentroid();
	}

	bool DxilInterpolationMode::IsAnySample() const
	{
		return this->IsLinearSample() || this->IsLinearNoperspectiveSample();
	}

	char const * DxilInterpolationMode::GetName() const
	{
		static char const * s_InterpolationModeNames[] =
		{
			"",
			"nointerpolation",
			"linear",
			"centroid",
			"noperspective",
			"noperspective centroid",
			"sample",
			"noperspective sample"
		};
		static_assert(std::size(s_InterpolationModeNames) == static_cast<uint32_t>(InterpolationMode::Invalid),
			"Wrong size of s_InterpolationModeNames");

		if (kind_ >= InterpolationMode::Invalid)
		{
			DILITHIUM_UNREACHABLE("invalid interpolation mode");
		}

		return s_InterpolationModeNames[static_cast<uint32_t>(kind_)];
	}
}
