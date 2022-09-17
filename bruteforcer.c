#include <smmintrin.h>

// debugging
#include <string.h>
#include <stdio.h>
#include <stdalign.h>
#include <time.h>


typedef __m128i v16n;
typedef unsigned char u8;

v16n t, one, zero;

// unsafe; output the sequence until -1
void outputSeq(int* sequence) {
    for(int i = 0; sequence[i] != -1; i++) {
        printf("%d ", sequence[i]);
    }
    printf("\n");
}

// print the v16n vector
void print_v16n(v16n a) {
    alignas(16) unsigned char v[16];
    _mm_store_si128((v16n*)v, a);
    printf("%u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u\n",
           v[0], v[1],  v[2],  v[3],  v[4],  v[5],  v[6],  v[7],
           v[8], v[9], v[10], v[11], v[12], v[13], v[14], v[15]);
}

// count the number of unique elements
int uniqueCount(v16n a) {
    alignas(16) unsigned char v[16];
    _mm_store_si128((v16n*)v, a);

    int count = 1;
    for(int i = 1; i < 16; i++) {
        int j;
        for(j = 0; j < i; j++)
            if(v[i] == v[j])
                break;

        if(i == j) count++;
    }

    return count;
}

// comparator
v16n comp(v16n a, v16n b, u8 m) {
    if(m) return _mm_max_epi8(_mm_sub_epi8(a, b), zero);
    return _mm_andnot_si128(_mm_cmplt_epi8(a, b), a);
}

// generates look up table
size_t genLut(v16n *lut, size_t lutsize, const v16n goal) {
    size_t newsize = 0;
    v16n x;
    int goaluc = uniqueCount(goal);

    for(int i = 0; i < 1024; i++) {
        u8 ma = (i & 0x100) >> 8;
        u8 mb = (i & 0x200) >> 9;
        u8 a  = (i & 0xF);
        u8 b  = (i & 0xF0) >> 4;


        x = _mm_max_epi8(
            comp(t, _mm_set1_epi8(a), ma),
            comp(_mm_set1_epi8(b), t, mb)
        );

        if(_mm_testz_si128(_mm_xor_si128(x, t), one)) continue;
        if(uniqueCount(x) < goaluc) continue;

        int j;
        for(j = 0; j < newsize; j++)
            if( _mm_testz_si128(_mm_xor_si128(x, lut[j]), one) )
                break;

        if(j==newsize)
            lut[newsize++] = x;
    }

    return newsize;
}

void findSeq(const v16n goal, const v16n *lut, const size_t lutsize, int *result) {
    memset(result, -1, 50*sizeof(int));

    v16n x;
    do {
        // increment counter
        for(int i = 0; ++result[i]==lutsize; i++)
            result[i] = 0;

        // do cummulative lookup
        x = t;
        for(int i = 0; result[i]!=-1; i++)
            x = _mm_shuffle_epi8(x, lut[result[i]]);
        
        // check equality
        x = _mm_xor_si128(x, goal);
    } while(!_mm_testz_si128(x, one));
}

int main(int argc, char **argv) {
    // initialize constants and variables
    t = _mm_set_epi8(15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0);
    one = _mm_set1_epi8(0xFF);
    zero = _mm_set1_epi8(0);
    
    v16n goal = _mm_set_epi8(
        0, 1, 2, 4, 3, 5, 8, 7, 6, 9, 10, 11, 12, 13, 14, 15
    );
    // flip goal
    goal = _mm_shuffle_epi8(goal, _mm_sub_epi8(_mm_set1_epi8(0xF), t));

    size_t lutsize = 739;
    v16n *lut = (v16n *)malloc(lutsize * sizeof(v16n));
    int *result = (int *)malloc(50 * sizeof(int));

    if(lut == NULL || result == NULL)
        printf("malloc failed\n");

    // generate lut
    printf("Generating LUT\n");
    lutsize = genLut(lut, lutsize, goal);
    printf("LUT generated with the size of %zd\n", lutsize);
    
    // find the sequence
    printf("Starting the search");
    time_t start = clock();
    findSeq(goal, lut, lutsize, result);
    printf("The search ended in %f\n", ((double)clock())/CLOCKS_PER_SEC);

    // output the result
    outputSeq(result);

    free(&lut);
    free(&result);
    return 0;
}