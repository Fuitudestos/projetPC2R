#define _XOPEN_SOURCE 700

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

const char des0[] = {'E', 'T', 'U', 'K', 'N', 'O'};
const char des1[] = {'E', 'V', 'G', 'T', 'I', 'N'};
const char des2[] = {'D', 'E', 'C', 'A', 'M', 'P'};
const char des3[] = {'I', 'E', 'L', 'R', 'U', 'W'};
const char des4[] = {'E', 'H', 'I', 'F', 'S', 'E'};
const char des5[] = {'R', 'E', 'C', 'A', 'L', 'S'};
const char des6[] = {'E', 'N', 'T', 'D', 'O', 'S'};
const char des7[] = {'O', 'F', 'X', 'R', 'I', 'A'};
const char des8[] = {'N', 'A', 'V', 'E', 'D', 'Z'};
const char des9[] = {'E', 'I', 'O', 'A', 'T', 'A'};
const char des10[] = {'G', 'L', 'E', 'N', 'Y', 'U'};
const char des11[] = {'B', 'M', 'A', 'Q', 'J', 'O'};
const char des12[] = {'T', 'L', 'I', 'B', 'R', 'A'};
const char des13[] = {'S', 'P', 'U', 'L', 'T', 'E'};
const char des14[] = {'A', 'I', 'M', 'S', 'O', 'R'};
const char des15[] = {'E', 'N', 'H', 'R', 'I', 'S'};

char* tirageGrille(char* res)
{
    res[0] = des0[rand()%5];
    res[1] = des1[rand()%5];
    res[2] = des2[rand()%5];
    res[3] = des3[rand()%5];
    res[4] = des4[rand()%5];
    res[5] = des5[rand()%5];
    res[6] = des6[rand()%5];
    res[7] = des7[rand()%5];
    res[8] = des8[rand()%5];
    res[9] = des9[rand()%5];
    res[10] = des10[rand()%5];
    res[11] = des11[rand()%5];
    res[12] = des12[rand()%5];
    res[13] = des13[rand()%5];
    res[14] = des14[rand()%5];
    res[15] = des15[rand()%5];

    return res;
}
