#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define MAX_CODE_LEN 100
#define PATTERN_LEN 5
#define MAX_LINE_LEN 2000
#define MAX_RECORDS 200

const char *digitPatterns[10] = {
    "00110", 
    "10001", 
    "01001", 
    "11000", 
    "00101", 
    "10100", 
    "01100", 
    "00011", 
    "11100", 
    "01110"  
};

const char *START_STOP = "10101";

int isAllDigits(const char *s) {
    for (int i = 0; s[i] != '\0'; i++) {
        if (!isdigit((unsigned char)s[i])) return 0;
    }
    return 1;
}


void generateBinaryBarcode(const char *code, char *output) {
    output[0] = '\0';
    strcat(output, START_STOP);

    for (int i = 0; code[i] != '\0'; i++) {
        int d = code[i] - '0';
        strcat(output, digitPatterns[d]);
    }

    strcat(output, START_STOP);
}


void binaryToVisual(const char *binary, char *visual) {
    int len = (int)strlen(binary);
    for (int i = 0; i < len; i++) {
        visual[i] = (binary[i] == '1') ? '|' : ' ';
    }
    visual[len] = '\0';
}


int visualToBinary(const char *visual, char *binary) {
    int len = (int)strlen(visual);
    for (int i = 0; i < len; i++) {
        if (visual[i] == '|') binary[i] = '1';
        else if (visual[i] == ' ') binary[i] = '0';
        else return 0; 
    }
    binary[len] = '\0';
    return 1;
}


int patternToDigit(const char *pattern) {
    for (int d = 0; d < 10; d++) {
        if (strcmp(pattern, digitPatterns[d]) == 0) return d;
    }
    return -1;
}


int decodeBinaryBarcode(const char *binary, char *decoded) {
    int len = (int)strlen(binary);
    int startLen = (int)strlen(START_STOP);

    if (len < 2 * startLen) return 0;

    if (strncmp(binary, START_STOP, startLen) != 0) return 0;
    if (strncmp(binary + (len - startLen), START_STOP, startLen) != 0) return 0;

    int middleLen = len - 2 * startLen;
    if (middleLen % PATTERN_LEN != 0) return 0;

    int digitsCount = middleLen / PATTERN_LEN;
    decoded[0] = '\0';

    for (int i = 0; i < digitsCount; i++) {
        char chunk[PATTERN_LEN + 1];
        int pos = startLen + i * PATTERN_LEN;

        strncpy(chunk, binary + pos, PATTERN_LEN);
        chunk[PATTERN_LEN] = '\0';

        int d = patternToDigit(chunk);
        if (d == -1) return 0;

        char temp[2];
        temp[0] = (char)('0' + d);
        temp[1] = '\0';
        strcat(decoded, temp);
    }

    return 1;
}


int appendToFile(const char *filename, const char *code, const char *visualBarcode) {
    FILE *f = fopen(filename, "a"); 
    if (!f) return 0;

    fprintf(f, "%s\t%s\n", code, visualBarcode);
    fclose(f);
    return 1;
}


int readAllRecords(const char *filename, char codes[][MAX_CODE_LEN], char visuals[][MAX_LINE_LEN]) {
    FILE *f = fopen(filename, "r");
    if (!f) return 0;

    int count = 0;
    char line[MAX_LINE_LEN];

    while (fgets(line, sizeof(line), f) && count < MAX_RECORDS) {
        int l = (int)strlen(line);
        if (l > 0 && line[l - 1] == '\n') line[l - 1] = '\0';

        char *tabPos = strchr(line, '\t');
        if (!tabPos) {
            continue;
        }

        *tabPos = '\0';
        tabPos++;

        strncpy(codes[count], line, MAX_CODE_LEN - 1);
        codes[count][MAX_CODE_LEN - 1] = '\0';

        strncpy(visuals[count], tabPos, MAX_LINE_LEN - 1);
        visuals[count][MAX_LINE_LEN - 1] = '\0';

        count++;
    }

    fclose(f);
    return count;
}

int main() {
    int choice;

    printf("=== Barcode Reader and Generator (Multiple Saved) ===\n");
    printf("1) Generate barcode and APPEND to file\n");
    printf("2) Read barcode list from file and choose one to decode\n");
    printf("Choose (1 or 2): ");
    scanf("%d", &choice);

    if (choice == 1) {
        char code[MAX_CODE_LEN];
        char binary[1000];
        char visual[1000];

        printf("Enter product code (digits only): ");
        scanf("%s", code);

        if (!isAllDigits(code)) {
            printf("Error: code must contain only digits!\n");
            return 0;
        }

        generateBinaryBarcode(code, binary);
        binaryToVisual(binary, visual);

        printf("\nGenerated barcode (visual):\n%s\n", visual);

        if (appendToFile("barcode.txt", code, visual)) {
            printf("\nAppended to barcode.txt (new record added)\n");
        } else {
            printf("\nError saving file.\n");
        }

    } else if (choice == 2) {
        char codes[MAX_RECORDS][MAX_CODE_LEN];
        char visuals[MAX_RECORDS][MAX_LINE_LEN];

        int count = readAllRecords("barcode.txt", codes, visuals);
        if (count == 0) {
            printf("No records found or cannot read barcode.txt\n");
            return 0;
        }

        printf("\nSaved barcodes in file:\n");
        for (int i = 0; i < count; i++) {
            printf("%d) Code: %s\n", i + 1, codes[i]);
        }

        int pick;
        printf("\nChoose number to decode (1-%d): ", count);
        scanf("%d", &pick);

        if (pick < 1 || pick > count) {
            printf("Invalid choice.\n");
            return 0;
        }

        char readBinary[2000];
        char decoded[MAX_CODE_LEN];

        printf("\nChosen record:\n");
        printf("Stored code: %s\n", codes[pick - 1]);
        printf("Visual barcode:\n%s\n", visuals[pick - 1]);

        if (!visualToBinary(visuals[pick - 1], readBinary)) {
            printf("Error: barcode contains invalid characters.\n");
            return 0;
        }

        if (decodeBinaryBarcode(readBinary, decoded)) {
            printf("\nDecoded product code: %s\n", decoded);
        } else {
            printf("\nError: cannot decode barcode (wrong format).\n");
        }

    } else {
        printf("Invalid choice.\n");
    }

    return 0;
}
