/*
 * Code to dump Credit Card table from Nortel Millennium Payphone
 * Table 134 (0x86)
 *
 * www.github.com/hharte/mm_manager
 *
 * Copyright (c) 2020-2022, Howard M. Harte
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include "./mm_manager.h"
#include "./mm_card.h"

int main(int argc, char *argv[]) {
    FILE *instream;
    FILE *ostream = NULL;
    int   index;
    int   ret = 0;

    dlog_mt_card_table_t* pcard_table_mtr2;
    dlog_mt_card_table_mtr1_t *pcard_table_mtr1;

    if (argc <= 2) {
        printf("Usage:\n" \
               "\tmm_convert_card_mtr2_to_mtr1 mm_table_86.bin mm_table_16.bin\n");
        return -1;
    }

    printf("Nortel Millennium Credit Card Table MTR 2 to MTR1 Converter\n\n");

    pcard_table_mtr2 = (dlog_mt_card_table_t*)calloc(1, sizeof(dlog_mt_card_table_t));

    if (pcard_table_mtr2 == NULL) {
        printf("Failed to allocate %zu bytes.\n", sizeof(dlog_mt_card_table_t));
        return -ENOMEM;
    }

    pcard_table_mtr1 = (dlog_mt_card_table_mtr1_t*)calloc(1, sizeof(dlog_mt_card_table_mtr1_t));

    if (pcard_table_mtr1 == NULL) {
        printf("Failed to allocate %zu bytes.\n", sizeof(dlog_mt_card_table_mtr1_t));
        free(pcard_table_mtr2);
        return -ENOMEM;
    }

    if ((instream = fopen(argv[1], "rb")) == NULL) {
        printf("Error opening %s\n", argv[1]);
        free(pcard_table_mtr2);
        free(pcard_table_mtr1);
        return -ENOENT;
    }

    if (fread(pcard_table_mtr2, sizeof(dlog_mt_card_table_t), 1, instream) != 1) {
        printf("Error reading MTR 2 CCARD table.\n");
        free(pcard_table_mtr2);
        free(pcard_table_mtr1);
        fclose(instream);
        return -EIO;
    }

    for (index = 0; index < CCARD_MAX_MTR1; index++) {
        card_entry_t *pcard_mtr2 = &pcard_table_mtr2->c[index];
        card_entry_mtr1_t *pcard_mtr1 = &pcard_table_mtr1->c[index];

        /* Copy the card entry */
        memcpy_s(pcard_mtr1, sizeof(pcard_mtr1), pcard_mtr2, sizeof(pcard_mtr1));
    }

    if ((ostream = fopen(argv[2], "wb")) == NULL) {
        printf("Error opening output file %s for write.\n", argv[2]);
        return -ENOENT;
    }

    /* If output file was specified, write it. */
    if (ostream != NULL) {
        printf("\nWriting new table to %s\n", argv[2]);

        if (fwrite(pcard_table_mtr1, sizeof(dlog_mt_card_table_mtr1_t), 1, ostream) != 1) {
            printf("Error writing output file %s\n", argv[2]);
            ret = -EIO;
        }
        fclose(ostream);
    }

    if (pcard_table_mtr2 != NULL) {
        free(pcard_table_mtr2);
    }

    if (pcard_table_mtr1 != NULL) {
        free(pcard_table_mtr1);
    }

    return ret;
}
