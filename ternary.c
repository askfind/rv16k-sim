
#include <assert.h>
#include <stdio.h>
#include <stdint.h>

#include "ternary.h"
#include "cpu_trit.h"
#include "bitpat.h"
#include "inst.h"
#include "log.h"

int getbool(trit t) {
     if (t == 1) {
        return 1;
     }
     else {
        return 0;
     }
}

int sign(int x) {
    return (x > 0) - (x < 0);
}

int signch(int x) {
    int r = (x > 0) - (x < 0);
    if( r > 0 ) return '+';
    if( r < 0 ) return '-';
    return '0';
}

uint8_t trits_to_uint8( struct trit8 * ts ) {
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

void uint8_to_trits(struct trit8 * rt, uint8_t v ) {
		
	    for(int i=0; i<TRIT8_SIZE; i++) {		
			if( (v & (1<<i)) > 0 ) { 
				rt->t[i] = 1;
			}
			else {
				rt->t[i] = 0;
			}			
		}		
}

uint16_t trits_to_uint16( struct trit16 * ts ) {
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

void uint16_to_trits( struct trit16 * rt, uint16_t v ) {
				
	    for(int i=0; i<TRIT16_SIZE; i++) {		
			if( (v & (1<<i)) > 0 ) { 
				rt->t[i] = 1;
			}
			else {
				rt->t[i] = 0;
			}			
		}		
}

void vlog( unsigned char * buf, uint16_t v ) {

		log_printf("\n\r");
		log_printf("( %s : ",buf);
   		log_printf("x%04x",v);		
     	log_printf(" )\n\r");

}

void tlog( unsigned char * buf, struct trit16 ts ) {
		
		log_printf("\n\r");
		log_printf("[ %s : t",buf);
   		for(int j=0;j<16;j++) { 
			log_printf("%c",signch( ts.t[j]));
		};
     	log_printf(" ]\n\r");
}

/* EOF ternary.c */
