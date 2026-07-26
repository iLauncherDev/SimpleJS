#include <bytecode.h>
#include <lib/byte_utils.h>

void simplejs_bytecode_encode(uint8_t *buffer, simplejs_bytecode_instruction_t *instruction, uint8_t *instruction_size)
{
    (*instruction_size) = SIMPLEJS_BYTECODE_BASE_INSTRUCTION_SIZE;

    *buffer++ = instruction->opcode | (instruction->extended_opcode << 7);
    *buffer++ = (instruction->reg_1 & 0x0F) | ((instruction->reg_2 & 0x0F) << 4);

    if (instruction->extended_opcode)
    {
        (*instruction_size) = SIMPLEJS_BYTECODE_EXTENDED_INSTRUCTION_SIZE;

        *buffer++ = (instruction->imm >> 0) & 0xFF;
        *buffer++ = (instruction->imm >> 8) & 0xFF;

        *buffer++ = (instruction->imm >> 16) & 0xFF;
        *buffer++ = (instruction->imm >> 24) & 0xFF;
    }
    else
    {
        uint16_t imm16 = (int16_t)instruction->imm_signed;

        *buffer++ = (imm16 >> 0) & 0xFF;
        *buffer++ = (imm16 >> 8) & 0xFF;
    }
}

void simplejs_bytecode_decode_imm16(uint8_t *buffer, simplejs_bytecode_instruction_t *instruction, uint8_t *instruction_size)
{
    uint32_t b3 = *buffer++;
    uint32_t b4 = *buffer++;

    instruction->imm = (int16_t)((b3 << 0) | (b4 << 8));
}

void simplejs_bytecode_decode_imm32(uint8_t *buffer, simplejs_bytecode_instruction_t *instruction, uint8_t *instruction_size)
{
    uint32_t b3 = *buffer++;
    uint32_t b4 = *buffer++;
    uint32_t b5 = *buffer++;
    uint32_t b6 = *buffer++;

    (*instruction_size) = SIMPLEJS_BYTECODE_EXTENDED_INSTRUCTION_SIZE;

    instruction->imm = (b3 << 0) | (b4 << 8) | (b5 << 16) | (b6 << 24);
}

void (*simplejs_bytecode_decode_immX[])(uint8_t *buffer, simplejs_bytecode_instruction_t *instruction, uint8_t *instruction_size) = {
    simplejs_bytecode_decode_imm16,
    simplejs_bytecode_decode_imm32,
};

void simplejs_bytecode_get_opcode_size(uint8_t *buffer, uint8_t *instruction_size)
{
    uint8_t b1 = *buffer++;
    uint8_t b2 = *buffer++;

    uint8_t cond = -(b1 >> 7);

    (*instruction_size) =
        (SIMPLEJS_BYTECODE_EXTENDED_INSTRUCTION_SIZE & cond) | (SIMPLEJS_BYTECODE_BASE_INSTRUCTION_SIZE & ~cond);
}

void simplejs_bytecode_decode(uint8_t *buffer, simplejs_bytecode_instruction_t *instruction, uint8_t *instruction_size)
{
    uint8_t b1 = *buffer++;
    uint8_t b2 = *buffer++;

    (*instruction_size) = SIMPLEJS_BYTECODE_BASE_INSTRUCTION_SIZE;

    instruction->opcode = b1 & 0x7F;
    instruction->extended_opcode = b1 >> 7;

    instruction->reg_1 = (b2 >> 0) & 0x0F;
    instruction->reg_2 = (b2 >> 4) & 0x0F;

    simplejs_bytecode_decode_immX[instruction->extended_opcode](buffer, instruction, instruction_size);
}

void simplejs_bytecode_header_decode(uint8_t *buffer, simplejs_bytecode_header_t *header)
{
    simplejs_buffer_to_struct_field(header, &header->size, uint16_t, buffer);
    simplejs_buffer_to_struct_field(header, &header->version, uint16_t, buffer);
    simplejs_buffer_to_struct_field(header, &header->flags, uint32_t, buffer);

    simplejs_buffer_to_struct_field(header, &header->debug_info_entry_size, uint16_t, buffer);

    simplejs_buffer_to_struct_field(header, &header->code_offset.start, uint32_t, buffer);
    simplejs_buffer_to_struct_field(header, &header->code_offset.end, uint32_t, buffer);

    simplejs_buffer_to_struct_field(header, &header->debug_info_offset.start, uint32_t, buffer);
    simplejs_buffer_to_struct_field(header, &header->debug_info_offset.end, uint32_t, buffer);
}

void simplejs_bytecode_header_encode(uint8_t *buffer, simplejs_bytecode_header_t *header)
{
    simplejs_struct_field_to_buffer(header, &header->size, uint16_t, buffer);
    simplejs_struct_field_to_buffer(header, &header->version, uint16_t, buffer);
    simplejs_struct_field_to_buffer(header, &header->flags, uint32_t, buffer);

    simplejs_struct_field_to_buffer(header, &header->debug_info_entry_size, uint16_t, buffer);

    simplejs_struct_field_to_buffer(header, &header->code_offset.start, uint32_t, buffer);
    simplejs_struct_field_to_buffer(header, &header->code_offset.end, uint32_t, buffer);

    simplejs_struct_field_to_buffer(header, &header->debug_info_offset.start, uint32_t, buffer);
    simplejs_struct_field_to_buffer(header, &header->debug_info_offset.end, uint32_t, buffer);
}

void simplejs_bytecode_debug_info_decode(uint8_t *buffer, simplejs_bytecode_debug_info_t *debug_info)
{
    simplejs_buffer_to_struct_field(debug_info, &debug_info->flags, uint32_t, buffer);
    simplejs_buffer_to_struct_field(debug_info, &debug_info->children_debug_count, uint32_t, buffer);

    simplejs_buffer_to_struct_field(debug_info, &debug_info->code_offset.start, uint64_t, buffer);
    simplejs_buffer_to_struct_field(debug_info, &debug_info->code_offset.end, uint64_t, buffer);

    simplejs_buffer_to_struct_field(debug_info, &debug_info->source_offset.start, uint64_t, buffer);
    simplejs_buffer_to_struct_field(debug_info, &debug_info->source_offset.end, uint64_t, buffer);
}

void simplejs_bytecode_debug_info_encode(uint8_t *buffer, simplejs_bytecode_debug_info_t *debug_info)
{
    simplejs_struct_field_to_buffer(debug_info, &debug_info->flags, uint32_t, buffer);
    simplejs_struct_field_to_buffer(debug_info, &debug_info->children_debug_count, uint32_t, buffer);

    simplejs_struct_field_to_buffer(debug_info, &debug_info->code_offset.start, uint64_t, buffer);
    simplejs_struct_field_to_buffer(debug_info, &debug_info->code_offset.end, uint64_t, buffer);

    simplejs_struct_field_to_buffer(debug_info, &debug_info->source_offset.start, uint64_t, buffer);
    simplejs_struct_field_to_buffer(debug_info, &debug_info->source_offset.end, uint64_t, buffer);
}
