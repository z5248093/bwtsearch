#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

struct OccEntry {
    char ascii;
    int value;
};

struct Occ {
    struct OccEntry* entries;
    int size;
};

void initializeOcc(struct Occ* occ, int size) {
    occ->entries = (struct OccEntry*)malloc(size * sizeof(struct OccEntry));
    occ->size = 0;
}

void add_occ(struct Occ* occ, char ascii, int value) {
    occ->entries[occ->size].ascii = ascii;
    occ->entries[occ->size].value = value;
    occ->size++;
}

struct DictionaryEntry {
    int value;
};

struct Dictionary {
    struct DictionaryEntry** entries;
    int rows;
    int cols;
};

void initializeDictionary(struct Dictionary* dictionary, int cols) {
    dictionary->entries = NULL;
    dictionary->rows = 0;
    dictionary->cols = cols;
}

void add_count(struct Dictionary* dictionary, int *ascii) {
    dictionary->entries = (struct DictionaryEntry**)realloc(dictionary->entries, (dictionary->rows + 1) * sizeof(struct DictionaryEntry*));
    dictionary->entries[dictionary->rows] = (struct DictionaryEntry*)malloc(dictionary->cols * sizeof(struct DictionaryEntry));
    for (int i = 0; i < dictionary->cols; i++) {
        dictionary->entries[dictionary->rows][i].value = ascii[i];
    }
    dictionary->rows++;
}

char* rlb_to_bwt(char* line, struct Dictionary* dictionary, int block) {
    char* bwt_text = NULL;
    int length = 0;
    int i, j;
    char cur, next;
    int char_len;
    int count;
    int count_block = 0;
    int ascii[118];
    int line_length = strlen(line);
    for (i = 0; i < 118; i++) {
        ascii[i] = 0;
    }
    
    for (i = 0; i < line_length; i++) {
        //printf("%d\n", i);
        cur = line[i];
        next = line[i + 1];
        if (next < 0) {
            char_len = next + 128;
            i++;
            next = line[i + 1];
            count = 1;
            while (next < 0) {
                char_len += (next + 128) << (7 * count);
                i++;
                next = line[i + 1];
                count++;
            }
            bwt_text = realloc(bwt_text, (length + char_len + 4) * sizeof(char));
            for (j = 0; j < char_len + 3; j++) {
                bwt_text[length + j] = cur;
                ascii[cur - 9]++;
                count_block++;
                if (count_block == block) {
                    add_count(dictionary, ascii);
                    count_block = 0;
                }
            }
            length += char_len + 3;
        } else {
            bwt_text = realloc(bwt_text, (length + 2) * sizeof(char));
            bwt_text[length] = cur;
            ascii[cur - 9]++;
            length++;
            count_block++;
            if (count_block == block) {
                add_count(dictionary, ascii);
                count_block = 0;
            }
        }
    }
    bwt_text[length] = '\0';
    return bwt_text;
}
    
void reverseString(char* str) {
    int length = strlen(str);
    int i, j;
    for (i = 0, j = length - 1; i < j; i++, j--) {
        // Swap characters
        char temp = str[i];
        str[i] = str[j];
        str[j] = temp;
    }
}

