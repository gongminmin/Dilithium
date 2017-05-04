/**
 * @file DilithiumDisasm.cpp
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

#include <cstdint>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include <iomanip>

#include <Dilithium/Dilithium.hpp>
#include <Dilithium/dxc/HLSL/DxilContainer.hpp>
#include <Dilithium/dxc/HLSL/DxilPipelineStateValidation.hpp>

using namespace Dilithium;

namespace
{
	template <typename T>
	T const * ByteOffset(void const * p, uint32_t byte_offset)
	{
		return reinterpret_cast<T const *>(static_cast<uint8_t const *>(p) + byte_offset);
	}

	void PrintFeatureInfo(DxilShaderFeatureInfo const * feature_info, std::ostream& os, char const * comment)
	{
		static char const * feature_info_names[] =
		{
			"Double-precision floating point",
			"Raw and Structured buffers",
			"UAVs at every shader stage",
			"64 UAV slots",
			"Minimum-precision data types",
			"Double-precision extensions for 11.1",
			"Shader extensions for 11.1",
			"Comparison filtering for feature level 9",
			"Tiled resources",
			"PS Output Stencil Ref",
			"PS Inner Coverage",
			"Typed UAV Load Additional Formats",
			"Raster Ordered UAVs",
			"SV_RenderTargetArrayIndex or SV_ViewportArrayIndex from any shader feeding rasterizer",
			"Wave level operations",
			"64-Bit integer",
		};

		uint64_t feature_flags = feature_info->FeatureFlags;
		if (!feature_flags)
		{
			return;
		}
		os << comment << std::endl;
		os << comment << " Note: shader requires additional functionality:" << std::endl;
		for (uint32_t i = 0; i < ShaderFeatureInfoCount; ++ i)
		{
			if (feature_flags & (1ULL << i))
			{
				os << comment << "       " << feature_info_names[i] << std::endl;
			}
		}
		os << comment << std::endl;
	}

	void PrintSignature(char const * name, DxilProgramSignature const * signature, bool is_input, std::ostream& os, char const * comment)
	{
		static char const * sys_value_names[] =
		{
			"NONE",
			"POS",
			"CLIPDST",
			"CULLDST",
			"RTINDEX",
			"VPINDEX",
			"VERTID",
			"PRIMID",
			"INSTID",
			"FFACE",
			"SAMPLE",
			"QUADEDGE",
			"QUADINT",
			"TRIEDGE",
			"TRIINT",
			"LINEDET",
			"LINEDEN",
			"TARGET",
			"DEPTH",
			"COVERAGE",
			"DEPTHGE",
			"DEPTHLE",
			"STENCILREF",
			"INNERCOV"
		};

		static char const * comp_type_names[] =
		{
			"unknown",
			"uint",
			"int",
			"float",
			"min16u",
			"min16i",
			"min16f",
			"uint64",
			"int64",
			"double"
		};

		os << comment << std::endl
			<< comment << " " << name << " signature:" << std::endl
			<< comment << std::endl
			<< comment << " Name                 Index   Mask Register SysValue  Format   Used" << std::endl
			<< comment << " -------------------- ----- ------ -------- -------- ------- ------" << std::endl;

		if (signature->ParamCount == 0)
		{
			os << comment << " no parameters" << std::endl;
			return;
		}

		auto sig_beg = ByteOffset<DxilProgramSignatureElement>(signature, signature->ParamOffset);
		auto sig_end = sig_beg + signature->ParamCount;

		bool has_streams = std::any_of(sig_beg, sig_end,
			[](DxilProgramSignatureElement const & signature) { return signature.Stream != 0; });
		for (auto sig = sig_beg; sig != sig_end; ++ sig)
		{
			os << comment << " ";
			auto semantic_name = ByteOffset<char>(signature, sig->SemanticName);
			if (has_streams)
			{
				os << "m" << sig->Stream << ":";
				os << std::left << std::setw(17) << semantic_name;
			}
			else
			{
				os << std::left << std::setw(20) << semantic_name;
			}

			os << std::right << std::setw(6) << sig->SemanticIndex;

			if (sig->Register == -1)
			{
				os << "    N/A";
				if (!_stricmp(semantic_name, "SV_Depth"))
				{
					os << "   oDepth";
				}
				else if (0 == _stricmp(semantic_name, "SV_DepthGreaterEqual"))
				{
					os << " oDepthGE";
				}
				else if (0 == _stricmp(semantic_name, "SV_DepthLessEqual"))
				{
					os << " oDepthLE";
				}
				else if (0 == _stricmp(semantic_name, "SV_Coverage"))
				{
					os << "    oMask";
				}
				else if (0 == _stricmp(semantic_name, "SV_StencilRef"))
				{
					os << "    oStencilRef";
				}
				else if (sig->SystemValue == DxilProgramSigSemantic::PrimitiveID)
				{
					os << "   primID";
				}
				else
				{
					os << "  special";
				}
			}
			else
			{
				os << "   ";
				for (int i = 0; i < 4; ++ i)
				{
					if (sig->Mask & (1UL << i))
					{
						os << "xyzw"[i];
					}
					else
					{
						os << ' ';
					}
				}
				os << std::right << std::setw(9) << sig->Register;
			}

			os << std::right << std::setw(9) << sys_value_names[static_cast<uint32_t>(sig->SystemValue)];
			os << std::right << std::setw(8) << comp_type_names[static_cast<uint32_t>(sig->CompType)];

			uint8_t rw_mask = sig->AlwaysReads_Mask;
			if (!is_input)
			{
				rw_mask = ~rw_mask;
			}

			if (sig->Register == -1)
			{
				os << (rw_mask ? "    YES" : "     NO");
			}
			else
			{
				os << "   ";
				for (int i = 0; i < 4; ++ i)
				{
					if (rw_mask & (1UL << i))
					{
						os << "xyzw"[i];
					}
					else
					{
						os << ' ';
					}
				}
			}

			os << std::endl;
		}
		os << comment << std::endl;
	}

	void PrintPipelineStateValidationRuntimeInfo(char const * buff, ShaderKind shader_kind, std::ostream& os, char const * comment)
	{
		static char const * input_primitive_names[] =
		{
			"invalid",
			"point",
			"line",
			"triangle",
			"invalid",
			"invalid",
			"lineadj",
			"triangleadj",
			"patch1",
			"patch2",
			"patch3",
			"patch4",
			"patch5",
			"patch6",
			"patch7",
			"patch8",
			"patch9",
			"patch10",
			"patch11",
			"patch12",
			"patch13",
			"patch14",
			"patch15",
			"patch16",
			"patch17",
			"patch18",
			"patch19",
			"patch20",
			"patch21",
			"patch22",
			"patch23",
			"patch24",
			"patch25",
			"patch26",
			"patch27",
			"patch28",
			"patch29",
			"patch30",
			"patch31",
			"patch32"
		};

		static char const * primitive_topology_names[] =
		{
			"invalid",
			"point",
			"invalid",
			"line",
			"invalid",
			"triangle"
		};

		static char const * tessellator_domain_names[] =
		{
			"invalid",
			"isoline",
			"tri",
			"quad"
		};

		static char const * tessellator_output_primitive_names[] =
		{
			"invalid",
			"point",
			"line",
			"triangle_cw",
			"triangle_ccw"
		};

		os << comment << std::endl
			<< comment << " Pipeline Runtime Information:" << std::endl
			<< comment << std::endl;

		uint32_t const offset = sizeof(uint32_t);
		auto info = reinterpret_cast<PSVRuntimeInfo0 const *>(buff + offset);

		switch (shader_kind)
		{
		case ShaderKind::Vertex:
			os << comment << " Vertex Shader" << std::endl;
			os << comment << " OutputPositionPresent=" << (info->VS.OutputPositionPresent ? true : false) << std::endl;
			break;

		case ShaderKind::Pixel:
			os << comment << " Pixel Shader" << std::endl;
			os << comment << " DepthOutput=" << (info->PS.DepthOutput ? true : false) << std::endl;
			os << comment << " SampleFrequency=" << (info->PS.SampleFrequency ? true : false) << std::endl;
			break;

		case ShaderKind::Geometry:
			os << comment << " Geometry Shader" << std::endl;
			os << comment << " InputPrimitive=" << input_primitive_names[info->GS.InputPrimitive] << std::endl;
			os << comment << " OutputTopology=" << primitive_topology_names[info->GS.OutputTopology] << std::endl;
			os << comment << " OutputStreamMask=" << info->GS.OutputStreamMask << std::endl;
			os << comment << " OutputPositionPresent=" << (info->GS.OutputPositionPresent ? true : false) << std::endl;
			break;

		case ShaderKind::Hull:
			os << comment << " Hull Shader" << std::endl;
			os << comment << " InputControlPointCount=" << info->HS.InputControlPointCount << std::endl;
			os << comment << " OutputControlPointCount=" << info->HS.OutputControlPointCount << std::endl;
			os << comment << " Domain=" << tessellator_domain_names[info->HS.TessellatorDomain] << std::endl;
			os << comment << " OutputPrimitive=" << tessellator_output_primitive_names[info->HS.TessellatorOutputPrimitive] << std::endl;
			break;

		case ShaderKind::Domain:
			os << comment << " Domain Shader" << std::endl;;
			os << comment << " InputControlPointCount=" << info->DS.InputControlPointCount << std::endl;
			os << comment << " OutputPositionPresent=" << (info->DS.OutputPositionPresent ? true : false) << std::endl;
			break;
		}

		os << comment << std::endl;
	}
}

void Usage()
{
	std::cerr << "Dilithium DirectX Intermediate Language Disassembler." << std::endl;
	std::cerr << "This program is free software, released under a MIT license" << std::endl;
	std::cerr << std::endl;
	std::cerr << "Usage: DilithiumDisasm INPUT [OUTPUT]" << std::endl;
	std::cerr << std::endl;
}

std::vector<uint8_t> LoadProgramFromStream(std::istream& in)
{
	in.seekg(0, std::ios_base::end);
	std::vector<uint8_t> program(in.tellg());
	in.seekg(0, std::ios_base::beg);
	in.read(reinterpret_cast<char*>(&program[0]), program.size());
	return program;
}

std::string Disassemble(std::vector<uint8_t> const & program)
{
	std::ostringstream oss;

	uint8_t const * il = program.data();
	uint32_t il_length = static_cast<uint32_t>(program.size());
	auto container = IsDxilContainerLike(il, il_length);
	if (container)
	{
		if (!IsValidDxilContainer(container, il_length))
		{
			TERROR("This container is invalid.");
		}

		for (uint32_t i = 0; i < container->PartCount; ++ i)
		{
			auto part = GetDxilContainerPart(container, i);
			if (part->PartFourCC == DFCC_FeatureInfo)
			{
				PrintFeatureInfo(reinterpret_cast<DxilShaderFeatureInfo const *>(GetDxilPartData(part)), oss, ";");
				break;
			}
		}
		for (uint32_t i = 0; i < container->PartCount; ++ i)
		{
			auto part = GetDxilContainerPart(container, i);
			if (part->PartFourCC == DFCC_InputSignature)
			{
				PrintSignature("Input", reinterpret_cast<DxilProgramSignature const *>(GetDxilPartData(part)), true, oss, ";");
				break;
			}
		}
		for (uint32_t i = 0; i < container->PartCount; ++ i)
		{
			auto part = GetDxilContainerPart(container, i);
			if (part->PartFourCC == DFCC_OutputSignature)
			{
				PrintSignature("Output", reinterpret_cast<DxilProgramSignature const *>(GetDxilPartData(part)), false, oss, ";");
				break;
			}
		}
		for (uint32_t i = 0; i < container->PartCount; ++ i)
		{
			auto part = GetDxilContainerPart(container, i);
			if (part->PartFourCC == DFCC_PatchConstantSignature)
			{
				PrintSignature("Patch Constant signature", reinterpret_cast<DxilProgramSignature const *>(GetDxilPartData(part)),
					false, oss, ";");
				break;
			}
		}

		uint32_t dxil_index = container->PartCount;
		for (uint32_t i = 0; i < container->PartCount; ++ i)
		{
			auto part = GetDxilContainerPart(container, i);
			if (part->PartFourCC == DFCC_DXIL)
			{
				dxil_index = i;
				break;
			}
		}

		if (dxil_index == container->PartCount)
		{
			TERROR("This container doesn't have DXIL.");
		}

		for (uint32_t i = 0; i < container->PartCount; ++ i)
		{
			auto part = GetDxilContainerPart(container, i);
			if (part->PartFourCC == DFCC_ShaderDebugInfoDXIL)
			{
				// Use dbg module if exist.
				dxil_index = i;
				break;
			}
		}

		auto dxil_part = GetDxilContainerPart(container, dxil_index);
		auto program_header = reinterpret_cast<DxilProgramHeader const *>(GetDxilPartData(dxil_part));
		if (!IsValidDxilProgramHeader(program_header, dxil_part->PartSize))
		{
			TERROR("The program header in this is container is invalid.");
		}

		for (uint32_t i = 0; i < container->PartCount; ++ i)
		{
			auto part = GetDxilContainerPart(container, i);
			if (part->PartFourCC == DFCC_PipelineStateValidation)
			{
				PrintPipelineStateValidationRuntimeInfo(GetDxilPartData(part), GetVersionShaderType(program_header->ProgramVersion),
					oss, ";");
				break;
			}
		}

		GetDxilProgramBitcode(program_header, &il, &il_length);
	}
	else
	{
		auto program_header = reinterpret_cast<DxilProgramHeader const *>(il);
		if (IsValidDxilProgramHeader(program_header, il_length))
		{
			GetDxilProgramBitcode(program_header, &il, &il_length);
		}
	}

	try
	{
		auto module = Dilithium::LoadLLVMModule(il, il_length, "");

		//DILITHIUM_NOT_IMPLEMENTED;

		return oss.str();
	}
	catch (std::error_code& ec)
	{
		std::cerr << ec.message() << std::endl;
		return "";
	}
}

int main(int argc, char** argv)
{
	if (argc < 2)
	{
		Usage();
		return 1;
	}

	std::ifstream in(argv[1], std::ios_base::in | std::ios_base::binary);
	auto program = LoadProgramFromStream(in);
	in.close();

	auto text = Disassemble(program);

	std::ofstream out;
	bool screen_only = false;
	if (argc < 3)
	{
		screen_only = true;
	}
	else
	{
		out.open(argv[2]);
	}

	std::cout << text;
	if (!screen_only)
	{
		out << text;
	}
}
