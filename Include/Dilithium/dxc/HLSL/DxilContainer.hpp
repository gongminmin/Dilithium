/**
 * @file DxilContainer.hpp
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

#include <Dilithium/Util.hpp>
#include <Dilithium/dxc/HLSL/DxilConstants.hpp>

namespace Dilithium
{
#pragma pack(push, 1)
	size_t constexpr DxilContainerHashSize = 16;
	uint16_t constexpr DxilContainerVersionMajor = 1;  // Current major version
	uint16_t constexpr DxilContainerVersionMinor = 0;  // Current minor version
	uint32_t constexpr DxilContainerMaxSize = 0x80000000; // Max size for container.

	struct DxilContainerHash
	{
		uint8_t Digest[DxilContainerHashSize];
	};

	struct DxilContainerVersion
	{
		uint16_t Major;
		uint16_t Minor;
	};

	struct DxilContainerHeader
	{
		uint32_t              HeaderFourCC;
		DxilContainerHash     Hash;
		DxilContainerVersion  Version;
		uint32_t              ContainerSizeInBytes; // From start of this header
		uint32_t              PartCount;
		// Structure is followed by uint32_t PartOffset[PartCount];
		// The offset is to a DxilPartHeader.
	};

	struct DxilPartHeader
	{
		uint32_t  PartFourCC; // Four char code for part type.
		uint32_t  PartSize;   // Byte count for PartData.
							  // Structure is followed by uint8_t PartData[PartSize].
	};

	enum DxilFourCC
	{
		DFCC_Container = MakeFourCC<'D', 'X', 'B', 'C'>::value, // for back-compat with tools that look for DXBC containers
		DFCC_ResourceDef = MakeFourCC<'R', 'D', 'E', 'F'>::value,
		DFCC_InputSignature = MakeFourCC<'I', 'S', 'G', '1'>::value,
		DFCC_OutputSignature = MakeFourCC<'O', 'S', 'G', '1'>::value,
		DFCC_PatchConstantSignature = MakeFourCC<'P', 'S', 'G', '1'>::value,
		DFCC_ShaderStatistics = MakeFourCC<'S', 'T', 'A', 'T'>::value,
		DFCC_ShaderDebugInfoDXIL = MakeFourCC<'I', 'L', 'D', 'B'>::value,
		DFCC_FeatureInfo = MakeFourCC<'S', 'F', 'I', '0'>::value,
		DFCC_PrivateData = MakeFourCC<'P', 'R', 'I', 'V'>::value,
		DFCC_RootSignature = MakeFourCC<'R', 'T', 'S', '0'>::value,
		DFCC_DXIL = MakeFourCC<'D', 'X', 'I', 'L'>::value,
		DFCC_PipelineStateValidation = MakeFourCC<'P', 'S', 'V', '0'>::value
	};

	uint32_t constexpr ShaderFeatureInfoCount = 16;

	struct DxilShaderFeatureInfo
	{
		uint64_t FeatureFlags;
	};

	struct DxilBitcodeHeader
	{
		uint32_t DxilMagic;       // ACSII "DXIL".
		uint32_t DxilVersion;     // DXIL version.
		uint32_t BitcodeOffset;   // Offset to LLVM bitcode (from start of header).
		uint32_t BitcodeSize;     // Size of LLVM bitcode.
	};

	struct DxilProgramHeader
	{
		uint32_t          ProgramVersion;   // Major and minor version, including type.
		uint32_t          SizeInUint32;     // Size in uint32_t units including this header.
		DxilBitcodeHeader BitcodeHeader;    // Bitcode-specific header.
											// Followed by uint8_t[BitcodeHeader.BitcodeOffset]
	};

	struct DxilProgramSignature
	{
		uint32_t ParamCount;
		uint32_t ParamOffset;
	};

	enum class DxilProgramSigMinPrecision : uint32_t
	{
		Default = 0,
		Float16 = 1,
		Float2_8 = 2,
		Reserved = 3,
		SInt16 = 4,
		UInt16 = 5,
		Any16 = 0xF0,
		Any10 = 0xF1
	};

	enum class DxilProgramSigSemantic : uint32_t
	{
		Undefined = 0,
		Position = 1,
		ClipDistance = 2,
		CullDistance = 3,
		RenderTargetArrayIndex = 4,
		ViewPortArrayIndex = 5,
		VertexID = 6,
		PrimitiveID = 7,
		InstanceID = 8,
		IsFrontFace = 9,
		SampleIndex = 10,
		FinalQuadEdgeTessfactor = 11,
		FinalQuadInsideTessfactor = 12,
		FinalTriEdgeTessfactor = 13,
		FinalTriInsideTessfactor = 14,
		FinalLineDetailTessfactor = 15,
		FinalLineDensityTessfactor = 16,
		Target = 64,
		Depth = 65,
		Coverage = 66,
		DepthGE = 67,
		DepthLE = 68,
		StencilRef = 69,
		InnerCoverage = 70
	};

	enum class DxilProgramSigCompType : uint32_t
	{
		Unknown = 0,
		UInt32 = 1,
		SInt32 = 2,
		Float32 = 3,
		UInt16 = 4,
		SInt16 = 5,
		Float16 = 6,
		UInt64 = 7,
		SInt64 = 8,
		Float64 = 9
	};

	struct DxilProgramSignatureElement
	{
		uint32_t Stream;					// Stream index (parameters must appear in non-decreasing stream order)
		uint32_t SemanticName;				// Offset to LPCSTR from start of DxilProgramSignature.
		uint32_t SemanticIndex;				// Semantic Index
		DxilProgramSigSemantic SystemValue;	// Semantic type. Similar to DxilSemantic::Kind, but a serialized rather than processing rep.
		DxilProgramSigCompType CompType;	// Type of bits.
		uint32_t Register;					// Register Index (row index)
		uint8_t Mask;						// Mask (column allocation)
		union								// Unconditional cases useful for validation of shader linkage.
		{
			uint8_t NeverWrites_Mask;		// For an output signature, the shader the signature belongs to never
											// writes the masked components of the output register.
			uint8_t AlwaysReads_Mask;		// For an input signature, the shader the signature belongs to always
											// reads the masked components of the input register.
		};
		uint16_t Pad;
		DxilProgramSigMinPrecision MinPrecision; // Minimum precision of input/output data
	};

	// Easy to get this wrong. Earlier assertions can help determine
	static_assert(sizeof(DxilProgramSignatureElement) == 0x20, "DxilProgramSignatureElement is misaligned");
#pragma pack(pop)

	DxilPartHeader const * GetDxilContainerPart(DxilContainerHeader const * header, uint32_t index);
	char const * GetDxilPartData(DxilPartHeader const * part);

	DxilContainerHeader const * IsDxilContainerLike(void const * ptr, size_t length);
	bool IsValidDxilContainer(DxilContainerHeader const * header, size_t length);

	struct DxilPartIsType
	{
		uint32_t IsFourCC;

		explicit DxilPartIsType(uint32_t FourCC)
			: IsFourCC(FourCC)
		{
		}

		bool operator()(DxilPartHeader const * part) const
		{
			return part->PartFourCC == IsFourCC;
		}
	};

	bool IsValidDxilBitcodeHeader(DxilBitcodeHeader const * header, uint32_t length);	
	void GetDxilProgramBitcode(DxilProgramHeader const * header, uint8_t const ** bitcode, uint32_t* bitcode_length);
	bool IsValidDxilProgramHeader(DxilProgramHeader const * header, uint32_t length);

	// Extract the shader type from the program version value.
	inline ShaderKind GetVersionShaderType(uint32_t program_version)
	{
		return static_cast<ShaderKind>((program_version & 0xFFFF0000U) >> 16);
	}
}
