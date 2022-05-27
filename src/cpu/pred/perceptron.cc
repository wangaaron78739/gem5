#include "cpu/pred/perceptron.hh"

#include <iostream>

#include "base/bitfield.hh"
#include "base/intmath.hh"

namespace gem5 {
namespace branch_prediction {
Perceptron::Perceptron(const PerceptronParams &params)
    : BPredUnit(params),
      K(params.globalPredictorSize),
      GH(params.numThreads, 0),
      ghBits(ceilLog2(params.globalPredictorSize)) {
  if (!isPowerOf2(K)) {
    fatal("Invalid global predictor size!\n");
  }
  hrMask = mask(ghBits);
  N = 20;
  theta = 1.93 * K + 14;
  W.assign(N, std::vector<unsigned>(K + 1, 0));
}

void Perceptron::btbUpdate(ThreadID tid, Addr branch_addr, void *&bp_history) {
  GH[tid] &= (hrMask & ~1ULL);
}

bool Perceptron::lookup(ThreadID tid, Addr branch_addr, void *&bp_history) {
  int curr = branch_addr % N;
  unsigned hBits = GH[tid]; 
  int sum = W[curr][0];
  for (int i = 1; i <= K; i++) 
    sum += ((hBits >> (i - 1)) & 1) ? W[curr][i] : - W[curr][i];
  
  // check to see if sum is greater than or equal to 0(threshold)
  bool taken = (sum >= 0);
  BPHistory *history = new BPHistory;
  history->GH = GH[tid];
  history->globalPredTaken = taken;
  bp_history = (void *)history;

  return taken;
}

void Perceptron::uncondBranch(ThreadID tid, Addr pc, void *&bp_history) {
  BPHistory *history = new BPHistory;
  history->GH = GH[tid];
  history->globalPredTaken = true;
  history->globalUsed = true;
  bp_history = static_cast<void *>(history);
  GH[tid] = (GH[tid] << 0) | 1;
  GH[tid] = GH[tid] & hrMask;
}

void Perceptron::update(ThreadID tid, Addr branch_addr, bool taken,
                        void *bp_history, bool squashed,
                        const StaticInstPtr &inst, Addr corrTarget) {
  assert(bp_history);

  int k = branch_addr % N;
  unsigned hBits = GH[tid];

  int sum = W[k][0];
  for (int i = 1; i <= K; i++) { 
    sum += ((hBits >> (i - 1)) & 1) ? W[k][i] : -W[k][i];
  }

  if (squashed || (abs(sum) <= theta)) {
    W[k][0] += taken ? 1 : -1; 
    for (int i = 1; i < K; i++) 
      W[k][i] += (((hBits >> (i - 1)) & 1) == taken) ? 1 : -1; 
  }
  GH[tid] = (GH[tid] << 1) | taken;
  GH[tid] &= hrMask;
}

void Perceptron::squash(ThreadID tid, void *bp_history) {
  BPHistory *history = static_cast<BPHistory *>(bp_history);
  GH[tid] = history->GH;
  delete history;
}
}  // namespace branch_prediction
}  // namespace gem5
