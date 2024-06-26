/*
 * Code to dump DLOG_MT_CARRIER_TABLE_EXP table from Nortel Millennium
 *
 * www.github.com/hharte/mm_manager
 *
 * Copyright (c) 2020-2023, Howard M. Harte
 *
 * Reference: https://wiki.millennium.management/dlog:dlog_mt_carrier_table
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#ifdef _WIN32
# include <winsock.h>
#else  /* ifdef _WIN32 */
# include <arpa/inet.h>
#endif /* ifdef _WIN32 */

#include "mm_manager.h"

#define TABLE_ID    DLOG_MT_CARRIER_TABLE_EXP

/* Default Carrier Mapping strings
 *
 * See: https://wiki.millennium.management/dlog:dlog_mt_carrier_table
 *
 * PIC = Presubscribed Interexchange Carrier:
 * https://en.wikipedia.org/wiki/Interexchange_carrier#Carrier_identification_code
 */
const char *str_default_carrier[] = {
    "PIC Inter-LATA carrier       ",
    "Coin Inter-LATA carrier      ",
    "Creditcard Inter-LATA carrier",
    "PIC Intra-LATA carrier       ",
    "Coin Intra-LATA carrier      ",
    "Creditcard Intra-LATA carrier",
    "PIC Local carrier            ",
    "Coin Local carrier           ",
    "Creditcard Local carrier     "
};

/* Control Byte strings: */
const char *str_cb[] = {
    "CARCD101XXXX",
    "SPEC_PROMPT",
    "COIN_CASH_CD",
    "ALT_BONG_TMO",
    "DLY_AFT_BONG",
    "INTRA_TO_LEC",
    "OUTDIAL_STR",
    "FEAT_GROUP_B"
};

/* Control Byte 2 strings: */
const char *str_cb2[] = {
    "FGB_PROMPT",
    "RM_PFX_LCL",
    "RM_PFX_INTRA",
    "RM_PFX_INTER",
    "RM_PFX_INT'L",
    "RM_PFX_DA",
    "RM_PFX_1800",
    "CB2_SPARE"
};

#define CB2_VAL (CB2_REM_CARRIER_PREFIX_ZM_LOCAL |      \
                 CB2_REM_CARRIER_PREFIX_INTRALATA |     \
                 CB2_REM_CARRIER_PREFIX_INTERLATA |     \
                 CB2_REM_CARRIER_PREFIX_INTERNATIONAL | \
                 CB2_REM_CARRIER_PREFIX_DA |            \
                 CB2_REM_CARRIER_PREFIX_1800)

