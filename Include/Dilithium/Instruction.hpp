/**
 * @file Instruction.hpp
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

#ifndef _DILITHIUM_INSTRUCTUION_HPP
#define _DILITHIUM_INSTRUCTUION_HPP

#pragma once

#include <Dilithium/User.hpp>
#include <Dilithium/Value.hpp>

namespace Dilithium
{
	class BasicBlock;

	class Instruction : public User
	{
		typedef BasicBlock ParentType;

		template <typename NodeType>
		friend void AddToSymbolTableList(NodeType*, typename NodeType::ParentType*);
		template <typename NodeType>
		friend void RemoveFromSymbolTableList(NodeType*);

	public:
		enum TermOps
		{
#define FIRST_TERM_INST(N)					TermOpsBegin = N,
#define HANDLE_TERM_INST(N, OPC, CLASS)		OPC = N,
#define LAST_TERM_INST(N)					TermOpsEnd = N + 1
#include "Instruction.inc"
		};

		enum BinaryOps
		{
#define FIRST_BINARY_INST(N)				BinaryOpsBegin = N,
#define HANDLE_BINARY_INST(N, OPC, CLASS)	OPC = N,
#define LAST_BINARY_INST(N)					BinaryOpsEnd = N + 1
#include "Instruction.inc"
		};

		enum MemoryOps
		{
#define FIRST_MEMORY_INST(N)				MemoryOpsBegin = N,
#define HANDLE_MEMORY_INST(N, OPC, CLASS)	OPC = N,
#define LAST_MEMORY_INST(N)					MemoryOpsEnd = N + 1
#include "Instruction.inc"
		};

		enum CastOps
		{
#define FIRST_CAST_INST(N)					CastOpsBegin = N,
#define HANDLE_CAST_INST(N, OPC, CLASS)		OPC = N,
#define LAST_CAST_INST(N)					CastOpsEnd = N + 1
#include "Instruction.inc"
		};

		enum OtherOps
		{
#define FIRST_OTHER_INST(N)					OtherOpsBegin = N,
#define HANDLE_OTHER_INST(N, OPC, CLASS)	OPC = N,
#define LAST_OTHER_INST(N)					OtherOpsEnd = N + 1
#include "Instruction.inc"
		};

	public:
		~Instruction() override;

		BasicBlock const * Parent() const
		{
			return parent_;
		}
		BasicBlock* Parent()
		{
			return parent_;
		}

		uint32_t Opcode() const
		{
			return this->GetValueId() - Value::InstructionVal;
		}

		static bool classof(Value const * v)
		{
			return v->GetValueId() >= Value::InstructionVal;
		}

	protected:
		Instruction(Type* ty, uint32_t type, uint32_t num_ops, uint32_t num_uses, Instruction* insert_before = nullptr);
		Instruction(Type* ty, uint32_t type, uint32_t num_ops, uint32_t num_uses, BasicBlock* insert_at_end);

		uint32_t SubclassDataFromInstruction() const
		{
			return this->GetSubclassDataFromValue() & ~HasMetadataBit;
		}
		void InstructionSubclassData(uint16_t d);

	private:
		enum
		{
			HasMetadataBit = 1 << 15
		};

	private:
		void Parent(BasicBlock* parent);

		uint16_t GetSubclassDataFromValue() const
		{
			return Value::GetSubclassDataFromValue();
		}
		void SetValueSubclassData(uint16_t d)
		{
			Value::SetValueSubclassData(d);
		}

		bool HasMetadataHashEntry() const
		{
			return (this->GetSubclassDataFromValue() & HasMetadataBit) != 0;
		}
		void HasMetadataHashEntry(bool v)
		{
			this->SetValueSubclassData((this->GetSubclassDataFromValue() & ~HasMetadataBit) | (v ? HasMetadataBit : 0));
		}

		void ClearMetadataHashEntries();

	private:
		BasicBlock* parent_;

		// DILITHIUM_NOT_IMPLEMENTED
	};
}

#endif		// _DILITHIUM_INSTRUCTUION_HPP
