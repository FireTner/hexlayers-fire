#include <smmintrin.h>

// debugging
#include <string.h>
#include <stdio.h>

typedef __m128i v16n;
typedef unsigned char u8;

v16n t, one, zero;

v16n comp(v16n a, v16n b, u8 m) {
    if(m) return _mm_max_epi8(_mm_sub_epi8(a, b), 0);
    return _mm_and_si128(a, _mm_cmpgt_epi8(b, a));
}

// generates look up table
size_t genLut(v16n *lut, size_t lutsize) {
    // very good error handling
    //if(lutsize < 1024) return -1;
    
    size_t newsize = 0;
    v16n x;

    for(int i = 0; i < 1024; i++) {
        u8 ma = (i & 0x100) >> 8;
        u8 mb = (i & 0x200) >> 9;
        u8 a  = (i & 0xF);
        u8 b  = (i & 0xF0) >> 4;


        x = _mm_max_epi8(
            comp(t, _mm_set1_epi8(a), ma),
            comp(_mm_set1_epi8(b), t, mb)
        );
        
        if( _mm_test_all_zeros(x, one) ) continue;

        int j = 0;
        for(j; j < newsize; j++)
            if( _mm_test_all_zeros(_mm_xor_si128(x, lut[j]), one) )
                break;
        if(j==newsize) lut[newsize++] = x;
    }

    printf("genLut finished\n");
    return newsize;
}

void findSeq(const v16n goal, const v16n *lut, const size_t lutsize, int *result) {
    int counter[50] = { [0 ... 49] = -1 };
    v16n x;

    do {
        // increment counter
        for(int i = 0; i < 50; i++) {
            if(++counter[i]!=lutsize) break;
            counter[i] = 0;
        }

        // do cummulative look up
        x = t;
        for(int i = 0; counter[i]!=-1; i++)
            x = _mm_shuffle_epi8(x, lut[counter[i]]);
        
        // check equality
        x = _mm_xor_si128(x, goal);
    } while(!_mm_testz_si128(x, one));

    printf("0; %2d\n", counter[0]);
    printf("1; %2d\n", counter[1]);
    printf("findSeq finished\n");
}

void outputSeq(unsigned int* sequence) {
    return; // TODO
}

int main(int argc, char **argv) {
    // initialize constants and variables
    t = _mm_set_epi8(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15);
    one = _mm_set1_epi8(1);
    zero = _mm_set1_epi8(0);
    
    v16n goal = _mm_set_epi8(15,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14);
    
    size_t olutsize = 1024;
    v16n *lut = (v16n *)malloc(olutsize * sizeof(v16n));
    int *result = (int *)malloc(50 * sizeof(int));

    // generate lut
    lutsize = genLut(lut, olutsize);
    printf("lutsize: %zu\n", lutsize);
    if(olutsize>lutsize)
        lut = (v16n *)realloc(lut, lutsize);
    
    // find the sequence
    findSeq(goal, lut, lutsize, result);
    
    // output the result
    outputSeq(result);

    return 0;
}