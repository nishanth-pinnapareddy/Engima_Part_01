#include <stdio.h>
#include <math.h>
#include <gsl/gsl_statistics.h>
#include "engima.h"
#include "hashmap.h"

// English alpabet frequencies
static double english_frequencies[26] =
    {8.167, 1.492, 2.782, 4.253, 12.70, 2.228, 2.015, 6.094, 6.966, 0.153, 0.772, 4.025, 2.406,
     6.749, 7.507, 1.929, 0.095, 5.987, 6.327, 9.056, 2.758, 0.978, 2.360, 0.150, 1.974, 0.074};


struct Results {
    char setting[16];
    double score;
    char plainText[512];
};

typedef struct data_struct_p
{
    char key_string[5];
    float number;
} data_struct_q;

typedef struct data_struct_s
{
    char key_string[5];
    int number;
} data_struct_t;


void addToResult(char* setting, char* plainText, struct Results* results, double score, int keyCount, int limit);
double computeScore(char* plainText, char* word, map_t digraphFreqMap, int isWordGiven);
void setting(int numL, int numM, int numR, int reflect, int init_L, int init_M, int init_R, char* key);
double computeDigraphScore(char* plainText, map_t digraphFreqMap);
void loadDiGraphFrequencies(map_t digraphFreqMap);
void printResults(struct Results* results, int limit);

double digraph_freq[111] = {0.0};
double plainText_freq[111] = {0.0};

