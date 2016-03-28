/*
 * LLVM Type ID
 * Copyright (c) 2016 Markus Himmel
 * This file is distributed under the terms of the MIT license.
 */

#pragma once

#include <llvm/ADT/SmallVector.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Type.h>

#include <climits>
#include <cstdint>
#include <type_traits>

namespace LLVMTypeID
{

template <typename Type, typename = void>
class TypeID;

#define TYPEID(type, cls, stmt)                                                \
	template <>                                                                \
	class TypeID<type>                                                         \
	{                                                                          \
	public:                                                                    \
		static cls *get(llvm::LLVMContext &C)                                  \
		{                                                                      \
			return stmt;                                                       \
		}                                                                      \
	};

TYPEID(bool, llvm::IntegerType, llvm::Type::getInt1Ty(C))
TYPEID(char, llvm::IntegerType, llvm::Type::getIntNTy(C, CHAR_BIT))

TYPEID(std::int8_t, llvm::IntegerType, llvm::Type::getInt8Ty(C))
TYPEID(std::int16_t, llvm::IntegerType, llvm::Type::getInt16Ty(C))
TYPEID(std::int32_t, llvm::IntegerType, llvm::Type::getInt32Ty(C))
TYPEID(std::int64_t, llvm::IntegerType, llvm::Type::getInt64Ty(C))

TYPEID(std::uint8_t, llvm::IntegerType, llvm::Type::getInt8Ty(C))
TYPEID(std::uint16_t, llvm::IntegerType, llvm::Type::getInt16Ty(C))
TYPEID(std::uint32_t, llvm::IntegerType, llvm::Type::getInt32Ty(C))
TYPEID(std::uint64_t, llvm::IntegerType, llvm::Type::getInt64Ty(C))

TYPEID(void, llvm::Type, llvm::Type::getVoidTy(C))

TYPEID(float, llvm::Type, llvm::Type::getFloatTy(C))
TYPEID(double, llvm::Type, llvm::Type::getDoubleTy(C))

template <typename T>
class TypeID<T *>
{
public:
	static llvm::PointerType *get(llvm::LLVMContext &C)
	{
		return llvm::PointerType::getUnqual(TypeID<T>::get(C));
	}
};

template <typename T>
class TypeID<T &>
{
public:
	static auto *get(llvm::LLVMContext &C)
	{
		return TypeID<T *>::get(C);
	}
};

template <typename T>
class TypeID<T[]>
{
public:
	static llvm::ArrayType *get(llvm::LLVMContext &C)
	{
		return llvm::ArrayType::get(TypeID<T>::get(C), 0);
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
class TypeID<T, std::enable_if_t<!std::is_same<T, std::remove_cv_t<T>>::value>>
{
public:
	static auto *get(llvm::LLVMContext &C)
	{
		return TypeID<typename std::remove_cv_t<T>>::get(C);
	}
};

template <typename... args>
inline llvm::StructType *Struct(llvm::StringRef name, llvm::LLVMContext &C)
{
	return llvm::StructType::create(
		C, llvm::SmallVector<llvm::Type *, sizeof...(args)>(
			   {TypeID<args>::get(C)...}),
		name);
}

template <typename res, typename... args>
class TypeID<res(args...)>
{
public:
	static llvm::FunctionType *get(llvm::LLVMContext &C)
	{
		return llvm::FunctionType::get(
			TypeID<res>::get(C),
			llvm::SmallVector<llvm::Type *, sizeof...(args)>(
				{TypeID<args>::get(C)...}),
			false);
	}

	static void annotateFunction(llvm::Function &F)
	{
		Annotator<res, args...>::annotate(F, 0);
	}

private:
	template <typename... Ts>
	class Annotator;

	template <typename single>
	class Annotator<single &>
	{
	public:
		static void annotate(llvm::Function &F, std::uint32_t index)
		{
			F.addDereferenceableAttr(index, sizeof(single));
		}
	};

	template <typename single>
	class Annotator<single>
	{
	public:
		static void annotate(llvm::Function &F, std::uint32_t index)
		{
		}
	};

	template <typename first, typename... rest>
	class Annotator<first, rest...>
	{
	public:
		static void annotate(llvm::Function &F, std::uint32_t index)
		{
			Annotator<first>::annotate(F, index);
			Annotator<rest...>::annotate(F, index + 1);
		}
	};
};

template <typename T>
inline llvm::Type *get(llvm::LLVMContext &C, T example)
{
	return TypeID<T>::get(C);
}

template <typename T>
inline llvm::FunctionType *getFunction(llvm::LLVMContext &C, T example)
{
	return llvm::cast<llvm::FunctionType>(
		TypeID<std::remove_pointer_t<T>>::get(C));
}

template <typename T>
inline void annotateFunction(llvm::Function &F, T example)
{
	TypeID<std::remove_pointer_t<T>>::annotateFunction(F);
}

} // namespace LLVMTypeID
