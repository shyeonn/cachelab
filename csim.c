//NAME : Sanghyeon Park 
//ID : tkdgus2916

#include "cachelab.h"
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <argp.h>

typedef struct {
	bool valid;
	long tag_val;
}line;

typedef struct {
	int hits;
	int misses;
	int evictions;
}result;


bool verbose_on = false;
int set_index_num;
int associativity;
int block_num;
char *file_name;
result res = {0, 0, 1};


void print_options(){
	printf(
			"Usage: ./csim-ref [-hv] -s <num> -E <num> -b <num> -t <file>\n"
			"Options:\n"
			"  -h         Print this help message.\n"
			"  -v         Optional verbose flag.\n"
			"  -s <num>   Number of set index bits.\n"
			"  -E <num>   Number of lines per set.\n"
			"  -b <num>   Number of block offset bits.\n"
			"  -t <file>  Trace file.\n\n"
			"Examples:\n"
			"  linux>  ./csim-ref -s 4 -E 1 -b 4 -t traces/yi.trace\n"
			"  linux>  ./csim-ref -v -s 8 -E 2 -b 4 -t traces/yi.trace\n");
}

void arg_parser(int argc, char *argv[]){
	if(argc < 2){
		printf("%s : Missing required command line argument\n", argv[0]);
		print_options();
        exit(EXIT_FAILURE);
	}

	int arg_count = 0;
    size_t optind;

    for (optind = 1; optind < argc && argv[optind][0] == '-'; optind++) {
		switch (argv[optind][1]) {
			case 'h': print_options(); exit(EXIT_FAILURE);
			case 'v': verbose_on = true; break;
			case 's': set_index_num = atoi(argv[++optind]); arg_count++; break;
			case 'E': associativity = atoi(argv[++optind]); arg_count++; break;
			case 'b': block_num = atoi(argv[++optind]); arg_count++; break;
			case 't': file_name = argv[++optind]; arg_count++; break;
			default:
				printf("%s : invalid option -- '%c'\n", argv[0], argv[optind][1]);
				print_options();
				exit(EXIT_FAILURE);
			}   
		}
		if(arg_count != 4){
			printf("%s : Missing required command line argument\n", argv[0]);
			print_options();
            exit(EXIT_FAILURE);
    }
    argv += optind;
}


line **init_cache(){
	//Allocate memory for line array
	line **cache = (line **)malloc(set_index_num * sizeof(line *));
	if(cache == NULL)
		return NULL;

	for (int i = 0; i < set_index_num; i++) {
        cache[i] = (line *)malloc(associativity * sizeof(line));
        if (cache[i] == NULL) {
            return NULL;
        }

		//Initial the valid bit value 
		for(int j = 0; j < associativity; j++) {
			cache[i][j].valid = false;
			//cache[i][j].tag_val = 0;
		}
    }
	return cache;
}

void free_cache(line **cache){
	for (int i = 0; i < associativity; ++i) {
        free(cache[i]);
    }
    free(cache);
}

void cache_store(line **cache, long addr, int size){


}

void cache_load(line **cache, long addr, int size){


}



int main(int argc, char *argv[])
{
	line **cache;

	//Parsed data
	char type;
	long addr;
	int size;

	//Get the argument
	arg_parser(argc, argv);
	
	//Check NULL pointer
	if(!(cache = init_cache())){
		printf("Can not allocate memory\n");
		return 1;
	}

	
	//Open file
    FILE *file = fopen(file_name, "r");
    if (file == NULL) {
		printf("File does not exist\n");
        return 1;
    }

    
	//Read line
    while (fscanf(file, " %c", &type) == 1) {

        fscanf(file, " %lx,%d", &addr, &size);

		if(type == 'I')
			continue;
		else if(type == 'S')
			cache_store(cache, addr, size);
		else if(type == 'L')
			cache_load(cache, addr, size);


		printf("%c %lx %d\n", type, addr, size);
    }

    fclose(file);  // 파일 닫기

    printSummary(res.hits, res.misses, res.evictions);

	free_cache(cache);
		
    return 0;
}