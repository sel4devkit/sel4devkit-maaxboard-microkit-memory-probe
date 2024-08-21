/*
 * Code: Crown Copyright NCSC, 2-Clause BSD licence.
 * Documentation: Crown Copyright NCSC, CC-BY licence.
 */

#include "probe.h"

#include <stdint.h>
#include <stdio.h>
#include <microkit.h>

uintptr_t probe_mem_region_vaddr_uintptr;

unsigned int column = 0;
unsigned int buffer_tail_index;
unsigned char buffer[3];

static char make_hex_char(unsigned int v)
{
    char r;
    switch (v) {
        case 0x0: r = '0'; break;
        case 0x1: r = '1'; break;
        case 0x2: r = '2'; break;
        case 0x3: r = '3'; break;
        case 0x4: r = '4'; break;
        case 0x5: r = '5'; break;
        case 0x6: r = '6'; break;
        case 0x7: r = '7'; break;
        case 0x8: r = '8'; break;
        case 0x9: r = '9'; break;
        case 0xA: r = 'A'; break;
        case 0xB: r = 'B'; break;
        case 0xC: r = 'C'; break;
        case 0xD: r = 'D'; break;
        case 0xE: r = 'E'; break;
        case 0xF: r = 'F'; break;
        default: r= 'X'; break;
    }
    return r;
}

static void put_hex_chars(uint64_t v)
{
    char hex_chars[16 + 1];
    unsigned int nibble = 0;

    hex_chars[16] = '\0';
    for (int i = 15; i >= 0; i--) {
        nibble = v & 0xF;
        hex_chars[i] = make_hex_char(nibble);
        v = v >> 4;
    }
    microkit_dbg_puts("0x");
    microkit_dbg_puts(hex_chars);
}

static char make_base64_char(unsigned char v)
{
    char r;
    switch (v) {
        case  0: r = 'A'; break;
        case  1: r = 'B'; break;
        case  2: r = 'C'; break;
        case  3: r = 'D'; break;
        case  4: r = 'E'; break;
        case  5: r = 'F'; break;
        case  6: r = 'G'; break;
        case  7: r = 'H'; break;
        case  8: r = 'I'; break;
        case  9: r = 'J'; break;
        case 10: r = 'K'; break;
        case 11: r = 'L'; break;
        case 12: r = 'M'; break;
        case 13: r = 'N'; break;
        case 14: r = 'O'; break;
        case 15: r = 'P'; break;
        case 16: r = 'Q'; break;
        case 17: r = 'R'; break;
        case 18: r = 'S'; break;
        case 19: r = 'T'; break;
        case 20: r = 'U'; break;
        case 21: r = 'V'; break;
        case 22: r = 'W'; break;
        case 23: r = 'X'; break;
        case 24: r = 'Y'; break;
        case 25: r = 'Z'; break;
        case 26: r = 'a'; break;
        case 27: r = 'b'; break;
        case 28: r = 'c'; break;
        case 29: r = 'd'; break;
        case 30: r = 'e'; break;
        case 31: r = 'f'; break;
        case 32: r = 'g'; break;
        case 33: r = 'h'; break;
        case 34: r = 'i'; break;
        case 35: r = 'j'; break;
        case 36: r = 'k'; break;
        case 37: r = 'l'; break;
        case 38: r = 'm'; break;
        case 39: r = 'n'; break;
        case 40: r = 'o'; break;
        case 41: r = 'p'; break;
        case 42: r = 'q'; break;
        case 43: r = 'r'; break;
        case 44: r = 's'; break;
        case 45: r = 't'; break;
        case 46: r = 'u'; break;
        case 47: r = 'v'; break;
        case 48: r = 'w'; break;
        case 49: r = 'x'; break;
        case 50: r = 'y'; break;
        case 51: r = 'z'; break;
        case 52: r = '0'; break;
        case 53: r = '1'; break;
        case 54: r = '2'; break;
        case 55: r = '3'; break;
        case 56: r = '4'; break;
        case 57: r = '5'; break;
        case 58: r = '6'; break;
        case 59: r = '7'; break;
        case 60: r = '8'; break;
        case 61: r = '9'; break;
        case 62: r = '+'; break;
        case 63: r = '/'; break;
        default: r= 'X'; break;
    }
    return r;
}

static void buffer_clear()
{
    buffer_tail_index = 0;
    buffer[0] = 0x0;
    buffer[1] = 0x0;
    buffer[2] = 0x0;
}

static void putc_wrap(char v)
{
    microkit_dbg_putc(v);
    column = column + 1;
    if (column == 80) {
        microkit_dbg_putc('\n');
        column = 0;
    }
}

static void buffer_show()
{
    // buffer|0       |1       |2       |
    //       |76543210|76543210|76543210|
    //      a|543210  |        |        |
    //      b|      54|3210    |        |
    //      c|        |    5432|10      |
    //      d|        |        |  543210|
    unsigned char a = ((buffer[0] & 0xFC) >> 2);
    unsigned char b = ((buffer[0] & 0x03) << 4) | ((buffer[1] & 0xF0) >> 4);
    unsigned char c = ((buffer[1] & 0x0F) << 2) | ((buffer[2] & 0xC0) >> 6);
    unsigned char d = (buffer[2] & 0x1F);

    switch (buffer_tail_index) {
        case 1:
            putc_wrap(make_base64_char(a));
            putc_wrap(make_base64_char(b));
            putc_wrap('=');
            putc_wrap('=');
        break;
        case 2:
            putc_wrap(make_base64_char(a));
            putc_wrap(make_base64_char(b));
            putc_wrap(make_base64_char(c));
            putc_wrap('=');
        break;
        case 3:
            putc_wrap(make_base64_char(a));
            putc_wrap(make_base64_char(b));
            putc_wrap(make_base64_char(c));
            putc_wrap(make_base64_char(d));
        break;
        default:
        break;
    }
}

static void buffer_push(unsigned char c)
{
    buffer[buffer_tail_index] = c;
    buffer_tail_index++;
    if (buffer_tail_index == 3) {
        buffer_show();
        buffer_clear();
    }
}

void init(void)
{
    unsigned long int remaining_size_byte;
    unsigned char * block_ptr;
    unsigned char block_char;

    // Request.
    microkit_dbg_puts("BLOCK_ADDR_HEX:");
    put_hex_chars(BLOCK_ADDR_PRE_HEX);
    microkit_dbg_putc('\n');
    microkit_dbg_puts("BLOCK_SIZE_BYTE_HEX:");
    put_hex_chars(BLOCK_SIZE_BYTE_PRE_HEX);
    microkit_dbg_putc('\n');

    // Dump begin.
    microkit_dbg_puts("BASE64:BEGIN\n");

    remaining_size_byte = BLOCK_SIZE_BYTE_PRE_HEX;
    block_ptr = (unsigned char *) probe_mem_region_vaddr_uintptr;
    block_ptr = block_ptr + OFFSET_SIZE_BYTE_PRE_HEX;
    buffer_clear();

    while (remaining_size_byte > 0) {
        block_char = *block_ptr;
        buffer_push(block_char);
        block_ptr++;
        remaining_size_byte--;
    }
    buffer_show();

    // Dump end.
    microkit_dbg_puts("\n");
    microkit_dbg_puts("BASE64:END\n");
}

void notified(microkit_channel ch)
{
}
