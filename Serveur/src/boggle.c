#define _XOPEN_SOURCE 700

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

char des0[] = {'E', 'T', 'U', 'K', 'N', 'O'};
char des1[] = {'E', 'V', 'G', 'T', 'I', 'N'};
char des2[] = {'D', 'E', 'C', 'A', 'M', 'P'};
char des3[] = {'I', 'E', 'L', 'R', 'U', 'W'};
char des4[] = {'E', 'H', 'I', 'F', 'S', 'E'};
char des5[] = {'R', 'E', 'C', 'A', 'L', 'S'};
char des6[] = {'E', 'N', 'T', 'D', 'O', 'S'};
char des7[] = {'O', 'F', 'X', 'R', 'I', 'A'};
char des8[] = {'N', 'A', 'V', 'E', 'D', 'Z'};
char des9[] = {'E', 'I', 'O', 'A', 'T', 'A'};
char des10[] = {'G', 'L', 'E', 'N', 'Y', 'U'};
char des11[] = {'B', 'M', 'A', 'Q', 'J', 'O'};
char des12[] = {'T', 'L', 'I', 'B', 'R', 'A'};
char des13[] = {'S', 'P', 'U', 'L', 'T', 'E'};
char des14[] = {'A', 'I', 'M', 'S', 'O', 'R'};
char des15[] = {'E', 'N', 'H', 'R', 'I', 'S'};

char* tirageGrille(char* res)
{
    int i;
    int tmp;
    char** mesDes = malloc(sizeof(char*) * 16);

    mesDes[0] = des0;
    mesDes[1] = des1;
    mesDes[2] = des2;
    mesDes[3] = des3;
    mesDes[4] = des4;
    mesDes[5] = des5;
    mesDes[6] = des6;
    mesDes[7] = des7;
    mesDes[8] = des8;
    mesDes[9] = des9;
    mesDes[10] = des10;
    mesDes[11] = des11;
    mesDes[12] = des12;
    mesDes[13] = des13;
    mesDes[14] = des14;
    mesDes[15] = des15;

    for(i = 0; i < 16; i++)
    {
        tmp = rand() % (16-i);

        res[i] = mesDes[tmp][rand() % 6];

        mesDes[tmp] = mesDes[15-i];
    }

    free(mesDes);

    return res;
}
