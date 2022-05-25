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
  
  
  
  unsigned globalPredictorSize;
  std::vector<unsigned> globalHistory;
  std::vector<unsigned> selectiveGlobalHistory;
  std::vector<unsigned> globalRegister;
  std::vector<unsigned> selectiveGlobalRegister;
  std::vector<unsigned> path;
  unsigned globalHistoryBits;
  unsigned globalHistoryMask;
  unsigned historyRegisterMask;
  unsigned numberOfPerceptrons;
  unsigned theta;
  unsigned max_weight;
  unsigned min_weight;
  std::vector<std::vector<unsigned>> weights;
  void updatePath(Addr branch_addr);
  struct BPHistory {
	unsigned globalHistory;
	bool globalPredTaken;
	bool globalUsed;
  };
  
  unsigned saturatedUpdate (unsigned weight, bool inc); 
  
};
}  // namespace branch_prediction
}

#endif