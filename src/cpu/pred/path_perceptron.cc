#include "cpu/pred/path_perceptron.hh"

#include <iostream>

#include "base/bitfield.hh"
#include "base/intmath.hh"

namespace gem5 {
namespace branch_prediction {
PathPerceptron::PathPerceptron(const PathPerceptronParams &params)
    : BPredUnit(params),
      K(params.globalPredictorSize),
      GH(params.numThreads, 0),
      SH(params.numThreads, 0),
      ghBits(ceilLog2(params.globalPredictorSize)) {
  if (!isPowerOf2(K)) {
    fatal("Invalid global predictor size!\n");
  } 
  ghMask = K - 1; 
  hrMask = mask(ghBits); 
  if (ghMask > hrMask)
    fatal("Global predictor too large for global history bits!\n"); 
  SR.assign(K + 1, 0); 
  GR.assign(K + 1, 0); 
  N = 10; 
  theta = 2.14 * (K + 1) + 20.58; 
  W.assign(N, std::vector<unsigned>(K + 1, 0)); 
  maxW = (1 << (ghBits - 1)) - 1;
  minW = -(maxW + 1);
}

void PathPerceptron::btbUpdate(ThreadID tid, Addr branch_addr,
                               void *&bp_history) {
  GH[tid] &= (hrMask & ~1ULL);
}

void PathPerceptron::updatePath(Addr branch_addr) {
  path.insert(path.begin(), branch_addr);
  if (path.size() > (K + 1)) path.pop_back();
}

unsigned PathPerceptron::saturatedUpdate(unsigned weight, bool inc) {
  if (inc && (weight < maxW))
    return weight + 1;
  if (!inc && (weight > minW))
    return weight - 1;
  return weight;
}

bool PathPerceptron::lookup(ThreadID tid, Addr branch_addr, void *&bp_history) {
  updatePath(branch_addr); 
  unsigned k_j; 
  int curr = branch_addr % N;
  int pred = W[curr][0] + SR[K];
  bool result = (pred >= 0);

  BPHistory *history = new BPHistory;
  history->GH = SH[tid];
  history->globalPredTaken = result;
  bp_history = (void *)history;

  std::vector<unsigned> newSR;
  newSR.assign(K + 1, 0);

  for (int j = 1; j <= K; j++) {
    k_j = K - j;
    newSR[k_j + 1] = SR[k_j] + (result) ? W[curr][j] : -W[curr][j];
  }

  SR = newSR;
  SR[0] = 0;

  SH[tid] = ((SH[tid] << 1) | result);
  SH[tid] = (SH[tid] & hrMask);
  return result;
}

void PathPerceptron::uncondBranch(ThreadID tid, Addr pc, void *&bp_history) {
  BPHistory *history = new BPHistory;
  history->GH = SH[tid];
  history->globalPredTaken = true;
  history->globalUsed = true;
  bp_history = static_cast<void *>(history);

  updatePath(pc);
  SH[tid] = ((SH[tid] << 1) | 1);
  SH[tid] &= hrMask;
}

void PathPerceptron::update(ThreadID tid, Addr branch_addr, bool taken,
                            void *bp_history, bool squashed,
                            const StaticInstPtr &inst, Addr corrTarget) {
  assert(bp_history);
  unsigned k, k_j;
  int curr = branch_addr % N;
  int pred = W[curr][0] + SR[K]; 
  unsigned thread_history = SH[tid]; 
  std::vector<unsigned> newR;
  newR.assign(K + 1, 0); 
  for (int j = 1; j <= K; j++) {
    k_j = K - j;
    newR[k_j + 1] = GR[k_j] + taken ? W[curr][j] : -W[curr][j];
  }

  GR = newR;
  GR[0] = 0; 
  GH[tid] = ((GH[tid] << 1) | taken);
  GH[tid] &= hrMask;

  if (squashed || (abs(pred) <= theta)) {
    if (squashed) {
      SH[tid] = GH[tid];
      SR = GR;
    }

    W[curr][0] = saturatedUpdate(W[curr][0], taken);
    for (int j = 1; j <= K; j++) {
      k = (path[j % path.size()] % N);
      W[k][j] = saturatedUpdate(W[k][j], ((thread_history >> j) & 1) == taken);
    }
  }
}

void PathPerceptron::squash(ThreadID tid, void *bp_history) {
  BPHistory *history = static_cast<BPHistory *>(bp_history);
  SH[tid] = GH[tid];
  SR = GR;
  delete history;
}

}  // namespace branch_prediction
}  // namespace gem5