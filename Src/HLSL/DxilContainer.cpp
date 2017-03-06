/**
 * @file DxilContainer.cpp
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

#include <Dilithium/dxc/HLSL/DxilContainer.hpp>

#include <cstddef>

namespace Dilithium
{
	DxilContainerHeader const * IsDxilContainerLike(void const * ptr, size_t length)
	{
		if (ptr == nullptr || length < 4)
		{
			return nullptr;
		}
		if (DFCC_Container != *reinterpret_cast<uint32_t const *>(ptr))
		{
			return nullptr;
		}
		return reinterpret_cast<DxilContainerHeader const *>(ptr);
	}

	bool IsValidDxilContainer(DxilContainerHeader const * header, size_t length)
	{
		// Validate that the header is where it's supposed to be.
		if (header == nullptr)
		{
			return false;
		}
		if (length < sizeof(DxilContainerHeader))
		{
			return false;
		}

		// Validate the header values.
		if (header->HeaderFourCC != DFCC_Container)
		{
			return false;
		}
		if (header->Version.Major != DxilContainerVersionMajor)
		{
			return false;
		}
		if (header->ContainerSizeInBytes > length)
		{
			return false;
		}
		if (header->ContainerSizeInBytes > DxilContainerMaxSize)
		{
			return false;
		}

		// Make sure that the count of offsets fits.
		size_t part_offset_table_bytes = sizeof(uint32_t) * header->PartCount;
		if (part_offset_table_bytes + sizeof(DxilContainerHeader) > header->ContainerSizeInBytes)
		{
			return false;
		}

		// Make sure that each part is within the bounds.
		auto linear_container = reinterpret_cast<uint8_t const *>(header);
		auto part_offset_table = reinterpret_cast<uint32_t const *>(header + 1);
		for (uint32_t i = 0; i < header->PartCount; ++ i)
		{
			// The part header should fit.
			if (part_offset_table[i] > (header->ContainerSizeInBytes - sizeof(DxilPartHeader)))
			{
				return false;
			}

			// The contents of the part should fit.
			auto part_header = reinterpret_cast<DxilPartHeader const *>(linear_container + part_offset_table[i]);
			if (part_offset_table[i] + sizeof(DxilPartHeader) + part_header->PartSize > header->ContainerSizeInBytes)
			{
				return false;
			}
		}

		// Note: the container parts may overlap and there may be holes
		// based on this validation

		return true;
	}

	DxilPartHeader const * GetDxilContainerPart(DxilContainerHeader const * header, uint32_t index)
	{
		auto linear_container = reinterpret_cast<uint8_t const *>(header);
		auto part_offset_table = reinterpret_cast<uint32_t const *>(header + 1);
		return reinterpret_cast<DxilPartHeader const *>(linear_container + part_offset_table[index]);
	}

	char const * GetDxilPartData(DxilPartHeader const * part)
	{
		return reinterpret_cast<char const *>(part + 1);
	}

	bool IsValidDxilBitcodeHeader(DxilBitcodeHeader const * header, uint32_t length)
	{
		return (length > sizeof(DxilBitcodeHeader))
			&& (header->BitcodeOffset + header->BitcodeSize > header->BitcodeOffset)
			&& (length >= header->BitcodeOffset + header->BitcodeSize)
			&& (header->DxilMagic == DFCC_DXIL);
	}

	void GetDxilProgramBitcode(DxilProgramHeader const * header, uint8_t const ** bitcode, uint32_t* bitcode_length)
	{
		*bitcode = reinterpret_cast<uint8_t const *>(&header->BitcodeHeader) + header->BitcodeHeader.BitcodeOffset;
		*bitcode_length = header->BitcodeHeader.BitcodeSize;
	}

	bool IsValidDxilProgramHeader(DxilProgramHeader const * header, uint32_t length)
	{
		return (length >= sizeof(DxilProgramHeader))
			&& (length >= (header->SizeInUint32 * sizeof(uint32_t)))
			&& IsValidDxilBitcodeHeader(&header->BitcodeHeader, length - offsetof(DxilProgramHeader, BitcodeHeader));
	}
}
