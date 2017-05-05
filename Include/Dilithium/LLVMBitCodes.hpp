/**
 * @file LLVMBitCodes.hpp
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

#ifndef _DILITHIUM_LLVM_BIT_CODES_HPP
#define _DILITHIUM_LLVM_BIT_CODES_HPP

#pragma once

#include <Dilithium/BitCodes.hpp>

namespace Dilithium
{
	namespace BitCode
	{
		// Majorly from LLVM

		// The only top-level block type defined is for a module.
		struct BlockId
		{
			enum
			{
				// Blocks
				Module = StandardBlockId::FirstApplicationBlockId,

				// Module sub-block id's.
				ParamAttr,
				ParamAttrGroup,

				Constants,
				Function,

				Unused0,

				ValueSymTab,
				Metadata,
				MetadataAttachment,

				Type,

				UseList
			};
		};


		/// Module blocks have a number of optional fields and subblocks.
		struct ModuleCode
		{
			enum
			{
				Version = 1,		// Version:     [version#]
				Triple = 2,			// Triple:      [strchr x N]
				DataLayout = 3,		// DataLayout:  [strchr x N]
				Asm = 4,			// Asm:         [strchr x N]
				SectionName = 5,    // SectionName: [strchr x N]

				// FIXME: Remove DepLib in 4.0.
				DepLib = 6,    // DepLib:      [strchr x N]

				// GlobalVar: [pointer type, isconst, initid,
				//             linkage, alignment, section, visibility, threadlocal]
				GlobalVar = 7,

				// Function:  [type, callingconv, isproto, linkage, paramattrs, alignment,
				//             section, visibility, gc, unnamed_addr]
				Function = 8,

				// Alias: [alias type, aliasee val#, linkage, visibility]
				Alias = 9,

				// PurgeVals: [numvals]
				PurgeVals = 10,

				GcName = 11,  // GcName: [strchr x N]
				Comdat = 12,  // Comdat: [selection_kind, name]
			};
		};

		/// ParamAttr blocks have code for defining a parameter attribute set.
		struct ParamAttrCode
		{
			enum
			{
				// FIXME: Remove `ParamAttrCode::EntryOld' in 4.0
				EntryOld = 1,	// ENTRY: [paramidx0, attr0, paramidx1, attr1...]
				Entry = 2,		// ENTRY: [paramidx0, attrgrp0, paramidx1, attrgrp1, ...]
				GrpEntry = 3	// ENTRY: [id, attr0, att1, ...]
			};
		};

		/// Type blocks have codes for each type primitive they use.
		struct TypeCode
		{
			enum
			{
				NumEntry = 1,	// NUMENTRY: [numentries]

				// Type Codes
				Void = 2,		// void
				Float = 3,		// float
				Double  = 4,	// double
				Label = 5,		// label
				Opaque = 6,		// opaque
				Integer = 7,	// integer: [width]
				Pointer = 8,	// pointer: [pointee type]

				FunctionOld = 9,	// function: [vararg, attrid, retty, paramty x N]

				Half = 10,		// half

				Array = 11,		// array: [numelts, eltty]
				Vector = 12,	// vector: [numelts, eltty]

				// These are not with the other floating point types because they're
				// a late addition, and putting them in the right place breaks
				// binary compatibility.
				X86Fp80 = 13,	// X86 long double
				Fp128 = 14,		// long double (112 bit mantissa)
				PpcFp128 = 15,	// PPC long double (2 doubles)

				Metadata = 16,	// Metadata

				X86Mmx = 17,	// X86 MMX

				StructAnon = 18,	// StructAnon: [ispacked, eltty x N]
				StructName = 19,	// StructName: [strchr x N]
				StructNamed = 20,	// StructNamed: [ispacked, eltty x N]

				Function = 21		// Function: [vararg, retty, paramty x N]
			};
		};

		// The value symbol table only has one code (VST_ENTRY_CODE).
		struct ValueSymTabCode
		{
			enum
			{
				Entry = 1,		// Entry: [valid, namechar x N]
				BbEntry = 2		// BbEntry: [bbid, namechar x N]
			};
		};

		struct MetadataCode
		{
			enum
			{
				String = 1,			// MDSTRING:      [values]
				Value = 2,			// VALUE:         [type num, value num]
				Node = 3,			// NODE:          [n x md num]
				Name = 4,			// STRING:        [values]
				DistinctNode = 5,	// DISTINCT_NODE: [n x md num]
				Kind = 6,			// [n x [id, name]]
				Location = 7,		// [distinct, line, col, scope, inlined-at?]
				OldNode = 8,		// OLD_NODE:      [n x (type num, value num)]
				OldFnNode = 9,		// OLD_FN_NODE:   [n x (type num, value num)]
				NamedNode = 10,		// NAMED_NODE:    [n x mdnodes]
				Attachment = 11,	// [m x [value, [n x [id, mdnode]]]
				GenericDebug = 12,	// [distinct, tag, vers, header, n x md num]
				Subrange = 13,		// [distinct, count, lo]
				Enumerator = 14,	// [distinct, value, name]
				BasicType = 15,		// [distinct, tag, name, size, align, enc]
				File = 16,			// [distinct, filename, directory]
				DerivedType = 17,	// [distinct, ...]
				CompositeType = 18,	// [distinct, ...]
				SubroutineType = 19,	// [distinct, flags, types]
				CompileUnit = 20,	// [distinct, ...]
				Subprogram = 21,	// [distinct, ...]
				LexicalBlock = 22,	// [distinct, scope, file, line, column]
				LexicalBlockFile = 23,	//[distinct, scope, file, discriminator]
				Namespace = 24,		// [distinct, scope, file, name, line]
				TemplateType = 25,	// [distinct, scope, name, type, ...]
				TemplateValue = 26,	// [distinct, scope, name, type, value, ...]
				GlobalVar = 27,		// [distinct, ...]
				LocalVar = 28,		// [distinct, ...]
				Expression = 29,	// [distinct, n x element]
				ObjCProperty = 30,	// [distinct, name, file, line, ...]
				ImportedEntity = 31,	// [distinct, tag, scope, entity, line, name]
				Module = 32,		// [distinct, scope, name, ...]
			};
		};

		// The constants block (BlockId::Constants) describes emission for each
		// constant and maintains an implicit current type value.
		struct ConstantsCode
		{
			enum
			{
				SetType = 1,		// SETTYPE:       [typeid]
				Null = 2,			// NULL
				Undef = 3,			// UNDEF
				Integer = 4,		// INTEGER:       [intval]
				WideInteger = 5,	// WIDE_INTEGER:  [n x intval]
				Float = 6,			// FLOAT:         [fpval]
				Aggregate = 7,		// AGGREGATE:     [n x value number]
				String = 8,			// STRING:        [values]
				CString = 9,		// CSTRING:       [values]
				CeBinop = 10,		// CE_BINOP:      [opcode, opval, opval]
				CeCast = 11,		// CE_CAST:       [opcode, opty, opval]
				CeGep = 12,			// CE_GEP:        [n x operands]
				CeSelect = 13,		// CE_SELECT:     [opval, opval, opval]
				CeExtractElt = 14,	// CE_EXTRACTELT: [opty, opval, opval]
				CeInsertElt = 15,	// CE_INSERTELT:  [opval, opval, opval]
				CeShuffleVec = 16,	// CE_SHUFFLEVEC: [opval, opval, opval]
				CeCmp = 17,			// CE_CMP:        [opty, opval, opval, pred]
				InlineAsmOld = 18,	// INLINEASM:     [sideeffect|alignstack, asmstr,conststr]
				ShuffleVecEx = 19,	// SHUFVEC_EX:    [opty, opval, opval, opval]
				InboundsGep = 20,	// INBOUNDS_GEP:  [n x operands]
				BlockAddress = 21,	// ConstantsCode::BlockAddress [fnty, fnval, bb#]
				Data = 22,			// DATA:          [n x elements]
				InlineAsm = 23		// INLINEASM:     [sideeffect|alignstack|asmdialect,asmstr,conststr]
			};
		};

		// The function body block (BlockId::Function) describes function bodies.  It
		// can contain a constant block (BlockId::Constants).
		struct FunctionCode
		{
			enum
			{
				DeclareBlocks = 1,			// DECLAREBLOCKS: [n]

				InstBinop = 2,				// BINOP:      [opcode, ty, opval, opval]
				InstCast = 3,				// CAST:       [opcode, ty, opty, opval]
				InstGepOld = 4,				// GEP:        [n x operands]
				InstSelect = 5,				// SELECT:     [ty, opval, opval, opval]
				InstExtractElt = 6,			// EXTRACTELT: [opty, opval, opval]
				InstInsertElt = 7,			// INSERTELT:  [ty, opval, opval, opval]
				InstShuffleVec = 8,			// SHUFFLEVEC: [ty, opval, opval, opval]
				InstCmp = 9,				// CMP:        [opty, opval, opval, pred]

				InstRet = 10,				// RET:        [opty,opval<both optional>]
				InstBr = 11,				// BR:         [bb#, bb#, cond] or [bb#]
				InstSwitch = 12,			// SWITCH:     [opty, op0, op1, ...]
				InstInvoke = 13,			// INVOKE:     [attr, fnty, op0,op1, ...]
				// 14 is unused.
				InstUnreachable = 15,		// UNREACHABLE

				InstPhi = 16,				// PHI:        [ty, val0,bb0, ...]
				// 17 is unused.
				// 18 is unused.
				InstAlloca = 19,			// ALLOCA:     [instty, opty, op, align]
				InstLoad = 20,				// LOAD:       [opty, op, align, vol]
				// 21 is unused.
				// 22 is unused.
				InstVaArg = 23,				// VAARG:      [valistty, valist, instty]
				// This store code encodes the pointer type, rather than the value type
				// this is so information only available in the pointer type (e.g. address
				// spaces) is retained.
				InstStoreOld = 24,			// STORE:      [ptrty,ptr,val, align, vol]
				// 25 is unused.
				InstExtractVal = 26,		// EXTRACTVAL: [n x operands]
				InstInsertVal = 27,			// INSERTVAL:  [n x operands]
				// fcmp/icmp returning Int1TY or vector of Int1Ty. Same as CMP, exists to
				// support legacy vicmp/vfcmp instructions.
				InstCmp2 = 28,				// CMP2:       [opty, opval, opval, pred]
				// new select on i1 or [N x i1]
				InstVSelect = 29,			// VSELECT:    [ty,opval,opval,predty,pred]
				InstInboundsGepOld = 30,	// INBOUNDS_GEP: [n x operands]
				InstIndirectBr = 31,		// INDIRECTBR: [opty, op0, op1, ...]
				// 32 is unused.
				DebugLocAgain = 33,			// DEBUG_LOC_AGAIN

				InstCall = 34,				// CALL:    [attr, cc, fnty, fnid, args...]

				DebugLoc = 35,				// DEBUG_LOC:  [Line,Col,ScopeVal, IAVal]
				InstFence = 36,				// FENCE: [ordering, synchscope]
				InstCmpXChgOld = 37,		// CMPXCHG: [ptrty,ptr,cmp,new, align, vol, ordering, synchscope]
				InstAtomicRmw = 38,			// ATOMICRMW: [ptrty,ptr,val, operation, align, vol, ordering, synchscope]
				InstResume = 39,			// RESUME:     [opval]
				InstLandingPadOld = 40,		// LANDINGPAD: [ty,val,val,num,id0,val0...]
				InstLoadAtomic = 41,		// LOAD: [opty, op, align, vol, ordering, synchscope]
				InstStoreAtomicOld = 42,	// STORE: [ptrty,ptr,val, align, vol, ordering, synchscope]
				InstGep = 43,				// GEP:  [inbounds, n x operands]
				InstStore = 44,				// STORE: [ptrty,ptr,valty,val, align, vol]
				InstStoreAtomic = 45,		// STORE: [ptrty,ptr,val, align, vol
				InstCmpXCHG = 46,			// CMPXCHG: [ptrty,ptr,valty,cmp,new, align, vol,ordering,synchscope]
				InstLandingPad = 47,		// LANDINGPAD: [ty,val,num,id0,val0...]
			};
		};

		struct UseListCode
		{
			enum
			{
				Default = 1,	// DEFAULT: [index..., value-id]
				Bb = 2			// BB: [index..., bb-id]
			};
		};

		struct AttributeKindCode
		{
			enum
			{
				// = 0 is unused
				Alignment = 1,
				AlwaysInline = 2,
				ByVal = 3,
				InlineHint = 4,
				InReg = 5,
				MinSize = 6,
				Naked = 7,
				Nest = 8,
				NoAlias = 9,
				NoBuiltin = 10,
				NoCapture = 11,
				NoDuplicate = 12,
				NoImplicitFloat = 13,
				NoInline = 14,
				NonLazyBind = 15,
				NoRedZone = 16,
				NoReturn = 17,
				NoUnwind = 18,
				OptimizeForSize = 19,
				ReadNone = 20,
				ReadOnly = 21,
				Returned = 22,
				ReturnsTwice = 23,
				SExt = 24,
				StackAlignment = 25,
				StackProtect = 26,
				StackProtectReq = 27,
				StackProtectStrong = 28,
				StructRet = 29,
				SanitizeAddress = 30,
				SanitizeThread = 31,
				SanitizeMemory = 32,
				UwTable = 33,
				ZExt = 34,
				Builtin = 35,
				Cold = 36,
				OptimizeNone = 37,
				InAlloca = 38,
				NonNull = 39,
				JumpTable = 40,
				Dereferenceable = 41,
				DereferenceableOrNull = 42,
				Convergent = 43,
				SafeStack = 44,
				ArgMemOnly = 45
			};
		};
	}
}

#endif
