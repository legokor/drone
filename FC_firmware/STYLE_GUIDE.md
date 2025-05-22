# LEGO drone Style Guide

## General

- formatting: use [clang-format](./.clang-format)
- (preferably) no global variables
- all private functions / variables marked as `static`

## Naming

- functions, module variables: `module_memberName` - module prefix with camelCase
- structs, unions, enums, enum members: `module_TypeName` - module prefix with PascalCase
- struct members: `fieldName` - regular camelCase
- macros: `module_MACRO_NAME` - module prefix with SCREAMING_SNAKE_CASE
