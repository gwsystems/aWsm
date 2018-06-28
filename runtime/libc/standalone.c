#include "../runtime.h"

// Functions that turn offsets into pointers
char* offset2ptr(u32 p) {
    assert(p < memory_size);
    char* mem_as_chars = (char*) memory;
    return &mem_as_chars[p];
}

void* offset2ptr_void(u32 p) {
    assert(p < memory_size);
    char* mem_as_chars = (char*) memory;
    return &mem_as_chars[p];
}

// Globals for the libc stubs
int free_file_entry = 3;
FILE* file_entries[100];

i32 env_stdout;
i32 env_stderr;

// Stub initialization routine
void stub_init(i32 offset) {
    env_stderr = offset;
    i32* stderr_ptr = offset2ptr_void(env_stderr);
    *stderr_ptr = 1;
    file_entries[*stderr_ptr] = stderr;
    offset += 4;

    env_stdout = offset;
    i32* stdout_ptr = offset2ptr_void(env_stdout);
    *stdout_ptr = 2;
    file_entries[*stdout_ptr] = stdout;
    offset += 4;
}

// libc stubs
// FIXME: Literally half these functions are unsafe
EXPORT double env_acos(double a) {
    return acos(a);
}

EXPORT double env_cos(double a) {
    return cos(a);
}

EXPORT double env_atof(u32 offset) {
    return atof(offset2ptr(offset));
}

EXPORT i32 env_atoi(u32 offset) {
    return (i32) atoi(offset2ptr(offset));
}

EXPORT i32 env_atol(u32 offset) {
    return env_atoi(offset);
}


EXPORT double env_exp(double x) {
    return exp(x);
}

EXPORT double env_exp2(double x) {
    return exp2(x);
}

EXPORT void env_exit(u32 status) {
    exit((int) status);
}

// FILE *fopen(const char *pathname, const char *mode);
i32 env_fopen(i32 pathname_off, i32 mode_off) {
    char* pathname = offset2ptr(pathname_off);
    char* mode = offset2ptr(mode_off);

    printf("fopen(%d: %s, %d: %s)\n", pathname_off, pathname, mode_off, mode);

    FILE* f = fopen(pathname, mode);
    if (!f) {
        printf("fopen done early\n");
        return 0;
    }

    int idx = free_file_entry;
    file_entries[idx] = f;
    free_file_entry += 1;

    printf("fopen done (%d)\n", idx);
    return idx;
}

// int fclose(FILE *stream);
i32 env_fclose(i32 idx) {
    FILE *f = file_entries[idx];
    return fclose(f);
}

// int fgetc(FILE *stream);
i32 env_fgetc(i32 idx) {
    FILE *f = file_entries[idx];
    return fgetc(f);
}

// char* fgets(char * restrict str, int size, FILE * restrict stream);
i32 env_fgets(i32 str_offset, i32 size, i32 idx) {
    void* str = offset2ptr_void(str_offset);
    FILE* stream = file_entries[idx];

    char* s = fgets(str, size, stream);
    if (s == NULL) {
        return 0;
    }

    return str_offset;
}

void env_fprintf() { assert(0); }

i32 env_fread(i32 ptr_offset, i32 size, i32 nitems, i32 idx) {
    void* ptr = offset2ptr_void(ptr_offset);
    FILE* stream = file_entries[idx];
    return fread(ptr, size, nitems, stream);
}

// size_t fwrite(const void *restrict ptr, size_t size, size_t nitems, FILE *restrict stream);
i32 env_fwrite(i32 ptr_offset, i32 size, i32 nitems, i32 idx) {
    void* ptr = offset2ptr_void(ptr_offset);
    FILE* stream = file_entries[idx];
    return fwrite(ptr, size, nitems, stream);
}

void env_free() { }

int env_getc(i32 idx) {
    int v = getc(file_entries[idx]);
    return v;
}

// void* memcpy(void *restrict dst, const void *restrict src, size_t n);
i32 env_memcpy(i32 dst_offset, i32 str_offset, i32 n) {
    void* dst = offset2ptr_void(dst_offset);
    void* str = offset2ptr_void(str_offset);
    memcpy(dst, str, n);

    return dst_offset;
}

void env_memset() { assert(0); }

EXPORT double env_pow(double a, double b) {
    return pow(a, b);
}

size_t count_args(char* format_str) {
    size_t res = 0;
    for (int i = 0; i < strlen(format_str); i++) {
        if (format_str[i] == '%') res++;
    }
    return res;
}

EXPORT i32 env_printf(i32 a, i32 b) {
    i32 ret;

    char* format_string = offset2ptr(a);
    size_t n = count_args(format_string);

    i32* args = offset2ptr_void(b);
    if (n == 0){
        ret = printf("%s", format_string);
    } else if (n == 1) {
        ret = printf(format_string, args[0]);
    } else if (n == 2) {
        ret = printf(format_string, args[0], args[1]);
    } else if (n == 3) {
        ret = printf(format_string, args[0], args[1], args[2]);
    } else if (n == 4) {
        ret = printf(format_string, args[0], args[1], args[2], args[3]);
    } else if (n == 5) {
        ret = printf(format_string, args[0], args[1], args[2], args[3], args[4]);
    } else {
        assert(0);
    }

    return ret;
}

EXPORT void env_put_int(i32 s, i32 i) {
    printf("%s = %d\n", offset2ptr(s), (int) i);
}

//int putc(int c, FILE *stream);
EXPORT i32 env_putc(i32 c, i32 idx) {
    FILE* stream = file_entries[idx];
    return putc(c, stream);
}

EXPORT i32 env_putchar(i32 a) {
    return putchar(a);
}

EXPORT i32 env_puts(i32 a) {
    return puts(offset2ptr(a));
}

u32 frontier = 0;

EXPORT i32 env_calloc(i32 a, i32 b) {
    if (frontier == 0) {
        frontier = memory_size;
    }

    // FIXME: Do overflow checking here
    u32 address = frontier;
    u32 total_size = a * b;
    frontier += total_size;

    while (frontier >= memory_size) {
        expand_memory();
    }

    return address;
}

EXPORT i32 env_malloc(i32 size) {
    return env_calloc(size, 1);
}

EXPORT i32 env_rand() {
    return rand();
}

EXPORT i32 env_clock() {
    return clock();
}
