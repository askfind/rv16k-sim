#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>

#include "cpu_trit.h"
#include "elf_parser.h"
#include "bitpat.h"
#include "inst.h"

#include "ternary.h"
#include "inst_trit.h"

#include "log.h"

extern int flag_quiet;
extern int flag_inst_trits;

void print_usage(FILE *fh)
{
    fprintf(fh, "Usage: trit-rv32e-sim [-q] [-m] [-t ROM] [-d RAM] [FILENAME] NCYCLES\n");
    fprintf(fh, "Options:\n");
    fprintf(fh, "  -q     : No log print\n");
    fprintf(fh, "  -m     : Dump memory\n");
    fprintf(fh, "  -t ROM : Initial ROM data\n");
    fprintf(fh, "  -d RAM : Initial RAM data\n");
    fprintf(fh, "  -a     : Set cpu trits\n");
}

_Noreturn void print_usage_to_exit(void)
{
    print_usage(stderr);
    exit(1);
}

void print_flags(struct cpu_trit *c){
     log_printf("FLAGS(SZCV) => %c%c%c%c ", signch(c->flag_sign), signch(c->flag_zero), signch(c->flag_carry), signch(c->flag_overflow));
}

void init_cpu(struct cpu_trit *c){

    c->inst_rom = &c->inst_rom_arr[INST_ROM_SIZE/2];
    c->data_ram = &c->data_ram_arr[DATA_RAM_SIZE/2];    
    
    for(int i=0;i<REGISTER_SIZE;i++){        
         uint16_to_trits(&c->reg[i], i);
    }

    for(int i=-INST_ROM_SIZE/2;i<INST_ROM_SIZE/2;i++){
        uint8_to_trits(&c->inst_rom[i],0);
    }

    for(int i=-DATA_RAM_SIZE/2;i<DATA_RAM_SIZE/2;i++){                
		for (int j = TRIT8_SIZE-1; j >= 0 ; j--) {                	
			c->data_ram[i].t[j] = sign(1-(rand()%3));
		}        
    }

    for(int j=0;j<TRIT16_SIZE;j++){
        c->pc.t[j] = 0;
    }

    c->flag_sign = sign(-1);
    c->flag_overflow = sign(0);
    c->flag_zero = sign(1);
    c->flag_carry = sign(-1);

}

void dump_memory(FILE *fh, tr8 *mem, int size)
{
	fprintf(fh, "\n\rDump ternary RAM[%0i]:\n\r",size);	
    
    for (int i = 0; i < size; i++) {
		
       	fprintf(fh, "t");
		for (int j = TRIT8_SIZE-1; j >= 0 ; j--) {                	
        	fprintf(fh, "%c", signch(mem[i].t[j]));
		}
		fprintf(fh," ");
		if (i % 8 == 7)   fprintf(fh, "\n");
	}
}

void set_bytes_from_str(tr8 *dst, const char * const src, int N)
{
    char *buf = (char *)malloc(strlen(src) + 1);
    strcpy(buf, src);

    int idst = 0;
    char *tp;
    tp = strtok(buf, " ");
    while (tp != NULL) {
        assert(idst < N && "Too large data!");
        uint8_t val = strtol(tp, NULL, 16);
        
		uint8_to_trits(&dst[idst++],val);
		
        tp = strtok(NULL, " ");
    }

    free(buf);
}

void set_cpu_support_trits(void)
{
     printf("TODO add code support cpu trit!\n\r\n\r");
}

int main(int argc, char *argv[]){

    static struct cpu_trit cput;

    init_cpu(&cput);
	
	//viv+ dbg ---------
        if( flag_inst_trits > 0 ) {
            trs_dbg_oper(&cput);
            return 0;
        }
	//------------------
	
    int flag_load_elf = 1, flag_memory_dump = 0, opt;
    while((opt = getopt(argc, argv, "aqmt:d:")) != -1) {
        switch(opt) {
            case 'q':
                flag_quiet = 1;
                break;

            case 'm':
                flag_memory_dump = 1;
                break;

            case 't':
                flag_load_elf = 0;
                set_bytes_from_str(cput.inst_rom, optarg, INST_ROM_SIZE);
                break;

            case 'd':
                flag_load_elf = 0;
                set_bytes_from_str(cput.data_ram, optarg, DATA_RAM_SIZE);
                break;

            case 'a':
                flag_inst_trits = 1;
                set_cpu_support_trits();
                break;

            default:
                print_usage_to_exit();
        }
    }

    if (optind >= argc) print_usage_to_exit();


    int iarg = optind;
    if (flag_load_elf)
        elf_parse(&cput, argv[iarg++]);


    int ncycles = 0;
    if (iarg >= argc) print_usage_to_exit();
    ncycles = atoi(argv[iarg]);


    for(int i=0;i<ncycles;i++){
        
        if( flag_inst_trits>0 ) {
			
			/* Отладка троичных инструкций cpu_trit */
			trs_dbg_oper(&cput);
			
			/* троичные инструкции cpu_trit */
			
			//TODO ~ code 
			//tr16 trs_inst = trs_rom_read_w(&cput);
			//for(int idx=0;trs_inst_list[idx].bit_pattern != NULL;idx++){        
			//	if(trs_bitpat_match_s(inst, inst_list[idx].bit_pattern)){
			//		trs_inst_list[idx].func(&cput, trs_inst);
			//		break;
			//	}                      	
			//	//vlog("t",inst); //viv+ test
			//}
			
		}
		else {
			uint16_t inst = rom_read_w(&cput);

			for(int idx=0;inst_list[idx].bit_pattern != NULL;idx++){        
				if(bitpat_match_s(inst, inst_list[idx].bit_pattern)){
					inst_list[idx].func(&cput, inst);
					break;
				}                      	
				//vlog("t",inst); //viv+ test
			}

		}        


        print_flags(&cput);
        log_printf("\n");

        if (flag_memory_dump){
            dump_memory(stdout, cput.data_ram, DATA_RAM_SIZE);
            printf("\n");
        }
    }

        if( flag_inst_trits>0 ) {
			for (int i = 0; i < REGISTER_SIZE; i++){
				uint16_t val = reg_read(&cput, i);
				printf("tx%d=%d\n\r", i, val);
			}
		}
		else {
			//TODO ~ вывод HEX 
			for (int i = 0; i < REGISTER_SIZE; i++){
				uint16_t val = reg_read(&cput, i);
				printf("x%d=%d\n\r", i, val);
			}	
		}
    puts("");

    return 0;
}
