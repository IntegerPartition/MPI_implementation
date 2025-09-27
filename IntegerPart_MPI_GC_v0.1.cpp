#include <iostream>
#include <mpi.h>
#include <stdio.h>
#include <time.h>
#include <chrono>

using namespace std;

struct  partt {
	int a[128];
}part;

const int MaxN = 140;
unsigned long long int P[MaxN];
unsigned long long int N[MaxN][MaxN];

unsigned long long int total = 0;

const unsigned int MaxProc = 48;
unsigned long long counter[MaxProc];

void printPartition(unsigned long long int k, int* b) {
	//printf("\n");
	for (int j = 0; j <= k; j++) { printf("%d ", b[j]); } printf("  k = %llu ", k);
	printf("\n");
}

unsigned long long int enumPart_basic(int n) {
	for (int i = 0; i < MaxN; i++) { P[i] = 0; }
	P[0] = 1; // Base case

	for (int i = 1; i <= n; i++) {
		for (int j = i; j <= n; j++) {
			P[j] += P[j - i];
		}
	}
	//printf("P[%d] = %llu\n", n, P[n]);
	return P[n];
}

void compute_partition_table(int NN) { // za unrank lex
	for (int i = 0; i <= NN; i++) {
		for (int j = 0; j <= NN; j++) {
			N[i][j] = 0;
		}
	}
	for (int n = 0; n <= NN; n++) {
		for (int m = NN; m >= 0; m--) {
			if (n == 0) {
				N[n][m] = 1;
			}
			else if (m > n) {
				N[n][m] = 0;
			}
			else {
				N[n][m] = N[n][m + 1] + N[n - m][m];
			}
		}
	}
	//print partition table
	//for (int i = 0; i <= NN; i++) {
	//	for (int j = 0; j <= NN; j++) {
	//		cout << N[i][j] << " ";
	//	}
	//	cout << endl;
	//}
}

unsigned long long int unrank_lex(int n, unsigned long long int rank, int* b) {
	int m = 1;
	unsigned long long int len = 0;
	while (n > 0) {
		for (int x = m; x <= n; x++) {
			// number of partitions of n with fixed first part k
			unsigned long long int count = N[n - x][x]; 
			if (rank < count) {
				b[len++] = x;
				n-= x;
				m = x;
				break;
			} 
			else { rank -= count; }
		}
	}
	return len - 1;
}

unsigned long long int PartitionSubsetAtoB(unsigned long long int A, unsigned long long int B, int n)
{
	int a_local[MaxN];
	unsigned long long int br = 0;
	unsigned long long int total_l = 0;

	unsigned long long int  k = unrank_lex(n, A, a_local);
	//printf("A:%llu, B:%llu \n", A, B);
	//printf("br:%llu   ", br);
	//printPartition(k, a_local);

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
		//usePartition(a_local);
		//printf("br:%llu   ", br);
		//printPartition(k, a_local);		
	} while (k > 0);
	return br;
}

int main(int argc, char** argv) 
{
	int n = 140;
	unsigned long long int counterPartitions = 0;
	unsigned long long int counterPartitionsAll = 0;

	MPI_Init(&argc, &argv);
	int rank, size;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	if (size <= MaxProc) {
		// Start measuring time
		auto startCalc = std::chrono::high_resolution_clock::now();
		clock_t beg = clock(), end;

		total = enumPart_basic(n);
		compute_partition_table(n);

		unsigned long long int words_per_process = total / (size);
		unsigned long long int words_last_proc = total - words_per_process * size;

		unsigned long long int send_count = 0;
		if (rank == size - 1) { send_count = 0; }
		else { send_count = words_per_process; }
		unsigned long long int rec_counts[MaxProc], displ[MaxProc];
		for (int i = 0; i < size - 1; i++) {
			rec_counts[i] = words_per_process;
			displ[i] = i * words_per_process;
		}
		rec_counts[size - 1] = 0;
		displ[size - 1] = (size - 1) * words_per_process;

		unsigned long long int A = rank * words_per_process, B = ((rank + 1) * words_per_process) - 1;
		if (rank == (size - 1)) B = total - 1;
		// if (rank != 0) { A--; }

		counterPartitions= PartitionSubsetAtoB(A, B, n);

		//MPI_Gatherv(T + rank * words_per_process, send_count, MPI_INT, T, rec_counts, displ, MPI_INT, size - 1, MPI_COMM_WORLD);
		//printf("\nrank = %d\nNumber Partitions = %llu\n\n",rank, counterPartitions+1);
		MPI_Gather(&counterPartitions, 1, MPI_UNSIGNED_LONG_LONG, counter, 1, MPI_UNSIGNED_LONG_LONG, size - 1, MPI_COMM_WORLD);

		end = clock();
		double time = (end - beg) / (double(CLOCKS_PER_SEC));
		double time_ms = (end - beg) * 1000.0 / CLOCKS_PER_SEC;
		
		// Stop measuring time and calculate the elapsed time
		auto endCalc = chrono::high_resolution_clock::now();
		auto elapsedGenMask = std::chrono::duration_cast<std::chrono::nanoseconds>(endCalc - startCalc);

		if (rank == size - 1) {
			printf("Elapsed time:%.3f ms\n", time_ms);
			printf("Elapsed time:%.3f s\n\n", time);	

			printf("Time measured (chrono): %.3f seconds.\n", elapsedGenMask.count() * 1e-9);
			printf("Time measured (chrono): %.3f miliseconds.\n", elapsedGenMask.count() * 1e-6);
			
			printf("\nTotal Number of Partitions= %llu\n", total);
			for (int i = 0; i < size; i++)
			{
				counterPartitionsAll = counterPartitionsAll+(counter[i] + 1);
			}
			printf("Compute Number of Partitions= %llu\n\n", counterPartitionsAll);
		}
	}
		else {
			cout << "The number of proccesses is more than the allowed static number MaxProc = " << MaxProc << endl;
		}

MPI_Finalize();
return 0;
}
