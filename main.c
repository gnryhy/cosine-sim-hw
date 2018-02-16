#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>
#include <math.h>

typedef struct Node{                    /* linked list node */
    
    int id;
    char* data;
    int* count_array;
    int total_count;
    struct Node* next;
} node_t;

/* inserts a node into the linked list */
void insert_node(node_t **head, char* data, int file_count, int file_id, int node_id, int data_len){
    
    node_t *new_node = malloc(sizeof(node_t));
    if (!new_node) return;

    new_node->data = malloc(sizeof(data));
    strncpy(new_node->data, data, sizeof(char) * data_len);
    
    new_node->id = node_id;
    
    new_node->count_array = calloc(file_count, sizeof(int));
    new_node->count_array[file_id]++;
    new_node->total_count = 1;
    
    new_node->next = *head;
    *head = new_node;
}

void free_list(node_t** head){              /* frees the linked list from the memory */
   
    node_t* tmp = malloc(sizeof(node_t));

    while (*head != NULL){
       
        tmp = *head;
        *head = tmp->next;
        free(tmp);
    }
}

int searchTerm(node_t **head, char* needle){            /* searchs an user-given term and returns its location in the list */
   
    node_t *haystack = malloc(sizeof(node_t));
    haystack = *head;
    int ret_val = -1;
    
   
    while(haystack != NULL){
    
        if(strcmp(needle, haystack->data) == 0){
            ret_val = haystack->id;
            break;
        }
        
        if(haystack->next == NULL)
            break;
    
        haystack = haystack->next;
    }
    
    return ret_val;
}

static int compare (const void * a, const void * b){
    
    return strcmp (*(const char **) a, *(const char **) b);
}

int enlistFiles(char** files){          /* finds the .txt files on working directory */

    DIR *dir;
    struct dirent *entry;     
    dir = opendir ("./");
    int i = 0;

    if (dir != NULL){
            
               
        while ((entry = readdir (dir))!=NULL){
            int length = strlen(entry->d_name);
            if (strncmp(entry->d_name + length - 4, ".txt", 4) == 0) {
                strncpy(files[i], entry->d_name, strlen(entry->d_name));
                i++;
            }    
        }

        (void)closedir (dir);
    }
    
    return i;
}

/* calculates similarity between files and prints on the console */
void calculateSimilarity(int terms_max, int files_max, int term_matrix[terms_max][files_max], char** file_names){

    int term_id = 0, left_file_id = 0, right_file_id = 0;
    

    int i, j;
    
    for(left_file_id = 0; left_file_id < files_max - 1; left_file_id++){     /* last file isn't needed to be compared so <<files_max - 1>> */
        for(right_file_id = left_file_id + 1; right_file_id < files_max; right_file_id++){
            double dot_product = 0;
            double magnitude_product = 0;
            double left_magnitude = 0;
            double right_magnitude = 0;
            
            for(term_id = 0; term_id < terms_max; term_id++){

                dot_product += term_matrix[term_id][left_file_id] * term_matrix[term_id][right_file_id];
                left_magnitude += term_matrix[term_id][left_file_id] * term_matrix[term_id][left_file_id];
                right_magnitude += term_matrix[term_id][right_file_id] * term_matrix[term_id][right_file_id];
            }
            magnitude_product = sqrt(left_magnitude) * sqrt(right_magnitude);
            if(magnitude_product == 0)
                printf("Cosine Similarity between %s and %s = 0\n", file_names[left_file_id], file_names[right_file_id]);
            else
                printf("Cosine Similarity between %s and %s = %.3f\n", file_names[left_file_id], file_names[right_file_id], dot_product / magnitude_product);
        }
    }
}

/*
 * 
 */
int main(int argc, char** argv) {
    
    char** files = malloc(10 * sizeof(char*)); /* up to 10 files */
    int i;  /* general purpose counter */
    
    /* memory allocation for file names */
    for(i = 0; i < 10; i++){
        files[i] = malloc(FILENAME_MAX * sizeof(char));
    }
       
    int files_num = enlistFiles(files);   /* holds the number of files in the directory */
    qsort(files, files_num, sizeof(const char *), compare); /* sort file names alphabetically */
         
    int id = 0; /* id counter for the list */
    
    node_t* first_node = malloc(sizeof(node_t));    /* create address to the list */
    first_node = NULL;                              /* null initialization */
        
    /* extracting terms file by file */
    for(i = 0; i < files_num; i++){
        
        FILE *fp;
        fp = fopen(files[i], "r");
        char* line = malloc(8192 * sizeof(char));   /* assuming a line of the test files can be 8KB at max */
        size_t len;                                 /* keeps size of each line of file */
        
        /* read line by line */
        while ((getline(&line, &len, fp)) != -1) {
            
            int j;  /* another general purpose counter */
            for(j = 0; line[j]; j++){       /* replace */
                if(!isalpha(line[j]))       /* non alphabetical characters*/
                    line[j] = ' ';          /* with whitespaces */
            }
            
            for(j = 0; line[j]; j++){
                line[j] = tolower(line[j]); /* convert uppercase to lowercase */
            }
            
            const char delim[2] = " ";      /* delimiter */
            
            char *token;
            token = strtok(line, delim);    
            /* split string by whitespaces */
            while(token != NULL){
               
                int added_before = searchTerm(&first_node, token); /* is greater than zero if added before */ 
                
                if(added_before == -1){ /* adding the term for the first time */
                
                    insert_node(&first_node, token, files_num, i, id, strlen(token));
                    id++;
                }else{                  /* change values of previously added node */
                
                    node_t *temp = malloc(sizeof(node_t));
                    temp = first_node;
                
                    while(temp != NULL){
                        if(temp->id == added_before){
                            temp->count_array[i]++;
                            temp->total_count++;
                            break;
                        }
                        temp = temp->next;
                    }
                
                }
                
                token = strtok(NULL, delim);
            }
                             
        }
        
        free(line);
        fclose(fp);
    }
    /* end of extracting terms file by file */
    
    
    node_t *temp = malloc(sizeof(node_t));
    temp = first_node;
    
    int valid_terms = 0;    
    while(temp != NULL){                                                        /* look for */
                                                                                /* the number of*/
        if(((temp->total_count >= files_num) && (2000 > temp->total_count))){     /* valid terms */    
            valid_terms++;
        }
    
        temp = temp->next;
    }
    
    int j;
    int term_matrix[valid_terms][files_num];    /* create term matrix */
    
    i = 0;
    j = 0;
    
    temp = first_node;
    
    while(temp != NULL){
        if(((temp->total_count >= files_num) && (2000 > temp->total_count))){     /* fetch the data */
            for(j = 0; j < files_num; j++)                                      /* from valid terms */
                term_matrix[i][j] = temp->count_array[j];                       /* into the term matrix */
            i++;
        }
        temp = temp->next;
    }
    
    free_list(&first_node);     /* free the linked list from memory */
    
    /* remove extension from file name */
    for(i = 0; i < files_num; i++)
        files[i][strlen(files[i])-4] = 0;
    
    /* similarity calculation */
    calculateSimilarity(valid_terms, files_num, term_matrix, files);
        
    return (EXIT_SUCCESS);
}