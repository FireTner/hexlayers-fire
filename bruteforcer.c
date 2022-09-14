#include <smmintrin.h>

// debugging
#include <string.h>
#include <stdio.h>

typedef __m128i v16n;
typedef unsigned char u8;

v16n t;

// can be negative
v16n ncomp(v16n a, v16n b, u8 m) {
    if(m) return _mm_sub_epi8(a, b);
    return _mm_and_si128(a, _mm_cmpgt_epi8(a, b));
}

// generates look up table
size_t genLut(v16n *lut, size_t lutsize) {
    // very good error handling
    if(lutsize < 1024) return -1;
    
    size_t newsize = 0;
    const v16n zero = _mm_set1_epi8(0);
    const v16n _mm_one = _mm_set1_epi8(0xFF);
    v16n x;

    for(int i = 0; i < 1024; i++) {
        u8 ma = (i & 0x100) >> 8;
        u8 mb = (i & 0x200) >> 9;
        u8 a  = (i & 0xF);
        u8 b  = (i & 0xF0) >> 4;


        x = _mm_max_epi8(zero, _mm_max_epi8(
            ncomp(_mm_set1_epi8(a), t, ma),
            ncomp(t, _mm_set1_epi8(b), mb)
        ));

        int isFound = 0;

        for(int j = 0; j < newsize; j++)
            if( _mm_test_all_zeros(_mm_xor_si128(x, lut[j]), _mm_one) )
                isFound = 1;

        if(isFound==0) lut[newsize++] = x;

        if(newsize==303) {
            printf("%d %d %d %d\n", ma, mb, a, b);
        }
    }

    printf("genLut finished\n");
    return newsize;
}

void findSeq(const v16n goal, const v16n *lut, const size_t lutsize, int *result) {
    int counter[50] = { [0 ... 49] = -1 };
    const v16n _mm_one = _mm_set1_epi8(0xFF);
    v16n x;

    do {
        for(int i = 0; i < 50; i++) {;
            if(counter[i]++!=lutsize) break;
            counter[i] = 0;
        }

        // do cummulative look up
        x = t;
        for(int i = 0; counter[i]!=-1; i++)
            x = _mm_shuffle_epi8(x, lut[counter[i]]);
        
        // check equality
        x = _mm_xor_si128(x, goal);
    } while(!_mm_testz_si128(x, _mm_one));

    printf("%d\n", counter[0]);
    printf("%d\n", counter[1]);
    //printf("findSeq finished\n");
}

/*void outputSeq(unsigned int* sequence) {
    return;
}*/

int main(int argc, char **argv) {
    t = _mm_set_epi8(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15);
    v16n goal = _mm_set_epi8(15,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14);
    size_t lutsize = 1024;
    v16n *lut = malloc(lutsize * sizeof(v16n));
    int *result = malloc(50 * sizeof(int));

    lutsize = genLut(lut, lutsize);
    printf("n: %zu\n", lutsize);
    findSeq(goal, lut, lutsize, result);
    //outputSeq(counter);

    return 0;
}