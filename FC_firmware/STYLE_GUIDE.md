# LEGO drone Style Guide

## General

- formatting: use [clang-format](./.clang-format)
- (preferably) no global variables
- all private functions / variables marked as `static`

## Naming

- functions, module variables: `module_functionName` - module prefix with camelCase
- structs, enums: `module_StructName` - module prefix with PascalCase
- struct members: `fieldName` - regular camelCase
