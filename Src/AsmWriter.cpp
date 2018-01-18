/**
 * @file AsmWriter.cpp
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
#include <Dilithium/AsmWriter.hpp>

#include <Dilithium/CFG.hpp>
#include <Dilithium/Constants.hpp>
#include <Dilithium/DerivedType.hpp>
#include <Dilithium/GlobalVariable.hpp>
#include <Dilithium/Instructions.hpp>
#include <Dilithium/LLVMModule.hpp>

#include <unordered_set>

namespace
{
	using namespace Dilithium;

	class SlotTracker;
	class TypePrinting;

	enum PrefixType
	{
		GlobalPrefix,
		ComdatPrefix,
		LabelPrefix,
		LocalPrefix,
		NoPrefix
	};

	// PrintEscapedString - Print each character of the specified string, escaping
	// it if it is not printable or if it is an escape char.
	void PrintEscapedString(std::string_view name, std::ostream& os)
	{
		for (size_t i = 0, e = name.size(); i != e; ++ i)
		{
			uint8_t const c = name[i];
			if (isprint(c) && (c != '\\') && (c != '"'))
			{
				os << c;
			}
			else
			{
				os << '\\' << std::hex << static_cast<int>(c);
			}
		}
	}

	/// PrintLLVMName - Turn the specified name into an 'LLVM name', which is either
	/// prefixed with % (if the string only contains simple characters) or is
	/// surrounded with ""'s (if it has special chars in it).  Print it out.
	void PrintLLVMName(std::ostream& os, std::string_view name, PrefixType prefix)
	{
		BOOST_ASSERT_MSG(!name.empty(), "Cannot get empty name!");
		switch (prefix)
		{
		case NoPrefix:
			break;
		case GlobalPrefix:
			os << '@';
			break;
		case ComdatPrefix:
			os << '$';
			break;
		case LabelPrefix:
			break;
		case LocalPrefix:
			os << '%';
			break;
		}

		// Scan the name to see if it needs quotes first.
		bool needs_quotes = isdigit(static_cast<unsigned char>(name[0]));
		if (!needs_quotes)
		{
			for (size_t i = 0, e = name.size(); i != e; ++ i)
			{
				// By making this unsigned, the value passed in to isalnum will always be
				// in the range 0-255.  This is important when building with MSVC because
				// its implementation will assert.  This situation can arise when dealing
				// with UTF-8 multibyte characters.
				uint8_t const c = name[i];
				if (!isalnum(static_cast<unsigned char>(c)) && (c != '-') && (c != '.') && (c != '_'))
				{
					needs_quotes = true;
					break;
				}
			}
		}

		// If we didn't need any quotes, just write out the name in one blast.
		if (!needs_quotes)
		{
			os << name;
			return;
		}

		// Okay, we need quotes.  Output the quotes and escape any scary characters as
		// needed.
		os << '"';
		PrintEscapedString(name, os);
		os << '"';
	}

	void PrintLLVMName(std::ostream& os, Value const * v)
	{
		PrintLLVMName(os, v->Name(), isa<GlobalValue>(v) ? GlobalPrefix : LocalPrefix);
	}

	void PrintLinkage(GlobalValue::LinkageTypes lt, std::ostream& os)
	{
		switch (lt)
		{
		case GlobalValue::ExternalLinkage:
			break;
		case GlobalValue::PrivateLinkage:
			os << "private ";
			break;
		case GlobalValue::InternalLinkage:
			os << "internal ";
			break;
		case GlobalValue::LinkOnceAnyLinkage:
			os << "linkonce ";
			break;
		case GlobalValue::LinkOnceODRLinkage:
			os << "linkonce_odr ";
			break;
		case GlobalValue::WeakAnyLinkage:
			os << "weak ";
			break;
		case GlobalValue::WeakODRLinkage:
			os << "weak_odr ";
			break;
		case GlobalValue::CommonLinkage:
			os << "common ";
			break;
		case GlobalValue::AppendingLinkage:
			os << "appending ";
			break;
		case GlobalValue::ExternalWeakLinkage:
			os << "extern_weak ";
			break;
		case GlobalValue::AvailableExternallyLinkage:
			os << "available_externally ";
			break;
		}
	}

	void PrintVisibility(GlobalValue::VisibilityTypes vis, std::ostream& os)
	{
		switch (vis)
		{
		case GlobalValue::DefaultVisibility:
			break;
		case GlobalValue::HiddenVisibility:
			os << "hidden ";
			break;
		case GlobalValue::ProtectedVisibility:
			os << "protected ";
			break;
		}
	}

	void PrintDLLStorageClass(GlobalValue::DLLStorageClassTypes sct, std::ostream& os)
	{
		switch (sct)
		{
		case GlobalValue::DefaultStorageClass:
			break;
		case GlobalValue::DLLImportStorageClass:
			os << "dllimport ";
			break;
		case GlobalValue::DLLExportStorageClass:
			os << "dllexport ";
			break;
		}
	}

	void PrintMetadataIdentifier(std::string_view name, std::ostream& os)
	{
		if (name.empty())
		{
			os << "<empty name> ";
		}
		else
		{
			if (isalpha(static_cast<uint8_t>(name[0])) || (name[0] == '-') || (name[0] == '$') || (name[0] == '.') || (name[0] == '_'))
			{
				os << name[0];
			}
			else
			{
				os << '\\' << std::hex << static_cast<int>(name[0]);
			}
			for (size_t i = 1, e = name.size(); i != e; ++ i)
			{
				uint8_t const c = name[i];
				if (isalnum(static_cast<uint8_t>(c)) || (c == '-') || (c == '$') || (c == '.') || (c == '_'))
				{
					os << c;
				}
				else
				{
					os << '\\' << std::hex << static_cast<int>(c);
				}
			}
		}
	}

	void WriteConstantInternal(std::ostream& os, Constant const * cv, TypePrinting& type_printer, SlotTracker* machine,
		LLVMModule const * context)
	{
		auto ci = dyn_cast<ConstantInt>(cv);
		if (ci)
		{
			if (ci->GetType()->IsIntegerType(1))
			{
				os << (ci->ZExtValue() ? "true" : "false");
				return;
			}
			os << ci->GetValue();
			return;
		}

		auto cfp = dyn_cast<ConstantFP>(cv);
		if (cfp)
		{
			if ((&cfp->GetValueMPF().Semantics() == &MPFloat::IEEESingle)
				|| (&cfp->GetValueMPF().Semantics() == &MPFloat::IEEEDouble))
			{
				// We would like to output the FP constant value in exponential notation,
				// but we cannot do this if doing so will lose precision.  Check here to
				// make sure that we only output it in exponential format if we can parse
				// the value back and get the same value.
				//
				bool ignored;
				bool is_half = &cfp->GetValueMPF().Semantics() == &MPFloat::IEEEHalf;
				bool is_double = &cfp->GetValueMPF().Semantics() == &MPFloat::IEEEDouble;
				bool is_inf = cfp->GetValueMPF().IsInfinity();
				bool is_nan = cfp->GetValueMPF().IsNaN();
				if (!is_half && !is_inf && !is_nan)
				{
					double val = is_double ? cfp->GetValueMPF().ConvertToDouble() : cfp->GetValueMPF().ConvertToFloat();
					std::string str_val = std::to_string(val);

					// Check to make sure that the stringized number is not some string like
					// "Inf" or NaN, that atof will accept, but the lexer will not.  Check
					// that the string matches the "[-+]?[0-9]" regex.
					//
					if (((str_val[0] >= '0') && (str_val[0] <= '9'))
						|| (((str_val[0] == '-') || (str_val[0] == '+')) && ((str_val[1] >= '0') && (str_val[1] <= '9'))))
					{
						// Reparse stringized version!
						if (MPFloat(MPFloat::IEEEDouble, str_val).ConvertToDouble() == val)
						{
							os << str_val;
							return;
						}
					}
				}
				// Otherwise we could not reparse it to exactly the same value, so we must
				// output the string in hexadecimal format!  Note that loading and storing
				// floating point types changes the bits of NaNs on some hosts, notably
				// x86, so we must not use these types.
				static_assert(sizeof(double) == sizeof(uint64_t), "assuming that double is 64 bits!");
				MPFloat mpf = cfp->GetValueMPF();
				// Halves and floats are represented in ASCII IR as double, convert.
				if (!is_double)
				{
					mpf.Convert(MPFloat::IEEEDouble, &ignored);
				}
				os << "0x" << std::hex << mpf.BitcastToMPInt().ZExtValue();
				return;
			}

			// Either half, or some form of long double.
			// These appear as a magic letter identifying the type, then a
			// fixed number of hex digits.
			os << "0x";
			// Bit position, in the current word, of the next nibble to print.
			int shift_count;
			if (&cfp->GetValueMPF().Semantics() == &MPFloat::IEEEHalf)
			{
				shift_count = 12;
				os << 'H';
			}
			else
			{
				DILITHIUM_UNREACHABLE("Unsupported floating point type");
			}
			// api needed to prevent premature destruction
			MPInt mpi = cfp->GetValueMPF().BitcastToMPInt();
			uint64_t const word = mpi.RawData();
			int const width = mpi.BitWidth();
			BOOST_ASSERT(width <= 64);
			for (int j = 0; j < width; j += 4, shift_count -= 4)
			{
				uint32_t nibble = (word >> shift_count) & 15;
				if (nibble < 10)
				{
					os << static_cast<uint8_t>(nibble + '0');
				}
				else
				{
					os << static_cast<uint8_t>(nibble - 10 + 'A');
				}
			}
			return;
		}

		if (isa<ConstantAggregateZero>(cv))
		{
			os << "zeroinitializer";
			return;
		}

		DILITHIUM_UNUSED(context);
		DILITHIUM_UNUSED(machine);
		DILITHIUM_UNUSED(type_printer);

		/*auto ba = dyn_cast<BlockAddress>(cv);
		if (ba)
		{
			os << "blockaddress(";
			WriteAsOperandInternal(os, ba->GetFunction(), &type_printer, machine, context);
			os << ", ";
			WriteAsOperandInternal(os, ba->GetBasicBlock(), &type_printer, machine, context);
			os << ")";
			return;
		}

		auto ca = dyn_cast<ConstantArray>(cv);
		if (ca)
		{
			auto e_ty = ca->GetType()->ElementType();
			os << '[';
			type_printer.Print(e_ty, os);
			os << ' ';
			WriteAsOperandInternal(os, ca->Operand(0), &type_printer, machine, context);
			for (uint32_t i = 1, e = ca->NumOperands(); i != e; ++ i)
			{
				os << ", ";
				type_printer.Print(e_ty, os);
				os << ' ';
				WriteAsOperandInternal(os, ca->Operand(i), &type_printer, machine, context);
			}
			os << ']';
			return;
		}

		auto cda = dyn_cast<ConstantDataArray>(cv);
		if (cda)
		{
			// As a special case, print the array as a string if it is an array of
			// i8 with ConstantInt values.
			if (cda->IsString())
			{
				os << "c\"";
				PrintEscapedString(cda->GetAsString(), os);
				os << '"';
				return;
			}

			auto e_ty = cda->GetType()->ElementType();
			os << '[';
			type_printer.Print(e_ty, os);
			os << ' ';
			WriteAsOperandInternal(os, cda->GetElementAsConstant(0), &type_printer, machine, context);
			for (uint32_t i = 1, e = cda->NumElements(); i != e; ++ i)
			{
				os << ", ";
				type_printer.Print(e_ty, os);
				Out << ' ';
				WriteAsOperandInternal(os, cda->GetElementAsConstant(i), &type_printer, machine, context);
			}
			os << ']';
			return;
		}

		if (const ConstantStruct *CS = dyn_cast<ConstantStruct>(CV)) {
			if (CS->GetType()->IsPacked())
				os << '<';
			os << '{';
			unsigned N = CS->getNumOperands();
			if (N) {
				os << ' ';
				type_printer.print(CS->getOperand(0)->GetType(), os);
				os << ' ';

				WriteAsOperandInternal(os, CS->getOperand(0), &type_printer, Machine,
					Context);

				for (unsigned i = 1; i < N; i++) {
					os << ", ";
					type_printer.print(CS->getOperand(i)->GetType(), Out);
					Out << ' ';

					WriteAsOperandInternal(Out, CS->getOperand(i), &type_printer, Machine,
						Context);
				}
				os << ' ';
			}

			os << '}';
			if (CS->GetType()->IsPacked())
				os << '>';
			return;
		}

		if (isa<ConstantVector>(CV) || isa<ConstantDataVector>(CV)) {
			Type *ETy = CV->GetType()->VectorElementType();
			os << '<';
			type_printer.print(ETy, os);
			os << ' ';
			WriteAsOperandInternal(Out, CV->getAggregateElement(0U), &type_printer,
				Machine, Context);
			for (unsigned i = 1, e = CV->GetType()->VectorNumElements(); i != e; ++i) {
				Out << ", ";
				TypePrinter.print(ETy, Out);
				Out << ' ';
				WriteAsOperandInternal(Out, CV->getAggregateElement(i), &type_printer,
					Machine, Context);
			}
			os << '>';
			return;
		}

		if (isa<ConstantPointerNull>(CV)) {
			os << "null";
			return;
		}*/

		if (isa<UndefValue>(cv))
		{
			os << "undef";
			return;
		}

		/*if (const ConstantExpr *CE = dyn_cast<ConstantExpr>(CV)) {
			os << CE->getOpcodeName();
			WriteOptimizationInfo(Out, CE);
			if (CE->isCompare())
				os << ' ' << getPredicateText(CE->GetPredicate());
			os << " (";

			if (const GEPOperator *GEP = dyn_cast<GEPOperator>(CE)) {
				type_printer.print(
					cast<PointerType>(GEP->getPointerOperandType()->ScalarType())
					->ElementType(),
					os);
				os << ", ";
			}

			for (User::const_op_iterator OI = CE->op_begin(); OI != CE->op_end(); ++OI) {
				type_printer.print((*OI)->GetType(), Out);
				os << ' ';
				WriteAsOperandInternal(Out, *OI, &type_printer, Machine, Context);
				if (OI + 1 != CE->op_end())
					os << ", ";
			}

			if (CE->hasIndices()) {
				ArrayRef<unsigned> Indices = CE->getIndices();
				for (unsigned i = 0, e = Indices.size(); i != e; ++i)
					os << ", " << Indices[i];
			}

			if (CE->isCast()) {
				os << " to ";
				type_printer.print(CE->GetType(), Out);
			}

			os << ')';
			return;
		}*/

		os << "<placeholder or erroneous Constant>";

		DILITHIUM_NOT_IMPLEMENTED;
	}

	void WriteAsOperandInternal(std::ostream& os, Metadata const * md, TypePrinting* type_printer, SlotTracker* machine,
		LLVMModule const * context, bool from_value = false);
	void WriteAsOperandInternal(std::ostream& os, Value const * v, TypePrinting* type_printer, SlotTracker* machine,
		LLVMModule const * context);

	/// This class provides computation of slot numbers for LLVM Assembly writing.
	class SlotTracker : boost::noncopyable
	{
	public:
		/// ValueMap - A mapping of Values to slot numbers.
		typedef std::unordered_map<const Value*, unsigned> ValueMap;

	public:
		/// Construct from a module.
		///
		/// If \c ShouldInitializeAllMetadata, initializes all metadata in all
		/// functions, giving correct numbering for metadata referenced only from
		/// within a function (even if no functions have been initialized).
		explicit SlotTracker(LLVMModule const * module, bool should_initialize_all_metadata = false)
			: module_(module), function_(nullptr), function_processed_(false),
				should_initialize_all_metadata_(should_initialize_all_metadata),
				module_next_(0), function_next_(0), mdn_next_(0), as_next_(0)
		{
		}
		/// Construct from a function, starting out in incorp state.
		///
		/// If \c ShouldInitializeAllMetadata, initializes all metadata in all
		/// functions, giving correct numbering for metadata referenced only from
		/// within a function (even if no functions have been initialized).
		explicit SlotTracker(Function const * func, bool should_initialize_all_metadata = false)
			: module_(func ? func->Parent() : nullptr), function_(func), function_processed_(false),
				should_initialize_all_metadata_(should_initialize_all_metadata),
				module_next_(0), function_next_(0), mdn_next_(0), as_next_(0)
		{
		}

		/// Return the slot number of the specified value in it's type
		/// plane.  If something is not in the SlotTracker, return -1.
		int GetLocalSlot(Value const * v)
		{
			BOOST_ASSERT_MSG(!isa<Constant>(v), "Can't get a constant or global slot with this!");

			// Check for uninitialized state and do lazy initialization.
			this->Initialize();

			auto iter = function_map_.find(v);
			return iter == function_map_.end() ? -1 : static_cast<int>(iter->second);
		}
		int GetGlobalSlot(GlobalValue const * v)
		{
			DILITHIUM_UNUSED(v);
			DILITHIUM_NOT_IMPLEMENTED;
		}
		int GetMetadataSlot(MDNode const * n)
		{
			// Check for uninitialized state and do lazy initialization.
			this->Initialize();

			// Find the MDNode in the module map
			auto iter = mdn_map_.find(n);
			return iter == mdn_map_.end() ? -1 : static_cast<int>(iter->second);
		}
		int GetAttributeGroupSlot(AttributeSet as)
		{
			// Check for uninitialized state and do lazy initialization.
			this->Initialize();

			// Find the AttributeSet in the module map.
			auto iter = as_map_.find(as);
			return iter == as_map_.end() ? -1 : static_cast<int>(iter->second);
		}

		/// If you'd like to deal with a function instead of just a module, use
		/// this method to get its data into the SlotTracker.
		void IncorporateFunction(Function const * func)
		{
			function_ = func;
			function_processed_ = false;
		}

		Function const * GetFunction() const
		{
			return function_;
		}

		/// After calling incorporateFunction, use this method to remove the
		/// most recently incorporated function from the SlotTracker. This
		/// will reset the state of the machine back to just the module contents.
		void PurgeFunction()
		{
			function_map_.clear(); // Simply discard the function level map
			function_ = nullptr;
			function_processed_ = false;
		}

		/// MDNode map iterators.
		typedef std::unordered_map<MDNode const *, uint32_t>::iterator mdn_iterator;
		mdn_iterator MdnBegin()
		{
			return mdn_map_.begin();
		}
		mdn_iterator MdnEnd()
		{
			return mdn_map_.end();
		}
		uint32_t MdnSize() const
		{
			return static_cast<uint32_t>(mdn_map_.size());
		}
		bool MdnEmpty() const
		{
			return mdn_map_.empty();
		}

		/// AttributeSet map iterators.
		typedef std::unordered_map<AttributeSet, uint32_t>::iterator as_iterator;
		as_iterator AsBegin()
		{
			return as_map_.begin();
		}
		as_iterator AsEnd()
		{
			return as_map_.end();
		}
		uint32_t AsSize() const
		{
			return static_cast<uint32_t>(as_map_.size());
		}
		bool AsEmpty() const
		{
			return as_map_.empty();
		}

		/// This function does the actual initialization.
		void Initialize()
		{
			if (module_)
			{
				this->ProcessModule();
				module_ = nullptr;
			}

			if (function_ && !function_processed_)
			{
				this->ProcessFunction();
			}
		}

	private:
		/// CreateModuleSlot - Insert the specified GlobalValue* into the slot table.
		void CreateModuleSlot(GlobalValue const * v)
		{
			BOOST_ASSERT_MSG(v, "Can't insert a null Value into SlotTracker!");
			BOOST_ASSERT_MSG(!v->GetType()->IsVoidType(), "Doesn't need a slot!");
			BOOST_ASSERT_MSG(!v->HasName(), "Doesn't need a slot!");

			uint32_t dest_dlot = module_next_;
			++ module_next_;
			module_map_[v] = dest_dlot;
		}

		/// CreateMetadataSlot - Insert the specified MDNode* into the slot table.
		void CreateMetadataSlot(MDNode const * n)
		{
			BOOST_ASSERT_MSG(n, "Can't insert a null Value into SlotTracker!");

			uint32_t dest_slot = mdn_next_;
			if (!mdn_map_.emplace(n, dest_slot).second)
			{
				return;
			}
			++ mdn_next_;

			// Recursively add any MDNodes referenced by operands.
			for (uint32_t i = 0, e = n->NumOperands(); i != e; ++ i)
			{
				auto op = dyn_cast_or_null<MDNode>(n->Operand(i));
				if (op)
				{
					this->CreateMetadataSlot(op);
				}
			}
		}

		/// CreateFunctionSlot - Insert the specified Value* into the slot table.
		void CreateFunctionSlot(Value const * v)
		{
			BOOST_ASSERT_MSG(!v->GetType()->IsVoidType() && !v->HasName(), "Doesn't need a slot!");

			uint32_t dest_slot = function_next_;
			++ function_next_;
			function_map_[v] = dest_slot;
		}

		/// \brief Insert the specified AttributeSet into the slot table.
		void CreateAttributeSetSlot(AttributeSet as)
		{
			BOOST_ASSERT_MSG(as.HasAttributes(AttributeSet::AI_FunctionIndex), "Doesn't need a slot!");

			auto iter = as_map_.find(as);
			if (iter != as_map_.end())
			{
				return;
			}

			uint32_t dest_slot = as_next_;
			++ as_next_;
			as_map_[as] = dest_slot;
		}

		/// Add all of the module level global variables (and their initializers)
		/// and function declarations, but not the contents of those functions.
		void ProcessModule()
		{
			// Add metadata used by named metadata.
			for (auto& nmd : module_->NamedMetadata())
			{
				for (uint32_t i = 0, e = nmd.get()->NumOperands(); i != e; ++ i)
				{
					CreateMetadataSlot(nmd.get()->Operand(i));
				}
			}

			for (auto& func : *module_)
			{
				if (!func->HasName())
				{
					// Add all the unnamed functions to the table.
					this->CreateModuleSlot(func.get());
				}

				if (should_initialize_all_metadata_)
				{
					this->ProcessFunctionMetadata(*func);
				}

				// Add all the function attributes to the table.
				// FIXME: Add attributes of other objects?
				auto fn_attrs = func->GetAttributes().GetFnAttributes();
				if (fn_attrs.HasAttributes(AttributeSet::AI_FunctionIndex))
				{
					this->CreateAttributeSetSlot(fn_attrs);
				}
			}
		}

		/// Add all of the functions arguments, basic blocks, and instructions.
		void ProcessFunction()
		{
			function_next_ = 0;

			// Add all the function arguments with no names.
			for (auto arg_iter = function_->ArgBegin(), arg_end_iter = function_->ArgEnd(); arg_iter != arg_end_iter; ++arg_iter)
			{
				if (!(*arg_iter)->HasName())
				{
					this->CreateFunctionSlot(arg_iter->get());
				}
			}

			// Add all of the basic blocks and instructions with no names.
			for (auto& bb : *function_)
			{
				if (!bb->HasName())
				{
					this->CreateFunctionSlot(bb.get());
				}

				this->ProcessFunctionMetadata(*function_);

				for (auto& inst : *bb)
				{
					if (!inst->GetType()->IsVoidType() && !inst->HasName())
					{
						this->CreateFunctionSlot(inst.get());
					}

					// We allow direct calls to any llvm.foo function here, because the
					// target may not be linked into the optimizer.
					auto const * ci = dyn_cast<CallInst>(inst.get());
					if (ci)
					{
						// Add all the call attributes to the table.
						auto attrs = ci->GetAttributes().GetFnAttributes();
						if (attrs.HasAttributes(AttributeSet::AI_FunctionIndex))
						{
							this->CreateAttributeSetSlot(attrs);
						}
					}
					else
					{
						auto const * ii = dyn_cast<InvokeInst>(inst.get());
						if (ii)
						{
							// TODO: Process InvokeInst
							DILITHIUM_NOT_IMPLEMENTED;
						}
					}
				}
			}

			function_processed_ = true;
		}

		/// Add all of the metadata from a function.
		void ProcessFunctionMetadata(Function const & func)
		{
			boost::container::small_vector<std::pair<uint32_t, MDNode*>, 4> mds;
			for (auto& bb : func)
			{
				func.GetAllMetadata(mds);
				for (auto& md : mds)
				{
					this->CreateMetadataSlot(md.second);
				}

				for (auto& inst : *bb)
				{
					this->ProcessInstructionMetadata(*inst);
				}
			}
		}

		/// Add all of the metadata from an instruction.
		void ProcessInstructionMetadata(Instruction const & inst)
		{
			// Process metadata attached to this instruction.
			boost::container::small_vector<std::pair<uint32_t, MDNode*>, 4> mds;
			inst.GetAllMetadata(mds);
			for (auto& md : mds)
			{
				this->CreateMetadataSlot(md.second);
			}
		}

	private:
		/// module_ - The module for which we are holding slot numbers.
		LLVMModule const * module_;

		/// function_ - The function for which we are holding slot numbers.
		Function const * function_;
		bool function_processed_;
		bool should_initialize_all_metadata_;

		/// module_map_ - The slot map for the module level data.
		ValueMap module_map_;
		uint32_t module_next_;

		/// function_map_ - The slot map for the function level data.
		ValueMap function_map_;
		uint32_t function_next_;

		/// mdn_map_ - Map for MDNodes.
		std::unordered_map<MDNode const *, uint32_t> mdn_map_;
		uint32_t mdn_next_;

		/// as_map_ - The slot map for attribute sets.
		std::unordered_map<AttributeSet, uint32_t> as_map_;
		uint32_t as_next_;
	};

	class TypeFinder
	{
	public:
		TypeFinder()
			: only_named_(false)
		{
		}

		void Run(LLVMModule const & module, bool only_named)
		{
			only_named_ = only_named;

			// Get types from functions.
			boost::container::small_vector<std::pair<unsigned, MDNode *>, 4> md_for_inst;
			for (auto func_iter = module.begin(), end_iter = module.end(); func_iter != end_iter; ++ func_iter)
			{
				auto& func = *(func_iter->get());

				this->IncorporateType(func.GetType());

				if (func.HasPrefixData())
				{
					this->IncorporateValue(func.GetPrefixData());
				}

				if (func.HasPrologueData())
				{
					this->IncorporateValue(func.GetPrologueData());
				}

				if (func.HasPersonalityFn())
				{
					this->IncorporateValue(func.GetPersonalityFn());
				}

				// First incorporate the arguments.
				for (auto arg_iter = func.ArgBegin(), arg_end_iter = func.ArgEnd(); arg_iter != arg_end_iter; ++ arg_iter)
				{
					this->IncorporateValue(arg_iter->get());
				}

				for (auto bb = func.begin(), end_bb = func.end(); bb != end_bb; ++ bb)
				{
					for (auto inst_iter = (*bb)->begin(), end_inst_iter = (*bb)->end(); inst_iter != end_inst_iter; ++ inst_iter)
					{
						auto const & inst = *(inst_iter->get());

						// Incorporate the type of the instruction.
						this->IncorporateType(inst.GetType());

						// Incorporate non-instruction operand types. (We are incorporating all
						// instructions with this loop.)
						for (auto op_iter = inst.OpBegin(), op_end_iter = inst.OpEnd(); op_iter != op_end_iter; ++ op_iter)
						{
							if (*op_iter && !isa<Instruction>(op_iter))
							{
								this->IncorporateValue(*op_iter);
							}
						}

						// Incorporate types hiding in metadata.
						inst.GetAllMetadataOtherThanDebugLoc(md_for_inst);
						for (size_t i = 0, e = md_for_inst.size(); i != e; ++ i)
						{
							this->IncorporateMDNode(md_for_inst[i].second);
						}

						md_for_inst.clear();
					}
				}
			}

			for (auto iter = module.NamedMetadataBegin(), end_iter = module.NamedMetadataEnd(); iter != end_iter; ++ iter)
			{
				auto nmd = iter->get();
				for (uint32_t i = 0, e = nmd->NumOperands(); i != e; ++ i)
				{
					this->IncorporateMDNode(nmd->Operand(i));
				}
			}
		}
		void Clear();

		typedef std::vector<StructType*>::iterator iterator;
		typedef std::vector<StructType*>::const_iterator const_iterator;

		iterator begin()
		{
			return struct_types_.begin();
		}
		iterator end()
		{
			return struct_types_.end();
		}

		const_iterator begin() const
		{
			return struct_types_.begin();
		}
		const_iterator end() const
		{
			return struct_types_.end();
		}

		bool empty() const
		{
			return struct_types_.empty();
		}
		size_t size() const
		{
			return struct_types_.size();
		}
		iterator erase(iterator beg, iterator end)
		{
			return struct_types_.erase(beg, end);
		}

		StructType*& operator[](size_t idx)
		{
			return struct_types_[idx];
		}

	private:
		/// IncorporateType - This method adds the type to the list of used
		/// structures if it's not in there already.
		void IncorporateType(Type* ty)
		{
			// Check to see if we've already visited this type.
			if (!visited_types_.insert(ty).second)
			{
				return;
			}

			boost::container::small_vector<Type *, 4> type_worklist;
			type_worklist.push_back(ty);
			do
			{
				ty = type_worklist.back();
				type_worklist.pop_back();

				// If this is a structure or opaque type, add a name for the type.
				if (auto s_ty = dyn_cast<StructType>(ty))
				{
					if (!only_named_ || s_ty->HasName())
					{
						struct_types_.push_back(s_ty);
					}
				}

				// Add all unvisited subtypes to worklist for processing
				for (auto iter = ty->SubtypeRBegin(), end_iter = ty->SubtypeREnd(); iter != end_iter; ++ iter)
				{
					if (visited_types_.insert(*iter).second)
					{
						type_worklist.push_back(*iter);
					}
				}
			} while (!type_worklist.empty());
		}

		/// IncorporateValue - This method is used to walk operand lists finding types
		/// hiding in constant expressions and other operands that won't be walked in
		/// other ways.  GlobalValues, basic blocks, instructions, and inst operands
		/// are all explicitly enumerated.
		void IncorporateValue(Value const * v)
		{
			auto m = dyn_cast<MetadataAsValue>(v);
			if (m)
			{
				auto n = dyn_cast<MDNode>(m->GetMetadata());
				if (n)
				{
					return this->IncorporateMDNode(n);
				}
				auto mdv = dyn_cast<ValueAsMetadata>(m->GetMetadata());
				if (mdv)
				{
					return this->IncorporateValue(mdv->GetValue());
				}
				return;
			}

			if (!isa<Constant>(v) || isa<GlobalValue>(v))
			{
				return;
			}

			// Already visited?
			if (!visited_constants_.insert(v).second)
			{
				return;
			}

			// Check this type.
			this->IncorporateType(v->GetType());

			// If this is an instruction, we incorporate it separately.
			if (isa<Instruction>(v))
			{
				return;
			}

			// Look in operands for types.
			auto u = cast<User>(v);
			for (auto iter = u->OpBegin(), end_iter = u->OpEnd(); iter != end_iter; ++ iter)
			{
				this->IncorporateValue(*iter);
			}
		}

		/// IncorporateMDNode - This method is used to walk the operands of an MDNode
		/// to find types hiding within.
		void IncorporateMDNode(MDNode const * v)
		{
			if (!visited_metadata_.insert(v).second)
			{
				return;
			}

			for (uint32_t i = 0, e = v->NumOperands(); i != e; ++ i)
			{
				Metadata* op = v->Operand(i);
				if (!op)
				{
					continue;
				}
				auto n = dyn_cast<MDNode>(op);
				if (n)
				{
					this->IncorporateMDNode(n);
					continue;
				}
				auto c = dyn_cast<ConstantAsMetadata>(op);
				if (c)
				{
					this->IncorporateValue(c->GetValue());
					continue;
				}
			}
		}

	private:
		// To avoid walking constant expressions multiple times and other IR
		// objects, we keep several helper maps.
		std::unordered_set<Value const *> visited_constants_;
		std::unordered_set<MDNode const *> visited_metadata_;
		std::unordered_set<Type*> visited_types_;

		std::vector<StructType*> struct_types_;
		bool only_named_;
	};

	class TypePrinting : boost::noncopyable
	{
	public:
		/// NamedTypes - The named types that are used by the current module.
		TypeFinder named_types_;

		/// NumberedTypes - The numbered types, along with their value.
		std::unordered_map<StructType*, uint32_t> numbered_types_;

		TypePrinting() = default;

		void IncorporateTypes(LLVMModule const & module)
		{
			named_types_.Run(module, false);

			// The list of struct types we got back includes all the struct types, split
			// the unnamed ones out to a numbering and remove the anonymous structs.
			uint32_t next_number = 0;

			auto next_to_use = named_types_.begin();
			for (auto iter = named_types_.begin(), end_iter = named_types_.end(); iter != end_iter; ++ iter)
			{
				auto s_ty = *iter;

				// Ignore anonymous types.
				if (s_ty->IsLiteral())
				{
					continue;
				}

				if (s_ty->Name().empty())
				{
					numbered_types_[s_ty] = next_number;
					++ next_number;
				}
				else
				{
					*next_to_use = s_ty;
					++ next_to_use;
				}
			}

			named_types_.erase(next_to_use, named_types_.end());
		}

		void Print(Type* ty, std::ostream& os)
		{
			switch (ty->GetTypeId())
			{
			case Type::TID_Void:
				os << "void";
				return;
			case Type::TID_Half:
				os << "half";
				return;
			case Type::TID_Float:
				os << "float";
				return;
			case Type::TID_Double:
				os << "double";
				return;
			case Type::TID_X86Fp80:
				os << "x86_fp80";
				return;
			case Type::TID_Fp128:
				os << "fp128";
				return;
			case Type::TID_PpcFp128:
				os << "ppc_fp128";
				return;
			case Type::TID_Label:
				os << "label";
				return;
			case Type::TID_Metadata:
				os << "metadata";
				return;
			case Type::TID_X86Mmx:
				os << "x86_mmx";
				return;
			case Type::TID_Integer:
				os << 'i' << cast<IntegerType>(ty)->BitWidth();
				return;

			case Type::TID_Function:
				{
					auto func_ty = cast<FunctionType>(ty);
					this->Print(func_ty->ReturnType(), os);
					os << " (";
					for (auto iter = func_ty->ParamBegin(), end_iter = func_ty->ParamEnd(); iter != end_iter; ++ iter)
					{
						if (iter != func_ty->ParamBegin())
						{
							os << ", ";
						}
						this->Print(*iter, os);
					}
					if (func_ty->IsVarArg())
					{
						if (func_ty->NumParams())
						{
							os << ", ";
						}
						os << "...";
					}
					os << ')';
					return;
				}
			case Type::TID_Struct:
				{
					auto s_ty = cast<StructType>(ty);

					if (s_ty->IsLiteral())
					{
						return this->PrintStructBody(s_ty, os);
					}

					if (!s_ty->Name().empty())
					{
						return PrintLLVMName(os, s_ty->Name(), LocalPrefix);
					}

					auto iter = numbered_types_.find(s_ty);
					if (iter != numbered_types_.end())
					{
						os << '%' << iter->second;
					}
					else  // Not enumerated, print the hex address.
					{
						os << "%\"type " << s_ty << '\"';
					}
					return;
				}
			case Type::TID_Pointer:
				{
					auto p_ty = cast<PointerType>(ty);
					this->Print(p_ty->ElementType(), os);
					uint32_t address_space = p_ty->AddressSpace();
					if (address_space)
					{
						os << " addrspace(" << address_space << ')';
					}
					os << '*';
					return;
				}
			case Type::TID_Array:
				{
					auto a_ty = cast<ArrayType>(ty);
					os << '[' << a_ty->NumElements() << " x ";
					this->Print(a_ty->ElementType(), os);
					os << ']';
					return;
				}
			case Type::TID_Vector:
				{
					auto p_ty = cast<VectorType>(ty);
					os << "<" << p_ty->NumElements() << " x ";
					this->Print(p_ty->ElementType(), os);
					os << '>';
					return;
				}
			}
			
			DILITHIUM_UNREACHABLE("Invalid TypeID");
		}
		void PrintStructBody(StructType const * ty, std::ostream& os)
		{
			if (ty->IsOpaque())
			{
				os << "opaque";
				return;
			}

			if (ty->IsPacked())
			{
				os << '<';
			}

			if (ty->NumElements() == 0)
			{
				os << "{}";
			}
			else
			{
				auto iter = ty->ElementBegin();
				os << "{ ";
				this->Print(*iter, os);
				++ iter;
				for (auto end_iter = ty->ElementEnd(); iter != end_iter; ++ iter)
				{
					os << ", ";
					this->Print(*iter, os);
				}
				os << " }";
			}
			if (ty->IsPacked())
			{
				os << '>';
			}
		}
	};

	std::unique_ptr<SlotTracker> CreateSlotTracker(Value const * v)
	{
		auto fa = dyn_cast<Argument>(v);
		if (fa)
		{
			return std::make_unique<SlotTracker>(fa->Parent());
		}

		auto inst = dyn_cast<Instruction>(v);
		if (inst)
		{
			if (inst->Parent())
			{
				return std::make_unique<SlotTracker>(inst->Parent()->Parent());
			}
		}

		auto bb = dyn_cast<BasicBlock>(v);
		if (bb)
		{
			return std::make_unique<SlotTracker>(bb->Parent());
		}

		auto gv = dyn_cast<GlobalVariable>(v);
		if (gv)
		{
			return std::make_unique<SlotTracker>(gv->Parent());
		}

		auto func = dyn_cast<Function>(v);
		if (func)
		{
			return std::make_unique<SlotTracker>(func);
		}

		return std::unique_ptr<SlotTracker>();
	}

	void WriteMDTuple(std::ostream& os, MDTuple const * node, TypePrinting* type_printer, SlotTracker* machine,
		LLVMModule const * context)
	{
		os << "!{";
		for (uint32_t mi = 0, me = node->NumOperands(); mi != me; ++ mi)
		{
			Metadata const* md = node->Operand(mi);
			if (!md)
			{
				os << "null";
			}
			else
			{
				auto mdv = dyn_cast<ValueAsMetadata>(md);
				if (mdv)
				{
					auto v = mdv->GetValue();
					type_printer->Print(v->GetType(), os);
					os << ' ';
					WriteAsOperandInternal(os, v, type_printer, machine, context);
				}
				else
				{
					WriteAsOperandInternal(os, md, type_printer, machine, context);
				}
			}

			if (mi + 1 != me)
			{
				os << ", ";
			}
		}

		os << "}";
	}

	void WriteMDNodeBodyInternal(std::ostream& os, MDNode const * node, TypePrinting* type_printer, SlotTracker* machine,
		LLVMModule const * context)
	{
		if (node->IsDistinct())
		{
			os << "distinct ";
		}
		else if (node->IsTemporary())
		{
			os << "<temporary!> "; // Handle broken code.
		}

		switch (node->MetadataId())
		{
		default:
			DILITHIUM_UNREACHABLE("Expected uniquable MDNode");
#define HANDLE_MDNODE_LEAF(CLASS)													\
		case Metadata::CLASS##Kind:													\
			Write##CLASS(os, cast<CLASS>(node), type_printer, machine, context);	\
			break;
#include "Dilithium/Metadata.inc"
		}
	}


	// Full implementation of printing a Value as an operand with support for
	// TypePrinting, etc.
	void WriteAsOperandInternal(std::ostream& os, Value const * v, TypePrinting* type_printer, SlotTracker* machine,
		LLVMModule const * context)
	{
		if (v->HasName())
		{
			PrintLLVMName(os, v);
			return;
		}

		auto cv = dyn_cast<Constant>(v);
		if (cv && !isa<GlobalValue>(cv))
		{
			BOOST_ASSERT_MSG(type_printer, "Constants require TypePrinting!");
			WriteConstantInternal(os, cv, *type_printer, machine, context);
			return;
		}

		auto* md = dyn_cast<MetadataAsValue>(v);
		if (md)
		{
			WriteAsOperandInternal(os, md->GetMetadata(), type_printer, machine, context, /* FromValue */ true);
			return;
		}

		char prefix = '%';
		int slot;
		// If we have a SlotTracker, use it.
		if (machine)
		{
			auto gv = dyn_cast<GlobalValue>(v);
			if (gv)
			{
				slot = machine->GetGlobalSlot(gv);
				prefix = '@';
			}
			else
			{
				slot = machine->GetLocalSlot(v);

				// If the local value didn't succeed, then we may be referring to a value
				// from a different function.  Translate it, as this can happen when using
				// address of blocks.
				if (slot == -1)
				{
					auto slot_tracker = CreateSlotTracker(v);
					if (slot_tracker)
					{
						slot = slot_tracker->GetLocalSlot(v);
					}
				}
			}
		}
		else
		{
			auto slot_tracker = CreateSlotTracker(v);
			if (slot_tracker)
			{
				// Otherwise, create one to get the # and then destroy it.
				auto gv = dyn_cast<GlobalValue>(v);
				if (gv)
				{
					slot = slot_tracker->GetGlobalSlot(gv);
					prefix = '@';
				}
				else
				{
					slot = slot_tracker->GetLocalSlot(v);
				}
			}
			else
			{
				slot = -1;
			}
		}

		if (slot != -1)
		{
			os << prefix << slot;
		}
		else
		{
			os << "<badref>";
		}
	}

	void WriteAsOperandInternal(std::ostream& os, Metadata const * md, TypePrinting* type_printer, SlotTracker* machine,
		LLVMModule const * context, bool from_value)
	{
		auto n = dyn_cast<MDNode>(md);
		if (n)
		{
			std::unique_ptr<SlotTracker> machine_storage;
			if (!machine) {
				machine_storage = std::make_unique<SlotTracker>(context);
				machine = machine_storage.get();
			}
			int const slot = machine->GetMetadataSlot(n);
			if (slot == -1)
			{
				// Give the pointer value instead of "badref", since this comes up all
				// the time when debugging.
				os << "<" << n << ">";
			}
			else
			{
				os << '!' << slot;
			}
			return;
		}

		auto mds = dyn_cast<MDString>(md);
		if (mds)
		{
			os << "!\"";
			PrintEscapedString(mds->String(), os);
			os << '"';
			return;
		}

		auto v = cast<ValueAsMetadata>(md);
		BOOST_ASSERT_MSG(type_printer, "TypePrinter required for metadata values");
		BOOST_ASSERT_MSG(from_value || !isa<LocalAsMetadata>(v), "Unexpected function-local metadata outside of value argument");
		DILITHIUM_UNUSED(from_value);

		type_printer->Print(v->GetValue()->GetType(), os);
		os << ' ';
		WriteAsOperandInternal(os, v->GetValue(), type_printer, machine, context);
	}

	class AssemblyWriter
	{
	public:
		AssemblyWriter(std::ostream& os, SlotTracker& mac, LLVMModule const * module, AssemblyAnnotationWriter* aaw)
			: os_(os), module_(module), machine_(mac), annotation_writer_(aaw)
		{
			this->Init();
		}

		void PrintMDNodeBody(MDNode const * node)
		{
			WriteMDNodeBodyInternal(os_, node, &type_printer_, &machine_, module_);
		}
		void PrintNamedMDNode(NamedMDNode const * nmd)
		{
			os_ << '!';
			PrintMetadataIdentifier(nmd->GetName(), os_);
			os_ << " = !{";
			for (uint32_t i = 0, e = nmd->NumOperands(); i != e; ++ i)
			{
				if (i)
				{
					os_ << ", ";
				}
				int slot = machine_.GetMetadataSlot(nmd->Operand(i));
				if (slot == -1)
				{
					os_ << "<badref>";
				}
				else
				{
					os_ << '!' << slot;
				}
			}
			os_ << "}\n";
		}

		void PrintModule(LLVMModule const * module)
		{
			machine_.Initialize();

			if (!module->Name().empty() &&
				// Don't print the ID if it will start a new line (which would
				// require a comment char before it).
				module->Name().find('\n') == std::string::npos)
			{
				os_ << "; ModuleID = '" << module->Name() << "'\n";
			}

			auto const & dl = module->GetDataLayoutStr();
			if (!dl.empty())
			{
				os_ << "target datalayout = \"" << dl << "\"\n";
			}
			if (!module->GetTargetTriple().empty())
			{
				os_ << "target triple = \"" << module->GetTargetTriple() << "\"\n";
			}

			this->PrintTypeIdentities();

			// Output all of the functions.
			for (auto& func : *module)
			{
				this->PrintFunction(func.get());
			}

			// Output all attribute groups.
			if (!machine_.AsEmpty())
			{
				os_ << '\n';
				this->WriteAllAttributeGroups();
			}

			// Output named metadata.
			if (!module->NamedMetadataEmpty())
			{
				os_ << '\n';
			}

			for (auto& node : module->NamedMetadata())
			{
				this->PrintNamedMDNode(node.get());
			}

			// Output metadata.
			if (!machine_.MdnEmpty())
			{
				os_ << '\n';
				this->WriteAllMDNodes();
			}
		}

		void WriteOperand(Value const * op, bool print_type)
		{
			if (!op)
			{
				os_ << "<null operand!>";
				return;
			}
			if (print_type)
			{
				type_printer_.Print(op->GetType(), os_);
				os_ << ' ';
			}
			WriteAsOperandInternal(os_, op, &type_printer_, &machine_, module_);
		}
		void WriteParamOperand(Value const * op, AttributeSet attrs, uint32_t idx)
		{
			if (!op)
			{
				os_ << "<null operand!>";
				return;
			}

			// Print the type
			type_printer_.Print(op->GetType(), os_);
			// Print parameter attributes list
			if (attrs.HasAttributes(idx))
			{
				os_ << ' ' << attrs.GetAsString(idx);
			}
			os_ << ' ';
			// Print the operand
			WriteAsOperandInternal(os_, op, &type_printer_, &machine_, module_);
		}

		void WriteAllMDNodes()
		{
			boost::container::small_vector<const MDNode *, 16> nodes;
			nodes.resize(machine_.MdnSize());
			for (auto iter = machine_.MdnBegin(), end_iter = machine_.MdnEnd(); iter != end_iter; ++iter)
			{
				nodes[iter->second] = cast<MDNode>(iter->first);
			}

			for (uint32_t i = 0, e = static_cast<uint32_t>(nodes.size()); i != e; ++ i)
			{
				this->WriteMDNode(i, nodes[i]);
			}
		}
		void WriteMDNode(uint32_t slot, MDNode const * node)
		{
			os_ << '!' << slot << " = ";
			this->PrintMDNodeBody(node);
			os_ << "\n";
		}
		void WriteAllAttributeGroups()
		{
			std::vector<std::pair<AttributeSet, uint32_t>> as_vec;
			as_vec.resize(machine_.AsSize());

			for (auto iter = machine_.AsBegin(), end_iter = machine_.AsEnd(); iter != end_iter; ++iter)
			{
				as_vec[iter->second] = *iter;
			}

			for (auto iter = as_vec.begin(), end_iter = as_vec.end(); iter != end_iter; ++ iter)
			{
				os_ << "attributes #" << iter->second << " = { "
					<< iter->first.GetAsString(AttributeSet::AI_FunctionIndex, true) << " }\n";
			}
		}

		void PrintTypeIdentities()
		{
			if (type_printer_.numbered_types_.empty() && type_printer_.named_types_.empty())
			{
				return;
			}

			os_ << '\n';

			// We know all the numbers that each type is used and we know that it is a
			// dense assignment.  Convert the map to an index table.
			std::vector<StructType*> numbered_types(type_printer_.numbered_types_.size());
			for (auto iter = type_printer_.numbered_types_.begin(), end_iter = type_printer_.numbered_types_.end();
				iter != end_iter; ++ iter)
			{
				BOOST_ASSERT_MSG(iter->second < numbered_types.size(), "Didn't get a dense numbering?");
				numbered_types[iter->second] = iter->first;
			}

			// Emit all numbered types.
			for (size_t i = 0, e = numbered_types.size(); i != e; ++ i)
			{
				os_ << '%' << i << " = type ";

				// Make sure we print out at least one level of the type structure, so
				// that we do not get %2 = type %2
				type_printer_.PrintStructBody(numbered_types[i], os_);
				os_ << '\n';
			}

			for (size_t i = 0, e = type_printer_.named_types_.size(); i != e; ++ i)
			{
				PrintLLVMName(os_, type_printer_.named_types_[i]->Name(), LocalPrefix);
				os_ << " = type ";

				// Make sure we print out at least one level of the type structure, so
				// that we do not get %FILE = type %FILE
				type_printer_.PrintStructBody(type_printer_.named_types_[i], os_);
				os_ << '\n';
			}
		}
		void PrintGlobal(GlobalVariable const * gv);
		void PrintFunction(Function const * func)
		{
			// Print out the return type and name.
			os_ << '\n';

			if (annotation_writer_)
			{
				annotation_writer_->EmitFunctionAnnot(func, os_);
			}

			if (func->IsMaterializable())
			{
				os_ << "; Materializable\n";
			}

			auto const & attrs = func->GetAttributes();
			if (attrs.HasAttributes(AttributeSet::AI_FunctionIndex))
			{
				auto as = attrs.GetFnAttributes();
				std::string attr_str;

				uint32_t idx = 0;
				for (uint32_t e = as.NumSlots(); idx != e; ++idx)
				{
					if (as.SlotIndex(idx) == AttributeSet::AI_FunctionIndex)
					{
						break;
					}
				}

				for (auto iter = as.Begin(idx), end_iter = as.End(idx); iter != end_iter; ++ iter)
				{
					auto attr = *iter;
					if (!attr.IsStringAttribute())
					{
						if (!attr_str.empty())
						{
							attr_str += ' ';
						}
						attr_str += attr.GetAsString();
					}
				}

				if (!attr_str.empty())
				{
					os_ << "; Function Attrs: " << attr_str << '\n';
				}
			}

			if (func->IsDeclaration())
			{
				os_ << "declare ";
			}
			else
			{
				os_ << "define ";
			}

			PrintLinkage(func->Linkage(), os_);
			PrintVisibility(func->Visibility(), os_);
			PrintDLLStorageClass(func->DLLStorageClass(), os_);

			// Print the calling convention.
			BOOST_ASSERT(func->GetCallingConv() == CallingConv::C);

			auto ft = func->GetFunctionType();
			if (attrs.HasAttributes(AttributeSet::AI_ReturnIndex))
			{
				os_ << attrs.GetAsString(AttributeSet::AI_ReturnIndex) << ' ';
			}
			type_printer_.Print(func->GetReturnType(), os_);
			os_ << ' ';
			WriteAsOperandInternal(os_, func, &type_printer_, &machine_, func->Parent());
			os_ << '(';
			machine_.IncorporateFunction(func);

			// Loop over the arguments, printing them...

			uint32_t idx = 1;
			if (!func->IsDeclaration())
			{
				// If this isn't a declaration, print the argument names as well.
				for (auto iter = func->ArgBegin(), end_iter = func->ArgEnd(); iter != end_iter; ++ iter)
				{
					// Insert commas as we go... the first arg doesn't get a comma
					if (iter != func->ArgBegin())
					{
						os_ << ", ";
					}
					this->PrintArgument(iter->get(), attrs, idx);
					++ idx;
				}
			}
			else
			{
				// Otherwise, print the types from the function type.
				for (uint32_t i = 0, e = ft->NumParams(); i != e; ++ i)
				{
					// Insert commas as we go... the first arg doesn't get a comma
					if (i)
					{
						os_ << ", ";
					}

					// Output type...
					type_printer_.Print(ft->ParamType(i), os_);

					if (attrs.HasAttributes(i + 1))
					{
						os_ << ' ' << attrs.GetAsString(i + 1);
					}
				}
			}

			// Finish printing arguments...
			if (ft->IsVarArg())
			{
				if (ft->NumParams())
				{
					os_ << ", ";
				}
				os_ << "...";  // Output varargs portion of signature!
			}
			os_ << ')';
			if (func->HasUnnamedAddr())
			{
				os_ << " unnamed_addr";
			}
			if (attrs.HasAttributes(AttributeSet::AI_FunctionIndex))
			{
				os_ << " #" << machine_.GetAttributeGroupSlot(attrs.GetFnAttributes());
			}
			if (func->HasSection())
			{
				os_ << " section \"";
				PrintEscapedString(func->GetSection(), os_);
				os_ << '"';
			}
			if (func->GetAlignment())
			{
				os_ << " align " << func->GetAlignment();
			}
			if (func->HasPrefixData())
			{
				os_ << " prefix ";
				this->WriteOperand(func->GetPrefixData(), true);
			}
			if (func->HasPrologueData())
			{
				os_ << " prologue ";
				this->WriteOperand(func->GetPrologueData(), true);
			}
			if (func->HasPersonalityFn())
			{
				os_ << " personality ";
				this->WriteOperand(func->GetPersonalityFn(), /*PrintType=*/true);
			}

			boost::container::small_vector<std::pair<unsigned, MDNode *>, 4> mds;
			func->GetAllMetadata(mds);
			this->PrintMetadataAttachments(mds, " ");

			if (func->IsDeclaration())
			{
				os_ << '\n';
			}
			else
			{
				os_ << " {";
				// Output all of the function's basic blocks.
				for (Function::const_iterator iter = func->begin(), end_iter = func->end(); iter != end_iter; ++ iter)
				{
					this->PrintBasicBlock(iter->get());
				}

				os_ << "}\n";
			}

			machine_.PurgeFunction();
		}

		void PrintArgument(Argument const * fa, AttributeSet attrs, uint32_t idx)
		{
			DILITHIUM_UNUSED(fa);
			DILITHIUM_UNUSED(attrs);
			DILITHIUM_UNUSED(idx);

			DILITHIUM_NOT_IMPLEMENTED;
		}

		void PrintBasicBlock(BasicBlock const * bb)
		{
			if (bb->HasName())
			{
				// Print out the label if it exists...
				os_ << "\n";
				PrintLLVMName(os_, bb->Name(), LabelPrefix);
				os_ << ':';
			}
			else if (!bb->UseEmpty())
			{
				// Don't print block # of no uses...
				os_ << "\n; <label>:";
				int slot = machine_.GetLocalSlot(bb);
				if (slot != -1)
				{
					os_ << slot;
				}
				else
				{
					os_ << "<badref>";
				}
			}

			if (!bb->Parent())
			{
				os_ << "; Error: Block without parent!";
			}
			else if (bb != &bb->Parent()->EntryBlock())
			{
				// Not the entry block?
				// Output predecessors for the block.
				os_ << ";";
				auto pred_iter = pred_begin(bb);
				auto pred_end_iter = pred_end(bb);

				if (pred_iter == pred_end_iter)
				{
					os_ << " No predecessors!";
				}
				else
				{
					os_ << " preds = ";
					this->WriteOperand(*pred_iter, false);
					for (++ pred_iter; pred_iter != pred_end_iter; ++ pred_iter)
					{
						os_ << ", ";
						this->WriteOperand(*pred_iter, false);
					}
				}
			}

			os_ << "\n";

			if (annotation_writer_)
			{
				annotation_writer_->EmitBasicBlockStartAnnot(bb, os_);
			}

			// Output all of the instructions in the basic block...
			for (auto iter = bb->begin(), end_iter = bb->end(); iter != end_iter; ++ iter)
			{
				this->PrintInstructionLine(*iter->get());
			}

			if (annotation_writer_)
			{
				annotation_writer_->EmitBasicBlockEndAnnot(bb, os_);
			}
		}

		void PrintInstructionLine(Instruction const & inst)
		{
			this->PrintInstruction(inst);
			os_ << '\n';
		}

		void PrintInstruction(Instruction const & inst)
		{
			if (annotation_writer_)
			{
				annotation_writer_->EmitInstructionAnnot(&inst, os_);
			}

			// Print out indentation for an instruction.
			os_ << "  ";

			// Print out name if it exists...
			if (inst.HasName())
			{
				PrintLLVMName(os_, &inst);
				os_ << " = ";
			}
			else if (!inst.GetType()->IsVoidType())
			{
				// Print out the def slot taken.
				int slot_num = machine_.GetLocalSlot(&inst);
				if (slot_num == -1)
				{
					os_ << "<badref> = ";
				}
				else
				{
					os_ << '%' << slot_num << " = ";
				}
			}

			{
				auto const * ci = dyn_cast<CallInst>(&inst);
				if (ci)
				{
					if (ci->IsMustTailCall())
					{
						os_ << "musttail ";
					}
					else if (ci->IsTailCall())
					{
						os_ << "tail ";
					}
				}
			}

			// Print out the opcode...
			os_ << inst.OpcodeName();

			// TODO: more instructions

			// Print out the type of the operands...
			auto const * operand = inst.NumOperands() ? inst.Operand(0) : nullptr;

			{
				auto const * ci = dyn_cast<CallInst>(&inst);
				if (ci)
				{
					// Print the calling convention being used.
					BOOST_ASSERT(ci->GetCallingConv() == CallingConv::C);

					operand = ci->GetCalledValue();
					auto fty = cast<FunctionType>(ci->GetFunctionType());
					auto ret_ty = fty->ReturnType();
					auto const & pal = ci->GetAttributes();

					if (pal.HasAttributes(AttributeSet::AI_ReturnIndex))
					{
						os_ << ' ' << pal.GetAsString(AttributeSet::AI_ReturnIndex);
					}

					// If possible, print out the short form of the call instruction.  We can
					// only do this if the first argument is a pointer to a nonvararg function,
					// and if the return type is not a pointer to a function.
					//
					os_ << ' ';
					type_printer_.Print(fty->IsVarArg() ? fty : ret_ty, os_);
					os_ << ' ';
					this->WriteOperand(operand, false);
					os_ << '(';
					for (uint32_t op = 0, end_op = ci->NumArgOperands(); op < end_op; ++op)
					{
						if (op > 0)
						{
							os_ << ", ";
						}
						this->WriteParamOperand(ci->ArgOperand(op), pal, op + 1);
					}

					// Emit an ellipsis if this is a musttail call in a vararg function.  This
					// is only to aid readability, musttail calls forward varargs by default.
					if (ci->IsMustTailCall() && ci->Parent()
						&& ci->Parent()->Parent()
						&& ci->Parent()->Parent()->IsVarArg())
					{
						os_ << ", ...";
					}

					os_ << ')';
					if (pal.HasAttributes(AttributeSet::AI_FunctionIndex))
					{
						os_ << " #" << machine_.GetAttributeGroupSlot(pal.GetFnAttributes());
					}
				}

				// TODO: More instructions
			}

			// Print Metadata info.
			boost::container::small_vector<std::pair<uint32_t, MDNode*>, 4> inst_md;
			inst.GetAllMetadata(inst_md);
			this->PrintMetadataAttachments(inst_md, ", ");

			// Print a nice comment.
			this->PrintInfoComment(inst);
		}

	private:
		void Init()
		{
			if (module_)
			{
				type_printer_.IncorporateTypes(*module_);
			}
		}

		/// \brief Print out metadata attachments.
		void PrintMetadataAttachments(boost::container::small_vector_base<std::pair<uint32_t, MDNode*>> const & mds,
			std::string_view separator)
		{
			if (mds.empty())
				return;

			if (md_names_.empty())
			{
				module_->MdKindNames(md_names_);
			}

			for (auto const & md : mds)
			{
				uint32_t kind = md.first;
				os_ << separator;
				if (kind < md_names_.size())
				{
					os_ << "!";
					PrintMetadataIdentifier(md_names_[kind], os_);
				}
				else
				{
					os_ << "!<unknown kind #" << kind << ">";
				}
				os_ << ' ';
				WriteAsOperandInternal(os_, md.second, &type_printer_, &machine_, module_);
			}
		}

		// PrintInfoComment - Print a little comment after the instruction indicating
		// which slot it occupies.
		void PrintInfoComment(Value const & val)
		{
			if (annotation_writer_)
			{
				annotation_writer_->PrintInfoComment(val, os_);
			}
		}

	private:
		std::ostream& os_;
		LLVMModule const * module_;
		std::unique_ptr<SlotTracker> slot_tracker_storage_;
		SlotTracker& machine_;
		TypePrinting type_printer_;
		AssemblyAnnotationWriter* annotation_writer_;
		boost::container::small_vector<std::string_view, 8> md_names_;
	};
}

