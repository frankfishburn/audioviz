#include "interpolator.h"

UnivariateInterpolator::UnivariateInterpolator(
    const std::vector<float> &known_pts, const std::vector<float> &query_pts) {
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

void UnivariateInterpolator::estimate(const std::vector<float> &known_values,
                                      std::vector<float> &query_values) const {
    for (unsigned long idx = 0; idx < data_.size(); idx++) {
        const interp &q = data_[idx];
        query_values[idx] = known_values[q.idxlow] * (1 - q.scale) +
                            known_values[q.idxhigh] * q.scale;
    }
}
