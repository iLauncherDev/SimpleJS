#pragma once
#include "default.h"
#include "variable.h"
#include "linemap.h"

#define SIMPLEJS_BYTECODE_VERSION 1

#define SIMPLEJS_BYTECODE_BASE_INSTRUCTION_SIZE 4
#define SIMPLEJS_BYTECODE_EXTENDED_INSTRUCTION_SIZE 6

typedef enum
{
    SIMPLEJS_BYTECODE_OPCODE_MOV_VAR,

    SIMPLEJS_BYTECODE_OPCODE_ADD_STACK,
    SIMPLEJS_BYTECODE_OPCODE_ADD_STACK_VAR_SIZE,

    SIMPLEJS_BYTECODE_OPCODE_SAVE_VAR,
    SIMPLEJS_BYTECODE_OPCODE_RESTORE_VAR,

    SIMPLEJS_BYTECODE_OPCODE_SAVE_CTX,
    SIMPLEJS_BYTECODE_OPCODE_RESTORE_CTX,

    SIMPLEJS_BYTECODE_OPCODE_INIT_LOC_OFFSET,
    SIMPLEJS_BYTECODE_OPCODE_INIT_ARG_OFFSET,
    SIMPLEJS_BYTECODE_OPCODE_SAVE_ARG_OFFSET,

    SIMPLEJS_BYTECODE_OPCODE_ALLOC_ARGS,
    SIMPLEJS_BYTECODE_OPCODE_FREE_ARGS,

    SIMPLEJS_BYTECODE_OPCODE_GET_RETURN_VAR,
    SIMPLEJS_BYTECODE_OPCODE_SET_RETURN_VAR,

    SIMPLEJS_BYTECODE_OPCODE_INIT_VAR,
    SIMPLEJS_BYTECODE_OPCODE_INIT_LOC_VAR,
    SIMPLEJS_BYTECODE_OPCODE_FREE_LOC_VAR,

    SIMPLEJS_BYTECODE_OPCODE_GET_VAR_PROP,
    SIMPLEJS_BYTECODE_OPCODE_SET_VAR_PROP,
    SIMPLEJS_BYTECODE_OPCODE_DELETE_VAR_PROP,

    SIMPLEJS_BYTECODE_OPCODE_GET_GLOBAL_VAR,

    SIMPLEJS_BYTECODE_OPCODE_GET_LOC_VAR,
    SIMPLEJS_BYTECODE_OPCODE_SET_LOC_VAR,

    SIMPLEJS_BYTECODE_OPCODE_GET_CALL_ARG_VAR,
    SIMPLEJS_BYTECODE_OPCODE_SET_CALL_ARG_VAR,

    SIMPLEJS_BYTECODE_OPCODE_GET_FUNC_ARG_VAR,
    SIMPLEJS_BYTECODE_OPCODE_SET_FUNC_ARG_VAR,

    SIMPLEJS_BYTECODE_OPCODE_SET_VAR_NUMBER,
    SIMPLEJS_BYTECODE_OPCODE_SET_VAR_NATIVE_FUNCTION,
    SIMPLEJS_BYTECODE_OPCODE_SET_VAR_FAST_STRING,

    SIMPLEJS_BYTECODE_OPCODE_CALL,
    SIMPLEJS_BYTECODE_OPCODE_RETURN,

    SIMPLEJS_BYTECODE_OPCODE_JMP,
    SIMPLEJS_BYTECODE_OPCODE_JMP_IF_ZERO,
    SIMPLEJS_BYTECODE_OPCODE_JMP_IF_NOT_ZERO,

    SIMPLEJS_BYTECODE_OPCODE_CONVERT_BOOLEAN_VAR,

    SIMPLEJS_BYTECODE_OPCODE_INC_VAR,
    SIMPLEJS_BYTECODE_OPCODE_DEC_VAR,

    SIMPLEJS_BYTECODE_OPCODE_LOGICAL_NOT_VAR,
    SIMPLEJS_BYTECODE_OPCODE_BITWISE_NOT_VAR,
    SIMPLEJS_BYTECODE_OPCODE_NEG_VAR,

    SIMPLEJS_BYTECODE_OPCODE_EQUAL_VAR,
    SIMPLEJS_BYTECODE_OPCODE_NOT_EQUAL_VAR,
    SIMPLEJS_BYTECODE_OPCODE_GREATER_VAR,
    SIMPLEJS_BYTECODE_OPCODE_BELOW_VAR,
    SIMPLEJS_BYTECODE_OPCODE_GREATER_EQUAL_VAR,
    SIMPLEJS_BYTECODE_OPCODE_BELOW_EQUAL_VAR,

    SIMPLEJS_BYTECODE_OPCODE_OR_VAR,
    SIMPLEJS_BYTECODE_OPCODE_AND_VAR,

    SIMPLEJS_BYTECODE_OPCODE_SHL_VAR,
    SIMPLEJS_BYTECODE_OPCODE_SHR_VAR,
    SIMPLEJS_BYTECODE_OPCODE_SAL_VAR,
    SIMPLEJS_BYTECODE_OPCODE_SAR_VAR,

    SIMPLEJS_BYTECODE_OPCODE_ADD_VAR,
    SIMPLEJS_BYTECODE_OPCODE_SUB_VAR,
    SIMPLEJS_BYTECODE_OPCODE_MUL_VAR,
    SIMPLEJS_BYTECODE_OPCODE_DIV_VAR,
    SIMPLEJS_BYTECODE_OPCODE_MOD_VAR,

    SIMPLEJS_BYTECODE_OPCODE_END = 0x7F,
} simplejs_bytecode_opcode_t;

