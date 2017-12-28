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
	enum ShaderFlag : uint32_t
	{
		SF_DisableOptimizations = 0x00000001,			// D3D11_1_SB_GLOBAL_FLAG_SKIP_OPTIMIZATION
		SF_DisableMathRefactoring = 0x00000002,			// ~D3D10_SB_GLOBAL_FLAG_REFACTORING_ALLOWED
		SF_EnableDoublePrecision = 0x00000004,			// D3D11_SB_GLOBAL_FLAG_ENABLE_DOUBLE_PRECISION_FLOAT_OPS
		SF_ForceEarlyDepthStencil = 0x00000008,			// D3D11_SB_GLOBAL_FLAG_FORCE_EARLY_DEPTH_STENCIL
		SF_EnableRawAndStructuredBuffers = 0x00000010,	// D3D11_SB_GLOBAL_FLAG_ENABLE_RAW_AND_STRUCTURED_BUFFERS
		SF_EnableMinPrecision = 0x00000020,				// D3D11_1_SB_GLOBAL_FLAG_ENABLE_MINIMUM_PRECISION
		SF_EnableDoubleExtensions = 0x00000040,			// D3D11_1_SB_GLOBAL_FLAG_ENABLE_DOUBLE_EXTENSIONS
		SF_EnableMSAD = 0x00000080,						// D3D11_1_SB_GLOBAL_FLAG_ENABLE_SHADER_EXTENSIONS
		SF_AllResourcesBound = 0x00000100				// D3D12_SB_GLOBAL_FLAG_ALL_RESOURCES_BOUND
	};

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

	enum class TessellatorPartitioning : uint32_t
	{
		Undefined = 0,
		Integer,
		Pow2,
		FractionalOdd,
		FractionalEven,

		LastEntry
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

	enum class SamplerKind : uint32_t
	{
		Default = 0,
		Comparison,
		Mono,
		Invalid,
	};

	enum class ResourceClass
	{
		SRV = 0,
		UAV,
		CBuffer,
		Sampler,
		Invalid
	};

	enum class ResourceKind : uint32_t
	{
		Invalid = 0,
		Texture1D,
		Texture2D,
		Texture2DMS,
		Texture3D,
		TextureCube,
		Texture1DArray,
		Texture2DArray,
		Texture2DMSArray,
		TextureCubeArray,
		TypedBuffer,
		RawBuffer,
		StructuredBuffer,
		CBuffer,
		Sampler,
		TBuffer,
		NumEntries,
	};

	enum class OpCode : uint32_t
	{
		// Reserved
		GlobalOrderedCountIncReserved = 129, // reserved
		ToDelete1 = 101, // reserved
		ToDelete2 = 102, // reserved
		ToDelete3 = 104, // reserved
		ToDelete4 = 105, // reserved
		ToDelete5 = 76, // reserved
		ToDelete6 = 77, // reserved

		// Binary float
		FMax = 34, // returns the FMax of the input values
		FMin = 35, // returns the FMin of the input values

		// Binary int with carry
		IAddc = 43, // returns the IAddc of the input values
		ISubc = 45, // returns the ISubc of the input values
		UAddc = 44, // returns the UAddc of the input values
		USubc = 46, // returns the USubc of the input values

		// Binary int with two outputs
		IMul = 40, // returns the IMul of the input values
		UDiv = 42, // returns the UDiv of the input values
		UMul = 41, // returns the UMul of the input values

		// Binary int
		IMax = 36, // returns the IMax of the input values
		IMin = 37, // returns the IMin of the input values
		UMax = 38, // returns the UMax of the input values
		UMin = 39, // returns the UMin of the input values

		// Bitcasts with different sizes
		BitcastF16toI16 = 133, // bitcast between different sizes
		BitcastF32toI32 = 135, // bitcast between different sizes
		BitcastF64toI64 = 137, // bitcast between different sizes
		BitcastI16toF16 = 132, // bitcast between different sizes
		BitcastI32toF32 = 134, // bitcast between different sizes
		BitcastI64toF64 = 136, // bitcast between different sizes

		// Compute shader
		FlattenedThreadIdInGroup = 96, // provides a flattened index for a given thread within a given group (SV_GroupIndex)
		GroupId = 94, // reads the group ID (SV_GroupID)
		ThreadId = 93, // reads the thread ID
		ThreadIdInGroup = 95, // reads the thread ID within the group (SV_GroupThreadID)

		// Domain and hull shader
		LoadOutputControlPoint = 106, // LoadOutputControlPoint
		LoadPatchConstant = 107, // LoadPatchConstant

		// Domain shader
		DomainLocation = 108, // DomainLocation

		// Dot
		Dot2 = 55, // two-dimensional vector dot-product
		Dot3 = 56, // three-dimensional vector dot-product
		Dot4 = 57, // four-dimensional vector dot-product

		// Double precision
		LegacyDoubleToFloat = 141, // legacy fuction to convert double to float
		LegacyDoubleToSInt32 = 142, // legacy fuction to convert double to int32
		LegacyDoubleToUInt32 = 143, // legacy fuction to convert double to uint32
		MakeDouble = 100, // creates a double value
		SplitDouble = 103, // splits a double into low and high parts

		// GS
		GSInstanceID = 138, // GSInstanceID

		// Geometry shader
		CutStream = 98, // completes the current primitive topology at the specified stream
		EmitStream = 97, // emits a vertex to a given stream
		EmitThenCutStream = 99, // equivalent to an EmitStream followed by a CutStream

		// Hull shader
		OutputControlPointID = 110, // OutputControlPointID
		PrimitiveID = 111, // PrimitiveID
		StorePatchConstant = 109, // StorePatchConstant

		// Legacy floating-point
		LegacyF16ToF32 = 140, // legacy fuction to convert half (f16) to float (f32) (this is not related to min-precision)
		LegacyF32ToF16 = 139, // legacy fuction to convert float (f32) to half (f16) (this is not related to min-precision)

		// Other
		CycleCounterLegacy = 112, // CycleCounterLegacy

		// Pixel shader
		CalculateLOD = 84, // calculates the level of detail
		Coverage = 147, // returns the coverage mask input in a pixel shader
		DerivCoarseX = 86, // computes the rate of change of components per stamp
		DerivCoarseY = 87, // computes the rate of change of components per stamp
		DerivFineX = 88, // computes the rate of change of components per pixel
		DerivFineY = 89, // computes the rate of change of components per pixel
		Discard = 85, // discard the current pixel
		EvalCentroid = 92, // evaluates an input attribute at pixel center
		EvalSampleIndex = 91, // evaluates an input attribute at a sample location
		EvalSnapped = 90, // evaluates an input attribute at pixel center with an offset
		InnerCoverage = 148, // returns underestimated coverage input from conservative rasterization in a pixel shader
		SampleIndex = 146, // returns the sample index in a sample-frequency pixel shader

		// Quaternary
		Bfi = 54, // given a bit range from the LSB of a number, places that number of bits in another number at any offset

		// Resources - gather
		TextureGather = 74, // gathers the four texels that would be used in a bi-linear filtering operation
		TextureGatherCmp = 75, // same as TextureGather, except this instrution performs comparison on texels, similar to SampleCmp

		// Resources - sample
		RenderTargetGetSampleCount = 80, // gets the number of samples for a render target
		RenderTargetGetSamplePosition = 79, // gets the position of the specified sample
		Sample = 61, // samples a texture
		SampleBias = 62, // samples a texture after applying the input bias to the mipmap level
		SampleCmp = 65, // samples a texture and compares a single component against the specified comparison value
		SampleCmpLevelZero = 66, // samples a texture and compares a single component against the specified comparison value
		SampleGrad = 64, // samples a texture using a gradient to influence the way the sample location is calculated
		SampleLevel = 63, // samples a texture using a mipmap-level offset
		Texture2DMSGetSamplePosition = 78, // gets the position of the specified sample

		// Resources
		BufferLoad = 69, // reads from a TypedBuffer
		BufferStore = 70, // writes to a RWTypedBuffer
		BufferUpdateCounter = 71, // atomically increments/decrements the hidden 32-bit counter stored with a Count or Append UAV
		CBufferLoad = 59, // loads a value from a constant buffer resource
		CBufferLoadLegacy = 60, // loads a value from a constant buffer resource
		CheckAccessFullyMapped = 72, // determines whether all values from a Sample, Gather, or Load operation accessed mapped tiles in a tiled resource
		CreateHandle = 58, // creates the handle to a resource
		GetDimensions = 73, // gets texture size information
		TextureLoad = 67, // reads texel data without any filtering or sampling
		TextureStore = 68, // reads texel data without any filtering or sampling

		// Synchronization
		AtomicBinOp = 81, // performs an atomic operation on two operands
		AtomicCompareExchange = 82, // atomic compare and exchange to memory
		Barrier = 83, // inserts a memory barrier in the shader

		// Temporary, indexable, input, output registers
		LoadInput = 4, // loads the value from shader input
		MinPrecXRegLoad = 2, // helper load operation for minprecision
		MinPrecXRegStore = 3, // helper store operation for minprecision
		StoreOutput = 5, // stores the value to shader output
		TempRegLoad = 0, // helper load operation
		TempRegStore = 1, // helper store operation

		// Tertiary float
		FMad = 47, // performs a fused multiply add (FMA) of the form a * b + c
		Fma = 48, // performs a fused multiply add (FMA) of the form a * b + c

		// Tertiary int
		IMad = 49, // performs an integral IMad
		Ibfe = 52, // performs an integral Ibfe
		Msad = 51, // performs an integral Msad
		UMad = 50, // performs an integral UMad
		Ubfe = 53, // performs an integral Ubfe

		// Unary float - rounding
		Round_ne = 25, // returns the Round_ne
		Round_ni = 26, // returns the Round_ni
		Round_pi = 27, // returns the Round_pi
		Round_z = 28, // returns the Round_z

		// Unary float
		Acos = 15, // returns the Acos
		Asin = 16, // returns the Asin
		Atan = 17, // returns the Atan
		Cos = 12, // returns cosine(theta) for theta in radians.
		Exp = 20, // returns the Exp
		FAbs = 6, // returns the absolute value of the input value.
		Frc = 21, // returns the Frc
		Hcos = 18, // returns the Hcos
		Hsin = 19, // returns the Hsin
		Htan = 113, // returns the hyperbolic tangent of the specified value
		IsFinite = 10, // returns the IsFinite
		IsInf = 9, // returns the IsInf
		IsNaN = 8, // returns the IsNaN
		IsNormal = 11, // returns the IsNormal
		Log = 22, // returns the Log
		Rsqrt = 24, // returns the Rsqrt
		Saturate = 7, // clamps the result of a single or double precision floating point value to [0.0f...1.0f]
		Sin = 13, // returns the Sin
		Sqrt = 23, // returns the Sqrt
		Tan = 14, // returns the Tan

		// Unary int
		Bfrev = 29, // returns the reverse bit pattern of the input value
		Countbits = 30, // returns the Countbits
		FirstbitHi = 32, // returns src != 0? (BitWidth-1 - FirstbitHi) : -1
		FirstbitLo = 31, // returns the FirstbitLo
		FirstbitSHi = 33, // returns src != 0? (BitWidth-1 - FirstbitSHi) : -1

		// Wave
		QuadOp = 131, // returns the result of a quad-level operation
		QuadReadLaneAt = 130, // reads from a lane in the quad
		WaveActiveAllEqual = 121, // returns 1 if all the lanes have the same value
		WaveActiveBallot = 122, // returns a struct with a bit set for each lane where the condition is true
		WaveActiveBit = 126, // returns the result of the operation across all lanes
		WaveActiveOp = 125, // returns the result the operation across waves
		WaveAllBitCount = 144, // returns the count of bits set to 1 across the wave
		WaveAllTrue = 120, // returns 1 if all the lanes evaluate the value to true
		WaveAnyTrue = 119, // returns 1 if any of the lane evaluates the value to true
		WaveCaptureReserved = 114, // reserved
		WaveGetLaneCount = 117, // returns the number of lanes in the wave
		WaveGetLaneIndex = 116, // returns the index of the current lane in the wave
		WaveGetOrderedIndex = 128, // reserved
		WaveIsFirstLane = 115, // returns 1 for the first lane in the wave
		WaveIsHelperLaneReserved = 118, // reserved
		WavePrefixBitCount = 145, // returns the count of bits set to 1 on prior lanes
		WavePrefixOp = 127, // returns the result of the operation on prior lanes
		WaveReadLaneAt = 123, // returns the value from the specified lane
		WaveReadLaneFirst = 124, // returns the value from the first lane

		NumOpCodes = 149 // exclusive last value of enumeration
	};

	// Groups for DXIL operations with equivalent function templates
	enum class OpCodeClass : uint32_t
	{
		// 
		Reserved,

		// Binary int with carry
		BinaryWithCarry,

		// Binary int with two outputs
		BinaryWithTwoOuts,

		// Binary int
		Binary,

		// Bitcasts with different sizes
		BitcastF16toI16,
		BitcastF32toI32,
		BitcastF64toI64,
		BitcastI16toF16,
		BitcastI32toF32,
		BitcastI64toF64,

		// Compute shader
		FlattenedThreadIdInGroup,
		GroupId,
		ThreadId,
		ThreadIdInGroup,

		// Domain and hull shader
		LoadOutputControlPoint,
		LoadPatchConstant,

		// Domain shader
		DomainLocation,

		// Dot
		Dot2,
		Dot3,
		Dot4,

		// Double precision
		LegacyDoubleToFloat,
		LegacyDoubleToSInt32,
		LegacyDoubleToUInt32,
		MakeDouble,
		SplitDouble,

		// GS
		GSInstanceID,

		// Geometry shader
		CutStream,
		EmitStream,
		EmitThenCutStream,

		// Hull shader
		OutputControlPointID,
		PrimitiveID,
		StorePatchConstant,

		// LLVM Instructions
		LlvmInst,

		// Legacy floating-point
		LegacyF16ToF32,
		LegacyF32ToF16,

		// Other
		CycleCounterLegacy,

		// Pixel shader
		CalculateLOD,
		Coverage,
		Discard,
		EvalCentroid,
		EvalSampleIndex,
		EvalSnapped,
		InnerCoverage,
		SampleIndex,

		// Quaternary
		Quaternary,

		// Resources - gather
		TextureGather,
		TextureGatherCmp,

		// Resources - sample
		RenderTargetGetSampleCount,
		RenderTargetGetSamplePosition,
		Sample,
		SampleBias,
		SampleCmp,
		SampleCmpLevelZero,
		SampleGrad,
		SampleLevel,
		Texture2DMSGetSamplePosition,

		// Resources
		BufferLoad,
		BufferStore,
		BufferUpdateCounter,
		CBufferLoad,
		CBufferLoadLegacy,
		CheckAccessFullyMapped,
		CreateHandle,
		GetDimensions,
		TextureLoad,
		TextureStore,

		// Synchronization
		AtomicBinOp,
		AtomicCompareExchange,
		Barrier,

		// Temporary, indexable, input, output registers
		LoadInput,
		MinPrecXRegLoad,
		MinPrecXRegStore,
		StoreOutput,
		TempRegLoad,
		TempRegStore,

		// Tertiary int
		Tertiary,

		// Unary float
		IsSpecialFloat,
		Unary,

		// Unary int
		UnaryBits,

		// Wave
		QuadOp,
		QuadReadLaneAt,
		WaveActiveAllEqual,
		WaveActiveBallot,
		WaveActiveBit,
		WaveActiveOp,
		WaveAllOp,
		WaveAllTrue,
		WaveAnyTrue,
		WaveGetLaneCount,
		WaveGetLaneIndex,
		WaveIsFirstLane,
		WavePrefixOp,
		WaveReadLaneAt,
		WaveReadLaneFirst,

		NumOpClasses = 94 // exclusive last value of enumeration
	};
}

#endif		// _DILITHIUM_DXIL_CONSTANTS_HPP
