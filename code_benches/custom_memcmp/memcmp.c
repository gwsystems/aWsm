volatile char a[] = "123456";
volatile char b[] = "123YF";


__attribute__ ((used))
int my_memcmp(volatile char *vl, volatile char *vr, unsigned long n)
{
        volatile char *l=vl, *r=vr;
        for (; n && *l == *r; n--, l++, r++);
        return n ? *l-*r : 0;
}

int main() {
    int v = my_memcmp(a, b, 4);
    if (v != 0) {
        return 0;
    } else {
        return -1;
    }
}