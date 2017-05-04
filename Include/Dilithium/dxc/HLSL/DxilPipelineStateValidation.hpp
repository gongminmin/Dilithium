/**
 * @file DxilPipelineStateValidation.hpp
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

#ifndef _DILITHIUM_DXIL_PIPELINE_STATE_VALIDATION_HPP
#define _DILITHIUM_DXIL_PIPELINE_STATE_VALIDATION_HPP

#pragma once

#include <Dilithium/Util.hpp>

namespace Dilithium
{
#pragma pack(push, 1)
	// Versioning is additive and based on size
	struct PSVRuntimeInfo0
	{
		union
		{
			struct VSInfo
			{
				char OutputPositionPresent;
			} VS;
			struct HSInfo
			{
				uint32_t InputControlPointCount;      // max control points == 32
				uint32_t OutputControlPointCount;     // max control points == 32
				uint32_t TessellatorDomain;           // hlsl::DXIL::TessellatorDomain/D3D11_SB_TESSELLATOR_DOMAIN
				uint32_t TessellatorOutputPrimitive;  // hlsl::DXIL::TessellatorOutputPrimitive/D3D11_SB_TESSELLATOR_OUTPUT_PRIMITIVE
			} HS;
			struct DSInfo
			{
				uint32_t InputControlPointCount;      // max control points == 32
				int8_t OutputPositionPresent;
				uint32_t TessellatorDomain;           // hlsl::DXIL::TessellatorDomain/D3D11_SB_TESSELLATOR_DOMAIN
			} DS;
			struct GSInfo
			{
				uint32_t InputPrimitive;              // hlsl::DXIL::InputPrimitive/D3D10_SB_PRIMITIVE
				uint32_t OutputTopology;              // hlsl::DXIL::PrimitiveTopology/D3D10_SB_PRIMITIVE_TOPOLOGY
				uint32_t OutputStreamMask;            // max streams == 4
				int8_t OutputPositionPresent;
			} GS;
			struct PSInfo
			{
				int8_t DepthOutput;
				int8_t SampleFrequency;
			} PS;
		};
		uint32_t MinimumExpectedWaveLaneCount;  // minimum lane count required, 0 if unused
		uint32_t MaximumExpectedWaveLaneCount;  // maximum lane count required, 0xffffffff if unused
	};
#pragma pack(pop)
}

#endif		// _DILITHIUM_DXIL_PIPELINE_STATE_VALIDATION_HPP
