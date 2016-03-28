/*
 * LLVM Type ID
 * Copyright (c) 2016 Markus Himmel
 * This file is distributed under the terms of the MIT license.
 */

#pragma once

#include <llvm/ADT/SmallVector.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/DerivedTypes.h>

#include <cstdint>

namespace LLVMTypeID
{

template <typename Type>
class TypeID;

#define TYPEID(type, cls, stmt) \
	template <> \
	class TypeID<type> \
	{ \
	public: \
		static cls *get(llvm::LLVMContext &C) \
		{ \
			return stmt; \
		} \
	};

TYPEID(std::int8_t, llvm::IntegerType, llvm::Type::getInt8Ty(C))
TYPEID(std::int16_t, llvm::IntegerType, llvm::Type::getInt16Ty(C))
TYPEID(std::int32_t, llvm::IntegerType, llvm::Type::getInt32Ty(C))
TYPEID(std::int64_t, llvm::IntegerType, llvm::Type::getInt64Ty(C))

TYPEID(std::uint8_t, llvm::IntegerType, llvm::Type::getInt8Ty(C))
TYPEID(std::uint16_t, llvm::IntegerType, llvm::Type::getInt16Ty(C))
TYPEID(std::uint32_t, llvm::IntegerType, llvm::Type::getInt32Ty(C))
TYPEID(std::uint64_t, llvm::IntegerType, llvm::Type::getInt64Ty(C))

template <typename T>
class TypeID<T *>
{
public:
	static llvm::PointerType *get(llvm::LLVMContext &C)
	{
		return llvm::PointerType::getUnqual(TypeID<T>::get(C));
	}
};

template <typename T, std::uint64_t num>
class TypeID<T[num]>
{
public:
	static llvm::ArrayType *get(llvm::LLVMContext &C)
	{
		return llvm::ArrayType::get(TypeID<T>::get(C), num);
	}
};

// Remove const and volatile (they are not reflected in IR)
template <typename T>
class TypeID
{
public:
	static auto *get(llvm::LLVMContext &C)
	{
		return TypeID<typename std::remove_cv_t<T>>::get(C);
	}
};

template <typename ...args>
inline llvm::StructType *Struct(llvm::StringRef name, llvm::LLVMContext &C)
{
	return llvm::StructType::create(C,
		llvm::SmallVector<llvm::Type *, sizeof...(args)>
		({ TypeID<args>::get(C)... }), name);
}

template <typename res, typename ...args>
class TypeID<res(args...)>
{
public:
	static llvm::FunctionType *get(llvm::LLVMContext &C)
	{
		return llvm::FunctionType::get(TypeID<res>::get(C),
			llvm::SmallVector<llvm::Type *, sizeof...(args)>
			({ TypeID<args>::get(C)... }), false);
	}
};

} // namespace LLVMTypeID
