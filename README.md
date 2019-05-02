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
./fork-merge -n
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
