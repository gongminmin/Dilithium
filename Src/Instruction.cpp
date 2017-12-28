/**
 * @file Instruction.cpp
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
#include <Dilithium/Instruction.hpp>
#include <Dilithium/LLVMContext.hpp>
#include <Dilithium/Type.hpp>
#include <Dilithium/SymbolTableList.hpp>
#include <Dilithium/Value.hpp>
#include "LLVMContextImpl.hpp"

namespace
{
	template <typename T>
	class UniquePtrEqual
	{
	public:
		explicit UniquePtrEqual(T* ptr)
			: ptr_(ptr)
		{
		}

		bool operator()(std::unique_ptr<T> const & rhs)
		{
			return ptr_ == rhs.get();
		}

	private:
		T* ptr_;
	};
}

namespace Dilithium 
{
	Instruction::Instruction(Type* ty, uint32_t type, uint32_t num_ops, uint32_t num_uses, Instruction* insert_before)
		: User(ty, Value::InstructionVal + type, num_ops, num_uses),
			parent_(nullptr)
	{
		if (insert_before)
		{
			auto bb = insert_before->Parent();
			BOOST_ASSERT_MSG(bb, "Instruction to insert before is not in a basic block!");
			auto iter = std::find_if(bb->InstList().begin(), bb->InstList().end(), UniquePtrEqual<Instruction>(insert_before));
			-- iter;
			bb->InstList().insert(iter, std::unique_ptr<Instruction>(this));
			AddToSymbolTableList(this, bb);
		}
	}

	Instruction::Instruction(Type* ty, uint32_t type, uint32_t num_ops, uint32_t num_uses, BasicBlock* insert_at_end)
		: User(ty, Value::InstructionVal + type, num_ops, num_uses),
			parent_(nullptr)
	{
		BOOST_ASSERT_MSG(insert_at_end, "Basic block to append to may not be NULL!");
		insert_at_end->InstList().push_back(std::unique_ptr<Instruction>(this));
		AddToSymbolTableList(this, insert_at_end);
	}

	Instruction::~Instruction()
	{
		RemoveFromSymbolTableList(this);

		BOOST_ASSERT_MSG(!parent_, "Instruction still linked in the program!");
		if (this->HasMetadataHashEntry())
		{
			this->ClearMetadataHashEntries();
		}
	}

	char const  *Instruction::OpcodeName() const
	{
		switch (this->Opcode())
		{
			// Terminators
		case Ret:
			return "ret";
		case Br:
			return "br";
		case Switch:
			return "switch";
		case IndirectBr:
			return "indirectbr";
		case Invoke:
			return "invoke";
		case Resume:
			return "resume";
		case Unreachable:
			return "unreachable";

			// Standard binary operators...
		case Add:
			return "add";
		case FAdd:
			return "fadd";
		case Sub:
			return "sub";
		case FSub:
			return "fsub";
		case Mul:
			return "mul";
		case FMul:
			return "fmul";
		case UDiv:
			return "udiv";
		case SDiv:
			return "sdiv";
		case FDiv:
			return "fdiv";
		case URem:
			return "urem";
		case SRem:
			return "srem";
		case FRem:
			return "frem";

			// Logical operators...
		case And:
			return "and";
		case Or:
			return "or";
		case Xor:
			return "xor";

			// Memory instructions...
		case Alloca:
			return "alloca";
		case Load:
			return "load";
		case Store:
			return "store";
		case AtomicCmpXchg:
			return "cmpxchg";
		case AtomicRMW:
			return "atomicrmw";
		case Fence:
			return "fence";
		case GetElementPtr:
			return "getelementptr";

			// Convert instructions...
		case Trunc:
			return "trunc";
		case ZExt:
			return "zext";
		case SExt:
			return "sext";
		case FPTrunc:
			return "fptrunc";
		case FPExt:
			return "fpext";
		case FPToUI:
			return "fptoui";
		case FPToSI:
			return "fptosi";
		case UIToFP:
			return "uitofp";
		case SIToFP:
			return "sitofp";
		case IntToPtr:
			return "inttoptr";
		case PtrToInt:
			return "ptrtoint";
		case BitCast:
			return "bitcast";
		case AddrSpaceCast:
			return "addrspacecast";

			// Other instructions...
		case ICmp:
			return "icmp";
		case FCmp:
			return "fcmp";
		case PHI:
			return "phi";
		case Select:
			return "select";
		case Call:
			return "call";
		case Shl:
			return "shl";
		case LShr:
			return "lshr";
		case AShr:
			return "ashr";
		case VAArg:
			return "va_arg";
		case ExtractElement:
			return "extractelement";
		case InsertElement:
			return "insertelement";
		case ShuffleVector:
			return "shufflevector";
		case ExtractValue:
			return "extractvalue";
		case InsertValue:
			return "insertvalue";
		case LandingPad:
			return "landingpad";

		default:
			return "<Invalid operator> ";
		}
	}

	bool Instruction::IsTerminator() const
	{
		uint32_t opcode = this->Opcode();
		return (opcode >= TermOpsBegin) && (opcode < TermOpsEnd);
	}

	bool Instruction::IsBinaryOp() const
	{
		uint32_t opcode = this->Opcode();
		return (opcode >= BinaryOpsBegin) && (opcode < BinaryOpsEnd);
	}

	bool Instruction::IsShift() const
	{
		uint32_t opcode = this->Opcode();
		return (opcode >= Shl) && (opcode < AShr);
	}

	bool Instruction::IsCast() const
	{
		uint32_t opcode = this->Opcode();
		return (opcode >= CastOpsBegin) && (opcode < CastOpsEnd);
	}

	MDNode* Instruction::GetMetadata(uint32_t kind_id) const
	{
		if (!this->HasMetadata())
		{
			return nullptr;
		}
		return this->GetMetadataImpl(kind_id);
	}

	MDNode* Instruction::GetMetadata(std::string_view kind) const
	{
		if (!this->HasMetadata())
		{
			return nullptr;
		}
		return this->GetMetadataImpl(kind);
	}

	void Instruction::GetAllMetadata(boost::container::small_vector_base<std::pair<uint32_t, MDNode*>>& mds) const
	{
		if (this->HasMetadata())
		{
			this->GetAllMetadataImpl(mds);
		}
	}

	void Instruction::GetAllMetadataOtherThanDebugLoc(boost::container::small_vector_base<std::pair<uint32_t, MDNode*>>& mds) const
	{
		if (this->HasMetadataOtherThanDebugLoc())
		{
			// TODO: No debug loc
			this->GetAllMetadataImpl(mds);
		}
	}

	void Instruction::InstructionSubclassData(uint16_t d)
	{
		BOOST_ASSERT_MSG((d & HasMetadataBit) == 0, "Out of range value put into field");
		this->SetValueSubclassData((this->GetSubclassDataFromValue() & HasMetadataBit) | d);
	}

	void Instruction::Parent(BasicBlock* parent)
	{
		parent_ = parent;
	}

	MDNode* Instruction::GetMetadataImpl(uint32_t kind_id) const
	{
		// Handle 'dbg' as a special case since it is not stored in the hash table.
		if (kind_id == LLVMContext::MD_Dbg)
		{
			DILITHIUM_NOT_IMPLEMENTED;
		}

		if (!this->HasMetadataHashEntry())
		{
			return nullptr;
		}
		auto& info = this->Context().Impl().instruction_metadata[this];
		BOOST_ASSERT_MSG(!info.empty(), "bit out of sync with hash table");

		return info.Lookup(kind_id);
	}

	MDNode* Instruction::GetMetadataImpl(std::string_view kind) const
	{
		return this->GetMetadataImpl(this->Context().MdKindId(kind));
	}

	void Instruction::GetAllMetadataImpl(boost::container::small_vector_base<std::pair<uint32_t, MDNode*>>& result) const
	{
		result.clear();
		BOOST_ASSERT_MSG(this->HasMetadataHashEntry() && this->Context().Impl().instruction_metadata.count(this),
			"Shouldn't have called this");
		const auto &Info = this->Context().Impl().instruction_metadata.find(this)->second;
		BOOST_ASSERT_MSG(!Info.empty(), "Shouldn't have called this");
		Info.GetAll(result);
	}

	void Instruction::ClearMetadataHashEntries()
	{
		BOOST_ASSERT_MSG(this->HasMetadataHashEntry(), "Caller should check");
		this->Context().Impl().instruction_metadata.erase(this);
		this->HasMetadataHashEntry(false);
	}
}