void inverse_bwt(struct Dictionary* dictionary, struct Occ* occ, char* bwt, char* to_search, int block) {
    char *result = NULL;
    int *number = NULL;
    int point;
    int i, j, m;
    int from = 0;
    int to = 0;
    int count_from = 0;
    int count_to = 0;
    int length = strlen(bwt);
    int to_add; 
    int ascii[118];
    int *bracket_seq = NULL;
    int result_num = 0;
    char *tmp = NULL;
    int first_bracket = 0;
    char *first_num = malloc(10 * sizeof(char));
    int first_num_length;
    int to_search_length = strlen(to_search);
    for (m = 0; m < 1000000000; m++) {
        sprintf(first_num, "%d", m);
        for (i = 0; i < occ->size; i++) {
            if (occ->entries[i].ascii == ']') {
                from = occ->entries[i].value;
                if (i + 1 >= occ->size) {
                    to = length - 1;
                } else {
                    to = occ->entries[i + 1].value - 1;
                }
                break;
            }
        }
        first_num_length = strlen(first_num);
        j = 1;
        while ((first_num_length - j) != -1) {
            
            to_add = (from - 1) / block;
            if (to_add == 0) {
                for (i = 0; i < 118; i++) {
                    ascii[i] = 0;
                }
            } else {
                for (i = 0; i < dictionary->cols; i++) {
                    ascii[i] = dictionary->entries[to_add - 1][i].value;
                }
            }
            for (i = to_add * block; i < to + 1; i++) {
                ascii[bwt[i] - 9]++;      
                if (i == from - 1) {
                    count_from = ascii[first_num[first_num_length - j] - 9];
                }
            }
            count_to = ascii[first_num[first_num_length - j] - 9];

            for (i = 0; i < occ->size; i++) {
                if (occ->entries[i].ascii == first_num[first_num_length - j]) {
                    from = occ->entries[i].value + count_from;
                    to = from + count_to - count_from - 1;
                    break;
                }
            }
            if (from > to) break;
            j++;
        }
        if (from > to) continue;
        to_add = (from - 1) / block;
        if (to_add == 0) {
            for (i = 0; i < 118; i++) {
                ascii[i] = 0;
            }
        } else {
            for (i = 0; i < dictionary->cols; i++) {
                ascii[i] = dictionary->entries[to_add - 1][i].value;
            }
        }
        for (i = to_add * block; i < to + 1; i++) {
            ascii[bwt[i] - 9]++;      
            if (i == from - 1) {
                count_from = ascii['[' - 9];
            }
        }
        count_to = ascii['[' - 9];

        for (i = 0; i < occ->size; i++) {
            if (occ->entries[i].ascii == '[') {
                from = occ->entries[i].value + count_from;
                to = from + count_to - count_from - 1;
                break;
            }
        }

        if (from == to) {
            first_bracket = from;
            break;
        }
    }
    free(first_num);

    for (i = 0; i < occ->size; i++) {
        if (occ->entries[i].ascii == to_search[to_search_length - 1]) {

            from = occ->entries[i].value;
            if (i + 1 >= occ->size) {
                to = length - 1;
            } else {
                to = occ->entries[i + 1].value - 1;
            }

            break;
        }
    }

    j = 2;
    while ((to_search_length - j) != -1) {
        
        to_add = (from - 1) / block;
        if (to_add == 0) {
            for (i = 0; i < 118; i++) {
                ascii[i] = 0;
            }
        } else {
            for (i = 0; i < dictionary->cols; i++) {
                ascii[i] = dictionary->entries[to_add - 1][i].value;
            }
        }
        for (i = to_add * block; i < to + 1; i++) {
            ascii[bwt[i] - 9]++;      
            if (i == from - 1) {
                count_from = ascii[to_search[to_search_length - j] - 9];
            }
        }
        count_to = ascii[to_search[to_search_length - j] - 9];

        for (i = 0; i < occ->size; i++) {
            if (occ->entries[i].ascii == to_search[to_search_length - j]) {
                from = occ->entries[i].value + count_from;
                to = from + count_to - count_from - 1;
                break;
            }
        }
        
        j++;
    }

    int digit;
    for (j = from; j <= to; j++) {
        point = j;
        while (bwt[point] != ']') {
            to_add = point / block;
            if (to_add == 0) {
                for (i = 0; i < 118; i++) {
                    ascii[i] = 0;
                }
            } else {
                for (i = 0; i < dictionary->cols; i++) {
                    ascii[i] = dictionary->entries[to_add - 1][i].value;
                }
            }
            for (i = to_add * block; i < point + 1; i++) {
                ascii[bwt[i] - 9]++;
            }
            for (i = 0; i < occ->size; i++) {
                if (occ->entries[i].ascii == bwt[point]) {
                    point = occ->entries[i].value + ascii[bwt[point] - 9] - 1;
                    
                    break;
                }
            }
        }
        digit = 0;
        while (bwt[point] != '[') {
            to_add = point / block;
            if (to_add == 0) {
                for (i = 0; i < 118; i++) {
                    ascii[i] = 0;
                }
            } else {
                for (i = 0; i < dictionary->cols; i++) {
                    ascii[i] = dictionary->entries[to_add - 1][i].value;
                }
            }

            for (i = to_add * block; i < point + 1; i++) {
                ascii[bwt[i] - 9]++;
            }

            for (i = 0; i < occ->size; i++) {
                if (occ->entries[i].ascii == bwt[point]) {
                    point = occ->entries[i].value + ascii[bwt[point] - 9] - 1;
                    digit++;
                    tmp = realloc(tmp, (digit + 1) * sizeof(char));
                    tmp[digit - 1] = bwt[point];
                    break;
                }
            }
        }
        
        tmp[digit - 1] = '\0';
        reverseString(tmp);
        
        for (i = 0; i < result_num; i++) {
            if (number[i] == atoi(tmp) + 1) break;
        }
        if (i != result_num && number[i] == atoi(tmp) + 1) continue;
        result_num++;
        number = realloc(number, result_num * sizeof(int));
        number[result_num - 1] = atoi(tmp) + 1;
    }
    free(tmp);

    int count;
    count = 0;
    j = 0;
    int num;
    int size_count = 0;
    while (count < result_num) {
        for (m = 0; m < result_num; m++) {
            if (j == number[m]) {
                num = j;
                count++;
                for (i = 0; i < occ->size; i++) {
                    if (occ->entries[i].ascii == ']') {
                        from = occ->entries[i].value;
                        if (i + 1 >= occ->size) {
                            to = length - 1;
                        } else {
                            to = occ->entries[i + 1].value - 1;
                        }
                        break;
                    }
                }
                while (num != 0) {
                    digit = num % 10;
                    num /= 10;
                    to_add = (from - 1) / block;
                    if (to_add == 0) {
                        for (i = 0; i < 118; i++) {
                            ascii[i] = 0;
                        }
                    } else {
                        for (i = 0; i < dictionary->cols; i++) {
                            ascii[i] = dictionary->entries[to_add - 1][i].value;
                        }
                    }
                    for (i = to_add * block; i < to + 1; i++) {
                        ascii[bwt[i] - 9]++;      
                        if (i == from - 1) {
                            count_from = ascii[digit + '0' - 9];
                        }
                    }
                    count_to = ascii[digit + '0' - 9];

                    for (i = 0; i < occ->size; i++) {
                        if (occ->entries[i].ascii == digit + '0') {
                            from = occ->entries[i].value + count_from;
                            to = from + count_to - count_from - 1;
                            break;
                        }
                    }
                }
                to_add = (from - 1) / block;
                if (to_add == 0) {
                    for (i = 0; i < 118; i++) {
                        ascii[i] = 0;
                    }
                } else {
                    for (i = 0; i < dictionary->cols; i++) {
                        ascii[i] = dictionary->entries[to_add - 1][i].value;
                    }
                }
                for (i = to_add * block; i < to + 1; i++) {
                    ascii[bwt[i] - 9]++;      
                    if (i == from - 1) {
                        count_from = ascii['[' - 9];
                    }
                }
                count_to = ascii['[' - 9];

                for (i = 0; i < occ->size; i++) {
                    if (occ->entries[i].ascii == '[') {
                        from = occ->entries[i].value + count_from;
                        to = from + count_to - count_from - 1;
                        break;
                    }
                }
                if (from > to) {
                    from = first_bracket;
                }
                size_count++;
                bracket_seq = realloc(bracket_seq, (size_count) * sizeof(int));
                bracket_seq[size_count - 1] = from;           
            }
        }
        j++;
    }
    free(number);
    
    for (j = 0; j < result_num; j++) {
        count = 0;
        point = bracket_seq[j];
        while (bwt[point] != '[') {
            to_add = point / block;
            if (to_add == 0) {
                for (i = 0; i < 118; i++) {
                    ascii[i] = 0;
                }
            } else {
                for (i = 0; i < dictionary->cols; i++) {
                    ascii[i] = dictionary->entries[to_add - 1][i].value;
                }
            }
            for (i = to_add * block; i < point + 1; i++) {
                ascii[bwt[i] - 9]++;
            }
            for (i = 0; i < occ->size; i++) {
                if (occ->entries[i].ascii == bwt[point]) {
                    result = realloc(result, sizeof(char) * (count + 2));
                    result[count] = bwt[point];
                    count++;
                    point = occ->entries[i].value + ascii[bwt[point] - 9] - 1;
                    break;
                }
            }
        }
        result = realloc(result, sizeof(char) * (count + 2));
        result[count] = '[';
        result[count + 1] = '\0';
        for (i = count; i >= 0; i--) {
            printf("%c", result[i]);
        }
        printf("\n");
    }
    free(result);
    free(bracket_seq);
}

