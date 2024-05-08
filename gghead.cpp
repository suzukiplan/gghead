#include <ctype.h>
#include <stdio.h>
#include <string.h>

static int hex2i(const char* str, int keta)
{
    if (keta != strlen(str)) {
        printf("ERROR: invalid length: %s\n", str);
        return -1;
    }
    int result = 0;
    for (int i = 0; i < keta; i++) {
        result <<= 8;
        if (isdigit(str[i])) {
            result |= str[i] - '0';
        } else {
            char c = toupper(str[i]);
            if ('A' <= c && c <= 'F') {
                result |= c - 'A' + 10;
            } else {
                printf("ERROR: invalid value: %s\n", str);
                return -1;
            }
        }
    }
    return result;
}

int main(int argc, char* argv[])
{
    const char* path = nullptr;
    int productCode = 0x00000;
    int versionCode = 0x0;
    int regionCode = 0x5;
    bool error = false;

    for (int i = 1; i < argc; i++) {
        if ('-' != argv[i][0]) {
            path = argv[i];
        } else {
            if (argc <= i + 1) {
                error = true;
                break;
            }
            switch (argv[i][1]) {
                case 'p': productCode = hex2i(argv[i + 1], 5); break;
                case 'v': versionCode = hex2i(argv[i + 1], 1); break;
                case 'r': regionCode = hex2i(argv[i + 1], 1); break;
                default:
                    printf("ERROR: unknown option %s\n", argv[i]);
                    error = true;
            }
            if (error) {
                break;
            }
            i++;
        }
    }

    if (!path || error || -1 == productCode || -1 == versionCode || -1 == regionCode) {
        puts("gghead [-p PRODUCT_CODE]");
        puts("       [-v VERSION_CODE]");
        puts("       [-r REGION_CODE]");
        puts("       /path/to/file.rom");
        puts("\nNotes:");
        puts("- PRODUCT_CODE = 00000~FFFFF (5-digit hexadecimal number)");
        puts("- VERSION_CODE = 0~F (1-digit hexadecimal number)");
        puts("- REGION_CODE = Valid codes include the following:");
        puts("  - 3: SMS Japan");
        puts("  - 4: SMS Export");
        puts("  - 5: GG Japan <default>");
        puts("  - 6: GG Export");
        puts("  - 7: GG International");
        return 1;
    }

    FILE* fp = fopen(path, "rb");
    if (!fp) {
        puts("File not found");
        return -1;
    }

    char rom[1024 * 1024 * 4];
    int size = fread(rom, 1, sizeof(rom), fp);
    if (size % 0x4000) {
        puts("Invalid rom file size");
        fclose(fp);
        return -1;
    }
    fclose(fp);

    memcpy(&rom[0x7FF0], "TMR SEGA  ", 10);      // eye-catch
    rom[0x7FFA] = 0x00;                          // check sum (update later)
    rom[0x7FFB] = 0x00;                          // check sum (update later)
    rom[0x7FFC] = productCode & 0x000FF;         // product code (8bits) xxxVV
    rom[0x7FFD] = (productCode & 0x0FF00) >> 8;  // product code (8bits) xVVxx
    rom[0x7FFE] = (productCode & 0xF0000) >> 12; // product code (4bits) Vxxxx
    rom[0x7FFE] |= versionCode & 0x0F;           // version code (4bits)
    rom[0x7FFF] = (regionCode & 0x0F) << 4;      // region code (4bits)

    // set size code (4bits)
    switch (size) {
        case 0x2000: rom[0x7FFF] |= 0x0A; break;   // 8KB
        case 0x4000: rom[0x7FFF] |= 0x0B; break;   // 16KB
        case 0x8000: rom[0x7FFF] |= 0x0C; break;   // 32KB
        case 0xC000: rom[0x7FFF] |= 0x0D; break;   // 48KB
        case 0x10000: rom[0x7FFF] |= 0x0E; break;  // 64KB
        case 0x20000: rom[0x7FFF] |= 0x0F; break;  // 128KB
        case 0x40000: rom[0x7FFF] |= 0x00; break;  // 256KB
        case 0x80000: rom[0x7FFF] |= 0x01; break;  // 512KB
        case 0x100000: rom[0x7FFF] |= 0x02; break; // 1MB
        case 0x200000: rom[0x7FFF] |= 0x03; break; // 2MB
        case 0x400000: rom[0x7FFF] |= 0x04; break; // 4MB
        default:
            printf("Invalid ROM size: %d bytes\n", size);
            return -1;
    }

    // Calc & Update the checksum
    unsigned short sum = 0x0000;
    unsigned short val;
    for (int i = 0; i < size / 2; i++) {
        memcpy(&val, &rom[i << 1], 2);
        sum += val;
    }
    memcpy(&rom[0x7FFA], &sum, 2);
    printf("checksum: %04X\n", sum);

    // Update ROM file
    fp = fopen(path, "wb");
    if (size != fwrite(rom, 1, size, fp)) {
        puts("File write error.");
        fclose(fp);
        return -1;
    }
    fclose(fp);
    return 0;
}