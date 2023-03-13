/* gcc -std=c89 oder c99 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <stdint.h>

typedef uint8_t U8;
typedef uint16_t U16;
typedef uint32_t U32;
typedef uint64_t U64;
typedef int8_t S8;
typedef int16_t S16;
typedef int32_t S32;
typedef int64_t S64;
typedef float F32;
typedef double F64;
typedef unsigned char byte;

#define HEAD 1*sizeof(int)
#define SIZE(s) ((int *)(s))[-1]
#define SET_SIZE(s, z) ((int *)(s))[-1] = z

char *create_string(unsigned int len) {
    char *str;
    void *mem = malloc(HEAD + len * sizeof(char));
    if (mem == NULL) return NULL;
    str = (char *) (HEAD + (char *) mem);
    SET_SIZE(str, len);
    return str;
}

int main() {
    /* The first 4 bytes store the string length, the rest the actual string.
     * 0D 00 00 00 48 65 6C 6C 6F 2C 20 57 6F 72 6C 64 21
     * |--------|  |-----------------------------------|
     *    len        string contents
     */
    char *str = create_string(13);
    memmove(str, "Hello, World!", 13);
    printf("string len: %d\n", SIZE(str));
    printf("string content: %.*s\n", SIZE(str), str);

    byte *p = (byte *) str - HEAD;
    int i;
    for (i = 0; i < HEAD+SIZE(str); ++i) {
        printf("%02hhX ", p[i]);
    }
    printf("\n");
}
