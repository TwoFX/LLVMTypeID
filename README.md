# LLVMTypeID
A convenient way of generating an `llvm::Type`

When building an LLVM pass, you sometimes have to declare a type of the value or
function in the IR you are operating on. This is done using `llvm::Type`, but
creating non-trivial types using this interface is cumbersome and error-prone.
Often the C-style type signature is available and has to be hand-parsed into the
corresponding constructor calls.

LLVMTypeID is a tiny header-only library which takes C-style type signatures as
a template and automatically generates the correct type without having to work
out the type yourself.

## An example

Consider a case where you have to emit a library call to the following function:

```c++
extern "C" void foo(unsigned long *bar, char *(*(**baz [][8])())[]);
```

The general LLVM code to generate a module and declare a function looks roughly
like this:

```c++
Module *mod = new Module("fooModule", getGlobalContext());

LLVMContext &C = mod->getContext();
FunctionType *func = /* ??? */;
mod->getOrInsertFunction("foo", func);

mod->print(outs(), nullptr);
```

Which should print the module like this:

```
; ModuleID = 'fooModule'

declare void @foo(i64*, [8 x [0 x i8*]* ()**]*)
```

Now how do we get the correct function type for `foo`?

### Without LLVMTypeID

After looking up at [cdecl](http://cdecl.org/) that the second parameter of
the function means
> declare baz as array of array 8 of pointer to pointer to function returning
> pointer to array of pointer to char,

we can set out to construct the function type by hand.


```c++
FunctionType *func = FunctionType::get(
	Type::getVoidTy(C),
	SmallVector<Type *, 2>(
		{PointerType::getUnqual(Type::getInt64Ty(C)),
		 PointerType::getUnqual(ArrayType::get(
			 PointerType::getUnqual(
				 PointerType::getUnqual(FunctionType::get(
					 PointerType::getUnqual(ArrayType::get(
						 PointerType::getUnqual(Type::getInt8Ty(C)), 0)),
					 ArrayRef<Type *>(), false))),
			 8))}),
	false);
```

Isn't it beautiful?

### With LLVMTypeID

```c++
FunctionType *func = LLVMTypeID::getFunction(C, foo);
```

LLVMTypeID automatically deduces the function type and builds the correct type
in IR. This does not even need foo to be defined.

## Usage

If you have a variable `foo` that is of the type you want to replicate in IR
with `LLVMContext &C`, you can use

```c++
LLVMTypeID::getFunction(C, foo);
```

for functions, and

```c++
LLVMTypeID::get(C, foo);
```

for everything else.

If you have the type signature `sig`, you can use

```c++
LLVMTypeID::TypeID<sig>::get(C);
```

for both functions and other types.

### What constructs are supported?

Currently there is support for basic types such as integers, `void`, floats as
well as functions, pointers, references, arrays, types with type modifiers such
as `const` and `volatile` and all combinations of the mentioned constructs.

### Functions and references

Most of the time, LLVM realizes references as pointers. However, if a function
parameter is a reference, an attribute is emitted to denote that the pointer
is guaranteed to be valid: `dereferenceable`. This attribute is part of the
function, not of the function type, which is why it cannot be emitted as part
of `getFunction`, which will simply emit a reference argument as a pointer. In
order to make the function comply with what clang generates, `dereferenceable`
attributes have to be added where necessary. There is a function which does
that automatically.

If `F` is your function in IR and `foo` is the function declaration, then you
can use:

```c++
LLVMTypeID::annotateFunction(F, foo);
```

If `F` is the IR function and `sig` is the function type signature, you can use:

```c++
LLVMTypeID::TypeID<sig>::annotateFunction(F);
```

### Declare your own types

If you want to provide support for a custom data type, you can simply add your
own specialization of LLVMTypeID::TypeID. The type will then be correctly
handled when it appears in function types or as an array, a pointer or
a reference. Each specialization looks roughly like this (for example for type
`foo`):

```c++
template <>
class LLVMTypeID::TypeID<foo>
{
public
	static Type *get(LLVMContext &C)
	{
		return /* Somehow get the types */;
	}
}
```

In order to be compatible and callable from the other specializations, `get`,
has to have exactly one parameter, a reference to an LLVMContext. If you can,
try to emit subclasses of `llvm::Type`, as this aids in overload resolution.

### Structured data

Support for structured data such as classes and structs is highly limited at
this point. In order for your structured type to be declared, you have to
provide your own specialization of LLVMTypeID::TypeID.

An example:

```c++
struct A
{
	int a, b, c, d, e;
};

template <>
class LLVMTypeID::TypeID<A>
{
public:
	static StructType *get(LLVMContext &C)
	{
		return LLVMTypeID::Struct<int, int, int, int, int>("A", C);
	}
}
```

`LLVMTypeID::Struct<>()` takes the types of the data members of the struct as
well as the name of the struct and a context and generates the according
structured type in IR.

Support for structs has not been tested for compatibility with what clang
generates from the same declaration, so I assume that it is not working.
