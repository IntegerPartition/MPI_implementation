# MPI Parallel implementation
Here we give a Message Passing Interface (MPI) parallel implementation (uploaded .cpp, file) of a known algorithm for generating integer partitions. 
A significant stage in developing a parallel implementation is deciding how to divide the problem into smaller subproblems and solve them in parallel. 
It is important to choose an appropriate data type and ensure minimal communication and data sharing.
MPI is designed for performing simultaneous computations across multiple processing units, each with its own memory. 
In this model, several independent processes—each a copy of the executable—run in parallel without sharing resources such as data.
A common practice in designing parallel algorithms is to divide the work and then combine the results of all parallel computations to obtain the final outcome. 
For this purpose, MPI provides specialized functions that enable communication between processes.

The considered algorithm generates a subset of integer partitions. The idea is to divide the set of partitions 
$P(n)$ into $t$ equal subsets, where $t$ is the number of processes, and each process computes $per_{pr}=p(n)/t$ partitions.
If $per_{pr}$ is not an integer, then each processes, except one, generates $per_{pr}=[p(n)/t]$ partitions, while a single process generates the remaining $p(n)−(t−1)* per_{pr}$ partitions.

The algorithm consists of the following main steps:
1. Compute $p(n)$ and construct a two-dimensional array $N$, where $N[n][k]$ stores the number of partitions of $n$ with parts greater than or equal to $k$.
2. Calculate $per_{pr}$ and determine the starting rank of the first partition ($rank*per_{pr}$, where $rank$ is the identifier of the current process) for each process.
3. Use the unranking function to generate the first partition.
4. Continue generating the next partitions until $per_{pr}$ partitions are obtained."

Using this division method, the workload is distributed almost equally among the processes, with only one process generating slightly more partitions than the others. 
For sufficiently large $p(n)$, this imbalance becomes negligible. 
For instance, when $p(20)=627$ and $24$ threads are used, each process generates $26$ partitions, while the last process generates only $3$ additional partitions, resulting in a nearly uniform distribution of work.
The idea is to generate the partitions $P(n)$ within a given range, from rank $A$ to rank $B$. 
This is done by applying an unranking function to obtain the initial partition corresponding to rank $A$.
Starting from this partition, we then generate the subsequent partitions in $P(n)$ until $B−A+1$ partitions are produced, ensuring that each process generates exactly $per_{pr}$ partitions.

Below is implementation of a subset generation:

	unsigned long long int PartitionSubsetAtoB(unsigned long long int A, unsigned long long int B, int n){ 
	int a_local[MaxN];
	unsigned long long int br = 0;
	unsigned long long int total_l = 0;
	unsigned long long int  k = unrank_lex(n, A, a_local);
	do {
		if (br == B - A) {
			total = total + br + 1;
			return br;		
		}
		int y = a_local[k] - 1;
		k--;
		int x = a_local[k] + 1;
		while (x <= y) {
			a_local[k] = x;
			y = y - x;
			k++;
		}
		a_local[k] = y + x;
		br++;
		//Function usePartition executes additional computations that use the generated partition.
		//usePartition(a_local); 
	} while (k > 0);
	return br;}

It can be use this algorithm to generate the all integer partitions with $𝐴 = 0$ and $𝐵 = P(n)$.

### Experimental results

We present preliminary experimental results used to evaluate the efficiency of the proposed parallel implementations. 
All computations were performed on a system equipped with two Intel Xeon Gold 5118 CPUs running at 2.30 GHz, providing a total of 24 cores and 48 threads, under the RHEL Server 7.8 operating system. 
The code was compiled with GCC 9.2, and Rmpi version 0.6-9 was used. 

Compile: **mpicxx -O3 -DNDEBUG name.cpu -o name**

Run: **mpirun -np x ./name** 


| n     |    CPU    |   PR=2    |   PR=4    |   PR=8   |  PR=16   |  PR=24   |  PR=32   |  PR=48   | 
|-------|-----------|-----------|-----------|----------|----------|----------|----------|----------|
|   80  |  38.310ms |  20.902ms |  14.481ms |  7.714ms |  3.261ms |  2.880ms |  2.352ms |  1.871ms |
|  100  | 413.182ms | 259.706ms | 147.012ms | 73.121ms | 39.793ms | 33.803ms | 25.441ms | 22.991ms |
|  120  | 4122.79ms | 2544.77ms | 1292.21ms | 663.71ms | 362.71ms | 252.39ms | 245.02ms | 168.52ms |
|  140  | 33908.5ms | 19979.7ms | 9970.20ms | 5340.7ms | 2930.9ms | 1961.1ms | 1912.3ms | 1386.7ms |

**Table 1**. Execution times for MPI implementation using subsets

Speedup ($S_p$) is given by the formula:

$S_p=T_1/T_P$

$P=$ number of processors; $T_1=$ time for optimal serial algorithm on one processor; $T_p$= time for parallel algorithm on $P$ processors.

|   n   |CPUvs.PR=2|CPUvs.PR=4|CPUvs.PR=8|CPUvs.PR=16|CPUvs.PR=24|CPUvs.PR=32|CPUvs.PR=48| 
|-------|----------|----------|----------|-----------|-----------|-----------|-----------|
|   80  |   1.83x  |   2.64x  |  4.96x   |  11.74x   |  13.30x   |  16.28x   |  20.47x   |
|  100  |   1.59x  |   2.81x  |  6.65x   |  10.38x   |  12.22x   |  16.24x   |  17.97x   |
|  120  |   1.62x  |   3.19x  |  6.21x   |  11.36x   |  16.33x   |  16.82x   |  24.46x   |
|  140  |   1.69x  |   3.40x  |  6.34x   |  11.57x   |  17.29x   |  17.73x   |  24.46x   |

**Table 2**. Speedup vs. Processes for diffrent $n$ values

![MPI_speedup](https://github.com/user-attachments/assets/62640770-2873-40ba-bc1d-8ccfe2acbca1)

**Figure 1**. Speedup vs. Processes for diffrent $n$ values

Parallel Efficiency is calculated as:

Parallel Efficiency=(Speedup/Processes)*100%

![ParallelEfficiency](https://github.com/user-attachments/assets/8a5bbea4-07f5-4be1-96b6-3ff37b63848d)

**Figure 2**. Parallel Efficiency vs. Processes for diffrent $n$ values

