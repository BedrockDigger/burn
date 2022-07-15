#include <stdio.h>
#include <complex.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include <smmintrin.h>
#include <memory.h>

void print128_num(__m128i var) {
    uint16_t val[8];
    memcpy(val, &var, sizeof(val));
    printf("%i%i %i%i %i%i %i%i\n",
           val[0], val[1], val[2], val[3], val[4], val[5],
           val[6], val[7]);
}

void burning_ship(size_t width, size_t height,
                  unsigned n) {
    // Platzhalter fürs Datenteil von img; später headers hinzufügen und direkt zu img schreiben
    unsigned char *img_data = malloc(sizeof(unsigned char) * width * height * 3);
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width - width % 4; j += 4) {
            __m128 re_c = _mm_set_ps((float) j, (float) j + 1, (float) j + 2, (float) j + 3);
            __m128 im_c = _mm_set_ps((float) i, (float) i + 1, (float) i + 2, (float) i + 3);
            __m128 re_z = _mm_set_ps(0, 0, 0, 0);
            __m128 im_z = _mm_set_ps(0, 0, 0, 0);
            __m128 threshold = _mm_set_ps(4, 4, 4, 4);
            __m128i bounded_iter_cnt = _mm_set_epi32(0, 0, 0, 0);
            for (int k = 0; k < n; ++k) {
                // das reale Teil rechnen
                // TODO: einfach so oder mit mov intrinsics?
                __m128 re_z_old = re_z;
                re_z = _mm_mul_ps(re_z, re_z);
                __m128 b_sqr = _mm_mul_ps(im_z, im_z);
                re_z = _mm_sub_ps(re_z, b_sqr);
                re_z = _mm_add_ps(re_z, re_c);
                // das imaginäre Teil rechnen
                im_z = _mm_mul_ps(re_z_old, im_z);
                // im_z ist jetzt der Betrag von im_z
                im_z = _mm_max_ps(_mm_sub_ps(_mm_setzero_ps(), im_z), im_z);
                im_z = _mm_add_ps(im_z, im_z);
                im_z = _mm_add_ps(im_z, im_c);
                // jetzt ist re_z re_z^2
                re_z = _mm_mul_ps(re_z, re_z);
                // jetzt ist im_z im_z^2
                im_z = _mm_mul_ps(im_z, im_z);
                // jetzt ist re_z a^2 + b^2
                re_z = _mm_add_ps(re_z, im_z);
                __m128 mask = _mm_cmpgt_ps(threshold, re_z);
                __m128i masked_inc = _mm_and_si128(
                        _mm_set_epi32(0x00000001, 0x00000001, 0x00000001, 0x00000001),
                        mask);
                bounded_iter_cnt = _mm_add_epi32(bounded_iter_cnt, masked_inc);
            }
            __m128i color = _mm_mul_epi32(
                    // TODO: oder 256?
                    _mm_set_epi32((int) (255 / n), (int) (255 / n), (int) (255 / n), (int) (255 / n)),
                    bounded_iter_cnt);
            print128_num(color);
//            for (int k = 0; k < 4; ++k) {
//                _mm_store_si128((__m128i *) img_data + k + (i * width + j) * 3 * sizeof(char), color);
//            }

        }
    }
}


// erstens als eine separate Funktion, die nur 4 float complex bearbeitet
float *burning_vector(float four_c_re[], float four_c_im[], int n) {
    __m128 re_c = _mm_set_ps(four_c_re[0], four_c_re[1], four_c_re[2], four_c_re[3]);
    __m128 im_c = _mm_set_ps(four_c_im[0], four_c_im[1], four_c_im[2], four_c_im[3]);
    __m128 re_z = _mm_set_ps(0, 0, 0, 0);
    __m128 im_z = _mm_set_ps(0, 0, 0, 0);
    for (int i = 0; i < n; ++i) {
        // das reale Teil rechnen
        // TODO: einfach so oder mit mov intrinsics?
        __m128 re_z_old = re_c;
        re_z = _mm_mul_ps(re_z, re_z);
        __m128 b_sqr = _mm_mul_ps(im_z, im_z);
        re_z = _mm_sub_ps(re_z, b_sqr);
        re_z = _mm_add_ps(re_z, re_c);
        // das imaginäre Teil rechnen
        im_z = _mm_mul_ps(re_z_old, im_z);
        im_z = _mm_max_ps(_mm_sub_ps(_mm_setzero_ps(), im_z), im_z);
        im_z = _mm_add_ps(im_z, im_z);
        im_z = _mm_add_ps(im_z, im_c);
    }
    // jetzt ist re_z re_z^2
    re_z = _mm_mul_ps(re_z, re_z);
    // jetzt ist im_z im_z^2
    im_z = _mm_mul_ps(im_z, im_z);
    // // jetzt ist re_z das result
    re_z = _mm_add_ps(re_z, im_z);
    float *result = malloc(sizeof(float) * 4);
    _mm_storeu_ps(result, re_z);
    return result;
}


int main() {
    burning_ship(100, 100, 256);
    return 0;
}
