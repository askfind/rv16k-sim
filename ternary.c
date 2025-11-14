
#include <assert.h>
#include <stdio.h>
#include <stdint.h>

#include "ternary.h"
#include "cpu_trit.h"
#include "bitpat.h"
#include "inst.h"
#include "log.h"


/** 
 *  ---------------------------------------------------------------------- 
 *  Функции для совместимости троичных типов данных троичного процессора с
 *  бинарными типами данных в двоичном процессоре.   
 */

/* 
 * Получить бинарное состояние трита равное '0' or '1' для бинарной совместимости памяти.  
 */
int getbool(trit t) {
     if (t == 1) {
        return 1;
     }
     else {
        return 0;
     }
}

/* 
 * Преобразовать число trit8 -> uint8
 */
uint8_t trits_to_uint8( tr8 * ts ) {
		uint8_t r;		
	    for(int i=0; i<TRIT8_SIZE; i++) {		
			if( getbool(ts->t[i]) > 0 ) { 
				r |= (1<<i);
			}
			else {
				r &= ~(1<<i);
			}
		}
		return r;
}

/* 
 * Преобразовать число uint8 -> trit8
 */
void uint8_to_trits(tr8 * rt, uint8_t v ) {
		
	    for(int i=0; i<TRIT8_SIZE; i++) {		
			if( (v & (1<<i)) > 0 ) { 
				rt->t[i] = 1;
			}
			else {
				rt->t[i] = 0;
			}			
		}		
}

/* 
 * Преобразовать число trit16 -> uint16
 */
uint16_t trits_to_uint16( tr16 * ts ) {
		uint16_t r = 0;		
	    for(int i=0; i<TRIT16_SIZE; i++) {		
			if( getbool(ts->t[i]) > 0 ) { 
				r |= (1<<i);
			}
			else {
				r &= ~(1<<i);
			}
		}
		return r;
}

/* 
 * Преобразовать число uint16 -> trit16
 */
void uint16_to_trits( tr16 * rt, uint16_t v ) {
				
	    for(int i=0; i<TRIT16_SIZE; i++) {		
			if( (v & (1<<i)) > 0 ) { 
				rt->t[i] = 1;
			}
			else {
				rt->t[i] = 0;
			}			
		}		
}


/** 
 *  ------------------------------------------------------
 *  Функции для троичных типов данных троичного процессора
 */

/*
 * Возведение в степень по модулю 3
 */
int32_t pow3(int8_t x)
{
	int8_t i;
	int32_t r = 1;
	for (i = 0; i < x; i++)
	{
		r *= 3;
	}
	return r;
}

/**
 * Преобразование троичного целого числа в целое со знаком 3 -> 10
 */
int32_t tr16_to_int32(tr16 tr)
{
	int32_t n = 0; 	
	
	for (int i = 0; i < TRIT16_SIZE ; i++)
	{
		trit x = get_trit(tr, i);
		if (x != 0)
		{
			n += pow3(i) * x;
		}
	}

	return n;
}  


/*
 * Преобразовать число со знаком в трит {-1, 0, +1}
 */
trit sign(int x) {
    return (x > 0) - (x < 0);
}

/* 
 * Получить значение трита как символ {'-',0,'+'}
 */
uint8_t signch(int x) {
    int r = (x > 0) - (x < 0);
    if( r > 0 ) return '+';
    if( r < 0 ) return '-';
    return '0';
}

/* 
 * Получить целое со знаком трита в позиции троичного числа
 */
trit get_trit(tr16 tr, uint8_t pos)
{
	trit r;	
	
	if( pos > TRIT16_SIZE-1) {				
		r = 0;		
	}
	else {		
		r = tr.t[pos];
	}	

	return r;
} 

/*
 * Установить значение трита в троичном числе.
 */
tr16 set_trit(tr16 tr, uint8_t pos, trit t)
{
	uint8_t p;
	tr16 res;
	
	if( pos > TRIT16_SIZE-1) {				
		res = tr;
	}
	else {
		res = tr;
		res.t[pos] = t;	
	}
	
	return res;
}


uint8_t symb_nine_form(tr16 v) {
	
	trit t1 = v.t[1];
	trit t0 = v.t[0];
	
	switch (3*t1 + t0) {
		case +4: return '4';
		case +3: return '3';
		case +2: return '2';
		case +1: return '1';
		case  0: return '0';
		case -1: return 'Z';
		case -2: return 'Y';
		case -3: return 'X';
		case -4: return 'W';	
	}
}

