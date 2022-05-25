#ifndef __CPU_PRED_ALWAYS_PRED_HH__
#define __CPU_PRED_ALWAYS_PRED_HH__

#include <vector>

#include "base/types.hh"
#include "cpu/pred/bpred_unit.hh"
// #include "base/sat_counter.hh"
#include "params/AlwaysBP.hh"

namespace gem5 {
namespace branch_prediction {
class AlwaysBP : public BPredUnit
{
  public:
    AlwaysBP(const AlwaysBPParams &params); 
    void uncondBranch(ThreadID tid, Addr pc, void * &bp_history) { }
    bool lookup(ThreadID tid, Addr branch_addr, void * &bp_history) { return true; }
    void btbUpdate(ThreadID tid, Addr branch_addr, void * &bp_history) { }
    void update(ThreadID tid, Addr branch_addr, bool taken, void *bp_history,
                bool squashed, const StaticInstPtr &inst, Addr corrTarget) {}
    void squash(ThreadID tid, void *bp_history) { assert(bp_history == NULL); }
};
}
}

#endif // __CPU_PRED_2BIT_LOCAL_PRED_HH__