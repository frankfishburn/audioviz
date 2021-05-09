#ifndef INTERPOLATOR_H
#define INTERPOLATOR_H

#include <vector>

struct interp {
    unsigned long idxlow;
    unsigned long idxhigh;
    float scale;
};

class UnivariateInterpolator {
   public:
    UnivariateInterpolator(){};
    UnivariateInterpolator(const std::vector<float> &known_pts,
                           const std::vector<float> &query_pts);

    void estimate(const std::vector<float> &known_values,
                  std::vector<float> &query_values) const;

   private:
    std::vector<interp> data_;
};

#endif /* INTERPOLATOR_H */
