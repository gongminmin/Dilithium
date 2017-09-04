/**
 * @file Value.cpp
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
#include <Dilithium/Casting.hpp>
#include <Dilithium/Hashing.hpp>
#include <Dilithium/Metadata.hpp>
#include <Dilithium/Operator.hpp>
#include <Dilithium/Type.hpp>
#include <Dilithium/Value.hpp>
#include <Dilithium/ValueHandle.hpp>
#include <Dilithium/ValueSymbolTable.hpp>

#include <algorithm>
#include <iostream>
#include <unordered_set>

#include <boost/assert.hpp>

namespace
{
	using namespace Dilithium;

	Value* StripPointerCastsAndOffsets(Value* val)
	{
		if (!val->GetType()->IsPointerType())
		{
			return val;
		}

		// TODO: Normally less than 4
		std::unordered_set<Value*> visited;
		visited.insert(val);
		do
		{
			GEPOperator* gep = dyn_cast<GEPOperator>(val);
			if (gep)
			{
				if (!gep->HasAllZeroIndices())
				{
					return val;
				}
				val = gep->PointerOperand();
			}
			else if ((Operator::Opcode(val) == Instruction::BitCast) || (Operator::Opcode(val) == Instruction::AddrSpaceCast))
			{
				val = cast<Operator>(val)->Operand(0);
			}
			else
			{
				return val;
			}
			BOOST_ASSERT_MSG(val->GetType()->IsPointerType(), "Unexpected operand type!");
		} while (visited.insert(val).second);

		return val;
	}

	bool GetSymTab(Value* val, ValueSymbolTable*& sym_tab)
	{
		sym_tab = nullptr;
		auto inst = dyn_cast<Instruction>(val);
		if (inst)
		{
			auto parent = inst->Parent();
			if (parent)
			{
				auto grandparent = parent->Parent();
				if (grandparent)
				{
					sym_tab = grandparent->GetValueSymbolTable();
				}
			}
		}
		else
		{
			auto bb = dyn_cast<BasicBlock>(val);
			if (bb)
			{
				auto parent = bb->Parent();
				if (parent)
				{
					sym_tab = parent->GetValueSymbolTable();
				}
			}
			else
			{
				auto gv = dyn_cast<GlobalValue>(val);
				if (gv)
				{
					auto parent = gv->Parent();
					if (gv)
					{
						sym_tab = parent->GetValueSymbolTable();
					}
				}
				else
				{
					auto arg = dyn_cast<Argument>(val);
					if (arg)
					{
						auto parent = arg->Parent();
						if (parent)
						{
							sym_tab = parent->GetValueSymbolTable();
						}
					}
					else
					{
						BOOST_ASSERT_MSG(isa<Constant>(val), "Unknown value type!");
						return true;  // no name is setable for this.
					}
				}
			}
		}

		return false;
	}

#ifdef DILITHIUM_DEBUG
	bool Contains(std::unordered_set<ConstantExpr*>& cache, ConstantExpr* expr, Constant* c)
	{
		if (!cache.insert(expr).second)
		{
			return false;
		}

		for (auto& op : expr->Operands())
		{
			if (op == c)
			{
				return true;
			}
			auto ce = dyn_cast<ConstantExpr>(op);
			if (!ce)
			{
				continue;
			}
			if (Contains(cache, ce, c))
			{
				return true;
			}
		}
		return false;
	}

	bool Contains(Value* expr, Value* val)
	{
		if (expr == val)
		{
			return true;
		}

		auto c = dyn_cast<Constant>(val);
		if (!c)
		{
			return false;
		}

		auto ce = dyn_cast<ConstantExpr>(expr);
		if (!ce)
		{
			return false;
		}

		// TODO: Normally less than 4
		std::unordered_set<ConstantExpr*> cache;
		return Contains(cache, ce, c);
	}
#endif
}

namespace Dilithium
{
	Value::Value(Type* ty, uint32_t subclass_id)
		: type_(ty), use_list_(nullptr), subclass_id_(static_cast<uint8_t>(subclass_id)),
			has_value_handle_(0), subclass_optional_data_(0), subclass_data_(0),
			num_user_operands_(0), is_used_by_md_(false), name_hash_(0)
	{
		BOOST_ASSERT_MSG(ty, "Value defined with a null type: Error!");

		// FIXME: Why isn't this in the subclass gunk??
		// Note, we cannot call isa<CallInst> before the CallInst has been constructed.
		if ((subclass_id_ == Instruction::Call) || (subclass_id_ == Instruction::Invoke))
		{
			BOOST_ASSERT_MSG(type_->IsFirstClassType() || type_->IsVoidType() || type_->IsStructType(), "invalid CallInst type!");
		}
		else if ((subclass_id_ != BasicBlockVal) && ((subclass_id_ < ConstantFirstVal) || (subclass_id_ > ConstantLastVal)))
		{
			BOOST_ASSERT_MSG(type_->IsFirstClassType() || type_->IsVoidType(), "Cannot create non-first-class values except for constants!");
		}
	}

	Value::~Value()
	{
		if (has_value_handle_)
		{
			ValueHandleBase::ValueIsDeleted(this);
		}
		if (this->IsUsedByMetadata())
		{
			ValueAsMetadata::HandleDeletion(this);
		}

#ifdef DILITHIUM_DEBUG
		if (!this->UseEmpty())
		{
			std::clog << "While deleting: " << *type_ << " %" << this->Name() << std::endl;
			for (auto user : this->Users())
			{
				std::clog << "Use still stuck around after Def is destroyed:" << *user << std::endl;
			}
		}
#endif
		BOOST_ASSERT_MSG(this->UseEmpty(), "Uses remain when a value is destroyed!");

		this->DestroyValueName();
	}

	void Value::DestroyValueName()
	{
		name_.clear();
		name_hash_ = 0;
	}

	std::string_view Value::Name() const
	{
		return name_;
	}

	void Value::Name(std::string_view new_name)
	{
		// Fast path for common IRBuilder case of Name("") when there is no name.
		if (new_name.empty() && !this->HasName())
		{
			return;
		}

		BOOST_ASSERT_MSG(new_name.find_first_of('\0') == std::string_view::npos, "Null bytes are not allowed in names");

		if (name_ == new_name)
		{
			return;
		}

		BOOST_ASSERT_MSG(!GetType()->IsVoidType(), "Cannot assign a name to void values!");

		ValueSymbolTable* sym_tab;
		if (GetSymTab(this, sym_tab))
		{
			return;	// Cannot set a name on this value (e.g. constant).
		}

		if (!sym_tab)
		{
			if (new_name.empty())
			{
				this->DestroyValueName();
			}
			else
			{
				this->DestroyValueName();

				name_ = std::string(new_name);
				name_hash_ = boost::hash_value(new_name);
			}
		}
		else
		{
			if (this->HasName())
			{
				sym_tab->RemoveValueName(name_hash_);
				this->DestroyValueName();

				if (new_name.empty())
				{
					return;
				}
			}

			name_ = sym_tab->CreateValueName(new_name, this);
			name_hash_ = boost::hash_value(std::string_view(name_));
		}
	}

	void Value::ReplaceAllUsesWith(Value* new_val)
	{
		BOOST_ASSERT_MSG(new_val, "Value::ReplaceAllUsesWith(<null>) is invalid!");
		BOOST_ASSERT_MSG(!Contains(new_val, this), "this->ReplaceAllUsesWith(expr(this)) is NOT valid!");
		BOOST_ASSERT_MSG(new_val->GetType() == this->GetType(), "replaceAllUses of value with new value of different type!");

		if (has_value_handle_)
		{
			ValueHandleBase::ValueIsRAUWd(this, new_val);
		}
		if (this->IsUsedByMetadata())
		{
			ValueAsMetadata::HandleRAUW(this, new_val);
		}

		while (!this->UseEmpty())
		{
			auto& use = *use_list_;
			auto c = dyn_cast<Constant>(use.GetUser());
			if (c)
			{
				if (!isa<GlobalValue>(c))
				{
					c->HandleOperandChange(this, new_val, &use);
					continue;
				}
			}

			use.Set(new_val);
		}

		auto bb = dyn_cast<BasicBlock>(this);
		if (bb)
		{
			bb->ReplaceSuccessorsPhiUsesWith(cast<BasicBlock>(new_val));
		}
	}

	// Like ReplaceAllUsesWith except it does not handle constants or basic blocks.
	// This routine leaves uses within BB.
	void Value::ReplaceUsesOutsideBlock(Value* new_val, BasicBlock* bb)
	{
		BOOST_ASSERT_MSG(new_val, "Value::replaceUsesOutsideBlock(<null>, BB) is invalid!");
		BOOST_ASSERT_MSG(!Contains(new_val, this), "this->replaceUsesOutsideBlock(expr(this), BB) is NOT valid!");
		BOOST_ASSERT_MSG(new_val->GetType() == GetType(), "replaceUses of value with new value of different type!");
		BOOST_ASSERT_MSG(bb, "Basic block that may contain a use of 'New' must be defined\n");

		for (use_iterator iter = this->UseBegin(), end_iter = this->UseEnd(); iter != end_iter;)
		{
			auto& use = *iter;
			++ iter;
			auto user = dyn_cast<Instruction>(use.GetUser());
			if (user && (user->Parent() == bb))
			{
				continue;
			}
			use.Set(new_val);
		}
	}

	Value* Value::StripPointerCasts()
	{
		return StripPointerCastsAndOffsets(this);
	}

	LLVMContext& Value::Context() const
	{
		return type_->Context();
	}

	void Value::SortUseList(std::function<bool(Use const & lhs, Use const & rhs)> cmp)
	{
		if (!use_list_ || !use_list_->next_)
		{
			// No need to sort 0 or 1 uses.
			return;
		}

		// Note: this function completely ignores Prev pointers until the end when
		// they're fixed en masse.

		// Create a binomial vector of sorted lists, visiting uses one at a time and
		// merging lists as necessary.
		uint32_t constexpr MAX_SLOTS = 32;
		Use* slots[MAX_SLOTS];

		// Collect the first use, turning it into a single-item list.
		Use* next = use_list_->next_;
		use_list_->next_ = nullptr;
		uint32_t num_slots = 1;
		slots[0] = use_list_;

		// Collect all but the last use.
		while (next->next_)
		{
			Use* current = next;
			next = current->next_;

			// Turn current into a single-item list.
			current->next_ = nullptr;

			// Save current in the first available slot, merging on collisions.
			uint32_t i;
			for (i = 0; i < num_slots; ++ i)
			{
				if (!slots[i])
				{
					break;
				}

				// Merge two lists, doubling the size of current and emptying slot i.
				//
				// Since the uses in slots[i] originally preceded those in Current, send
				// slots[i] in as the left parameter to maintain a stable sort.
				current = Value::MergeUseLists(slots[i], current, cmp);
				slots[i] = nullptr;
			}
			// Check if this is a new slot.
			if (i == num_slots)
			{
				++ num_slots;
				BOOST_ASSERT_MSG(num_slots <= MAX_SLOTS, "Use list bigger than 2^32");
			}

			// Found an open slot.
			slots[i] = current;
		}

		// Merge all the lists together.
		BOOST_ASSERT_MSG(next, "Expected one more Use");
		BOOST_ASSERT_MSG(!next->next_, "Expected only one Use");
		use_list_ = next;
		for (uint32_t i = 0; i < num_slots; ++ i)
		{
			if (slots[i])
			{
				// Since the uses in slots[i] originally preceded those in use_list_, send
				// slots[i] in as the left parameter to maintain a stable sort.
				use_list_ = Value::MergeUseLists(slots[i], use_list_, cmp);
			}
		}

		// Fix the prev pointers.
		auto prev = &use_list_;
		for (auto node = use_list_; node; node = node->next_)
		{
			node->prev_ptr_ = prev;
			prev = &node->next_;
		}
	}

	Use* Value::MergeUseLists(Use* l, Use* r, std::function<bool(Use const & lhs, Use const & rhs)> cmp)
	{
		Use* merged;
		Value::MergeUseListsImpl(l, r, &merged, cmp);
		return merged;
	}

	void Value::MergeUseListsImpl(Use* l, Use* r, Use** next, std::function<bool(Use const & lhs, Use const & rhs)> cmp)
	{
		if (!l)
		{
			*next = r;
			return;
		}
		if (!r)
		{
			*next = l;
			return;
		}
		if (cmp(*r, *l))
		{
			*next = r;
			Value::MergeUseListsImpl(l, r->next_, &r->next_, cmp);
			return;
		}
		*next = l;
		Value::MergeUseListsImpl(l->next_, r, &l->next_, cmp);
	}

	void Value::Print(std::ostream& os) const
	{
		DILITHIUM_UNUSED(os);
		DILITHIUM_NOT_IMPLEMENTED;
	}

	void Value::Print(std::ostream& os, ModuleSlotTracker& mst) const
	{
		DILITHIUM_UNUSED(os);
		DILITHIUM_UNUSED(mst);
		DILITHIUM_NOT_IMPLEMENTED;
	}

	void Value::PrintAsOperand(std::ostream& os, bool print_type, LLVMModule const * mod) const
	{
		DILITHIUM_UNUSED(os);
		DILITHIUM_UNUSED(print_type);
		DILITHIUM_UNUSED(mod);
		DILITHIUM_NOT_IMPLEMENTED;
	}

	void Value::PrintAsOperand(std::ostream& os, bool print_type, ModuleSlotTracker& mst) const
	{
		DILITHIUM_UNUSED(os);
		DILITHIUM_UNUSED(print_type);
		DILITHIUM_UNUSED(mst);
		DILITHIUM_NOT_IMPLEMENTED;
	}
}
