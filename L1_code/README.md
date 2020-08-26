# Lab 1

### Exercise 1
when `fork()` is called, it creates a copy of the program in the child process. Since the `printf` statement that follows after is not bounded by any conditions, both the main and child process will execute it. Moving the `printf` statement into the condition where `fork_ret == 0` will cause it to only execute in the child process

### Exercise 2
Yes, this is because the threads are always being created in the same order. The final value of `counter` is always the same for a fixed value of `NUM_THREADS`. Each thread will be created sequentially in the `for` loop.

### Exercise 3
### Exercise 4
1. `gcc -pthread -o ex4 ex4-race-condition.c`
2. `./ex4`

Final result of global variable is printed after all threads are completed.

### Exercise 5
1. `gcc -pthread -o ex5 ex5-race-condition.c`
2. `./ex5`

### Exercise 7
1. `gcc -o ex7 ex7-prod-con-threads.c`
2. `./ex7`

### Exercise 8
1. `gcc -o ex8 ex8-prod-con-processes.c`
2. `./ex8`

### Exercise 9




