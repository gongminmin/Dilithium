/**
 * @file DxilConstants.hpp
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

#ifndef _DILITHIUM_DXIL_CONSTANTS_HPP
#define _DILITHIUM_DXIL_CONSTANTS_HPP

#pragma once

#include <Dilithium/Util.hpp>

namespace Dilithium
{
	enum class ComponentType : uint8_t
	{
		Invalid = 0,
		I1, I16, U16, I32, U32, I64, U64,
		F16, F32, F64,
		SNormF16, UNormF16, SNormF32, UNormF32, SNormF64, UNormF64,
		LastEntry
	};

	enum class InterpolationMode : uint8_t
	{
		Undefined = 0,
		Constant,
		Linear,
		LinearCentroid,
		LinearNoperspective,
		LinearNoperspectiveCentroid,
		LinearSample,
		LinearNoperspectiveSample,
		Invalid
	};

	enum class SignatureKind
	{
		Invalid = 0,
		Input,
		Output,
		PatchConstant
	};

	enum class ShaderKind
	{
		Pixel = 0,
		Vertex,
		Geometry,
		Hull,
		Domain,
		Compute,
		Invalid
	};

	enum class TessellatorDomain
	{
		Undefined = 0,
		IsoLine = 1,
		Tri = 2,
		Quad = 3
	};

	enum class TessellatorOutputPrimitive
	{
		Undefined = 0,
		Point = 1,
		Line = 2,
		TriangleCW = 3,
		TriangleCCW = 4
	};

	enum class InputPrimitive : uint32_t
	{
		Undefined = 0,
		Point = 1,
		Line = 2,
		Triangle = 3,
		Reserved4 = 4,
		Reserved5 = 5,
		LineWithAdjacency = 6,
		TriangleWithAdjacency = 7,
		ControlPointPatch1 = 8,
		ControlPointPatch2 = 9,
		ControlPointPatch3 = 10,
		ControlPointPatch4 = 11,
		ControlPointPatch5 = 12,
		ControlPointPatch6 = 13,
		ControlPointPatch7 = 14,
		ControlPointPatch8 = 15,
		ControlPointPatch9 = 16,
		ControlPointPatch10 = 17,
		ControlPointPatch11 = 18,
		ControlPointPatch12 = 19,
		ControlPointPatch13 = 20,
		ControlPointPatch14 = 21,
		ControlPointPatch15 = 22,
		ControlPointPatch16 = 23,
		ControlPointPatch17 = 24,
		ControlPointPatch18 = 25,
		ControlPointPatch19 = 26,
		ControlPointPatch20 = 27,
		ControlPointPatch21 = 28,
		ControlPointPatch22 = 29,
		ControlPointPatch23 = 30,
		ControlPointPatch24 = 31,
		ControlPointPatch25 = 32,
		ControlPointPatch26 = 33,
		ControlPointPatch27 = 34,
		ControlPointPatch28 = 35,
		ControlPointPatch29 = 36,
		ControlPointPatch30 = 37,
		ControlPointPatch31 = 38,
		ControlPointPatch32 = 39
	};

	enum class PrimitiveTopology : uint32_t
	{
		Undefined = 0,
		PointList = 1,
		LineList = 2,
		LineStrip = 3,
		TriangleList = 4,
		TriangleStrip = 5
	};

	enum class SigPointKind : uint32_t
	{
		VSIn,
		VSOut,
		PCIn,
		HSIn,
		HSCPIn,
		HSCPOut,
		PCOut,
		DSIn,
		DSCPIn,
		DSOut,
		GSVIn,
		GSIn,
		GSOut,
		PSIn,
		PSOut,
		CSIn,
		Invalid
	};

	enum class PackingKind : uint32_t
	{
		None,
		InputAssembler,
		Vertex,
		PatchConstant,
		Target,
		Invalid
	};

	enum class SemanticInterpretationKind : uint32_t
	{
		NA,
		SV,
		SGV,
		Arb,
		NotInSig,
		NotPacked,
		Target,
		TessFactor,
		Shadow,
		Invalid
	};

	enum class SemanticKind : uint32_t
	{
		Arbitrary,
		VertexID,
		InstanceID,
		Position,
		RenderTargetArrayIndex,
		ViewPortArrayIndex,
		ClipDistance,
		CullDistance,
		OutputControlPointID,
		DomainLocation,
		PrimitiveID,
		GSInstanceID,
		SampleIndex,
		IsFrontFace,
		Coverage,
		InnerCoverage,
		Target,
		Depth,
		DepthLessEqual,
		DepthGreaterEqual,
		StencilRef,
		DispatchThreadID,
		GroupID,
		GroupIndex,
		GroupThreadID,
		TessFactor,
		InsideTessFactor,
		Invalid
	};
}

#endif		// _DILITHIUM_DXIL_CONSTANTS_HPP
