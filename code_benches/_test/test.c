#define CALLBACK_COUNT 1024
typedef int (*int_to_int)(int);
struct {
    int_to_int addr;
} callbacks[CALLBACK_COUNT];

void assert(int v);

int invoke_callback(int idx, int v) {
    assert(idx < CALLBACK_COUNT);
    int_to_int fn = callbacks[idx].addr;
    return fn(v);
}

void add_callback(int idx, int_to_int addr) {
    assert(idx < CALLBACK_COUNT);
    callbacks[idx].addr = addr;
}