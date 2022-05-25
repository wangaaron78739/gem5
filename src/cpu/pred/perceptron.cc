#include "cpu/pred/perceptron.hh"

#include <iostream>

#include "base/bitfield.hh"
#include "base/intmath.hh"

namespace gem5 {
namespace branch_prediction {
Perceptron::Perceptron(const PerceptronParams &params)
    : BPredUnit(params),
      globalPredictorSize(params.globalPredictorSize),
      globalHistory(params.numThreads, 0),
      globalHistoryBits(ceilLog2(params.globalPredictorSize)) {
  if (!isPowerOf2(globalPredictorSize)) {
    fatal("Invalid global predictor size!\n");
  }
  historyRegisterMask = mask(globalHistoryBits);
  perceptronNum = 20;
  theta = 1.93 * globalPredictorSize + 14;  // based on paper for threshold
  weights.assign(perceptronNum,
                 std::vector<unsigned>(globalPredictorSize + 1, 0));
}

void Perceptron::updateGlobalHistTaken(ThreadID tid) {
  globalHistory[tid] = (globalHistory[tid] << 1) | 1;
  globalHistory[tid] = globalHistory[tid] & historyRegisterMask;
}

void Perceptron::updateGlobalHistNotTaken(ThreadID tid) {
  globalHistory[tid] = (globalHistory[tid] << 1);
  globalHistory[tid] = globalHistory[tid] & historyRegisterMask;
}

void Perceptron::btbUpdate(ThreadID tid, Addr branch_addr, void *&bp_history) {
  globalHistory[tid] &= (historyRegisterMask & ~1ULL);
}

bool Perceptron::lookup(ThreadID tid, Addr branch_addr, void *&bp_history) {
  int currentPerceptron = branch_addr % perceptronNum;
  unsigned thread_history = globalHistory[tid];

  int sum = weights[currentPerceptron][0];
  for (int i = 1; i <= globalPredictorSize; i++) {
    if ((thread_history >> (i - 1)) & 1)
      sum += weights[currentPerceptron][i];
    else
      sum -= weights[currentPerceptron][i];
  }
  // check to see if sum is greater than or equal to 0(threshold)
  bool taken = (sum >= 0);
  BPHistory *history = new BPHistory;
  history->globalHistory = globalHistory[tid];
  history->globalPredTaken = taken;
  bp_history = (void *)history;

  return taken;
}

void Perceptron::uncondBranch(ThreadID tid, Addr pc, void *&bp_history) {
  BPHistory *history = new BPHistory;
  history->globalHistory = globalHistory[tid];
  history->globalPredTaken = true;
  history->globalUsed = true;
  bp_history = static_cast<void *>(history);
  updateGlobalHistTaken(tid);
}

void Perceptron::update(ThreadID tid, Addr branch_addr, bool taken,
                        void *bp_history, bool squashed,
                        const StaticInstPtr &inst, Addr corrTarget) {
  assert(bp_history);

  int k = branch_addr % perceptronNum;
  unsigned thread_history = globalHistory[tid];

  int sum = weights[k][0];
  for (int i = 1; i <= globalPredictorSize; i++) {
    if ((thread_history >> (i - 1)) & 1)
      sum += weights[k][i];
    else
      sum -= weights[k][i];
  }

  if (squashed || (abs(sum) <= theta)) {
    if (taken)
      weights[k][0] += 1;
    else
      weights[k][0] -= 1;

    for (int i = 1; i < globalPredictorSize; i++) {
      if (((thread_history >> (i - 1)) & 1) == taken)
        weights[k][i] += 1;
      else
        weights[k][i] -= 1;
    }
  }
  globalHistory[tid] = (globalHistory[tid] << 1) | taken;
  globalHistory[tid] &= historyRegisterMask;
}

void Perceptron::squash(ThreadID tid, void *bp_history) {
  BPHistory *history = static_cast<BPHistory *>(bp_history);
  globalHistory[tid] = history->globalHistory;
  delete history;
}
}  // namespace branch_prediction

// branch_prediction::Perceptron *PerceptronParams::create() const {
//   return new branch_prediction::Perceptron(*this);
// }
}  // namespace gem5
