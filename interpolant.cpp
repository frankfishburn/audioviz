#include "interpolant.h"

interpolant::interpolant(int N0, float *x0, int Nq, float *xq) {
    N = Nq;
    data.resize(Nq);

    for (int iq = 0; iq < Nq; iq++) {
        float querypt = xq[iq];

        // Identify relevant indices in original frequency vector
        int lowidx = 0;
        int highidx = N0;
        for (int id0 = 1; id0 < N0; id0++) {
            if (x0[id0] > querypt) {
                lowidx = id0 - 1;
                highidx = id0;
                break;
            }
        }

        // Set the scaling values
        data[iq].idxlow = lowidx;
        data[iq].idxhigh = highidx;
        data[iq].scale = (querypt - x0[lowidx]) / (x0[highidx] - x0[lowidx]);
    }
}

interpolant::interpolant(const interpolant &orig) {
    N = orig.N;
    data = orig.data;
}

interpolant::~interpolant() {}

void interpolant::estimate(float *y0, float *yq) {
    for (int idx = 0; idx < N; idx++) {
        interp q = data[idx];
        yq[idx] = y0[q.idxlow] * (1 - q.scale) + y0[q.idxhigh] * q.scale;
    }

    return;
}
