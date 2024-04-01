#pragma once

class fastFloat {
public:
    static int fastFloor(float x) {
        if (x < 0 && int(x) != x)
            return int(x - 1);
        else
            return int(x);
    }

    static int fastFloor(double x) {
        if (x < 0 && int(x) != x)
            return int(x - 1);
        else
            return int(x);
    }

    static int fastCeil(float x) {
        if (x > 0 && int(x) != x)
            return int(x + 1);
        else
            return int(x);
    }

    static int fastCeil(double x) {
        if (x > 0 && int(x) != x)
            return int(x + 1);
        else
            return int(x);
    }
    
    static int mod(int a, int m) {
        int b = a % m;
        if (b >= 0)
            return b;
        else
            return m + b;
    }
    
    static bool atBit(const int value, const unsigned int position) {
        // position equals zero means rightmost digit
        return (value >> position) & 1;
    }

    static unsigned int atBits(const int value, const unsigned int position, const unsigned int numBits) {
        unsigned int mask = 0;
        for (int i = 0; i < numBits; i++)
            mask = (mask << 1) + 1;
        return (value >> position) & mask;
    }
};