carrier_table_entry_t new_carriers[] = {
    { /* Carrier 0 */
        .carrier_ref = 0,
        .carrier_num = LE16(0x0000),
        .valid_cards = LE32(0x00003fff),

        //                "                    ",
        .display_prompt             = "C0 PIC  Inter-LATA  ",
        .control_byte2              = CB2_VAL,
        .control_byte               = (CB_USE_SPEC_DISPLAY_PROMPT | CB_ACCEPTS_COIN_CASH_CARDS),
        .fgb_timer                  = LE16(500),
        .international_accept_flags = 0,
        .call_entry                 = 0x00,
    },
    { /* Carrier 1 */
        .carrier_ref = 1,
        .carrier_num = LE16(0x0000),
        .valid_cards = LE32(0x00003fff),

        //                "                    ",
        .display_prompt             = "C1 Coin Inter-LATA  ",
        .control_byte2              = CB2_VAL,
        .control_byte               = (CB_USE_SPEC_DISPLAY_PROMPT | CB_ACCEPTS_COIN_CASH_CARDS),
        .fgb_timer                  = LE16(500),
        .international_accept_flags = 0,
        .call_entry                 = 0x00,
    },
    { /* Carrier 2 */
        .carrier_ref = 2,
        .carrier_num = LE16(0x0000),
        .valid_cards = LE32(0x00003fff),

        //                "                    ",
        .display_prompt             = "C2 Card Inter-LATA  ",
        .control_byte2              = CB2_VAL,
        .control_byte               = (CB_USE_SPEC_DISPLAY_PROMPT | CB_ACCEPTS_COIN_CASH_CARDS),
        .fgb_timer                  = LE16(500),
        .international_accept_flags = 0,
        .call_entry                 = 0x00,
    },
    { /* Carrier 3 */
        .carrier_ref = 3,
        .carrier_num = LE16(0x0000),
        .valid_cards = LE32(0x00003fff),

        //                "                    ",
        .display_prompt             = "C3 PIC  Intra-LATA  ",
        .control_byte2              = CB2_VAL,
        .control_byte               = (CB_USE_SPEC_DISPLAY_PROMPT | CB_ACCEPTS_COIN_CASH_CARDS),
        .fgb_timer                  = LE16(500),
        .international_accept_flags = 0,
        .call_entry                 = 0x00,
    },
    { /* Carrier 4 */
        .carrier_ref = 4,
        .carrier_num = LE16(0x0000),
        .valid_cards = LE32(0x00003fff),

        //                "                    ",
        .display_prompt             = "C4 Coin Intra-LATA  ",
        .control_byte2              = CB2_VAL,
        .control_byte               = (CB_USE_SPEC_DISPLAY_PROMPT | CB_ACCEPTS_COIN_CASH_CARDS),
        .fgb_timer                  = LE16(500),
        .international_accept_flags = 0,
        .call_entry                 = 0x00,
    },
    { /* Carrier 5 */
        .carrier_ref = 5,
        .carrier_num = LE16(0x0000),
        .valid_cards = LE32(0x00003fff),

        //                "                    ",
        .display_prompt             = "C5 Card Intra-LATA  ",
        .control_byte2              = CB2_VAL,
        .control_byte               = (CB_USE_SPEC_DISPLAY_PROMPT | CB_ACCEPTS_COIN_CASH_CARDS),
        .fgb_timer                  = LE16(500),
        .international_accept_flags = 0,
        .call_entry                 = 0x00,
    },
    { /* Carrier 6 */
        .carrier_ref = 6,
        .carrier_num = LE16(0x0000),
        .valid_cards = LE32(0x00003fff),

        //                "                    ",
        .display_prompt             = "C6 PIC  Local       ",
        .control_byte2              = CB2_VAL,
        .control_byte               = (CB_USE_SPEC_DISPLAY_PROMPT | CB_ACCEPTS_COIN_CASH_CARDS),
        .fgb_timer                  = LE16(500),
        .international_accept_flags = 0,
        .call_entry                 = 0x00,
    },
    { /* Carrier 7 */
        .carrier_ref = 7,
        .carrier_num = LE16(0x0000),
        .valid_cards = LE32(0x00003fff),

        //                "                    ",
        .display_prompt             = "C7 Coin Local       ",
        .control_byte2              = CB2_VAL,
        .control_byte               = (CB_USE_SPEC_DISPLAY_PROMPT | CB_ACCEPTS_COIN_CASH_CARDS),
        .fgb_timer                  = LE16(500),
        .international_accept_flags = 0,
        .call_entry                 = 0x00,
    },
    { /* Carrier 8 */
        .carrier_ref = 8,
        .carrier_num = LE16(0x0000),
        .valid_cards = LE32(0x00003fff),

        //                "                    ",
        .display_prompt             = "C8 Card Local       ",
        .control_byte2              = CB2_VAL,
        .control_byte               = (CB_USE_SPEC_DISPLAY_PROMPT | CB_ACCEPTS_COIN_CASH_CARDS),
        .fgb_timer                  = LE16(500),
        .international_accept_flags = 0,
        .call_entry                 = 0x00,
    },
    { /* Carrier 9 */
        .carrier_ref = 9,
        .carrier_num = LE16(0x0000),
        .valid_cards = LE32(0x00003fff),

        //                "                    ",
        .display_prompt             = "CARRIER 9           ",
        .control_byte2              = CB2_VAL,
        .control_byte               = (CB_USE_SPEC_DISPLAY_PROMPT | CB_ACCEPTS_COIN_CASH_CARDS),
        .fgb_timer                  = LE16(500),
        .international_accept_flags = 0,
        .call_entry                 = 0x00,
    }
};

