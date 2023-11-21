//NAME : Sanghyeon Park 
//ID : tkdgus2916

#include "cachelab.h"
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <argp.h>

typedef unsigned long addr_size;


typedef struct {
	int hits;
	int misses;
	int evictions;
}result;

typedef struct Elem {
    bool valid;
    addr_size tag_val;
    struct Elem* prev;
    struct Elem* next;
} Elem;

typedef struct Cache {
    int capacity;
    int size;
    Elem* head;
    Elem* tail;
} Cache;



bool verbose_on = false;
int set_index_num;
int associativity;
int block_num;
char *file_name;
result res = {0, 0, 0};


/* Argument Pasrser */
void print_options(){
	printf(
			"Usage: ./csim [-hv] -s <num> -E <num> -b <num> -t <file>\n"
			"Options:\n"
			"  -h         Print this help message.\n"
			"  -v         Optional verbose flag.\n"
			"  -s <num>   Number of set index bits.\n"
			"  -E <num>   Number of lines per set.\n"
			"  -b <num>   Number of block offset bits.\n"
			"  -t <file>  Trace file.\n\n"
			"Examples:\n"
			"  linux>  ./csim -s 4 -E 1 -b 4 -t traces/yi.trace\n"
			"  linux>  ./csim -v -s 8 -E 2 -b 4 -t traces/yi.trace\n");
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

/*Bit API*/
addr_size get_mask_bit(int num){
	addr_size mask = 0xFFFFFFFFFFFFFFFF;
	return ~(mask << num);
}


addr_size get_set_bit(addr_size addr){
	addr >>= block_num;
	return addr & get_mask_bit(set_index_num);
}

unsigned get_tag_bit(addr_size addr){
	unsigned tag_idx = block_num + set_index_num;
	addr >>= tag_idx;
	return addr & get_mask_bit(64 - tag_idx);
}



// Cache line & elem API
Elem* create_elem(addr_size tag_val) {
    Elem* newelem = (Elem*)malloc(sizeof(Elem));
    newelem->tag_val = tag_val;
    newelem->prev = NULL;
    newelem->next = NULL;

    return newelem;
}

Cache* init_cache_line(int capacity) {
    Cache* cache = (Cache*)malloc(sizeof(Cache));
    cache->capacity = capacity;
    cache->size = 0;
    cache->head = NULL;
    cache->tail = NULL;

    return cache;
}

bool find_elem(Cache* cache, addr_size tag_val) {

	//Find from the first element
	Elem *current = cache->head;

	// Search the matching elem in list
	while (current != NULL) {
		if (current->tag_val == tag_val) {
			// When find the matching elem, then move to front
			if (current != cache->head) {
				if (current == cache->tail) {
					cache->tail = current->prev;
					cache->tail->next = NULL;
				} else {
					current->prev->next = current->next;
					current->next->prev = current->prev;
				}
				current->prev = NULL;
				current->next = cache->head;
				cache->head->prev = current;
				cache->head = current;
			}
			
			if(verbose_on)
				printf(" hit");
			res.hits++;

			return true;
		}
		current = current->next;
	}

	if(verbose_on)
		printf(" miss");
	res.misses++;

	return false;
}

void add_elem(Cache* cache, addr_size tag_val) {
    if (cache == NULL) exit(1);

	if(find_elem(cache, tag_val))
		return;


    Elem* newelem = create_elem(tag_val);
    newelem->next = cache->head;


    if (cache->head != NULL) {
        cache->head->prev = newelem;
    }

    cache->head = newelem;

	if (cache->tail == NULL) {
		cache->tail = newelem;
	}

    // When cache is full, evict the old cache element
    if (cache->size == cache->capacity) {

		if(verbose_on)
			printf(" eviction");
		res.evictions++;

        Elem* temp = cache->tail;
        cache->tail = cache->tail->prev;
        cache->tail->next = NULL;
        free(temp);

        cache->size--;
    }

    cache->size++;
}

void printCache(Cache* cache) {
    printf("LRU Cache: ");
    Elem* current = cache->head;
    while (current != NULL) {
        printf("(%lx) ", current->tag_val);
        current = current->next;
    }
    printf("\n");
}

void free_cache_line(Cache* cache) {

    Elem* current = cache->head;
    while (current != NULL) {
        Elem* temp = current;
        current = current->next;
        free(temp);
    }
    free(cache);
}

/* Cache API*/
Cache **init_cache(){
	Cache **cache = (Cache **)malloc((1<<set_index_num) * sizeof(Cache *));
	if(cache == NULL)
		return NULL;

	for (int i = 0; i < (1<<set_index_num); i++) {
        cache[i] = init_cache_line(associativity);
    }
	return cache;
}

void free_cache(Cache **cache){
	for (int i = 0; i < set_index_num; ++i) {
		free_cache_line(cache[i]);
    }
    free(cache);
}

void cache_store(Cache **cache, addr_size addr, int size){
	addr_size set_bit = get_set_bit(addr);
	addr_size tag_bit = get_tag_bit(addr);

	add_elem(cache[set_bit], tag_bit);
}

void cache_load(Cache **cache, addr_size addr, int size){
	addr_size set_bit = get_set_bit(addr);
	addr_size tag_bit = get_tag_bit(addr);

	add_elem(cache[set_bit], tag_bit);
}



int main(int argc, char *argv[])
{
	Cache **cache;

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

		if(verbose_on)
			printf("%c %lx,%d", type, addr, size);
		

		if(type == 'I')
			continue;
		else if(type == 'S')
			cache_store(cache, addr, size);
		else if(type == 'L')
			cache_load(cache, addr, size);
		else{
			cache_load(cache, addr, size);
			cache_store(cache, addr, size);
		}

		if(verbose_on)
			printf("\n");

    }

    fclose(file);  // 파일 닫기

    printSummary(res.hits, res.misses, res.evictions);

	free_cache(cache);
		
    return 0;
}
