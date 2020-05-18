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
