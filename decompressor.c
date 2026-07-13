#include <stdio.h>
#include <ctype.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <inttypes.h>
#include <sys/stat.h>
#include <sys/mman.h>

typedef struct list
{
    unsigned char letter;
    uint32_t repetition;
    struct list *next;
} list;

typedef struct node
{
    unsigned char letter;
    uint32_t repetition;
    struct node *right;
    struct node *left;
} node;

typedef struct pair
{
    node *first;
    node *seconde;
} pair;

uint32_t node_count = 0;
long long byte_count = 0;
long long rep_length = 0;
long after_meta = 0;

void get_result(node *tree, unsigned char *payload, char *name, long long fsize);
const char *get_filename_ext(const char *filename);
void put_in_arr(list *l, int buffer, node **arr);
void reverse_arr(node **arr, int len);
void free_temp_list(list *temp_list);
pair find_two_smallest(node **arr);
node *build_huffman(node **arr);
void initialize_list(list *l);
void free_list(list *first);
void free_tree(node *t);
void print_all(list *l);

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        printf("Usage: ./decompressor compressed decompressed\n");
        return 1;
    }

    // Get the first file extension
    const char *first = get_filename_ext(argv[1]);
    const char *second = get_filename_ext(argv[2]);
    if (strcmp(first, second) != 0)
    {
        printf("extensions must match\n");
        return 1;
    }

    // read the metadata
    FILE *f = fopen(argv[1], "r");
    if (f == NULL)
    {
        printf("Error while opening compressed file\n");
        return 1;
    }

    // Put it in a char of size of one byte
    const int BYTELEN = 8;

    list *temp_list = malloc(sizeof(list));
    initialize_list(temp_list);

    // First byte is always the length
    fread(&node_count, sizeof(uint32_t), 1, f);
    printf("count: %" PRIu32 "\n", node_count);

    unsigned char letter;
    uint32_t rep = 0;

    for (uint32_t i = 0; i < node_count && fread(&letter, sizeof(unsigned char), 1, f) == 1 &&fread(&rep, sizeof(uint32_t), 1, f) == 1; i++)
    {
        // Put in full repetition
        rep_length += rep;

        // Malloc 
        list *temp = malloc(sizeof(list));

        // Put the values
        temp->letter = letter;
        temp->repetition = rep;
        printf("temp: %" PRIu32 "\n", rep);

        // Put temporary pnt to point to the front
        list *tmp_pnt = temp_list;

        // Put the head to the temp
        temp_list = temp;

        // Put the next to the old head
        temp_list->next = tmp_pnt;
    }

    // I cant put directly here since i can't know the count before
    node **meta_arr = malloc(sizeof(node *) * node_count);
    if (meta_arr == NULL)
    {
        printf("error while malloc\n");
        return 1;
    }

    // Malloc every index
    for (int i = 0; i < node_count; i++)
    {
        meta_arr[i] = malloc(sizeof(node));
    }
    
    // Put it in the meta_arr
    unsigned int b = 0;
    put_in_arr(temp_list, b, meta_arr);

    free_temp_list(temp_list);

    // Reverse array
    reverse_arr(meta_arr, node_count);
    printf("%" PRIu32 "\n", node_count);

    for (int i = 0; i < node_count; i++)
    {
        printf("%c:%i||", meta_arr[i]->letter, meta_arr[i]->repetition);
        byte_count += meta_arr[i]->repetition;
        after_meta = ftell(f);
    }
    printf("\n");

    // Build the three
    node *tree = build_huffman(meta_arr);

    if (tree == NULL)
    {
        printf("error while building\n");
        return 1;
    }

    fclose(f);

    // Now reopen the file in byte
    FILE *fb = fopen(argv[1], "rb");

    // Skip metadata
    fseek(fb, after_meta, SEEK_SET);
    long current = ftell(fb);

    // get file size
    fseek(fb, 0, SEEK_END);
    long long end = ftell(fb);
    fseek(fb, after_meta, SEEK_SET);

    int fd = fileno(fb);
    char *filedata = mmap(NULL, end, PROT_READ, MAP_PRIVATE, fd, 0);
    if (filedata == MAP_FAILED)
    {
        fclose(fb);
        printf("Map failed for filedata");
        return 1;
    }

    unsigned char *payload = (unsigned char *)(filedata + after_meta); // payload of [0] is the first byte after the metadata

    unsigned char byte = 0;

    fclose(fb);

    get_result(tree, payload, argv[2], (end - after_meta));
    printf("got\n");

    free(meta_arr);

    free_tree(tree);
    munmap(filedata, end);

    printf("Successfull\n");

    return 0;
}

