#include "domain.h"

namespace transcat {

    StopPair StopPair::Reverse() const noexcept {
        return {to, from};
    }

    size_t StopPairHasher::operator()(const StopPair &stop_pair) const {
        return reinterpret_cast<int64_t>(stop_pair.from) * 37 * 37 + reinterpret_cast<int64_t>(stop_pair.to) * 17;
    }

} // namespace transcat