int main(int argc, const char *argv[])
{

    if(argc < 2)
    {
        oops:   fprintf(stderr, "\n\nUsage: %s infile possibleWord\n\n",
                        argv[0]);
        exit(0);
    }


    char *buffer = NULL;
    size_t size = 0;
    char infname[100];
    char word[100];
    int limit = 10;
    int isWordGiven = 0;
    sprintf(infname, "%s", argv[1]);
    if (argc == 3) {
        sprintf(word, "%s", argv[2]);
        isWordGiven = 1;
    }

    /* Open your_file in read-only mode */
    FILE* fp = fopen(infname, "r");
    if(fp == NULL)
    {
        fprintf(stderr, "\n\nError opening file %s\nTry again\n\n", infname);
        goto oops;
    }

    /* Get the buffer size */
    fseek(fp, 0, SEEK_END); /* Go to end of file */
    size = ftell(fp); /* How many bytes did we pass ? */

    /* Set position of stream to the beginning */
    rewind(fp);

    /* Allocate the buffer (no need to initialize it with calloc) */
    buffer = malloc((size + 1) * sizeof(*buffer)); /* size + 1 byte for the \0 */

    /* Read the file into the buffer */
    fread(buffer, size, 1, fp); /* Read 1 chunk of size bytes from fp into buffer */

    /* NULL-terminate the buffer */
    buffer[size] = '\0';

    /* Close the file */
    fclose(fp);


    map_t digraphFreqMap;
    digraphFreqMap = hashmap_new();
    loadDiGraphFrequencies(digraphFreqMap);
    const int rotors[5] = {0, 1, 2, 3, 4};
    const int letters[26] = {65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90};
    const int reflectors[2] = {66, 67};

    int l_rot, m_rot, r_rot, reflector;
    int init_l, init_m, init_r;
    int i,j,k,l,m,n,o;
    int keyCount=1;

    struct Results results[limit];
    char *plainText = malloc(512 * sizeof(char));
    char key[16];

    for (i=0; i<5; i++){
        l_rot = rotors[i];
        for (j=0; j<5; j++){
            if (i != j) {
                m_rot = rotors[j];
                for (k = 0; k < 5; k++) {
                    if (i != k && j != k) {
                        r_rot = rotors[k];
                        for (l = 0; l < 2; l++) {
                            reflector = reflectors[l];
                            for (m = 0; m < 26; m++) {
                                init_l = letters[m];
                                for (n = 0; n < 26; n++) {
                                    init_m = letters[n];
                                    for (o = 0; o < 26; o++) {
                                        init_r = letters[o];
                                        keyCount++;
                                        getEngimaresult(buffer, plainText, l_rot, m_rot, r_rot, reflector, init_l, init_m, init_r);
                                        double score = computeScore(plainText, word, digraphFreqMap, isWordGiven);
                                        setting(l_rot, m_rot, r_rot, reflector, init_l, init_m, init_r, key);
                                        addToResult(key, plainText, results, score, keyCount-1, limit);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    printResults(results, limit);
    free(buffer);
    return 0;
}

void printResults(struct Results* results, int limit){
    int  i;

    for (i=0; i<limit; i++){
        printf("\n %d. Score : %f, Key: %s", i+1, results[i].score, results[i].setting);
        printf("\n PlainText: %s", results[i].plainText);
        printf("\n ====================================\n");
    }
}


void addToResult(char* setting, char* plainText, struct Results* results, double score, int keyCount, int limit){
    int index,i,j;

    if (keyCount > limit) {
        if (results[0].score < fabs(score)) {
            results[0].score = fabs(score);
            strcpy(results[0].setting, setting);
            strcpy(results[0].plainText, plainText);

            struct Results temp;
            for (i = 1; i < limit; i++){
                for (j = 0; j < limit - i; j++) {
                    if (results[j].score > results[j + 1].score) {
                        temp = results[j];
                        results[j] = results[j + 1];
                        results[j + 1] = temp;
                    }
                }
            }
        }
    }
    else {
        results[keyCount-1].score = fabs(score);
        strcpy(results[keyCount-1].setting, setting);
        strcpy(results[keyCount-1].plainText, plainText);
        if (keyCount == limit) {
            struct Results temp;
            for (i = 1; i < limit; i++){
                for (j = 0; j < limit - i; j++) {
                    if (results[j].score > results[j + 1].score) {
                        temp = results[j];
                        results[j] = results[j + 1];
                        results[j + 1] = temp;
                    }
                }
            }
        }
    }
}

double computeScore(char* plainText, char* word, map_t digraphFreqMap, int isWordGiven){
    double frequencies[26] = {0};

    int index = 0;
    while (1){
        char  c = plainText[index++];
        if (c == EOF || c== '\0'){
            break;
        }

        frequencies[c-'A']++;
    }

    double score = fabs(gsl_stats_correlation(english_frequencies, 1, frequencies, 1, 26));
    score += computeDigraphScore(plainText,digraphFreqMap);

    if (isWordGiven == 1) {
        char *pch = strstr(plainText, word);
        if (pch != NULL)
            score += 1.0;
    }

   return score;
}

void setting(int numL, int numM, int numR, int reflect, int init_L, int init_M, int init_R, char * key){
    int index =0;

    key[index] = (char)numL + '0';
    key[++index] = (char)numM + '0';
    key[++index] = (char)numR + '0';

    key[++index] = ' ';

    key[++index] = (char)reflect;

    key[++index] = ' ';
    key[++index] = (char)init_L;
    key[++index] = (char)init_M;
    key[++index] = (char)init_R;

    key[++index] = '\0';

    return;
}

double computeDigraphScore(char* plainText, map_t digraphFreqMap){
    int index=0;
    int error;
    map_t plainTextmap;
    data_struct_t* value;
    char key[3];

    plainTextmap = hashmap_new();
    while (1){
        if (plainText[index] == '\0' || plainText[index+1] == '\0')
            break;

        key[0] = plainText[index];
        key[1] = plainText[index+1];
        key[2] = '\0';

        error = hashmap_get(plainTextmap, key, (void**)(&value));
        if (error == MAP_OK){
            value->number  = value->number +1;
        } else{
            value = malloc(sizeof(data_struct_t));
            sprintf(value->key_string, "%s", key);
            value->number = 1;
            hashmap_put(plainTextmap, value->key_string, value);
        }

        index++;
    }

    data_struct_q *value2;
    int i = 0;

    while (1){
        if (plainText[i] == '\0' || plainText[i+1] == '\0')
            break;

        key[0] = plainText[i];
        key[1] = plainText[i+1];
        key[2] = '\0';

        error = hashmap_get(plainTextmap, key, (void**)(&value));
        if (error == MAP_OK) {
            plainText_freq[i] = value->number;
            error = hashmap_remove(plainTextmap, key);
            free(value);
        }
        else{
            plainText_freq[i] = 0.0;
        }

        error = hashmap_get(digraphFreqMap, key, (void**)(&value2));
        if (error == MAP_OK)
            digraph_freq[i] = value2->number;
        else
            digraph_freq[i] = 0.0;

        i++;
    }

    hashmap_free(plainTextmap);

    double score = fabs(gsl_stats_correlation(digraph_freq, 1, plainText_freq, 1, i));

    return score;
}

void loadDiGraphFrequencies(map_t digraphFreqMap){

    FILE* in;
    char fileName[100];
    char line[100];
    char key[3];
    char value[12];
    data_struct_q *valuept;


    sprintf(fileName, "%s", "digraph.txt");
    in = fopen("digraph.txt", "r");

    while (fgets(line, sizeof(line), in)) {
        key[0] = line[0];
        key[1] = line[1];
        key[2] = '\0';

        int index = 3;
        int valIndex = 0;

        while(1){
            if (line[index] == EOF || line[index] == '\r')
                break;

            value[valIndex++] = line[index++];
        }
        value[valIndex] ='\0';

        float freq = atof(value);

        //printf("\n Key : %s, Value : %.2f", key, freq);
        valuept = malloc(sizeof(data_struct_q));
        sprintf(valuept->key_string, "%s", key);
        valuept->number = freq;
        hashmap_put(digraphFreqMap, valuept->key_string, valuept);
    }
    printf("\n Load Digraph Frequencies in to hash Map");
}

