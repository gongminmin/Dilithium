/**
 * @file DxilSigPoint.cpp
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
#include <Dilithium/dxc/HLSL/DxilSigPoint.hpp>

namespace
{
	using namespace Dilithium;

	// TODO: Fill those tables in script

#define DO_SIGPOINTS(DO)												\
	DO(VSIn,     Invalid, Vertex,     InputAssembler, Input)			\
	DO(VSOut,    Invalid, Vertex,     Vertex,         Output)			\
	DO(PCIn,     HSCPIn,  Hull,       None,           Invalid)			\
	DO(HSIn,     HSCPIn,  Hull,       None,           Invalid)			\
	DO(HSCPIn,   Invalid, Hull,       Vertex,         Input)			\
	DO(HSCPOut,  Invalid, Hull,       Vertex,         Output)			\
	DO(PCOut,    Invalid, Hull,       PatchConstant,  PatchConstant)	\
	DO(DSIn,     Invalid, Domain,     PatchConstant,  PatchConstant)	\
	DO(DSCPIn,   Invalid, Domain,     Vertex,         Input)			\
	DO(DSOut,    Invalid, Domain,     Vertex,         Output)			\
	DO(GSVIn,    Invalid, Geometry,   Vertex,         Input)			\
	DO(GSIn,     GSVIn,   Geometry,   None,           Invalid)			\
	DO(GSOut,    Invalid, Geometry,   Vertex,         Output)			\
	DO(PSIn,     Invalid, Pixel,      Vertex,         Input)			\
	DO(PSOut,    Invalid, Pixel,      Target,         Output)			\
	DO(CSIn,     Invalid, Compute,    None,           Invalid)			\
	DO(Invalid,  Invalid, Invalid,    Invalid,        Invalid)

	DxilSigPoint const SIG_POINTS[] =
	{
#define DEF_SIGPOINT(spk, rspk, shk, pk, sigk) \
		DxilSigPoint(SigPointKind::spk, #spk, SigPointKind::rspk, ShaderKind::shk, SignatureKind::sigk, PackingKind::pk),
		DO_SIGPOINTS(DEF_SIGPOINT)
#undef DEF_SIGPOINT
	};
	uint32_t constexpr NUM_SIG_POINTS = sizeof(SIG_POINTS) / sizeof(SIG_POINTS[0]);


	struct VersionedSemanticInterpretation
	{
		VersionedSemanticInterpretation(SemanticInterpretationKind k, uint16_t major_version = 0, uint16_t minor_version = 0)
			: kind(k), major(major_version), minor(minor_version)
		{
		}

		SemanticInterpretationKind kind;
		uint16_t major;
		uint16_t minor;
	};

#define DO_INTERPRETATION_TABLE(DO) \
	DO(Arbitrary,              Arb,  Arb,   NA,       NA,       Arb,    Arb,     Arb,        Arb,        Arb,    Arb,   Arb,   NA,       Arb,   Arb,          NA,            NA) \
	DO(VertexID,               SV,   NA,    NA,       NA,       NA,     NA,      NA,         NA,         NA,     NA,    NA,    NA,       NA,    NA,           NA,            NA) \
	DO(InstanceID,             SV,   Arb,   NA,       NA,       Arb,    Arb,     NA,         NA,         Arb,    Arb,   Arb,   NA,       Arb,   Arb,          NA,            NA) \
	DO(Position,               Arb,  SV,    NA,       NA,       SV,     SV,      Arb,        Arb,        SV,     SV,    SV,    NA,       SV,    SV,           NA,            NA) \
	DO(RenderTargetArrayIndex, Arb,  SV,    NA,       NA,       SV,     SV,      Arb,        Arb,        SV,     SV,    SV,    NA,       SV,    SV,           NA,            NA) \
	DO(ViewPortArrayIndex,     Arb,  SV,    NA,       NA,       SV,     SV,      Arb,        Arb,        SV,     SV,    SV,    NA,       SV,    SV,           NA,            NA) \
	DO(ClipDistance,           Arb,  SV,    NA,       NA,       SV,     SV,      Arb,        Arb,        SV,     SV,    SV,    NA,       SV,    SV,           NA,            NA) \
	DO(CullDistance,           Arb,  SV,    NA,       NA,       SV,     SV,      Arb,        Arb,        SV,     SV,    SV,    NA,       SV,    SV,           NA,            NA) \
	DO(OutputControlPointID,   NA,   NA,    NA,       NotInSig, NA,     NA,      NA,         NA,         NA,     NA,    NA,    NA,       NA,    NA,           NA,            NA) \
	DO(DomainLocation,         NA,   NA,    NA,       NA,       NA,     NA,      NA,         NotInSig,   NA,     NA,    NA,    NA,       NA,    NA,           NA,            NA) \
	DO(PrimitiveID,            NA,   NA,    NotInSig, NotInSig, NA,     NA,      NA,         NotInSig,   NA,     NA,    NA,    Shadow,   SGV,   SGV,          NA,            NA) \
	DO(GSInstanceID,           NA,   NA,    NA,       NA,       NA,     NA,      NA,         NA,         NA,     NA,    NA,    NotInSig, NA,    NA,           NA,            NA) \
	DO(SampleIndex,            NA,   NA,    NA,       NA,       NA,     NA,      NA,         NA,         NA,     NA,    NA,    NA,       NA,    Shadow _41,   NA,            NA) \
	DO(IsFrontFace,            NA,   NA,    NA,       NA,       NA,     NA,      NA,         NA,         NA,     NA,    NA,    NA,       SGV,   SGV,          NA,            NA) \
	DO(Coverage,               NA,   NA,    NA,       NA,       NA,     NA,      NA,         NA,         NA,     NA,    NA,    NA,       NA,    NotInSig _50, NotPacked _41, NA) \
	DO(InnerCoverage,          NA,   NA,    NA,       NA,       NA,     NA,      NA,         NA,         NA,     NA,    NA,    NA,       NA,    NotInSig _50, NA,            NA) \
	DO(Target,                 NA,   NA,    NA,       NA,       NA,     NA,      NA,         NA,         NA,     NA,    NA,    NA,       NA,    NA,           Target,        NA) \
	DO(Depth,                  NA,   NA,    NA,       NA,       NA,     NA,      NA,         NA,         NA,     NA,    NA,    NA,       NA,    NA,           NotPacked,     NA) \
	DO(DepthLessEqual,         NA,   NA,    NA,       NA,       NA,     NA,      NA,         NA,         NA,     NA,    NA,    NA,       NA,    NA,           NotPacked _50, NA) \
	DO(DepthGreaterEqual,      NA,   NA,    NA,       NA,       NA,     NA,      NA,         NA,         NA,     NA,    NA,    NA,       NA,    NA,           NotPacked _50, NA) \
	DO(StencilRef,             NA,   NA,    NA,       NA,       NA,     NA,      NA,         NA,         NA,     NA,    NA,    NA,       NA,    NA,           NotPacked _50, NA) \
	DO(DispatchThreadID,       NA,   NA,    NA,       NA,       NA,     NA,      NA,         NA,         NA,     NA,    NA,    NA,       NA,    NA,           NA,            NotInSig) \
	DO(GroupID,                NA,   NA,    NA,       NA,       NA,     NA,      NA,         NA,         NA,     NA,    NA,    NA,       NA,    NA,           NA,            NotInSig) \
	DO(GroupIndex,             NA,   NA,    NA,       NA,       NA,     NA,      NA,         NA,         NA,     NA,    NA,    NA,       NA,    NA,           NA,            NotInSig) \
	DO(GroupThreadID,          NA,   NA,    NA,       NA,       NA,     NA,      NA,         NA,         NA,     NA,    NA,    NA,       NA,    NA,           NA,            NotInSig) \
	DO(TessFactor,             NA,   NA,    NA,       NA,       NA,     NA,      TessFactor, TessFactor, NA,     NA,    NA,    NA,       NA,    NA,           NA,            NA) \
	DO(InsideTessFactor,       NA,   NA,    NA,       NA,       NA,     NA,      TessFactor, TessFactor, NA,     NA,    NA,    NA,       NA,    NA,           NA,            NA)

	VersionedSemanticInterpretation const SEMANTIC_INTERPRETATION_TABLE[][static_cast<uint32_t>(SigPointKind::Invalid)] =
	{
#define _41 ,4,1
#define _50 ,5,0
#define DO(k) VersionedSemanticInterpretation(SemanticInterpretationKind::k)
#define DO_ROW(SEM, VSIn, VSOut, PCIn, HSIn, HSCPIn, HSCPOut, PCOut, DSIn, DSCPIn, DSOut, GSVIn, GSIn, GSOut, PSIn, PSOut, CSIn) \
	{ DO(VSIn), DO(VSOut), DO(PCIn), DO(HSIn), DO(HSCPIn), DO(HSCPOut), DO(PCOut), DO(DSIn), DO(DSCPIn), DO(DSOut), DO(GSVIn), DO(GSIn), DO(GSOut), DO(PSIn), DO(PSOut), DO(CSIn) },
		DO_INTERPRETATION_TABLE(DO_ROW)
#undef DO_ROW
#undef DO
	};
}

namespace Dilithium
{
	DxilSigPoint::DxilSigPoint(SigPointKind spk, std::string_view name, SigPointKind rspk, ShaderKind shk,
		SignatureKind sigk, PackingKind pk)
		: kind_(spk), name_(name), related_kind_(rspk), shader_kind_(shk), signature_kind_(sigk), packing_kind_(pk)
	{
	}

	SignatureKind DxilSigPoint::GetSignatureKindWithFallback() const
	{
		auto sig_kind = this->GetSignatureKind();
		if (sig_kind == SignatureKind::Invalid)
		{
			auto rk = this->GetRelatedKind();
			if (rk != SigPointKind::Invalid)
			{
				sig_kind = GetSigPoint(rk)->GetSignatureKind();
			}
		}
		return sig_kind;
	}

	DxilSigPoint const * DxilSigPoint::GetSigPoint(SigPointKind kind)
	{
		return (static_cast<uint32_t>(kind) < NUM_SIG_POINTS)
			? &SIG_POINTS[static_cast<uint32_t>(kind)] : &SIG_POINTS[static_cast<uint32_t>(SigPointKind::Invalid)];
	}

	SigPointKind DxilSigPoint::GetKind(ShaderKind shader_kind, SignatureKind sig_kind,
		bool is_patch_constant_function, bool is_special_input)
	{
		if (is_special_input)
		{
			switch (shader_kind)
			{
			case ShaderKind::Hull:
				if (sig_kind == SignatureKind::Input)
				{
					return is_patch_constant_function ? SigPointKind::PCIn : SigPointKind::HSIn;
				}
				break;
			case ShaderKind::Geometry:
				if (sig_kind == SignatureKind::Input)
				{
					return SigPointKind::GSIn;
				}
				break;

			default:
				break;
			}
		}

		switch (shader_kind)
		{
		case ShaderKind::Vertex:
			switch (sig_kind)
			{
			case SignatureKind::Input:
				return SigPointKind::VSIn;
			case SignatureKind::Output:
				return SigPointKind::VSOut;

			default:
				break;
			}
			break;
		case ShaderKind::Hull:
			switch (sig_kind)
			{
			case SignatureKind::Input:
				return SigPointKind::HSCPIn;
			case SignatureKind::Output:
				return SigPointKind::HSCPOut;
			case SignatureKind::PatchConstant:
				return SigPointKind::PCOut;

			default:
				break;
			}
			break;
		case ShaderKind::Domain:
			switch (sig_kind)
			{
			case SignatureKind::Input:
				return SigPointKind::DSCPIn;
			case SignatureKind::Output:
				return SigPointKind::DSOut;
			case SignatureKind::PatchConstant:
				return SigPointKind::DSIn;

			default:
				break;
			}
			break;
		case ShaderKind::Geometry:
			switch (sig_kind)
			{
			case SignatureKind::Input:
				return SigPointKind::GSVIn;
			case SignatureKind::Output:
				return SigPointKind::GSOut;

			default:
				break;
			}
			break;
		case ShaderKind::Pixel:
			switch (sig_kind)
			{
			case SignatureKind::Input:
				return SigPointKind::PSIn;
			case SignatureKind::Output:
				return SigPointKind::PSOut;

			default:
				break;
			}
			break;
		case ShaderKind::Compute:
			switch (sig_kind)
			{
			case SignatureKind::Input:
				return SigPointKind::CSIn;

			default:
				break;
			}
			break;
		}

		return SigPointKind::Invalid;
	}

	SemanticInterpretationKind DxilSigPoint::GetInterpretation(SemanticKind sk, SigPointKind kind,
		uint32_t major_version, uint32_t minor_version)
	{
		if ((sk < SemanticKind::Invalid) && (kind < SigPointKind::Invalid))
		{
			auto const & vsi = SEMANTIC_INTERPRETATION_TABLE[static_cast<uint32_t>(sk)][static_cast<uint32_t>(kind)];
			if (vsi.kind != SemanticInterpretationKind::NA)
			{
				if ((major_version > static_cast<uint32_t>(vsi.major))
					|| ((major_version == static_cast<uint32_t>(vsi.major)) && (minor_version >= static_cast<uint32_t>(vsi.minor))))
				{
					return vsi.kind;
				}
			}
		}
		return SemanticInterpretationKind::NA;
	}

	SigPointKind DxilSigPoint::RecoverKind(SemanticKind sk, SigPointKind kind)
	{
		if ((sk == SemanticKind::PrimitiveID) && (kind == SigPointKind::GSVIn))
		{
			return SigPointKind::GSIn;
		}
		else
		{
			return kind;
		}
	}
}
