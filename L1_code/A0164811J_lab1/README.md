# Lab 1

### Exercise 7
1. `gcc -pthread -o ex7 ex7-prod-con-threads.c`
2. `./ex7`

### Exercise 8
1. `gcc -pthread -o ex8 ex8-prod-con-processes.c`
2. `./ex8`

### Exercise 9
Exercise 7 (multi-threaded approach) runs faster than Exercise 8 (multi-process approach). This is because multi-processing incurs a larger overhead as the program needs to allocate more resources (registers and stack). In multi-threading, there is no need for copying of the address space which makes thread generation faster than process generation. When I limited the number of total items in both exercises to 100, exercise 7 ran ~1ms faster than exercise 8. 

Steps to test:
1. Change the `THRESHOLD` value to 100
2. `gcc -pthread -o ex7 ex7-prod-con-threads.c`
3. `time ./ex7`
4. `gcc -pthread -o ex8 ex8-prod-con-processes.c`
5. `time ./ex8`
6. Observe `real` values for both outputs  

I also created a separate implementation for exercise 7 using condition variables. Both implementations of ex7 take approximately the same time to run.

Steps to time ex7 with cv:
1. Change the `THRESHOLD` value to 100
2. `gcc -pthread -o ex7-cv ex7-prod-con-threads-cv.c`
3. `time ./ex7-cv`