int main(int argc, char *argv[]) {
    FILE *instream;
    FILE *ostream = NULL;
    int   carrier_index;
    uint16_t carrier_num;
    char     display_prompt_string[21];
    uint8_t  i;
    int ret = 0;

    dlog_mt_carrier_table_t *ptable;
    uint8_t* load_buffer;

    if (argc <= 1) {
        printf("Usage:\n" \
               "\tmm_carrier mm_table_%02x.bin [outputfile.bin]\n", TABLE_ID);
        return -1;
    }

    printf("Nortel Millennium %s Table %d (0x%02x) Dump\n\n", table_to_string(TABLE_ID), TABLE_ID, TABLE_ID);

    ptable = (dlog_mt_carrier_table_t *)calloc(1, sizeof(dlog_mt_carrier_table_t));

    if (ptable == NULL) {
        printf("Failed to allocate %zu bytes.\n", sizeof(dlog_mt_carrier_table_t));
        return -ENOMEM;
    }

    if ((instream = fopen(argv[1], "rb")) == NULL) {
        printf("Error opening %s\n", argv[1]);
        free(ptable);
        return -ENOENT;
    }

    if (mm_validate_table_fsize(TABLE_ID, instream, sizeof(dlog_mt_carrier_table_t) - 1) != 0) {
        free(ptable);
        fclose(instream);
        return -EIO;
    }

    load_buffer = ((uint8_t*)ptable) + 1;
    if (fread(load_buffer, sizeof(dlog_mt_carrier_table_t) - 1, 1, instream) != 1) {
        printf("Error reading %s table.\n", table_to_string(TABLE_ID));
        free(ptable);
        fclose(instream);
        return -EIO;
    }

    fclose(instream);

    printf("Default Carriers:\n");

    for (i = 0; i < DEFAULT_CARRIERS_MAX; i++) {
        printf("\t%d %s = 0x%02x (%3d)\n",
               i,
               str_default_carrier[i],
               ptable->defaults[i],
               ptable->defaults[i]);
    }
    printf("\n+---------------------------------------------------------------------------------------------------------------------+\n" \
           "|  # | Ref  | Number | Valid Cards | Display Prompt       |  CB2 |  CB  | FGB Tmr | Int'l | Call Entry | CB2/CB Flags |\n"   \
           "+----+------+--------+-------------+----------------------+------+------+---------+-------+------------+--------------+");

    for (carrier_index = 0; carrier_index < CARRIER_TABLE_MAX_CARRIERS; carrier_index++) {
        if (ptable->carrier[carrier_index].display_prompt[0] >= 0x20) {
            memcpy(display_prompt_string, ptable->carrier[carrier_index].display_prompt,
                   sizeof(ptable->carrier[carrier_index].display_prompt));
            display_prompt_string[20] = '\0';
        } else {
            if ((ptable->carrier[carrier_index].carrier_ref == 0) &&
                (ptable->carrier[carrier_index].call_entry == 0)) {
                continue;
            }
            snprintf(display_prompt_string, sizeof(display_prompt_string), "                    ");
        }

        carrier_num = LE16(ntohs(ptable->carrier[carrier_index].carrier_num));

        printf("\n| %2d | 0x%02x | 0x%04x |  0x%08x | %s | 0x%02x | 0x%02x |  %5d  |  0x%02x | 0x%02x   %3d | ",
               carrier_index,
               ptable->carrier[carrier_index].carrier_ref,
               LE16(carrier_num),
               LE32(ptable->carrier[carrier_index].valid_cards),
               display_prompt_string,
               ptable->carrier[carrier_index].control_byte2,
               ptable->carrier[carrier_index].control_byte,
               LE16(ptable->carrier[carrier_index].fgb_timer),
               ptable->carrier[carrier_index].international_accept_flags,
               ptable->carrier[carrier_index].call_entry,
               ptable->carrier[carrier_index].call_entry);

        print_bits(ptable->carrier[carrier_index].control_byte2, (char **)str_cb2);
        print_bits(ptable->carrier[carrier_index].control_byte,  (char **)str_cb);
    }

    printf("\n+------------------------------------------------------------------------------------------------------+\n");

    printf("Spare: ");
    for (i = 0; i < sizeof(ptable->spare); i++) {
        printf("0x%02x, ", ptable->spare[i]);
    }
    printf("\n");

    if (argc > 2) {
        if ((ostream = fopen(argv[2], "wb")) == NULL) {
            printf("Error opening output file %s for write.\n", argv[2]);
            return -ENOENT;
        }
    }

    for (i = 0; i < DEFAULT_CARRIERS_MAX; i++) {
        ptable->defaults[i] = 0;
    }

    memcpy(ptable->carrier, new_carriers, sizeof(new_carriers));

    /* If output file was specified, write it. */
    if (ostream != NULL) {
        printf("\nWriting new table to %s\n", argv[2]);

        if (fwrite(load_buffer, sizeof(dlog_mt_carrier_table_t) - 1, 1, ostream) != 1) {
            printf("Error writing output file %s\n", argv[2]);
            ret = -EIO;
        }
        fclose(ostream);
    }

    free(ptable);

    return ret;
}
