#ifndef TERNARY_H
#define TERNARY_H

#include <stdint.h>

#define min(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })

#define max(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })   

#define TRIT8_SIZE  8
#define TRIT16_SIZE 16
#define TRIT32_SIZE 32

typedef int trit;

struct trit8 {
       trit t[TRIT8_SIZE];
};

struct trit16 {
       trit t[TRIT16_SIZE];
};

struct trit32 {
       trit t[TRIT32_SIZE];
};

/* Определить троичные типы чисел */
typedef struct trit8  tr8;
typedef struct trit16 tr16;
typedef struct trit32 tr32;

trit sign(int x);
uint8_t signch(int x); 

uint8_t symb_nine_form(tr16 v);
void tr16_to_nine_string(uint8_t * buf, tr16 v);
void tr32_to_nine_string(uint8_t * buf, tr32 v);

/*
 *   Операции с тритами
 *   trit = {-1,0,1}  - трит значение трита
 */
void and_t(trit *a, trit *b, trit *s);
void xor_t(trit *a, trit *b, trit *s);
void or_t(trit *a, trit *b, trit *s);
void not_t(trit *a, trit *s);
void sum_t(trit *a, trit *b, trit *p0, trit *s, trit *p1);

/*
 *   Троичные числа
 *
 *   TRITS-16 = [t15...t0] - обозначение позиции тритов в числе
 *
 */
trit get_trit16(tr16 t, uint8_t pos);
tr16 set_trit16(tr16 tr, uint8_t pos, trit t);

trit get_trit32(tr32 t, uint8_t pos);
tr32 set_trit32(tr32 tr, uint8_t pos, trit t);

int32_t tr16_to_int32(tr16 tr);
int64_t tr32_to_int32(tr32 tr);

uint8_t trits_to_uint8(tr8 * ts);
void uint8_to_trits(tr8 * rt,  uint8_t v);
uint16_t trits_to_uint16(tr16 * ts);
void uint16_to_trits(tr16 * rt, uint16_t v);


/*
 * Общие функции для троичных чисел из тритов
 */
void clear_tr16(tr16 *t);
void clear_tr32(tr32 *t);

void inc_trs(tr16 *tr);
void dec_trs(tr16 *tr);

tr32 and_trs(tr32 a, tr32 b);
tr32 or_trs(tr32 a, tr32 b);
tr32 xor_trs(tr32 a, tr32 b);
tr32 add_trs(tr32 a, tr32 b);
tr32 sub_trs(tr32 a, tr32 b);
tr32 mul_trs(tr32 a, tr32 b);
tr32 div_trs(tr32 a, tr32 b);
tr32 shift_trs(tr32 t, int8_t s);
int cmp_trs(tr32 a, tr32 b); 

/*
 * Отладочные функции для троичных чисел из тритов
 */
void vlog16( unsigned char * buf, uint16_t v );
void vlog32( unsigned char * buf, uint32_t v );

void tlog16(unsigned char * buf, tr16 ts );
void tlog32(unsigned char * buf, tr32 ts );

void buflog8( uint8_t * buf );
void buflog16( uint8_t * buf );

#endif  /* TERNARY_H */
