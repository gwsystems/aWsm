int a_cas(int* p, int old_val, int new_val) {
    int val = *p;
    if (val == old_val)
        *p = new_val;
    return val;
}

int a_swap(int* p, int v) {
    int old;
    do old = *p;
    while (a_cas(p, old, v) != old);
    return old;
}

int a_fetch_add(int* p, int v) {
    int old = *p;
    *p += v;

	return old;
}

int a_ctz_l(unsigned int x) {
	static const char debruijn32[32] = {
		0, 1, 23, 2, 29, 24, 19, 3, 30, 27, 25, 11, 20, 8, 4, 13,
		31, 22, 28, 18, 26, 10, 7, 12, 21, 17, 9, 6, 16, 5, 15, 14
	};
	return debruijn32[(x&-x)*0x076be629 >> 27];
}

int a_ctz_64(unsigned long long x) {
	unsigned int y = x;
	if (!y) {
		y = x>>32;
		return 32 + a_ctz_l(y);
	}
	return a_ctz_l(y);
}

void a_and_64(unsigned long long* p, unsigned long long v) {
    *p &= v;
}

void a_or_64(unsigned long long* p, unsigned long long v) {
    *p |= v;
}


void a_store(int* p, int x) {
//    assert(sizeof(i32) == sizeof(volatile int));
//    volatile int* p = get_memory_ptr_void(p_off, sizeof(i32));
    *p = x;
}

void a_dec(int* x) {
    *x = *x - 1;
}

void a_inc(int* x) {
    *x = *x + 1;
}
