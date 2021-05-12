#include "resampler.h"

Resampler::Resampler(const std::vector<float> &known_pts,
                     const std::vector<float> &query_pts) {
    const unsigned long num_known = known_pts.size();
    const unsigned long num_query = query_pts.size();

    data_.resize(num_query);

    for (unsigned long query_idx = 0; query_idx < num_query; query_idx++) {
        const float query_pt = query_pts[query_idx];

        // Identify relevant indices in original frequency vector
        unsigned long lowidx = 0;
        unsigned long highidx = num_known;
        for (unsigned long known_index = 1; known_index < num_known;
             known_index++) {
            if (known_pts[known_index] > query_pt) {
                lowidx = known_index - 1;
                highidx = known_index;
                break;
            }
        }

        // Set the scaling values
        data_[query_idx].idxlow = lowidx;
        data_[query_idx].idxhigh = highidx;
        data_[query_idx].scale = (query_pt - known_pts[lowidx]) /
                                 (known_pts[highidx] - known_pts[lowidx]);
    }
}

std::vector<float> Resampler::resample(const std::vector<float> &values) const {
    std::vector<float> result(data_.size());
    for (unsigned long idx = 0; idx < data_.size(); idx++) {
        const interp &q = data_[idx];
        result[idx] =
            values[q.idxlow] * (1 - q.scale) + values[q.idxhigh] * q.scale;
    }
    return result;
}
