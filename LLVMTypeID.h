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
