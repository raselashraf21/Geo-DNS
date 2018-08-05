
#include <stdio.h>
#include <inttypes.h>   /*for MACRO PRIu8*/
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <config.h>
#include "create_iso_file.h"


__uint8_t create_node() {


    char *file_path = DEFAULT_PATH_READ_ISO;
    FILE *file_pointer = fopen(file_path, "r");

    if(file_pointer == NULL) {
        printf("error opening file\n");
        return 0;
    }
    char *country_iso = (char*) malloc(sizeof(char) * 4);

    __uint8_t count = 0;
    while(fgets(country_iso, 4, file_pointer)) {
        country_iso = strtok(country_iso, "\n");

        iso_node *temp = (iso_node *) malloc(sizeof(iso_node));
        //temp -> group_id = (__uint8_t *) malloc(sizeof(__uint8_t));
        temp -> group_id = (__uint8_t*) calloc(_MAX, sizeof(__uint8_t));
        temp -> iso = strdup(country_iso);
        temp -> next = NULL;
        if(head_ptr == NULL) {
            head_ptr = temp;
            counter = temp;
        }
        else {
            counter -> next = temp;
            counter = temp;
        }
        count++;
    }
    fclose(file_pointer);
    return count;
}

int search_and_set_iso_node(iso_node *head_ptr, char *token, int branch_id){
    iso_node *temp = head_ptr;
    int index = 0;
    while(temp != NULL) {
        if(strcmp(temp -> iso, token) == 0) {
            temp -> group_id[branch_id] = true;
            return index;
        }
        temp = temp -> next;
        index++;
    }
    return index;
}


int reset_iso_group(iso_node *head_ptr, int iso_count) {

    char *file_path = DEFAULT_PATH_INPUT_ISO;
    FILE *file_pointer = fopen(file_path, "r");

    int max_group_no = 0;
    if(file_pointer == NULL) {
        return -1;
    }
    char str[1000];
    fgets(str, 100, file_pointer);
    int set_count = atoi(str);
    while (set_count) {
        int ciso = 0;
        fgets(str, 60, file_pointer);
        int branch_id_count = atoi(str);
        //printf("%d\n", branch_id_count);
        branch_id_count--;
        uint8_t *visited = (uint8_t*) calloc(iso_count, sizeof(uint8_t));
        while(branch_id_count) {
            //printf("%d\n", branch_id_count);
            fgets(str, 1000, file_pointer);
            char *rest_part;
            long int branch_id = strtol(str, &rest_part, 10);
            if(branch_id > max_group_no) {
                max_group_no = branch_id;
            }
            //printf("%ld\n", branch_id);
            char *token = " ";
            token = strtok(rest_part, token);
            token[2] = '\0';
            //printf("%s --> %d\n", token, branch_id);
            if((ciso = search_and_set_iso_node(head_ptr, token, branch_id)) < 0) {
                printf("%s no such iso\n", token);
            }
            visited[ciso] = true;
            while(token != NULL) {
                token = strtok(NULL, " ");
                if(token != NULL) {
                    token[2] = '\0';
                    //printf("%s --> %d\n", token, branch_id);
                    if((ciso = search_and_set_iso_node(head_ptr, token, branch_id)) < 0) {  		//setting branch_id of the iso_node(if exists) to true
                        printf("%s no such iso\n", token);
                    }
                    visited[ciso] = true;
                }
            }
            branch_id_count--;
        }

        fgets(str, 1000, file_pointer);
        char *rest_part;
        long int branch_id = strtol(str, &rest_part, 10);
        if(branch_id > max_group_no) {
            max_group_no = branch_id;
        }
        iso_node *temp = head_ptr;
        int index = 0;
        while (temp != NULL) {
            if(visited[index] == 0) {
                temp -> group_id[branch_id] = true;
            }
            temp = temp -> next;
            visited[index++] = true;
        }
        set_count--;
    }

    fclose(file_pointer);

    return max_group_no;
}


void print_node(iso_node *head_ptr, __uint8_t group_count, __uint8_t iso_count) {
    iso_node * temp = head_ptr;
    int bytes_used = 0;

    char *file_path = DEFAULT_PATH_COUNTRY_ISO;
    FILE *file_pointer = fopen(file_path, "w");
    while (temp != NULL) {
        //printf("%" PRIu8 " %" PRIu8 "\n", temp -> group_id[0], temp -> group_id[249]);
        //printf("\n%s", temp -> iso);
        fprintf(file_pointer, "%s", temp -> iso);
        //printf("%d\n", sizeof(temp) * CHAR_BIT);    //checking total number of bits in one node
        //printf("%d\n\n", sizeof(temp));
        int i;
        for(i = 0; i < group_count; i++) {
            if( temp -> group_id[i] ) {
                //printf(" %d", i);
                fprintf(file_pointer, ".%d", i);
            }
        }
        if(--iso_count)fprintf(file_pointer, "\n");
        bytes_used+= sizeof(temp);
        temp = temp -> next;
    }
    printf("total memory space %d bytes\n", bytes_used);
    fclose(file_pointer);
}


int create_iso_file() {
//    clock_t begin = clock();

    __uint8_t iso_count = create_node();
    __uint8_t group_count;

    //printf();
    //printf("%"PRIu8" \n", count);
    //printf("head_ptr --> %x %x\n", &head_ptr, head_ptr + sizeof(head_ptr));
    //print_node(head_ptr);

    if((group_count = reset_iso_group(head_ptr, iso_count)) < 0) {
        printf("error opening input file\n");
    }
    else {
        print_node(head_ptr, group_count + 1, iso_count);
    }
    //int *ptr = &head_ptr;
    //ptr = ptr + 0x24;
    //print_node(*ptr);
//    printf("head_ptr --> %d %x\n", ptr, head_ptr);
//    int *cgroups = (int*) malloc(sizeof(int) * _MAX);
//    char *country_iso = "AG";
//    int iso_group_count = get_country_groups(country_iso, cgroups, head_ptr);
//    int i;
////    for(i = 0; i < iso_group_count; i++) {
////        printf("%d ", cgroups[i]);
////    }
//    printf("\n");
//    free(cgroups);
//    clock_t end = clock();
    //printf("\nrun time %lf seconds\n", (double) (end - begin) / CLOCKS_PER_SEC);
    //printf("\nhead_ptr size %d\n", sizeof(head_ptr));
    return 0;
}