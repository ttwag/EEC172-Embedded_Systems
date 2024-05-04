int start = 0;
int buffer[20];
int buffer_int = 0;
int readReady = 0;
void handler() {
    int time;
    time = TICKS_TO_US(ulsystick_delta);

    // Captures the wave only after 12ms pulse
    if (120000 < time < 125000) start = 1;
    if (start) {
        buffer[buffer_int] = time;
        buffer_int++;
        if (buffer_int == 17) {
            start = 0;
            readReady = 1;
        }
    }
}

int decoder() {
    int sum = 0x0;
    for (int i = 7; i < 17; i++) {
        if (buffer[i] > 2000) {
            sum = sum | 0x1; 
        }
        sum = sum << 1;
    }
}

int main() {
    if (readReady) {
        
        readReady = 0;
    }
    return 0;
}