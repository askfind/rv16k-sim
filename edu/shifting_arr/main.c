//
// Lab 1.
//
// Троичные операции для обработки тритов на языке С.
//
// Пример реализации операции сдвига тритов. Триты объявляются как массив int8_t.
//

// Компиляция: gcc main.c


//
// Синтаксис перемещения для языка C
//
// The syntax for the memmove function in the C Language is:
// void *memmove(void *s1, const void *s2, size_t n);
//


#include <stdint.h>
#include <stdio.h>
#include <string.h>

// Вернуть значение трита {-1, 0, +1}
int sign(int x) {
    return (x > 0) - (x < 0);
}

// Вернуть символ трита {'-', '0', '+'}
int signch(int x) {
    int r = (x > 0) - (x < 0);
    if( r > 0 ) return '+';
    if( r < 0 ) return '-';
    return '0';
}

// Троичное число с фиксированной длиной
// языке С как структуру
int8_t arr1_trits[] = {-1,+1, 0,+1,-1};
int8_t arr2_trits[] = { 0,-1,+1, 0, 0 };

int main(void) {

    //
    // Троичная функция сдвига влево троичного числа.
    //
    memmove(arr1_trits, arr1_trits+1, 4*sizeof(int8_t));
    arr1_trits[4] = 0;

    // Печать троичного числа.

    printf("Shift to the left:\n\r  arr1_trits[] = ");
    for(int i=0 ; i<5 ; i++)
    {
        printf("%d ", sign( arr1_trits[i]) );
    }
    printf("\n\r");


    //
    // Троичная функция сдвига вправо троичного числа.
    //
    memmove(arr2_trits+1, arr2_trits+1, 4*sizeof(int8_t));
    arr2_trits[0] = 0;

    // Печать троичного числа.
    printf("Shift to the right:\n\r arr2_trits[] = ");
    for(int i=0 ; i<5 ; i++)
    {
        printf("%d ", sign( arr2_trits[i] ));
    }
    printf("\n\r");

}