void tr16_to_nine_string(uint8_t * buf,tr16 v) {
	
	uint8_t index = 0;
	
	tr16 t;	

	for(int i=0;i<8;i++) {
		t.t[0] = get_trit(v,index++);
		t.t[1] = get_trit(v,index++);
		buf[7-i] = symb_nine_form(t);
	}	
}

 
/*
 * Очистить поле битов троичного числа
 */
void clear_tr8(tr8 *tr)
{
	for(int i=0; i<TRIT8_SIZE; i++) {
		tr->t[i] = 0;
	}
}  
 
void clear_tr16(tr16 *tr)
{
	for(int i=0; i<TRIT16_SIZE; i++) {
		tr->t[i] = 0;
	}
}  

/*
 * Получить трит в младшей позиции троичного числа v.t[0]
 */
trit tr16pt0_to_trit(tr16 v)
{
	return sign(v.t[0]);
}

/*
 *   Операции с тритами
 *   trit = {-1,0,1}  - трит значение трита
 *   Троичное сложение двух тритов с переносом
 */
void sum_t(trit *a, trit *b, trit *p0, trit *s, trit *p1)
{
	*a = sign(*a);
	*b = sign(*b);
	*p0 = sign(*p0);
	*s = sign(*s);
	*p1 = sign(*p1);
		
	switch (*a + *b + *p0)
	{
	case -3:
	{
		*s = 0;
		*p1 = -1;
	}
	break;
	case -2:
	{
		*s = 1;
		*p1 = -1;
	}
	break;
	case -1:
	{
		*s = -1;
		*p1 = 0;
	}
	break;
	case 0:
	{
		*s = 0;
		*p1 = 0;
	}
	break;
	case 1:
	{
		*s = 1;
		*p1 = 0;
	}
	break;
	case 2:
	{
		*s = -1;
		*p1 = 1;
	}
	break;
	case 3:
	{
		*s = 0;
		*p1 = 1;
	}
	break;
	default:
	{
		*s = 0;
		*p1 = 0;
	}
	break;
	}
}  

/* 
 * Троичное умножение тритов
 */
void and_t(trit *a, trit *b, trit *s)
{
	*a = sign(*a);
	*b = sign(*b);
	*s = sign(*s);
		
	if ((*a * *b) > 0)
	{
		*s = 1;
	}
	else if ((*a * *b) < 0)
	{
		*s = -1;
	}
	else
	{
		*s = 0;
	}
}   

/*
 *  Троичное отрицание трита
 */
void not_t(trit *a, trit *s)
{
	trit tr = sign(*a);
	
	if (tr > 0)
	{
		*s = -1;
	}
	else if (tr < 0)
	{
		*s = 1;
	}
	else
	{
		*s = 0;
	}
} 

/*
 * Троичное xor тритов
 */
void xor_t(trit *a, trit *b, trit *s)
{
	*a = sign(*a);
	*b = sign(*b);
	
	if (*a == -1 && *b == -1)
	{
		*s = -1;
	}
	else if (*a == 1 && *b == -1)
	{
		*s = 1;
	}
	else if (*a == -1 && *b == 1)
	{
		*s = 1;
	}
	else if (*a == 1 && *b == 1)
	{
		*s = -1;
	}
	else if (*a == 0 && *b == 1)
	{
		*s = 0;
	}
	else if (*a == 0 && *b == -1)
	{
		*s = 0;
	}
	else if (*a == 1 && *b == 0)
	{
		*s = 0;
	}
	else if (*a == -1 && *b == 0)
	{
		*s = 0;
	}
	else
	{
		*s = 0;
	}
}  

/* 
 * Троичное or тритов
 * OR (A, B)
 *   A ∨ B
 *       B
 *	  - 0 +
 *   - - 0 +
 * A	0 0	0 +
 *   + + + +
 */
void or_t(trit *a, trit *b, trit *s)
{
	*a = sign(*a);
	*b = sign(*b);
	
	if (*a == -1 && *b == -1)
	{
		*s = -1;
	}
	else if (*a == 1 && *b == -1)
	{
		*s = 1;
	}
	else if (*a == -1 && *b == 1)
	{
		*s = 1;
	}
	else if (*a == 1 && *b == 1)
	{
		*s = 1;
	}
	else if (*a == 0 && *b == 1)
	{
		*s = 1;
	}
	else if (*a == 0 && *b == -1)
	{
		*s = -1;
	}
	else if (*a == 1 && *b == 0)
	{
		*s = 1;
	}
	else if (*a == -1 && *b == 0)
	{
		*s = 0;
	}
	else
	{
		*s = 0;
	}
}


