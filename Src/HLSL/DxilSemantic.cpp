/**
 * @file DxilSemantic.cpp
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

#include <Dilithium/dxc/HLSL/DxilSemantic.hpp>
#include <Dilithium/dxc/HLSL/DxilSigPoint.hpp>

#include <algorithm>

namespace
{
	using namespace Dilithium;

	int ascii_strncasecmp(char const * lhs, char const * rhs, size_t length)
	{
		for (size_t i = 0; i < length; ++ i)
		{
			uint8_t lhc = static_cast<uint8_t>(tolower(lhs[i]));
			uint8_t rhc = static_cast<uint8_t>(tolower(rhs[i]));
			if (lhc != rhc)
			{
				return lhc < rhc ? -1 : 1;
			}
		}
		return 0;
	}

	int compare_lower(std::string_view lhs, std::string_view rhs)
	{
		int res = ascii_strncasecmp(lhs.data(), rhs.data(), std::min(lhs.size(), rhs.size()));
		if (res != 0)
		{
			return res;
		}
		if (lhs.size() == rhs.size())
		{
			return 0;
		}
		return lhs.size() < rhs.size() ? -1 : 1;
	}

	const DxilSemantic SEMANTIC_TABLE[] =
	{
		DxilSemantic(SemanticKind::Arbitrary,             nullptr),
		DxilSemantic(SemanticKind::VertexID,              "SV_VertexID"),
		DxilSemantic(SemanticKind::InstanceID,            "SV_InstanceID"),
		DxilSemantic(SemanticKind::Position,              "SV_Position"),
		DxilSemantic(SemanticKind::RenderTargetArrayIndex,"SV_RenderTargetArrayIndex"),
		DxilSemantic(SemanticKind::ViewPortArrayIndex,    "SV_ViewportArrayIndex"),
		DxilSemantic(SemanticKind::ClipDistance,          "SV_ClipDistance"),
		DxilSemantic(SemanticKind::CullDistance,          "SV_CullDistance"),
		DxilSemantic(SemanticKind::OutputControlPointID,  "SV_OutputControlPointID"),
		DxilSemantic(SemanticKind::DomainLocation,        "SV_DomainLocation"),
		DxilSemantic(SemanticKind::PrimitiveID,           "SV_PrimitiveID"),
		DxilSemantic(SemanticKind::GSInstanceID,          "SV_GSInstanceID"),
		DxilSemantic(SemanticKind::SampleIndex,           "SV_SampleIndex"),
		DxilSemantic(SemanticKind::IsFrontFace,           "SV_IsFrontFace"),
		DxilSemantic(SemanticKind::Coverage,              "SV_Coverage"),
		DxilSemantic(SemanticKind::InnerCoverage,         "SV_InnerCoverage"),
		DxilSemantic(SemanticKind::Target,                "SV_Target"),
		DxilSemantic(SemanticKind::Depth,                 "SV_Depth"),
		DxilSemantic(SemanticKind::DepthLessEqual,        "SV_DepthLessEqual"),
		DxilSemantic(SemanticKind::DepthGreaterEqual,     "SV_DepthGreaterEqual"),
		DxilSemantic(SemanticKind::StencilRef,            "SV_StencilRef"),
		DxilSemantic(SemanticKind::DispatchThreadID,      "SV_DispatchThreadID"),
		DxilSemantic(SemanticKind::GroupID,               "SV_GroupID"),
		DxilSemantic(SemanticKind::GroupIndex,            "SV_GroupIndex"),
		DxilSemantic(SemanticKind::GroupThreadID,         "SV_GroupThreadID"),
		DxilSemantic(SemanticKind::TessFactor,            "SV_TessFactor"),
		DxilSemantic(SemanticKind::InsideTessFactor,      "SV_InsideTessFactor"),
		DxilSemantic(SemanticKind::Invalid,               nullptr),
	};
}

namespace Dilithium
{
	DxilSemantic::DxilSemantic(SemanticKind kind, char const * name)
		: kind_(kind), name_(name)
	{
	}

	DxilSemantic const * DxilSemantic::GetByName(std::string_view name)
	{
		if (!HasSVPrefix(name))
		{
			return GetArbitrary();
		}

		// The search is a simple linear scan as it is fairly infrequent operation and the list is short.
		// The search can be improved if linear traversal has inadequate performance.
		for (uint32_t i = static_cast<uint32_t>(SemanticKind::Arbitrary) + 1; i < static_cast<uint32_t>(SemanticKind::Invalid); ++ i)
		{
			if (compare_lower(name, SEMANTIC_TABLE[i].name_) == 0)
			{
				return &SEMANTIC_TABLE[i];
			}
		}

		return GetInvalid();
	}

	DxilSemantic const * DxilSemantic::GetByName(std::string_view Name, SigPointKind sig_point_kind,
		uint32_t major_version, uint32_t minor_version)
	{
		return Get(GetByName(Name)->GetKind(), sig_point_kind, major_version, minor_version);
	}

	DxilSemantic const * DxilSemantic::Get(SemanticKind kind)
	{
		if (kind < SemanticKind::Invalid)
		{
			return &SEMANTIC_TABLE[static_cast<uint32_t>(kind)];
		}
		return GetInvalid();
	}

	const DxilSemantic *DxilSemantic::Get(SemanticKind kind, SigPointKind sig_point_kind,
		uint32_t major_version, uint32_t minor_version)
	{
		if (sig_point_kind == SigPointKind::Invalid)
		{
			return GetInvalid();
		}

		auto semantic = Get(kind);
		auto si = DxilSigPoint::GetInterpretation(semantic->GetKind(), sig_point_kind, major_version, minor_version);
		if (si == SemanticInterpretationKind::NA)
		{
			return GetInvalid();
		}
		else if (si == SemanticInterpretationKind::Arb)
		{
			return GetArbitrary();
		}
		else
		{
			return semantic;
		}
	}

	const DxilSemantic *DxilSemantic::GetInvalid()
	{
		return &SEMANTIC_TABLE[static_cast<uint32_t>(SemanticKind::Invalid)];
	}
	const DxilSemantic *DxilSemantic::GetArbitrary()
	{
		return &SEMANTIC_TABLE[static_cast<uint32_t>(SemanticKind::Arbitrary)];
	}

	bool DxilSemantic::HasSVPrefix(std::string_view name)
	{
		if (name.size() >= 3)
		{
			if ((name[0] == 'S') || (name[0] == 's'))
			{
				if ((name[1] == 'V') || (name[1] == 'v'))
				{
					if (name[2] == '_')
					{
						return true;
					}
				}
			}
		}

		return false;
	}

	void DxilSemantic::DecomposeNameAndIndex(std::string_view full_name, std::string_view* name, uint32_t* index)
	{
		uint32_t len = static_cast<uint32_t>(full_name.size());
		
		uint32_t i;
		for (i = len; i > 0; -- i)
		{
			char d = full_name[i - 1];
			if (('0' > d) || (d > '9'))
			{
				break;
			}
		}

		*name = full_name.substr(0, i);

		if (i < len)
		{
			*index = atoi(full_name.data() + i);
		}
		else
		{
			*index = 0;
		}
	}

	SemanticKind DxilSemantic::GetKind() const
	{
		return kind_;
	}

	char const * DxilSemantic::GetName() const
	{
		return name_;
	}

	bool DxilSemantic::IsArbitrary() const
	{
		return kind_ == SemanticKind::Arbitrary;
	}

	bool DxilSemantic::IsInvalid() const
	{
		return kind_ == SemanticKind::Invalid;
	}
}
