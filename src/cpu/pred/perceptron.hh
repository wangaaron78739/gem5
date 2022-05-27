#ifndef _PERCEPTRON_
#define _PERCEPTRON_

#include <stdlib.h>

#include <vector>

#include "base/types.hh"
#include "cpu/pred/bpred_unit.hh"
// #include "base/sat_counter.hh"
#include "params/Perceptron.hh"

namespace gem5 {
namespace branch_prediction {
class Perceptron : public BPredUnit {
 public:
  Perceptron(const PerceptronParams &params);
  bool lookup(ThreadID tid, Addr branch_addr, void *&bp_history);
  void uncondBranch(ThreadID tid, Addr pc, void *&bp_history);
  void btbUpdate(ThreadID tid, Addr branch_addr, void *&bp_history);
  void update(ThreadID tid, Addr branch_addr, bool taken, void *bp_history,
              bool squashed, const StaticInstPtr &inst, Addr corrTarget);
  void squash(ThreadID tid, void *bp_history);

 private:
  unsigned K;
  std::vector<unsigned> GH;
  unsigned ghBits;
  unsigned ghMask;
  unsigned hrMask;
  unsigned N;
  unsigned theta;
  std::vector<std::vector<unsigned>> W;
  struct BPHistory {
    unsigned GH;
    bool globalPredTaken;
    bool globalUsed;
  };
};
}  // namespace branch_prediction
}  // namespace gem5

#endif