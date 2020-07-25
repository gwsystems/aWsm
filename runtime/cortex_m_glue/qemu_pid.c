unsigned long backing_millis = 1000000;

unsigned long env_millis() {
    return backing_millis++;
}

double backing_v = 1.3;

double env_analogRead(int pin) {
    return backing_v;
}

void env_analogWrite(int pin, double v) {
    backing_v = v;
}