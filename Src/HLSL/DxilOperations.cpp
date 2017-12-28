/**
 * @file DxilOperations.cpp
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
#include <Dilithium/dxc/HLSL/DxilOperations.hpp>

#include <Dilithium/ArrayRef.hpp>
#include <Dilithium/LLVMContext.hpp>
#include <Dilithium/LLVMModule.hpp>
#include <Dilithium/Type.hpp>
#include <Dilithium/Constants.hpp>
#include <Dilithium/Instructions.hpp>

#include <iterator>

#include <boost/assert.hpp>

namespace Dilithium
{
	//------------------------------------------------------------------------------
	//
	//  OP class const-static data and related static methods.
	//
	/* <py>
	import hctdb_instrhelp
	</py> */
	/* <py::lines('OPCODE-OLOADS')>hctdb_instrhelp.get_oloads_props()</py>*/
	// OPCODE-OLOADS:BEGIN
	const OP::OpCodeProperty OP::op_code_props_[] =
	{
		//   OpCode                           OpCode name,                OpCodeClass                            OpCodeClass name,              void,     h,     f,     d,    i1,    i8,   i16,   i32,   i64  function attribute
		  // Temporary, indexable, input, output registers                                                                                      void,     h,     f,     d,    i1,    i8,   i16,   i32,   i64  function attribute
		  {  OpCode::TempRegLoad,             "TempRegLoad",              OpCodeClass::TempRegLoad,              "tempRegLoad",                false,  true,  true, false, false, false,  true,  true, false, Attribute::AK_ReadOnly, },
		  {  OpCode::TempRegStore,            "TempRegStore",             OpCodeClass::TempRegStore,             "tempRegStore",               false,  true,  true, false, false, false,  true,  true, false, Attribute::AK_None,     },
		  {  OpCode::MinPrecXRegLoad,         "MinPrecXRegLoad",          OpCodeClass::MinPrecXRegLoad,          "minPrecXRegLoad",            false,  true, false, false, false, false,  true, false, false, Attribute::AK_ReadOnly, },
		  {  OpCode::MinPrecXRegStore,        "MinPrecXRegStore",         OpCodeClass::MinPrecXRegStore,         "minPrecXRegStore",           false,  true, false, false, false, false,  true, false, false, Attribute::AK_None,     },
		  {  OpCode::LoadInput,               "LoadInput",                OpCodeClass::LoadInput,                "loadInput",                  false,  true,  true, false, false, false,  true,  true, false, Attribute::AK_ReadNone, },
		  {  OpCode::StoreOutput,             "StoreOutput",              OpCodeClass::StoreOutput,              "storeOutput",                false,  true,  true, false, false, false,  true,  true, false, Attribute::AK_None,     },

		  // Unary float                                                                                                                        void,     h,     f,     d,    i1,    i8,   i16,   i32,   i64  function attribute
		  {  OpCode::FAbs,                    "FAbs",                     OpCodeClass::Unary,                    "unary",                      false,  true,  true,  true, false, false, false, false, false, Attribute::AK_ReadNone, },
		  {  OpCode::Saturate,                "Saturate",                 OpCodeClass::Unary,                    "unary",                      false,  true,  true,  true, false, false, false, false, false, Attribute::AK_ReadNone, },
		  {  OpCode::IsNaN,                   "IsNaN",                    OpCodeClass::IsSpecialFloat,           "isSpecialFloat",             false,  true,  true, false, false, false, false, false, false, Attribute::AK_ReadNone, },
		  {  OpCode::IsInf,                   "IsInf",                    OpCodeClass::IsSpecialFloat,           "isSpecialFloat",             false,  true,  true, false, false, false, false, false, false, Attribute::AK_ReadNone, },
		  {  OpCode::IsFinite,                "IsFinite",                 OpCodeClass::IsSpecialFloat,           "isSpecialFloat",             false,  true,  true, false, false, false, false, false, false, Attribute::AK_ReadNone, },
		  {  OpCode::IsNormal,                "IsNormal",                 OpCodeClass::IsSpecialFloat,           "isSpecialFloat",             false,  true,  true, false, false, false, false, false, false, Attribute::AK_ReadNone, },
		  {  OpCode::Cos,                     "Cos",                      OpCodeClass::Unary,                    "unary",                      false,  true,  true, false, false, false, false, false, false, Attribute::AK_ReadNone, },
		  {  OpCode::Sin,                     "Sin",                      OpCodeClass::Unary,                    "unary",                      false,  true,  true, false, false, false, false, false, false, Attribute::AK_ReadNone, },
		  {  OpCode::Tan,                     "Tan",                      OpCodeClass::Unary,                    "unary",                      false,  true,  true, false, false, false, false, false, false, Attribute::AK_ReadNone, },
		  {  OpCode::Acos,                    "Acos",                     OpCodeClass::Unary,                    "unary",                      false,  true,  true, false, false, false, false, false, false, Attribute::AK_ReadNone, },
		  {  OpCode::Asin,                    "Asin",                     OpCodeClass::Unary,                    "unary",                      false,  true,  true, false, false, false, false, false, false, Attribute::AK_ReadNone, },
		  {  OpCode::Atan,                    "Atan",                     OpCodeClass::Unary,                    "unary",                      false,  true,  true, false, false, false, false, false, false, Attribute::AK_ReadNone, },
		  {  OpCode::Hcos,                    "Hcos",                     OpCodeClass::Unary,                    "unary",                      false,  true,  true, false, false, false, false, false, false, Attribute::AK_ReadNone, },
		  {  OpCode::Hsin,                    "Hsin",                     OpCodeClass::Unary,                    "unary",                      false,  true,  true, false, false, false, false, false, false, Attribute::AK_ReadNone, },
		  {  OpCode::Exp,                     "Exp",                      OpCodeClass::Unary,                    "unary",                      false,  true,  true, false, false, false, false, false, false, Attribute::AK_ReadNone, },
		  {  OpCode::Frc,                     "Frc",                      OpCodeClass::Unary,                    "unary",                      false,  true,  true, false, false, false, false, false, false, Attribute::AK_ReadNone, },
		  {  OpCode::Log,                     "Log",                      OpCodeClass::Unary,                    "unary",                      false,  true,  true, false, false, false, false, false, false, Attribute::AK_ReadNone, },
		  {  OpCode::Sqrt,                    "Sqrt",                     OpCodeClass::Unary,                    "unary",                      false,  true,  true, false, false, false, false, false, false, Attribute::AK_ReadNone, },
		  {  OpCode::Rsqrt,                   "Rsqrt",                    OpCodeClass::Unary,                    "unary",                      false,  true,  true, false, false, false, false, false, false, Attribute::AK_ReadNone, },

		  // Unary float - rounding                                                                                                             void,     h,     f,     d,    i1,    i8,   i16,   i32,   i64  function attribute
		  {  OpCode::Round_ne,                "Round_ne",                 OpCodeClass::Unary,                    "unary",                      false,  true,  true, false, false, false, false, false, false, Attribute::AK_ReadNone, },
		  {  OpCode::Round_ni,                "Round_ni",                 OpCodeClass::Unary,                    "unary",                      false,  true,  true, false, false, false, false, false, false, Attribute::AK_ReadNone, },
		  {  OpCode::Round_pi,                "Round_pi",                 OpCodeClass::Unary,                    "unary",                      false,  true,  true, false, false, false, false, false, false, Attribute::AK_ReadNone, },
		  {  OpCode::Round_z,                 "Round_z",                  OpCodeClass::Unary,                    "unary",                      false,  true,  true, false, false, false, false, false, false, Attribute::AK_ReadNone, },

		  // Unary int                                                                                                                          void,     h,     f,     d,    i1,    i8,   i16,   i32,   i64  function attribute
		  {  OpCode::Bfrev,                   "Bfrev",                    OpCodeClass::Unary,                    "unary",                      false, false, false, false, false, false,  true,  true,  true, Attribute::AK_ReadNone, },
		  {  OpCode::Countbits,               "Countbits",                OpCodeClass::UnaryBits,                "unaryBits",                  false, false, false, false, false, false,  true,  true,  true, Attribute::AK_ReadNone, },
		  {  OpCode::FirstbitLo,              "FirstbitLo",               OpCodeClass::UnaryBits,                "unaryBits",                  false, false, false, false, false, false,  true,  true,  true, Attribute::AK_ReadNone, },
		  {  OpCode::FirstbitHi,              "FirstbitHi",               OpCodeClass::UnaryBits,                "unaryBits",                  false, false, false, false, false, false,  true,  true,  true, Attribute::AK_ReadNone, },
		  {  OpCode::FirstbitSHi,             "FirstbitSHi",              OpCodeClass::UnaryBits,                "unaryBits",                  false, false, false, false, false, false,  true,  true,  true, Attribute::AK_ReadNone, },

		  // Binary float                                                                                                                      void,     h,     f,     d,    i1,    i8,   i16,   i32,   i64  function attribute
		  {  OpCode::FMax,                    "FMax",                     OpCodeClass::Binary,                   "binary",                     false,  true,  true,  true, false, false, false, false, false, Attribute::AK_ReadNone, },
		  {  OpCode::FMin,                    "FMin",                     OpCodeClass::Binary,                   "binary",                     false,  true,  true,  true, false, false, false, false, false, Attribute::AK_ReadNone, },

		  // Binary int                                                                                                                        void,     h,     f,     d,    i1,    i8,   i16,   i32,   i64  function attribute
		  {  OpCode::IMax,                    "IMax",                     OpCodeClass::Binary,                   "binary",                     false, false, false, false, false, false,  true,  true,  true, Attribute::AK_ReadNone, },
		  {  OpCode::IMin,                    "IMin",                     OpCodeClass::Binary,                   "binary",                     false, false, false, false, false, false,  true,  true,  true, Attribute::AK_ReadNone, },
		  {  OpCode::UMax,                    "UMax",                     OpCodeClass::Binary,                   "binary",                     false, false, false, false, false, false,  true,  true,  true, Attribute::AK_ReadNone, },
		  {  OpCode::UMin,                    "UMin",                     OpCodeClass::Binary,                   "binary",                     false, false, false, false, false, false,  true,  true,  true, Attribute::AK_ReadNone, },

		  // Binary int with two outputs                                                                                                       void,     h,     f,     d,    i1,    i8,   i16,   i32,   i64  function attribute
		  {  OpCode::IMul,                    "IMul",                     OpCodeClass::BinaryWithTwoOuts,        "binaryWithTwoOuts",          false, false, false, false, false, false, false,  true, false, Attribute::AK_ReadNone, },
		  {  OpCode::UMul,                    "UMul",                     OpCodeClass::BinaryWithTwoOuts,        "binaryWithTwoOuts",          false, false, false, false, false, false, false,  true, false, Attribute::AK_ReadNone, },
		  {  OpCode::UDiv,                    "UDiv",                     OpCodeClass::BinaryWithTwoOuts,        "binaryWithTwoOuts",          false, false, false, false, false, false, false,  true, false, Attribute::AK_ReadNone, },

		  // Binary int with carry                                                                                                             void,     h,     f,     d,    i1,    i8,   i16,   i32,   i64  function attribute
		  {  OpCode::IAddc,                   "IAddc",                    OpCodeClass::BinaryWithCarry,          "binaryWithCarry",            false, false, false, false, false, false, false,  true, false, Attribute::AK_ReadNone, },
		  {  OpCode::UAddc,                   "UAddc",                    OpCodeClass::BinaryWithCarry,          "binaryWithCarry",            false, false, false, false, false, false, false,  true, false, Attribute::AK_ReadNone, },
		  {  OpCode::ISubc,                   "ISubc",                    OpCodeClass::BinaryWithCarry,          "binaryWithCarry",            false, false, false, false, false, false, false,  true, false, Attribute::AK_ReadNone, },
		  {  OpCode::USubc,                   "USubc",                    OpCodeClass::BinaryWithCarry,          "binaryWithCarry",            false, false, false, false, false, false, false,  true, false, Attribute::AK_ReadNone, },

		  // Tertiary float                                                                                                                    void,     h,     f,     d,    i1,    i8,   i16,   i32,   i64  function attribute
		  {  OpCode::FMad,                    "FMad",                     OpCodeClass::Tertiary,                 "tertiary",                   false,  true,  true,  true, false, false, false, false, false, Attribute::AK_ReadNone, },
		  {  OpCode::Fma,                     "Fma",                      OpCodeClass::Tertiary,                 "tertiary",                   false, false, false,  true, false, false, false, false, false, Attribute::AK_ReadNone, },

		  // Tertiary int                                                                                                                      void,     h,     f,     d,    i1,    i8,   i16,   i32,   i64  function attribute
		  {  OpCode::IMad,                    "IMad",                     OpCodeClass::Tertiary,                 "tertiary",                   false, false, false, false, false, false,  true,  true,  true, Attribute::AK_ReadNone, },
		  {  OpCode::UMad,                    "UMad",                     OpCodeClass::Tertiary,                 "tertiary",                   false, false, false, false, false, false,  true,  true,  true, Attribute::AK_ReadNone, },
		  {  OpCode::Msad,                    "Msad",                     OpCodeClass::Tertiary,                 "tertiary",                   false, false, false, false, false, false, false,  true,  true, Attribute::AK_ReadNone, },
		  {  OpCode::Ibfe,                    "Ibfe",                     OpCodeClass::Tertiary,                 "tertiary",                   false, false, false, false, false, false, false,  true,  true, Attribute::AK_ReadNone, },
		  {  OpCode::Ubfe,                    "Ubfe",                     OpCodeClass::Tertiary,                 "tertiary",                   false, false, false, false, false, false, false,  true,  true, Attribute::AK_ReadNone, },

		  // Quaternary                                                                                                                        void,     h,     f,     d,    i1,    i8,   i16,   i32,   i64  function attribute
		  {  OpCode::Bfi,                     "Bfi",                      OpCodeClass::Quaternary,               "quaternary",                 false, false, false, false, false, false, false,  true, false, Attribute::AK_ReadNone, },

		  // Dot                                                                                                                               void,     h,     f,     d,    i1,    i8,   i16,   i32,   i64  function attribute
		  {  OpCode::Dot2,                    "Dot2",                     OpCodeClass::Dot2,                     "dot2",                       false,  true,  true, false, false, false, false, false, false, Attribute::AK_ReadNone, },
		  {  OpCode::Dot3,                    "Dot3",                     OpCodeClass::Dot3,                     "dot3",                       false,  true,  true, false, false, false, false, false, false, Attribute::AK_ReadNone, },
		  {  OpCode::Dot4,                    "Dot4",                     OpCodeClass::Dot4,                     "dot4",                       false,  true,  true, false, false, false, false, false, false, Attribute::AK_ReadNone, },

		  // Resources                                                                                                                         void,     h,     f,     d,    i1,    i8,   i16,   i32,   i64  function attribute
		  {  OpCode::CreateHandle,            "CreateHandle",             OpCodeClass::CreateHandle,             "createHandle",                true, false, false, false, false, false, false, false, false, Attribute::AK_ReadOnly, },
		  {  OpCode::CBufferLoad,             "CBufferLoad",              OpCodeClass::CBufferLoad,              "cbufferLoad",                false,  true,  true,  true, false,  true,  true,  true,  true, Attribute::AK_ReadOnly, },
		  {  OpCode::CBufferLoadLegacy,       "CBufferLoadLegacy",        OpCodeClass::CBufferLoadLegacy,        "cbufferLoadLegacy",          false,  true,  true,  true, false, false,  true,  true, false, Attribute::AK_ReadOnly, },

		  // Resources - sample                                                                                                                void,     h,     f,     d,    i1,    i8,   i16,   i32,   i64  function attribute
		  {  OpCode::Sample,                  "Sample",                   OpCodeClass::Sample,                   "sample",                     false,  true,  true, false, false, false, false, false, false, Attribute::AK_ReadOnly, },
		  {  OpCode::SampleBias,              "SampleBias",               OpCodeClass::SampleBias,               "sampleBias",                 false,  true,  true, false, false, false, false, false, false, Attribute::AK_ReadOnly, },
		  {  OpCode::SampleLevel,             "SampleLevel",              OpCodeClass::SampleLevel,              "sampleLevel",                false,  true,  true, false, false, false, false, false, false, Attribute::AK_ReadOnly, },
		  {  OpCode::SampleGrad,              "SampleGrad",               OpCodeClass::SampleGrad,               "sampleGrad",                 false,  true,  true, false, false, false, false, false, false, Attribute::AK_ReadOnly, },
		  {  OpCode::SampleCmp,               "SampleCmp",                OpCodeClass::SampleCmp,                "sampleCmp",                  false,  true,  true, false, false, false, false, false, false, Attribute::AK_ReadOnly, },
		  {  OpCode::SampleCmpLevelZero,      "SampleCmpLevelZero",       OpCodeClass::SampleCmpLevelZero,       "sampleCmpLevelZero",         false,  true,  true, false, false, false, false, false, false, Attribute::AK_ReadOnly, },

		  // Resources                                                                                                                         void,     h,     f,     d,    i1,    i8,   i16,   i32,   i64  function attribute
		  {  OpCode::TextureLoad,             "TextureLoad",              OpCodeClass::TextureLoad,              "textureLoad",                false,  true,  true, false, false, false,  true,  true, false, Attribute::AK_ReadOnly, },
		  {  OpCode::TextureStore,            "TextureStore",             OpCodeClass::TextureStore,             "textureStore",               false,  true,  true, false, false, false,  true,  true, false, Attribute::AK_None,     },
		  {  OpCode::BufferLoad,              "BufferLoad",               OpCodeClass::BufferLoad,               "bufferLoad",                 false,  true,  true, false, false, false,  true,  true,  true, Attribute::AK_ReadOnly, },
		  {  OpCode::BufferStore,             "BufferStore",              OpCodeClass::BufferStore,              "bufferStore",                false,  true,  true, false, false, false,  true,  true,  true, Attribute::AK_None,     },
		  {  OpCode::BufferUpdateCounter,     "BufferUpdateCounter",      OpCodeClass::BufferUpdateCounter,      "bufferUpdateCounter",         true, false, false, false, false, false, false, false, false, Attribute::AK_None,     },
		  {  OpCode::CheckAccessFullyMapped,  "CheckAccessFullyMapped",   OpCodeClass::CheckAccessFullyMapped,   "checkAccessFullyMapped",     false, false, false, false, false, false, false,  true, false, Attribute::AK_ReadOnly, },
		  {  OpCode::GetDimensions,           "GetDimensions",            OpCodeClass::GetDimensions,            "getDimensions",               true, false, false, false, false, false, false, false, false, Attribute::AK_ReadOnly, },

		  // Resources - gather                                                                                                                void,     h,     f,     d,    i1,    i8,   i16,   i32,   i64  function attribute
		  {  OpCode::TextureGather,           "TextureGather",            OpCodeClass::TextureGather,            "textureGather",              false, false,  true, false, false, false, false,  true, false, Attribute::AK_ReadOnly, },
		  {  OpCode::TextureGatherCmp,        "TextureGatherCmp",         OpCodeClass::TextureGatherCmp,         "textureGatherCmp",           false, false,  true, false, false, false, false,  true, false, Attribute::AK_ReadOnly, },

		  //                                                                                                                                    void,     h,     f,     d,    i1,    i8,   i16,   i32,   i64  function attribute
		  {  OpCode::ToDelete5,               "ToDelete5",                OpCodeClass::Reserved,                 "reserved",                    true, false, false, false, false, false, false, false, false, Attribute::AK_None,     },
		  {  OpCode::ToDelete6,               "ToDelete6",                OpCodeClass::Reserved,                 "reserved",                    true, false, false, false, false, false, false, false, false, Attribute::AK_None,     },

		  // Resources - sample                                                                                                                                     void,     h,     f,     d,    i1,    i8,   i16,   i32,   i64  function attribute
		  {  OpCode::Texture2DMSGetSamplePosition, "Texture2DMSGetSamplePosition", OpCodeClass::Texture2DMSGetSamplePosition, "texture2DMSGetSamplePosition",       true, false, false, false, false, false, false, false, false, Attribute::AK_ReadOnly, },
		  {  OpCode::RenderTargetGetSamplePosition, "RenderTargetGetSamplePosition", OpCodeClass::RenderTargetGetSamplePosition, "renderTargetGetSamplePosition",   true, false, false, false, false, false, false, false, false, Attribute::AK_ReadOnly, },
		  {  OpCode::RenderTargetGetSampleCount, "RenderTargetGetSampleCount", OpCodeClass::RenderTargetGetSampleCount, "renderTargetGetSampleCount",               true, false, false, false, false, false, false, false, false, Attribute::AK_ReadOnly, },

		  // Synchronization                                                                                                                    void,     h,     f,     d,    i1,    i8,   i16,   i32,   i64  function attribute
		  {  OpCode::AtomicBinOp,             "AtomicBinOp",              OpCodeClass::AtomicBinOp,              "atomicBinOp",                false, false, false, false, false, false, false,  true, false, Attribute::AK_None,     },
		  {  OpCode::AtomicCompareExchange,   "AtomicCompareExchange",    OpCodeClass::AtomicCompareExchange,    "atomicCompareExchange",      false, false, false, false, false, false, false,  true, false, Attribute::AK_None,     },
		  {  OpCode::Barrier,                 "Barrier",                  OpCodeClass::Barrier,                  "barrier",                     true, false, false, false, false, false, false, false, false, Attribute::AK_None,     },

		  // Pixel shader                                                                                                                       void,     h,     f,     d,    i1,    i8,   i16,   i32,   i64  function attribute
		  {  OpCode::CalculateLOD,            "CalculateLOD",             OpCodeClass::CalculateLOD,             "calculateLOD",               false, false,  true, false, false, false, false, false, false, Attribute::AK_ReadOnly, },
		  {  OpCode::Discard,                 "Discard",                  OpCodeClass::Discard,                  "discard",                     true, false, false, false, false, false, false, false, false, Attribute::AK_None,     },
		  {  OpCode::DerivCoarseX,            "DerivCoarseX",             OpCodeClass::Unary,                    "unary",                      false,  true,  true, false, false, false, false, false, false, Attribute::AK_ReadNone, },
		  {  OpCode::DerivCoarseY,            "DerivCoarseY",             OpCodeClass::Unary,                    "unary",                      false,  true,  true, false, false, false, false, false, false, Attribute::AK_ReadNone, },
		  {  OpCode::DerivFineX,              "DerivFineX",               OpCodeClass::Unary,                    "unary",                      false,  true,  true, false, false, false, false, false, false, Attribute::AK_ReadNone, },
		  {  OpCode::DerivFineY,              "DerivFineY",               OpCodeClass::Unary,                    "unary",                      false,  true,  true, false, false, false, false, false, false, Attribute::AK_ReadNone, },
		  {  OpCode::EvalSnapped,             "EvalSnapped",              OpCodeClass::EvalSnapped,              "evalSnapped",                false,  true,  true, false, false, false, false, false, false, Attribute::AK_ReadNone, },
		  {  OpCode::EvalSampleIndex,         "EvalSampleIndex",          OpCodeClass::EvalSampleIndex,          "evalSampleIndex",            false,  true,  true, false, false, false, false, false, false, Attribute::AK_ReadNone, },
		  {  OpCode::EvalCentroid,            "EvalCentroid",             OpCodeClass::EvalCentroid,             "evalCentroid",               false,  true,  true, false, false, false, false, false, false, Attribute::AK_ReadNone, },

		  // Compute shader                                                                                                                     void,     h,     f,     d,    i1,    i8,   i16,   i32,   i64  function attribute
		  {  OpCode::ThreadId,                "ThreadId",                 OpCodeClass::ThreadId,                 "threadId",                   false, false, false, false, false, false, false,  true, false, Attribute::AK_ReadNone, },
		  {  OpCode::GroupId,                 "GroupId",                  OpCodeClass::GroupId,                  "groupId",                    false, false, false, false, false, false, false,  true, false, Attribute::AK_ReadNone, },
		  {  OpCode::ThreadIdInGroup,         "ThreadIdInGroup",          OpCodeClass::ThreadIdInGroup,          "threadIdInGroup",            false, false, false, false, false, false, false,  true, false, Attribute::AK_ReadNone, },
		  {  OpCode::FlattenedThreadIdInGroup, "FlattenedThreadIdInGroup", OpCodeClass::FlattenedThreadIdInGroup, "flattenedThreadIdInGroup",   false, false, false, false, false, false, false,  true, false, Attribute::AK_ReadNone, },

		  // Geometry shader                                                                                                                    void,     h,     f,     d,    i1,    i8,   i16,   i32,   i64  function attribute
		  {  OpCode::EmitStream,              "EmitStream",               OpCodeClass::EmitStream,               "emitStream",                  true, false, false, false, false, false, false, false, false, Attribute::AK_None,     },
		  {  OpCode::CutStream,               "CutStream",                OpCodeClass::CutStream,                "cutStream",                   true, false, false, false, false, false, false, false, false, Attribute::AK_None,     },
		  {  OpCode::EmitThenCutStream,       "EmitThenCutStream",        OpCodeClass::EmitThenCutStream,        "emitThenCutStream",           true, false, false, false, false, false, false, false, false, Attribute::AK_None,     },

		  // Double precision                                                                                                                   void,     h,     f,     d,    i1,    i8,   i16,   i32,   i64  function attribute
		  {  OpCode::MakeDouble,              "MakeDouble",               OpCodeClass::MakeDouble,               "makeDouble",                 false, false, false,  true, false, false, false, false, false, Attribute::AK_ReadNone, },

		  //                                                                                                                                    void,     h,     f,     d,    i1,    i8,   i16,   i32,   i64  function attribute
		  {  OpCode::ToDelete1,               "ToDelete1",                OpCodeClass::Reserved,                 "reserved",                    true, false, false, false, false, false, false, false, false, Attribute::AK_None,     },
		  {  OpCode::ToDelete2,               "ToDelete2",                OpCodeClass::Reserved,                 "reserved",                    true, false, false, false, false, false, false, false, false, Attribute::AK_None,     },

		  // Double precision                                                                                                                   void,     h,     f,     d,    i1,    i8,   i16,   i32,   i64  function attribute
		  {  OpCode::SplitDouble,             "SplitDouble",              OpCodeClass::SplitDouble,              "splitDouble",                false, false, false,  true, false, false, false, false, false, Attribute::AK_ReadNone, },

		  //                                                                                                                                    void,     h,     f,     d,    i1,    i8,   i16,   i32,   i64  function attribute
		  {  OpCode::ToDelete3,               "ToDelete3",                OpCodeClass::Reserved,                 "reserved",                    true, false, false, false, false, false, false, false, false, Attribute::AK_None,     },
		  {  OpCode::ToDelete4,               "ToDelete4",                OpCodeClass::Reserved,                 "reserved",                    true, false, false, false, false, false, false, false, false, Attribute::AK_None,     },

		  // Domain and hull shader                                                                                                             void,     h,     f,     d,    i1,    i8,   i16,   i32,   i64  function attribute
		  {  OpCode::LoadOutputControlPoint,  "LoadOutputControlPoint",   OpCodeClass::LoadOutputControlPoint,   "loadOutputControlPoint",     false,  true,  true, false, false, false,  true,  true, false, Attribute::AK_ReadNone, },
		  {  OpCode::LoadPatchConstant,       "LoadPatchConstant",        OpCodeClass::LoadPatchConstant,        "loadPatchConstant",          false,  true,  true, false, false, false,  true,  true, false, Attribute::AK_ReadNone, },

		  // Domain shader                                                                                                                      void,     h,     f,     d,    i1,    i8,   i16,   i32,   i64  function attribute
		  {  OpCode::DomainLocation,          "DomainLocation",           OpCodeClass::DomainLocation,           "domainLocation",             false, false,  true, false, false, false, false, false, false, Attribute::AK_ReadNone, },

		  // Hull shader                                                                                                                        void,     h,     f,     d,    i1,    i8,   i16,   i32,   i64  function attribute
		  {  OpCode::StorePatchConstant,      "StorePatchConstant",       OpCodeClass::StorePatchConstant,       "storePatchConstant",         false,  true,  true, false, false, false,  true,  true, false, Attribute::AK_None,     },
		  {  OpCode::OutputControlPointID,    "OutputControlPointID",     OpCodeClass::OutputControlPointID,     "outputControlPointID",       false, false, false, false, false, false, false,  true, false, Attribute::AK_ReadNone, },
		  {  OpCode::PrimitiveID,             "PrimitiveID",              OpCodeClass::PrimitiveID,              "primitiveID",                false, false, false, false, false, false, false,  true, false, Attribute::AK_ReadNone, },

		  // Other                                                                                                                              void,     h,     f,     d,    i1,    i8,   i16,   i32,   i64  function attribute
		  {  OpCode::CycleCounterLegacy,      "CycleCounterLegacy",       OpCodeClass::CycleCounterLegacy,       "cycleCounterLegacy",          true, false, false, false, false, false, false, false, false, Attribute::AK_ReadNone, },

		  // Unary float                                                                                                                        void,     h,     f,     d,    i1,    i8,   i16,   i32,   i64  function attribute
		  {  OpCode::Htan,                    "Htan",                     OpCodeClass::Unary,                    "unary",                      false,  true,  true, false, false, false, false, false, false, Attribute::AK_ReadNone, },

		  // Wave                                                                                                                               void,     h,     f,     d,    i1,    i8,   i16,   i32,   i64  function attribute
		  {  OpCode::WaveCaptureReserved,      "WaveCaptureReserved",      OpCodeClass::Reserved,                "reserved",                    true, false, false, false, false, false, false, false, false, Attribute::AK_None,     },
		  {  OpCode::WaveIsFirstLane,          "WaveIsFirstLane",          OpCodeClass::WaveIsFirstLane,         "waveIsFirstLane",             true, false, false, false, false, false, false, false, false, Attribute::AK_ReadOnly, },
		  {  OpCode::WaveGetLaneIndex,         "WaveGetLaneIndex",         OpCodeClass::WaveGetLaneIndex,        "waveGetLaneIndex",            true, false, false, false, false, false, false, false, false, Attribute::AK_ReadOnly, },
		  {  OpCode::WaveGetLaneCount,         "WaveGetLaneCount",         OpCodeClass::WaveGetLaneCount,        "waveGetLaneCount",            true, false, false, false, false, false, false, false, false, Attribute::AK_ReadOnly, },
		  {  OpCode::WaveIsHelperLaneReserved, "WaveIsHelperLaneReserved", OpCodeClass::Reserved,                "reserved",                    true, false, false, false, false, false, false, false, false, Attribute::AK_None,     },
		  {  OpCode::WaveAnyTrue,              "WaveAnyTrue",              OpCodeClass::WaveAnyTrue,             "waveAnyTrue",                 true, false, false, false, false, false, false, false, false, Attribute::AK_ReadOnly, },
		  {  OpCode::WaveAllTrue,              "WaveAllTrue",              OpCodeClass::WaveAllTrue,             "waveAllTrue",                 true, false, false, false, false, false, false, false, false, Attribute::AK_ReadOnly, },
		  {  OpCode::WaveActiveAllEqual,       "WaveActiveAllEqual",       OpCodeClass::WaveActiveAllEqual,      "waveActiveAllEqual",         false,  true,  true,  true,  true,  true,  true,  true,  true, Attribute::AK_ReadOnly, },
		  {  OpCode::WaveActiveBallot,         "WaveActiveBallot",         OpCodeClass::WaveActiveBallot,        "waveActiveBallot",            true, false, false, false, false, false, false, false, false, Attribute::AK_ReadOnly, },
		  {  OpCode::WaveReadLaneAt,           "WaveReadLaneAt",           OpCodeClass::WaveReadLaneAt,          "waveReadLaneAt",             false,  true,  true,  true,  true,  true,  true,  true,  true, Attribute::AK_ReadOnly, },
		  {  OpCode::WaveReadLaneFirst,        "WaveReadLaneFirst",        OpCodeClass::WaveReadLaneFirst,       "waveReadLaneFirst",          false,  true,  true, false,  true,  true,  true,  true,  true, Attribute::AK_ReadOnly, },
		  {  OpCode::WaveActiveOp,             "WaveActiveOp",             OpCodeClass::WaveActiveOp,            "waveActiveOp",               false,  true,  true,  true,  true,  true,  true,  true,  true, Attribute::AK_ReadOnly, },
		  {  OpCode::WaveActiveBit,            "WaveActiveBit",            OpCodeClass::WaveActiveBit,           "waveActiveBit",              false, false, false, false, false,  true,  true,  true,  true, Attribute::AK_ReadOnly, },
		  {  OpCode::WavePrefixOp,             "WavePrefixOp",             OpCodeClass::WavePrefixOp,            "wavePrefixOp",               false,  true,  true,  true, false,  true,  true,  true,  true, Attribute::AK_ReadOnly, },
		  {  OpCode::WaveGetOrderedIndex,      "WaveGetOrderedIndex",      OpCodeClass::Reserved,                "reserved",                    true, false, false, false, false, false, false, false, false, Attribute::AK_None,     },

		  //                                                                                                                                    void,     h,     f,     d,    i1,    i8,   i16,   i32,   i64  function attribute
		  {  OpCode::GlobalOrderedCountIncReserved, "GlobalOrderedCountIncReserved", OpCodeClass::Reserved,                 "reserved",         true, false, false, false, false, false, false, false, false, Attribute::AK_None,     },

		  // Wave                                                                                                                               void,     h,     f,     d,    i1,    i8,   i16,   i32,   i64  function attribute
		  {  OpCode::QuadReadLaneAt,          "QuadReadLaneAt",           OpCodeClass::QuadReadLaneAt,           "quadReadLaneAt",             false,  true,  true,  true,  true,  true,  true,  true,  true, Attribute::AK_ReadOnly, },
		  {  OpCode::QuadOp,                  "QuadOp",                   OpCodeClass::QuadOp,                   "quadOp",                     false,  true,  true,  true, false,  true,  true,  true,  true, Attribute::AK_ReadOnly, },

		  // Bitcasts with different sizes                                                                                                      void,     h,     f,     d,    i1,    i8,   i16,   i32,   i64  function attribute
		  {  OpCode::BitcastI16toF16,         "BitcastI16toF16",          OpCodeClass::BitcastI16toF16,          "bitcastI16toF16",             true, false, false, false, false, false, false, false, false, Attribute::AK_ReadNone, },
		  {  OpCode::BitcastF16toI16,         "BitcastF16toI16",          OpCodeClass::BitcastF16toI16,          "bitcastF16toI16",             true, false, false, false, false, false, false, false, false, Attribute::AK_ReadNone, },
		  {  OpCode::BitcastI32toF32,         "BitcastI32toF32",          OpCodeClass::BitcastI32toF32,          "bitcastI32toF32",             true, false, false, false, false, false, false, false, false, Attribute::AK_ReadNone, },
		  {  OpCode::BitcastF32toI32,         "BitcastF32toI32",          OpCodeClass::BitcastF32toI32,          "bitcastF32toI32",             true, false, false, false, false, false, false, false, false, Attribute::AK_ReadNone, },
		  {  OpCode::BitcastI64toF64,         "BitcastI64toF64",          OpCodeClass::BitcastI64toF64,          "bitcastI64toF64",             true, false, false, false, false, false, false, false, false, Attribute::AK_ReadNone, },
		  {  OpCode::BitcastF64toI64,         "BitcastF64toI64",          OpCodeClass::BitcastF64toI64,          "bitcastF64toI64",             true, false, false, false, false, false, false, false, false, Attribute::AK_ReadNone, },

		  // GS                                                                                                                                 void,     h,     f,     d,    i1,    i8,   i16,   i32,   i64  function attribute
		  {  OpCode::GSInstanceID,            "GSInstanceID",             OpCodeClass::GSInstanceID,             "gsInstanceID",               false, false, false, false, false, false, false,  true, false, Attribute::AK_ReadNone, },

		  // Legacy floating-point                                                                                                              void,     h,     f,     d,    i1,    i8,   i16,   i32,   i64  function attribute
		  {  OpCode::LegacyF32ToF16,          "LegacyF32ToF16",           OpCodeClass::LegacyF32ToF16,           "legacyF32ToF16",              true, false, false, false, false, false, false, false, false, Attribute::AK_ReadNone, },
		  {  OpCode::LegacyF16ToF32,          "LegacyF16ToF32",           OpCodeClass::LegacyF16ToF32,           "legacyF16ToF32",              true, false, false, false, false, false, false, false, false, Attribute::AK_ReadNone, },

		  // Double precision                                                                                                                   void,     h,     f,     d,    i1,    i8,   i16,   i32,   i64  function attribute
		  {  OpCode::LegacyDoubleToFloat,     "LegacyDoubleToFloat",      OpCodeClass::LegacyDoubleToFloat,      "legacyDoubleToFloat",         true, false, false, false, false, false, false, false, false, Attribute::AK_ReadNone, },
		  {  OpCode::LegacyDoubleToSInt32,    "LegacyDoubleToSInt32",     OpCodeClass::LegacyDoubleToSInt32,     "legacyDoubleToSInt32",        true, false, false, false, false, false, false, false, false, Attribute::AK_ReadNone, },
		  {  OpCode::LegacyDoubleToUInt32,    "LegacyDoubleToUInt32",     OpCodeClass::LegacyDoubleToUInt32,     "legacyDoubleToUInt32",        true, false, false, false, false, false, false, false, false, Attribute::AK_ReadNone, },

		  // Wave                                                                                                                               void,     h,     f,     d,    i1,    i8,   i16,   i32,   i64  function attribute
		  {  OpCode::WaveAllBitCount,         "WaveAllBitCount",          OpCodeClass::WaveAllOp,                "waveAllOp",                   true, false, false, false, false, false, false, false, false, Attribute::AK_ReadOnly, },
		  {  OpCode::WavePrefixBitCount,      "WavePrefixBitCount",       OpCodeClass::WavePrefixOp,             "wavePrefixOp",                true, false, false, false, false, false, false, false, false, Attribute::AK_ReadOnly, },

		  // Pixel shader                                                                                                                       void,     h,     f,     d,    i1,    i8,   i16,   i32,   i64  function attribute
		  {  OpCode::SampleIndex,             "SampleIndex",              OpCodeClass::SampleIndex,              "sampleIndex",                false, false, false, false, false, false, false,  true, false, Attribute::AK_ReadNone, },
		  {  OpCode::Coverage,                "Coverage",                 OpCodeClass::Coverage,                 "coverage",                   false, false, false, false, false, false, false,  true, false, Attribute::AK_ReadNone, },
		  {  OpCode::InnerCoverage,           "InnerCoverage",            OpCodeClass::InnerCoverage,            "innerCoverage",              false, false, false, false, false, false, false,  true, false, Attribute::AK_ReadNone, },
	};

	char const * OP::name_prefix_ = "dx.op.";

	char const * OP::GetOpCodeName(OpCode op)
	{
		BOOST_ASSERT_MSG((static_cast<uint32_t>(op) >= 0) && (op < OpCode::NumOpCodes), "Invalid opcode");
		return op_code_props_[static_cast<uint32_t>(op)].op_code_name;
	}

	bool OP::IsDxilOpFunc(Function const * func)
	{
		std::string_view name = func->Name();
		return name.find(OP::name_prefix_) == 0;
	}

	OP::OP(LLVMContext& ctx, LLVMModule* module)
		: context_(ctx), module_(module)
	{
		static_assert(std::size(OP::op_code_props_) == static_cast<size_t>(OpCode::NumOpCodes), "forgot to update OP::op_code_props_");
	}
}