void free_tree(node *t)
{
    if (t == NULL)
        return;

    if (t->left)
        free_tree(t->left);

    if (t->right)
        free_tree(t->right);

    free(t);
}

void free_temp_list(list *temp_list)
{
    if (temp_list->next != NULL)
    {
        free_temp_list(temp_list->next);
    }

    free(temp_list);
}

const char *get_filename_ext(const char *filename)
{
    const char *dot = strrchr(filename, '.');

    if (!dot || dot == filename)
        return "";

    return dot + 1;
}

void get_result(node *tree, unsigned char *payload, char *name, long long fsize)
{
    printf("opened\n");
    FILE *fw = fopen(name, "wb");
    if (fw == NULL)
    {
        printf("Opening write file failed");
        return;
    }

    node *current = tree;

    const int bytelen = 8;
    long long byte_read = 0;

    // For every nodes
    for (long long i = 0; byte_read < rep_length && i < fsize; i++)
    {
        // For every letters in the byte
        for (int j = 0; j < bytelen; j++)
        {
            if (byte_read >= rep_length)
                break;

            int bit = (payload[i] >> (7 - j)) & 1;

            if (bit == 0)
                current = current->left;

            else if (bit == 1)
                current = current->right;

            // if its a leaf or a 0 byte
            if (current->left == NULL && current->right == NULL)
            {
                fwrite(&current->letter, 1, 1, fw);

                byte_read++;
                current = tree; // reset
            }
        }
    }

    fclose(fw);
}

void reverse_arr(node **arr, int len)
{
    int middle = len / 2;

    for (int i = 0; i < middle; i++)
    {
        node *tmp = arr[i];
        arr[i] = arr[len - 1 - i];  
        arr[len - 1 - i] = tmp; 
    }
}

void print_all(list *l)
{
    if (l == NULL)
    {
        return;
    }

    if (l->letter != '\0')
    {
        // Print metadata
        printf("%c:%i||", l->letter, l->repetition);
    }

    if (l->next != NULL)
    {
        print_all(l->next);
    }
}

/// Build a huffman tree and return the top element
node *build_huffman(node **arr)
{
    while (1)
    {
        // Find the two smallest ones globally
        pair values = find_two_smallest(arr);

        // If there is no second node, the tree is fully built
        if (values.seconde == NULL)
        {
            break; 
        }
        
        int sum = values.first->repetition + values.seconde->repetition;

        // Create a new parent node
        node *parent = malloc(sizeof(node));
        parent->repetition = sum;
        parent->letter = '\0';

        parent->right = values.first;
        parent->left = values.seconde;

        int f_value = -1;
        int s_value = -1;

        // Find the matching indices in the array to update them
        for (int j = 0; j < node_count; j++)
        {
            if (arr[j] == values.first && f_value == -1)
            {
                f_value = j;
            }
            if (arr[j] == values.seconde && s_value == -1)
            {
                s_value = j;
            }
        }
        
        // Remove the second node and replace the first one with the parent
        arr[s_value] = NULL;
        arr[f_value] = parent;
    }

    // Find and return the single remaining root node
    for (int i = 0; i < node_count; i++)
    {
        if (arr[i] != NULL)
        {
            return arr[i];
        }
    }
    
    return NULL;
}

pair find_two_smallest(node **arr)
{
    pair results;
    
    results.first = NULL;
    results.seconde = NULL;

    for (int i = 0; i < node_count; i++)
    {
        // Check for null
        if (arr[i] == NULL)
        {
            continue;
        }

        if (results.first == NULL || arr[i]->repetition < results.first->repetition)
        {
            // Swap them and put value
            results.seconde = results.first;
            results.first = arr[i];
        }

        else if (results.seconde == NULL || arr[i]->repetition < results.seconde->repetition)
        {
            results.seconde = arr[i];
        }
    }

    return results;
}

void put_in_arr(list *l, int buffer, node **arr)
{
    // put in arr
    arr[buffer]->letter = l->letter;
    arr[buffer]->repetition = l->repetition;

    // initialize the other values
    arr[buffer]->right = NULL;
    arr[buffer]->left = NULL;

    // Incremente
    buffer++;
    
    // do next if not null
    if ((l->next != NULL) && (buffer < node_count))
    {
        put_in_arr(l->next, buffer, arr);
    }
}

void free_list(list *first)
{
    if (first->next != NULL)
    {
        free_list(first->next);
    }

    free(first);
}

void initialize_list(list *l)
{
    if (l != NULL)
    {
        l->letter = '\0';
        l->repetition = 0;
        l->next = NULL;
    }
}