#ifndef TERNARY_H
#define TERNARY_H

#include <stdint.h>

#define TRIT8_SIZE  8
#define TRIT16_SIZE 16

typedef int trit;

struct trit8 {
       trit t[TRIT8_SIZE];
};

struct trit16 {
       trit t[TRIT16_SIZE];
};

/* Прототипы функций для работы с тритами */
int getbool(trit t) ;
int sign(int x);
int signch(int x) ;

uint8_t trits_to_uint8( struct trit8 * ts );
void uint8_to_trits(struct trit8 * rt,  uint8_t v );
uint16_t trits_to_uint16( struct trit16 * ts );
void uint16_to_trits(struct trit16 * rt, uint16_t v );

void vlog( unsigned char * buf, uint16_t v );
void tlog(unsigned char * buf, struct trit16 ts );

#endif  /* TERNARY_H */
