## Forks
Both programs use **shared memory**.
### Gauss method
Generate a random matrix of size 5 and bring it to the upper triangular form according to the Gauss method:
```
./fork-gauss -n 5 -c 4 -d
```
**Flags:**
- `-n`: matrix size (default: 5).
- `-c`: core count (if not specified, the count of cores is taken from the system information).
- `-d`: debug mode (print some additional information).
### Merge sort
Generate a random array of size 10 and sort it using a combination of bubble sort and merge sort:
```
./fork-merge -n 20 -d
```
**Flags:**
- `-n`: array size (default: 10).
- `-d`: debug mode (print some additional information).

## Threads
Threads work exactly the same way using **pthread**:
```
./thread-gauss -n 4 -n 2 -d
./thread-merge -n 20
```

## Program speed
Let's compare forks and threads by speed. Let's run the Gauss method with the same parameters:
```
yukatan@LEO-DEB:~/Forks$ time ./fork-gauss -n 1000 -c 4
Count of cores: 4
Matrix size: 1000

Done.

real	0m2,460s
user	0m2,188s
sys	0m0,644s
```
```
yukatan@LEO-DEB:~/Threads$ time ./thread-gauss -n 1000 -c 4
Count of cores: 4
Matrix size: 1000

Done.

real	0m1,417s
user	0m5,104s
sys	0m0,136s
```
Let's compare merge sort:
```
yukatan@LEO-DEB:~/Forks$ time ./fork-merge -n 50000
Array size: 50000
Done.

real	0m4,564s
user	0m9,112s
sys	0m0,000s
```

```
yukatan@LEO-DEB:~/Threads$ time ./thread-merge -n 50000
Array size: 50000
Done.

real	0m2,980s
user	0m5,924s
sys	0m0,000s
```

So the threads are obviously faster.
