//
// Lab 2.
//
// Троичные операции для обработки тритов на языке С.
//
// Пример Объявления массива тритов. Отрицательный индекс.
//

// Компиляция: gcc main.c

#include <stdint.h>
#include <stdio.h>
#include <string.h>


int main(void) {

    int test_array[5] = {1,2,3,4,5,};
    int *p = test_array + 2;
    int i;
    int j;

    // This is equivalent to:
    i = p[-2];   // i now has the value 1
    j = *(p-2);  // j now has the value 1
    printf(" i=%d, j=%d\n\r", i,j );

    // This is equivalent to:
    i = p[+2];   // i now has the value 5
    j = *(p+2);  // j now has the value 5
    printf(" i=%d, j=%d\n\r", i,j );

    return 0;
}
