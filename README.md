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

Compile: **mpicxx -O3 -DNDEBUG name.cpu -o name** and its flags.

Run: **mpirun -np x ./name** 


| n     |  TH=1    |  TH=2    |  TH=4    |  TH=8   | TH=16   | TH=24   | TH=32   | TH=48   | 
|-------|----------|----------|----------|---------|---------|---------|---------|---------|
|   80  | 38.310ms | 20.902ms | 14.481ms | 7.714ms | 3.261ms | 2.880ms | 2.352ms | 1.871ms |
|  100  | 481.18ms | 259.70ms | 147.01ms | 73.12ms | 39.79ms | 33.80ms | 25.44ms | 27.99ms |
|  120  |  4763.79ms | 2544.77ms | 1292.21ms | 663.71ms | 362.7ms | 252.3ms | 245.0ms | 168.5ms |
|  140  | 39243.5ms | 19979.7ms | 9970.2ms | 5340.7ms | 2930.9ms | 1961.1ms | 1912.3ms | 1386.7ms |


