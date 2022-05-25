#include "cpu/pred/always.hh"

#include "base/intmath.hh"
#include "base/trace.hh"

namespace gem5 {
namespace branch_prediction {

AlwaysBP::AlwaysBP(const AlwaysBPParams &params)
    : BPredUnit(params)
{ }

}
}
