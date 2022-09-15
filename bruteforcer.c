#include <smmintrin.h>

// debugging
#include <string.h>
#include <stdio.h>

typedef __m128i v16n;
typedef unsigned char u8;

v16n t, one, zero;

v16n comp(v16n a, v16n b, u8 m) {
    if(m) return _mm_max_epi8(_mm_sub_epi8(a, b), zero);
    return _mm_and_si128(a, _mm_cmplt_epi8(b, a));
}

// generates look up table
size_t genLut(v16n *lut, size_t lutsize) {
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
        
        //if( _mm_test_all_zeros(_mm_xor_si128(x, t), one) ) continue;

        int j;
        for(j = 0; j < newsize; j++)
            if( _mm_test_all_zeros(_mm_xor_si128(x, lut[j]), one) )
                break;
        
        if(j==newsize) lut[newsize++] = x;
    }

    printf("genLut finished\n");
    return newsize;
}

void findSeq(const v16n goal, const v16n *lut, const size_t lutsize, int *result) {
    v16n x;

    memset(result, -1, 50);

    printf("Started\n");
    do {
        // increment counter
        for(int i = 0; i < 50; i++) {
            if(result[i]++ != lutsize) break;
            result[i] = 0;
        }

        // do cummulative look up
        x = t;
        for(int i = 0; result[i]!=-1; i++)
            x = _mm_shuffle_epi8(x, lut[result[i]]);
        
        // check equality
        x = _mm_xor_si128(x, goal);
    } while(!_mm_testz_si128(x, one));

    
    printf("findSeq finished\n");
}

void outputSeq(int* sequence) {
    for(int i = 0; sequence[i] != -1;) {
        printf("%d ", sequence[i++]);
    }
    printf("\n");
}

int main(int argc, char **argv) {
    // initialize constants and variables
    t = _mm_set_epi8(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15);
    one = _mm_set1_epi8(0xFF);
    zero = _mm_set1_epi8(0);
    
    v16n goal = _mm_set_epi8(
        3,13,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1,15
    );
    
    size_t olutsize = 1024;
    v16n *lut = (v16n *)malloc(olutsize * sizeof(v16n));
    int *result = (int *)malloc(50 * sizeof(int));

    // generate lut
    size_t lutsize = genLut(lut, olutsize);
    printf("lutsize: %zd\n", lutsize);
    if(olutsize>lutsize)
        lut = (v16n *)realloc(lut, lutsize);
    
    // find the sequence
    findSeq(goal, lut, lutsize, result);
    
    // output the result
    outputSeq(result);

    return 0;
}