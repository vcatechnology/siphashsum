/*
 * This file is part of the siphashsum project.
 *
 * Copyright (C) 2016 Joel Holdsworth <joel.holdsworth@vcatechnology.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include "siphash.h"

const char *app_name;
struct sipkey key;

static void usage(void) {
    printf(
        "Usage: %s KEY [FILE]...\n"
        "Prints siphash24 (64-bit) checksums.\n"
        "With no FILE, or when FILE -, read standard input.\n"
        "\n"
        "  KEY                  The key value as a 128-bit hexadecimal number.\n\n",
        app_name);
}

static void siphash24_file(const char *path) {
    struct siphash hash;
    uint8_t buf[4096];
    size_t bytes_read;

    FILE *const f = (strcmp("-", path) == 0) ? stdin : fopen(path, "rb");
    if (!f) {
        fprintf(stderr, "%s: %s: Failed to open file\n", app_name, path);
        return;
    }

    sip24_init(&hash, &key);
    do {
        bytes_read = fread(buf, 1, sizeof(buf), f);
        sip24_update(&hash, buf, bytes_read);
    } while (bytes_read == sizeof(buf));

    fclose(f);

    printf("%016" PRIx64 "  %s\n", sip24_final(&hash), path);
}

static int hex2dec(const char c) {
    return (c >= '0' && c <= '9') ? (c - '0') :
        ((c >= 'a' && c <= 'f') ? (c - 'a' + 0xA) :
        ((c >= 'A' && c <= 'F') ? (c - 'A' + 0xA) :
        0));
}

int main(int argc, const char **argv) {
    int i;
    uint8_t key_bytes[16];
    const char *key_str = argv[1];

    app_name = argv[0];
    if (argc < 2) {
        usage();
        return 0;
    }

    if (strlen(key_str) != 32) {
        usage();
	return 1;
    }

    for (i = 0; i < sizeof(key_bytes); i++) {
        key_bytes[i] = (hex2dec(key_str[0]) << 4) | hex2dec(key_str[1]);
	key_str += 2;
    }
    sip_tokey(&key, (const void*)key_bytes);

    if (argc == 2)
        siphash24_file("-");
    else
        for (i = 2; i < argc; i++)
            siphash24_file(argv[i]);

    return 0;
}
