#ifndef INTERPOLANT_H
#define INTERPOLANT_H

#include <vector>

struct interp {
    int   idxlow;
    int   idxhigh;
    float scale;
};

class interpolant {
   public:
    interpolant(int N, float* x0, int Nq, float* xq);
    interpolant(const interpolant& orig);
    ~interpolant();

    void estimate(float* y0, float* yq);

   private:
    int                 N;
    std::vector<interp> data;
};

#endif /* INTERPOLANT_H */
