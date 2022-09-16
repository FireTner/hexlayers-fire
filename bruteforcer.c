#include <smmintrin.h>

// debugging
#include <string.h>
#include <stdio.h>
#include <stdalign.h>

typedef __m128i v16n;
typedef unsigned char u8;

v16n t, one, zero;

void outputSeq(int *sequence);

v16n comp(v16n a, v16n b, u8 m) {
    if(m) return _mm_max_epi8(_mm_sub_epi8(a, b), zero);
    return _mm_andnot_si128(_mm_cmplt_epi8(a, b), a);
}

void print_v16n(v16n in) {
    alignas(16) unsigned char v[16];
    _mm_store_si128((v16n*)v, in);
    printf(": %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u\n",
           v[0], v[1],  v[2],  v[3],  v[4],  v[5],  v[6],  v[7],
           v[8], v[9], v[10], v[11], v[12], v[13], v[14], v[15]);
}

// correct
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
        
        if(j==newsize) {
            lut[newsize++] = x;
            print_v16n(x);
        }
    }

    printf("genLut finished\n");
    return newsize;
}

void findSeq(const v16n goal, const v16n *lut, const size_t lutsize, int *result) {
    printf("Started\n");
    memset(result, -1, 50*sizeof(int));
    
    int count = 0;
    v16n x;
    do {
        //if(++count%1000000000==0) outputSeq(result);
        // increment counter
        for(int i = 0; i < 50; i++) {
            if(++result[i]!=lutsize) break;
            result[i] = 0;
        }

        // do cummulative look up
        x = t;
        for(int i = 0; result[i]!=-1; i++)
            x = _mm_shuffle_epi8(x, lut[result[i]]);
        
        // check equality
        x = _mm_xor_si128(x, goal);
        if(_mm_test_all_zeros(x, one)) break;
    } while(!_mm_test_all_zeros(x, one));

    
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
        3, 3, 3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 15
    );
    
    size_t lutsize = 739;
    v16n *lut = (v16n *)malloc(lutsize * sizeof(v16n));
    int *result = (int *)malloc(50 * sizeof(int));

    // generate lut
    lutsize = genLut(lut, lutsize);
    printf("lutsize: %zd\n", lutsize);
    
    // find the sequence
    findSeq(goal, lut, lutsize, result);
    
    // output the result
    outputSeq(result);

    return 0;
}