namespace Dilithium
{
	AssemblyAnnotationWriter::~AssemblyAnnotationWriter()
	{
	}

	void AssemblyAnnotationWriter::EmitFunctionAnnot(Function const * func, std::ostream& os)
	{
		DILITHIUM_UNUSED(func);
		DILITHIUM_UNUSED(os);
	}

	void AssemblyAnnotationWriter::EmitBasicBlockStartAnnot(BasicBlock const * bb, std::ostream& os)
	{
		DILITHIUM_UNUSED(bb);
		DILITHIUM_UNUSED(os);
	}

	void AssemblyAnnotationWriter::EmitBasicBlockEndAnnot(BasicBlock const * bb, std::ostream& os)
	{
		DILITHIUM_UNUSED(bb);
		DILITHIUM_UNUSED(os);
	}

	void AssemblyAnnotationWriter::EmitInstructionAnnot(Instruction const * inst, std::ostream& os)
	{
		DILITHIUM_UNUSED(inst);
		DILITHIUM_UNUSED(os);
	}

	void AssemblyAnnotationWriter::PrintInfoComment(Value const & value, std::ostream& os)
	{
		DILITHIUM_UNUSED(value);
		DILITHIUM_UNUSED(os);
	}


	void LLVMModule::Print(std::ostream& os, AssemblyAnnotationWriter* aaw) const
	{
		SlotTracker slot_table(this);
		AssemblyWriter w(os, slot_table, this, aaw);
		w.PrintModule(this);
	}
}
