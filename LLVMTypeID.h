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

using namespace llvm;

namespace LLVMTypeID
{

template <typename Type>
class TypeID;

#define TYPEID(type, cls, stmt) \
	template <> \
	class TypeID<type> \
	{ \
	public: \
		static cls *get(LLVMContext &C) \
		{ \
			return stmt; \
		} \
	};

TYPEID(std::int8_t, IntegerType, Type::getInt8Ty(C))
TYPEID(std::int16_t, IntegerType, Type::getInt16Ty(C))
TYPEID(std::int32_t, IntegerType, Type::getInt32Ty(C))
TYPEID(std::int64_t, IntegerType, Type::getInt64Ty(C))

TYPEID(std::uint8_t, IntegerType, Type::getInt8Ty(C))
TYPEID(std::uint16_t, IntegerType, Type::getInt16Ty(C))
TYPEID(std::uint32_t, IntegerType, Type::getInt32Ty(C))
TYPEID(std::uint64_t, IntegerType, Type::getInt64Ty(C))

template <typename T>
class TypeID<T *>
{
public:
	static PointerType *get(LLVMContext &C)
	{
		return PointerType::getUnqual(TypeID<T>::get(C));
	}
};

template <typename T, std::uint64_t num>
class TypeID<T[num]>
{
public:
	static ArrayType *get(LLVMContext &C)
	{
		return ArrayType::get(TypeID<T>::get(C), num);
	}
};

// Remove const and volatile (they are not reflected in IR)
template <typename T>
class TypeID
{
public:
	static auto *get(LLVMContext &C)
	{
		return TypeID<typename std::remove_cv<T>::type>::get(C);
	}
};

template <typename ...args>
inline StructType *Struct(StringRef name, LLVMContext &C)
{
	return StructType::create(C, SmallVector<Type *, sizeof...(args)>
		({ TypeID<args>::get(C)... }), name);
}

template <typename res, typename ...args>
class TypeID<res(args...)>
{
public:
	static FunctionType *get(LLVMContext &C)
	{
		return FunctionType::get(TypeID<res>::get(C),
			SmallVector<Type *, sizeof...(args)>({ TypeID<args>::get(C)... }),
				false);
	}
};

} // namespace LLVMTypeID