int main(int argc, char *argv[]) {
    FILE *file;
    char* line = NULL;
    int length = 0;
    char ch;
    int i;
    char *to_search = NULL;
    int block = 500;
    if (argc != 4) {
        printf("Usage: %s <input> <output>\n", argv[0]);
        return 1;
    }

    file = fopen(argv[1], "r");
    if (file == NULL) {
        printf("Error opening the file.\n");
        return 1;
    }
    while (!feof(file)) {
        ch = fgetc(file);
        line = realloc(line, (length + 1) * sizeof(char));
        line[length] = ch;
        length++;
    }
    fclose(file);
    line[length - 1] = '\0';
    //printf("123\n");
    to_search = malloc(sizeof(char) * strlen(argv[3]) + 1);
    strcpy(to_search, argv[3]);
    to_search[strlen(argv[3])] = '\0';

    struct Dictionary dictionary;
    initializeDictionary(&dictionary, 118);
    struct Occ occ;
    initializeOcc(&occ, 98);

    char* bwt_text = rlb_to_bwt(line, &dictionary, block);
    free(line);
    int bwt_text_length = strlen(bwt_text);
    int to_add = bwt_text_length / block;
    length = 0;
    int ascii[118];
    if (to_add == 0) {
        for (i = 0; i < 118; i++) {
            ascii[i] = 0;
        }
    } else {
        for (i = 0; i < dictionary.cols; i++) {
            ascii[i] = dictionary.entries[dictionary.rows - 1][i].value;
        }
    }
    
    for (i = to_add * block; i < bwt_text_length; i++) {
        ascii[bwt_text[i] - 9]++;
    }
    for (i = 0; i < 118; i++) {
        if (ascii[i] > 0) {
            add_occ(&occ, i + 9, length);
            length += ascii[i];
        }
    }

    occ.entries = (struct OccEntry*)realloc(occ.entries, occ.size * sizeof(struct OccEntry));
    
    inverse_bwt(&dictionary, &occ, bwt_text, to_search, block);

    for (i = 0; i < dictionary.rows; i++) {
        free(dictionary.entries[i]);
    }
    free(dictionary.entries);
    free(occ.entries);
    free(bwt_text);
    free(to_search);
    
    return 0;
}
