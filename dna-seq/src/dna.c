#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <time.h>
#include <sys/time.h>

// input and output files
FILE *fdatabase, *fquery, *fout;

// MAX char table (ASCII)
#define MAX 256

// Boyer-Moore-Horspool-Sunday algorithm for string matching
int bmhs(char *string, int n, char *substr, int m) {

	int d[MAX];
	int i, j, k;

	// pre-processing
	for (j = 0; j < MAX; j++)
		d[j] = m + 1;
	for (j = 0; j < m; j++)
		d[(int) substr[j]] = m - j;

	// searching
	i = m - 1;
	while (i < n) {
		k = i;
		j = m - 1;
		while ((j >= 0) && (string[k] == substr[j])) {
			j--;
			k--;
		}
		if (j < 0)
			return k + 1;
		i = i + d[(int) string[i + 1]];
	}

	return -1;
}

void openfiles(char *database, char *query) {

	fdatabase = fopen(database, "r+");
	if (fdatabase == NULL) {
		perror(database);
		exit(EXIT_FAILURE);
	}

	fquery = fopen(query, "r");
	if (fquery == NULL) {
		perror(query);
		exit(EXIT_FAILURE);
	}

	fout = fopen("output/dna.out", "w");
	if (fout == NULL) {
		perror("fout");
		exit(EXIT_FAILURE);
	}

}

void closefiles() {
	fflush(fdatabase);
	fclose(fdatabase);

	fflush(fquery);
	fclose(fquery);

	fflush(fout);
	fclose(fout);
}

inline void remove_eol(char *line) {
	int i = strlen(line) - 1;
	while (line[i] == '\n' || line[i] == '\r') {
		line[i] = 0;
		i--;
	}
}

int get_next_query_descripton(char query_description[]) {
	fgets(query_description, 100, fquery);
	remove_eol(query_description);

	if(!feof(fquery))
		return 1;

	return 0;
}

void get_next_query_string(char *str) {
	char line[100];
	int i = 0;
	long prev_pos = ftell(fquery);

	fgets(line, 100, fquery);
	remove_eol(line);

	str[0] = 0;
	i = 0;

	do {
		strcat(str + i, line);

		prev_pos = ftell(fquery);
		if (fgets(line, 100, fquery) == NULL)
			break;
		remove_eol(line);
		i += 80;
	} while (line[0] != '>');

	fseek(fquery, prev_pos, SEEK_SET);
}

int get_next_base_description(char base_description[]) {
	fgets(base_description, 100, fdatabase);
	remove_eol(base_description);

	if(!feof(fdatabase))
		return 1;

	return 0;
}

void get_next_base_string(char *base) {
	char line[100];
	int i = 0;
	long prev_pos = ftell(fdatabase);

	base[0] = 0;
	i = 0;

	fgets(line, 100, fdatabase);
	remove_eol(line);

	do {
		strcat(base + i, line);

		prev_pos = ftell(fdatabase);
		if (fgets(line, 100, fdatabase) == NULL)
			break;
		remove_eol(line);
		i += 80;
	} while (line[0] != '>');

	fseek(fdatabase, prev_pos, SEEK_SET);
}

int main(int argc, char *argv[]) {

	if (argc != 3) {
		printf("Insufficient number of input files!\nUsage: %s <database file> <query file>\n\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	struct timeval start_computation, end_computation;
	struct timeval start_total, end_total;
	unsigned long int time_computation = 0, time_total;

	gettimeofday(&start_total, NULL);

	openfiles(argv[1], argv[2]);

	char *base = (char*) malloc(sizeof(char) * 1000001);
	if (base == NULL) {
		perror("malloc");
		exit(EXIT_FAILURE);
	}
	char *str = (char*) malloc(sizeof(char) * 1000001);
	if (str == NULL) {
		perror("malloc str");
		exit(EXIT_FAILURE);
	}

	char desc_dna[100], desc_query[100];
	int found, result;

	while (get_next_query_descripton(desc_query)) {
		fprintf(fout, "%s\n", desc_query);

		get_next_query_string(str);

		// read database and search
		found = 0;
		fseek(fdatabase, 0, SEEK_SET);
		while (get_next_base_description(desc_dna)) {

			get_next_base_string(base);

   		gettimeofday(&start_computation, NULL);
			result = bmhs(base, strlen(base), str, strlen(str));
			gettimeofday(&end_computation, NULL);
			time_computation += (end_computation.tv_sec - start_computation.tv_sec) * 1000000 + end_computation.tv_usec - start_computation.tv_usec;

			if (result > 0) {
				fprintf(fout, "%s\n%d\n", desc_dna, result);
				found++;
			}
		}

		if (!found)
			fprintf(fout, "NOT FOUND\n");
	}

	closefiles();

	free(str);
	free(base);

	gettimeofday(&end_total, NULL);
	time_total = (end_total.tv_sec - start_total.tv_sec) * 1000000 + end_total.tv_usec - start_total.tv_usec;

	printf("Elapsed computing time: %ld microseconds.\nI/O time: %ld microseconds\n", time_computation, time_total - time_computation);

	return EXIT_SUCCESS;
}
