#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "cJSON.h"
cJSON* cJSON_Parse(const char *value);

char rfc3986[256] = {0};
char html5[256] = {0};

void url_encoder_rfc_tables_init() {
    for (int i = 0; i < 256; i++) {
        rfc3986[i] = isalnum(i) || i == '~' || i == '-' || i == '.' || i == '_' ? i : 0;
        html5[i] = isalnum(i) || i == '*' || i == '-' || i == '.' || i == '_' ? i : (i == ' ') ? '+' : 0;
    }
}

char* url_encode(char *table, unsigned char *s, char *enc) {
    for (; *s; s++){

        if (table[*s]) sprintf( enc, "%c", table[*s]);
        else sprintf( enc, "%%%02X", *s);
        while (*++enc);
    }

    return( enc);
}

void urldecode2(char *dst, const char *src) {
        char a, b;
        while (*src) {
                if ((*src == '%') &&
                    ((a = src[1]) && (b = src[2])) &&
                    (isxdigit(a) && isxdigit(b))) {
                        if (a >= 'a')
                                a -= 'a'-'A';
                        if (a >= 'A')
                                a -= ('A' - 10);
                        else
                                a -= '0';
                        if (b >= 'a')
                                b -= 'a'-'A';
                        if (b >= 'A')
                                b -= ('A' - 10);
                        else
                                b -= '0';
                        *dst++ = 16*a+b;
                        src+=3;
                } else if (*src == '+') {
                        *dst++ = ' ';
                        src++;
                } else {
                        *dst++ = *src++;
                }
        }
        *dst++ = '\0';
}

int main(int argc, char *argv[]) {
//    url_encoder_rfc_tables_init();

//    char* in_file_name = argv[1];
//    FILE* in_file = fopen(in_file_name, "r");
//    if(in_file == NULL) {
//        perror("Error opening input");
//        return -1;
//    }
//
//    char* out_file_name = argv[2];
//    FILE* out_file = fopen(out_file_name, "w");
//    if(out_file == NULL) {
//        perror("Error opening output");
//        return -1;
//    }

//    char working_buf[4096];
//    char encoded_buf[4096];
//    printf("beginning loop\n");
//    while (1) {
//        if(fgets(working_buf, 4096, in_file) != NULL) {
//            printf("working buf %s\n", working_buf);
//            urldecode2(encoded_buf, working_buf);
//            printf("encoded buf %s\n", encoded_buf);
//            cJSON *item = cJSON_Parse(encoded_buf);
//            if (item == NULL) {
//                printf("Parse of working buf failed\n");
//            }
//            char* in_char = cJSON_Print(item);
//            printf("input %s\n", in_char);
//            url_encode(html5, "{\"response_body\":\"all good!!!\",\"http_response_code\":200}", encoded_buf);
//            printf("encoded buf %s\n", encoded_buf);
//            size_t out = fwrite(encoded_buf, strlen(encoded_buf) + 1, 1, out_file);
//            fflush(out_file);
//            printf("out = %d\n", (int) out);
//        }
//    }

//    url_encode(html5, "{\"response_body\":\"all good!!!\",\"http_response_code\":200}", encoded_buf);
    printf("200;all good bud!!\n");

    return 0;
}