/*
 * Троичное ADD сложение тритов
 */
//TODO info RISC-V bits
tr16 add_trs(tr16 x, tr16 y)
{
	int8_t i, j;
	trit a, b, s, p0, p1;
	tr16 r;

	/* Результат */
	uint8_t m = 0;

	j = TRIT16_SIZE;

	p0 = 0;
	p1 = 0;

	for (i = 0; i < j; i++)
	{
		a = get_trit(x, i);
		b = get_trit(y, i);
		sum_t(&a, &b, &p0, &s, &p1);

		if (s > 0)
		{
			r.t[m] = +1;			
		}
		else if (s < 0)
		{
			r.t[m] = -1;			
		}
		else
		{
			r.t[m] = 0;			
		}
		m += 1;
		p0 = p1;
		p1 = 0;
	}

	return r;
}

/* 
 * Троичное вычитание тритов
 */
tr16 sub_trs(tr16 x, tr16 y)
{
	uint8_t i, j;
	trit a, b, s, p0, p1;
	tr16 r;

	/* Результат для Сетунь-1958 R,S */
	uint8_t m = 0;

	j = TRIT16_SIZE;

	p0 = 0;
	p1 = 0;

	for (i = 0; i < j; i++)
	{
		a = get_trit(x, i);
		b = get_trit(y, i);
		b = -b;
		sum_t(&a, &b, &p0, &s, &p1);

		if (s > 0)
		{
			r.t[m] = +1;
		}
		else if (s < 0)
		{
			r.t[m] = -1;
		}
		else
		{
			r.t[m] = 0;
		}
		m += 1;
		p0 = p1;
		p1 = 0;
	}
	
	return r;
}

/*
 * Троичная операция OR тритов 
 */
tr16 or_trs(tr16 x, tr16 y)
{
	tr16 r;
	uint8_t i, j;
	trit a, b, s;

	j = TRIT16_SIZE;

	for (i = 0; i < j; i++)
	{
		a = get_trit(x, i);
		b = get_trit(y, i);
		or_t(&a, &b, &s);
		r = set_trit(x, i, s);
	}
    
	return r;
}  


/* 
 * Операции AND троичная
 */
tr16 and_trs(tr16 x, tr16 y)
{
	tr16 r;

	uint8_t i, j;
	trit a, b, s;

	j = TRIT16_SIZE;

	for (i = 0; i < j; i++)
	{
		a = get_trit(x, i);
		b = get_trit(y, i);
		and_t(&a, &b, &s);
		r = set_trit(x, i, s);
	}
	
	return r;
}

/* 
 * Операции XOR trs
 */
tr16 xor_trs(tr16 x, tr16 y)
{
	tr16 r;

	int8_t i, j, ll;
	trit a, b, s;

	j = TRIT16_SIZE;

	for (i = 0; i < j; i++)
	{
		a = get_trit(x, i);
		b = get_trit(y, i);
		xor_t(&a, &b, &s);
		r = set_trit(x, i, s);
	}
	
	return r;
}

/* 
 * Операция NOT троичная
 */
tr16 not_trs(tr16 x)
{
	tr16 r = x;

	return r;
}

/* 
 * Троичное NEG отрицания тритов
 */
tr16 neg_trs(tr16 t)
{
	return not_trs(t);
}

#if 1 //TODO переписать для нового типа данных как массив тритов
	//TODO add code
#endif

/*
 *  Печать при отладке
 */
void vlog( unsigned char * buf, uint16_t v ) {
		//log_printf("\n\r");
		log_printf("( %s : ",buf);
   		log_printf("x%04x",v);
   		log_printf(" [%d]",v);				
     	log_printf(" )\n\r");     	
}

/* 
 * Печать при отладке
 */
void tlog8( unsigned char * buf, tr8 ts ) {
		log_printf("[ %s : t",buf);
   		for(int j=0;j<8;j++) { 
			log_printf("%c",signch( ts.t[7-j]));
		};
     	log_printf(" ]\n\r");
}
void tlog( unsigned char * buf, tr16 ts ) {
		log_printf("[ %s : t",buf);
   		for(int j=0;j<16;j++) { 
			log_printf("%c",signch( ts.t[15-j]));
		};
     	log_printf(" ]\n\r");
}

/* EOF ternary.c */
