#ifndef RESAMPLER_H
#define RESAMPLER_H

#include <vector>

struct interp {
    unsigned long idxlow;
    unsigned long idxhigh;
    float scale;
};

class Resampler {
   public:
    Resampler(){};
    Resampler(const std::vector<float> &known_pts,
              const std::vector<float> &query_pts);
    std::vector<float> resample(const std::vector<float> &values) const;

   private:
    std::vector<interp> data_;
};

#endif /* RESAMPLER_H */
