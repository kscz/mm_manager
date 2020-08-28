/*
 * This is a "Manager" for the Nortel Millennium payhone.
 *
 * It can provision a Nortel Millennium payphone with Rev 1.0 or 1.3
 * Control PCP.  CDRs, Alarms, and Maintenance Reports can also be
 * retieved.
 *
 * www.github.com/hharte/mm_manager
 *
 * (c) 2020, Howard M. Harte
 */

#include <stdio.h>   /* Standard input/output definitions */
#include <stdlib.h>
#include <stdint.h>
#include <string.h>  /* String function definitions */

#include "mm_manager.h"

#define POLY 0xa001     /* Polynomial to use for CRC-16 calculation */

/* Calculate CRC-16 checksum using 0xA001 polynomial. */
unsigned crc16(unsigned crc, uint8_t *buf, size_t len)
{
    while (len--) {
        crc ^= *buf++;
        crc = crc & 1 ? (crc >> 1) ^ POLY : crc >> 1;
        crc = crc & 1 ? (crc >> 1) ^ POLY : crc >> 1;
        crc = crc & 1 ? (crc >> 1) ^ POLY : crc >> 1;
        crc = crc & 1 ? (crc >> 1) ^ POLY : crc >> 1;
        crc = crc & 1 ? (crc >> 1) ^ POLY : crc >> 1;
        crc = crc & 1 ? (crc >> 1) ^ POLY : crc >> 1;
        crc = crc & 1 ? (crc >> 1) ^ POLY : crc >> 1;
        crc = crc & 1 ? (crc >> 1) ^ POLY : crc >> 1;
    }
    return crc;
}

void dump_hex(uint8_t *data, int len)
{
    uint8_t ascii[32];
    uint8_t *pascii = ascii;
    int i;

    printf("\n");
    if (len > 0) {
        printf("\tData: ");

        for (i = 0; i < len; i++) {
            if (i % 16 == 0) {
                if (i > 0) {
                    *pascii++ = '\0';
                    printf("%s", ascii);
                }
                printf("\n\t%03d: ", i);
                pascii = ascii;
            }
            printf("%02x, ", data[i]);
            if ((data[i] >= 0x20) && (data[i] < 0x7F)) {
                *pascii++ = data[i];
            } else {
                *pascii++ = '.';
            }

        }
        *pascii++ = '\0';
        if (strlen((char *)ascii) > 0) {
            for (i = 0; i < 16 - strlen((char *)ascii); i++) {
                printf("    ");
            }
            printf("%s", ascii);
        }
    }
    printf("\n");
}

/* Convert encoded phone number into string. */
extern char *phone_num_to_string(char *string_buf, int string_buf_len, uint8_t* num_buf, int num_buf_len)
{
    char *pstr = string_buf;
    int i, j, pn_digit;

    j = 0;

    for (i = 0; i < num_buf_len; i++) {
        pn_digit = num_buf[i] >> 4;
        if (pn_digit == 0xe) break;
        *pstr++ = (pn_digit) + '0';
        j++;
        if (j >= (string_buf_len - 1)) break;

        pn_digit = num_buf[i] & 0x0f;
         if (pn_digit == 0xe) break;
        *pstr++ = (pn_digit) + '0';
        j++;
        if (j >= (string_buf_len - 1)) break;
    }

    *pstr = '\0';
    return string_buf;
}

/* Convert NULL terminated string to packed BCD (0 digits replaced with 0xa) */
extern uint8_t string_to_bcd_a(char *number_string, uint8_t *buffer, uint8_t buff_len)
{
    uint8_t i;

    memset(buffer, 0, buff_len);

    for (i = 0; (i < (strlen(number_string)) && (i < (buff_len * 2))); i++) {
        if (i % 2 == 0) {
            if (number_string[i] == '0') {
                buffer[(i >> 1)] = 0xa0;
            }
            else {
                buffer[(i >> 1)] = (number_string[i] - '0') << 4;
            }
        }
        else {
            if (number_string[i] == '0') {
                buffer[(i >> 1)] |= 0x0a;
            }
            else {
                buffer[(i >> 1)] |= (number_string[i] - '0');
            }
        }
    }

    return (i);
}

/* Lookup table to translate number string into text.  Not sure what B, C, D, E, F are used for. */
const char pn_lut[16] = { '\0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', 'B', 'C', 'D', 'E', 'F' };

/* Convert encoded phone number terminated with zero into a string. */
char *callscrn_num_to_string(char *string_buf, int string_buf_len, uint8_t* num_buf, int num_buf_len)
{
    char *pstr = string_buf;
    int i, j, pn_digit;

    j = 0;

    for (i = 0; i < num_buf_len; i++) {
        pn_digit = num_buf[i] >> 4;

        *pstr++ = pn_lut[pn_digit];
        j++;

        if (j >= (string_buf_len - 1)) break;

        pn_digit = num_buf[i] & 0x0f;
        *pstr++ = pn_lut[pn_digit];
        j++;
        if (j >= (string_buf_len - 1)) break;
    }

    *pstr = '\0';
    return string_buf;
}

/* Call Type (lower 4-bits) of CALLTYP */
char *call_type_str[16] = {
    "Incoming",
    "Unanswered",
    "Abandoned",
    "Local",
    "Intra-LATA",
    "Inter-LATA",
    "Internatonal",
    "Operator",
    "Zero+",
    "1-800",
    "Directory Assistance",
    "Denied",
    "Unassigned",
    "Unassigned2",
    "e-Purse",
    "Unknown"
};

/* Payment Type (upper 4-bits) of CALLTYP */
char *pmt_type_str[16] = {
    "Unused0",
    "Unused1",
    "No Charge",
    "Coin",
    "Credit Card",
    "Calling Card",
    "Cash Card",
    "Inmate",
    "Mondex",
    "Visa Stored Value",
    "Smart City",
    "Proton",
    "UndefinedC",
    "UndefinedD",
    "UndefinedE",
    "UndefinedF",
};

char *call_type_to_string(uint8_t call_type, char *string_buf, int string_buf_len)
{
    int len_call_type, len_pmt_type;

    len_call_type = strlen(call_type_str[call_type & 0x0f]);
    len_pmt_type = strlen(pmt_type_str[call_type >> 4]);

    if ((len_call_type + len_pmt_type + 1) > string_buf_len) {
        return NULL;
    }

    sprintf(string_buf, "%s %s",
        call_type_str[call_type & 0x0f],
        pmt_type_str[call_type >> 4]);

    return string_buf;

}

void print_bits(uint8_t bits, char *str_array[])
{
    int i = 0;
    while (bits) {
        if (bits & 1) {
            printf("%s | ", str_array[i]);
        }
        bits >>= 1;
        i++;
    }
}

int mm_read_instsv_params(dlog_mt_install_params_t *instsv, char *filename)
{
    FILE *instream = NULL;
    instream = fopen(filename, "rb");

    if (fread(instsv, sizeof(dlog_mt_install_params_t), 1, instream) <= 0) {
        printf("Error reading INSTSV table.\n");
            return (-1);
    }

    fclose(instream);

    return (0);
}
