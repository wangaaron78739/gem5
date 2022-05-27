#ifndef _PATHPERCEPTRON_
#define _PATHPERCEPTRON_

#include <vector>
#include <stdlib.h>

#include "base/types.hh"
#include "cpu/pred/bpred_unit.hh"
// #include "base/sat_counter.hh"
#include "params/PathPerceptron.hh"

namespace gem5 {
namespace branch_prediction {
class PathPerceptron : public BPredUnit
{
public:
  
  PathPerceptron(const PathPerceptronParams &params);
  bool lookup(ThreadID tid, Addr branch_addr, void * &bp_history);
  void uncondBranch(ThreadID tid, Addr pc, void * &bp_history);
  void btbUpdate(ThreadID tid, Addr branch_addr, void * &bp_history);
  void update(ThreadID tid, Addr branch_addr, bool taken, void *bp_history,
              bool squashed, const StaticInstPtr &inst, Addr corrTarget);
  void squash(ThreadID tid, void *bp_history);

private:
  
  
  
  unsigned K;
  std::vector<unsigned> GH;
  std::vector<unsigned> SH;
  std::vector<unsigned> GR;
  std::vector<unsigned> SR;
  std::vector<unsigned> path;
  unsigned ghBits;
  unsigned ghMask;
  unsigned hrMask;
  unsigned N;
  unsigned theta;
  unsigned maxW;
  unsigned minW;
  std::vector<std::vector<unsigned>> W;
  void updatePath(Addr branch_addr);
  struct BPHistory {
  	unsigned GH;
  	bool globalPredTaken;
  	bool globalUsed;
  };
  
  unsigned saturatedUpdate (unsigned weight, bool inc); 
  
};
}  // namespace branch_prediction
}

#endif