#pragma pack(push, 1)

#define SIMPLEJS_BYTECODE_HEADER_DEBUG_INFO_FLAG (1 << 0)

typedef struct simplejs_bytecode_header
{
    uint16_t size;
    uint16_t version;
    uint32_t flags;

    uint16_t debug_info_entry_size;

    struct
    {
        uint32_t start, end;
    } code_offset;

    struct
    {
        uint32_t start, end;
    } debug_info_offset;
} simplejs_bytecode_header_t;

#define SIMPLEJS_BYTECODE_DEBUG_INFO_BINARY_OP_FLAG (1 << 0)
#define SIMPLEJS_BYTECODE_DEBUG_INFO_UNARY_OP_FLAG (1 << 1)
#define SIMPLEJS_BYTECODE_DEBUG_INFO_LEFT_FLAG (1 << 2)
#define SIMPLEJS_BYTECODE_DEBUG_INFO_RIGHT_FLAG (1 << 3)
#define SIMPLEJS_BYTECODE_DEBUG_INFO_DIAGNOSTIC_FLAG (1 << 4)

#define SIMPLEJS_BYTECODE_DEBUG_INFO_HINT_RIGHT_FLAG (1 << 5)

typedef struct simplejs_bytecode_debug_info
{
    uint32_t flags;
    uint32_t children_debug_count;

    simplejs_linemap_offset_t code_offset;
    simplejs_linemap_offset_t source_offset;
} simplejs_bytecode_debug_info_t;

#pragma pack(pop)

#pragma pack(push, 1)

typedef struct simplejs_bytecode_instruction
{
    uint8_t opcode : 7;
    uint8_t extended_opcode : 1;

    uint8_t reg_1 : 4;
    uint8_t reg_2 : 4;

    union
    {
        uint32_t imm;
        int32_t imm_signed;
    };
} simplejs_bytecode_instruction_t;

#pragma pack(pop)

typedef enum
{
    SIMPLEJS_BYTECODE_VARIABLE_THIS,
    SIMPLEJS_BYTECODE_VARIABLE_PARENT,

    SIMPLEJS_BYTECODE_VARIABLE_ASSIGN_A,
    SIMPLEJS_BYTECODE_VARIABLE_ASSIGN_B,
    SIMPLEJS_BYTECODE_VARIABLE_ASSIGN_C,
    SIMPLEJS_BYTECODE_VARIABLE_PROPERTY,

    SIMPLEJS_BYTECODE_VARIABLE_OP_A,
    SIMPLEJS_BYTECODE_VARIABLE_OP_B,

    SIMPLEJS_BYTECODE_VARIABLE_FUNCTION,
    SIMPLEJS_BYTECODE_VARIABLE_ARGUMENT,
} simplejs_bytecode_variable_t;

pvoid simplejs_bytecode_find_debug_info_by_stop_pointer(void *executable, size_t executable_size, void *stop_pointer);
pvoid simplejs_bytecode_find_debug_info_by_source_offset(void *executable, size_t executable_size, simplejs_linemap_offset_t source_offset, uint32_t required_flags);
pvoid simplejs_bytecode_find_children_debug_info(
    void *executable, size_t executable_size,
    pvoid debug_info, uint32_t required_debug_flags, uint32_t children_debug_flags);

void simplejs_bytecode_encode(uint8_t *buffer, simplejs_bytecode_instruction_t *instruction, uint8_t *instruction_size);

void simplejs_bytecode_get_opcode_size(uint8_t *buffer, uint8_t *instruction_size);
void simplejs_bytecode_decode(uint8_t *buffer, simplejs_bytecode_instruction_t *instruction, uint8_t *instruction_size);

void simplejs_bytecode_header_decode(uint8_t *buffer, simplejs_bytecode_header_t *header);
void simplejs_bytecode_header_encode(uint8_t *buffer, simplejs_bytecode_header_t *header);
void simplejs_bytecode_debug_info_decode(uint8_t *buffer, simplejs_bytecode_debug_info_t *debug_info);
void simplejs_bytecode_debug_info_encode(uint8_t *buffer, simplejs_bytecode_debug_info_t *debug_info);
