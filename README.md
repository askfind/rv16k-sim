## Functional Simulator RISC-V, TRIT-RISC-V


# RISC-V RV32E/RV32I
# TRIT-RISC-V TRV32E/TRV32I


# How to use

## Build
```
make
```

## Clean
```
make clean
```


## Test RISC-V Instruction RV32E/RV32I
```
make test
```

## Test TRIT-RISC-V Instruction TRV32E/TRV32I
```
make test_trit
```


## Use
```
Usage: ./main NCYCLES [-a] [-q] [-m] [-t ROM] [-d RAM] [FILENAME]
Options:
  -q       : No log print
  -m       : Dump memory
  -t ROM   : Initial ROM data
  -d RAM   : Initial RAM data
  -a       : Set cpu trits
  FILENAME : ELF Binary
```

