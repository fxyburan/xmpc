## A Secure Multi-party Computing System Supporting Operator Parallel Scheduling

Expression parsing and operator scheduling are the beginning of high-performance parallel computing. This project mainly implements expression parsing and parallel scheduling for secure multi-party computation (MPC) operators, which is quite different from the process of plaintext computing.

### Steps
1. An Expression String
2. Reverse Polish Notation
3. Abstract Syntax Tree (AST)
4. Directed Acyclic Graph (DAG)
5. Topological Sort
6. Multi-thread Parallel Execution

### Examples
For `a * log(b + d * (c - d)) + sqrt(e)` where `a, b, c, d, e` are MPC variables. The priorities of operators are as follows.
```
<index, token, priority>
<1, a, Priority: 0>
<2, b, Priority: 0>
<3, d, Priority: 0>
<4, c, Priority: 0>
<5, d, Priority: 0>
<11, e, Priority: 0>
<6, -, 4, 5, Priority: 1>
<12, sqrt, 11, Priority: 1>
<7, *, 3, 6, Priority: 2>
<8, +, 2, 7, Priority: 3>
<9, log, 8, Priority: 4>
<10, *, 1, 9, Priority: 5>
<13, +, 10, 12, Priority: 6>
```

Note that both of the variables and the operators are treated as a node in AST. The leaf nodes (where we do secret sharings at the very beginning) have the highest priority.

### Dependencies
- The MPC protocols are based on [The ABY3 Protocols](https://eprint.iacr.org/2018/403.pdf).
- Library: OpenSSL (for PRNG).
- Hardware: Intel CPU supporting AES Intrinsics. ARM CPUs may also be supported (see [sse2neon](https://github.com/fxyburan/xmpc/blob/main/include/sse2neon.h)), but not tested.
- Testing Platform: Ubuntu 20.04 LTS.

### Build
```bash
mkdir build & cd build
cmake ..
make -j 4
./crypto # For the whole procedure
./x-scheduler-test # For testing the scheduler only
```
