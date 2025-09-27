# MPI Parallel implementation
Here we give a Message Passing Interface (MPI) parallel implementation of a known algorithm for generating integer partitions. 
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
