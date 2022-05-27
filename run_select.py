import os
import sys

BP_NAMES = [
    'AlwaysBP','Perceptron', 'PathPerceptron', 'BiModeBP', 'LocalBP', 'MultiperspectivePerceptron64KB', 'TAGE', 'TournamentBP'
]
BENCHMARKS = ["Bubblesort", "IntMM", "Oscar", "Perm", "Puzzle", "Queens", "Quicksort", "RealMM", "Towers", "Treesort"]
STATS_FILE = "m5out/stats.txt"
COMMAND = "./build/X86/gem5.opt ./configs/example/se.py --bp-type={} --cpu-type=O3CPU --l1d_size=64kB --l1i_size=16kB --caches -c ./tests/test-progs/test-suite/SingleSource/Benchmarks/Stanford/{} > output/{}_{}.txt 2>&1"

STATS_MAPPING = {"cond_misses": "system.cpu.branchPred.condIncorrect", 
                 "indirect_misses": "system.cpu.branchPred.indirectMispredicted",
                 "host_seconds": "hostSeconds",
                 "sim seconds": "simSeconds",
                 "cond_num": "system.cpu.branchPred.condPredicted",
                 "indirect_num": "system.cpu.branchPred.indirectLookups",
                 "instruction_num": "simInsts",
                 "lookup_num": "system.cpu.branchPred.lookups"}


if __name__ == "__main__":
    bp = BP_NAMES[int(sys.argv[1])]
    benchmark = BENCHMARKS[int(sys.argv[2])]
    print(bp, benchmark)
    os.system(COMMAND.format(bp, benchmark, bp, benchmark))
    stats_file = open(STATS_FILE, "r").readlines()

    stats = {name:[l.strip() for l in stats_file if stat in l][0].split()[1] for name, stat in STATS_MAPPING.items()}

    print(f"Done: {bp}-{benchmark}")
    print(stats)
    with open("results/{}_{}.txt".format(bp, benchmark), "w") as f: 
        for stat, value in stats.items():
            f.write("{} : {}\n".format(stat, value)) 
        conditional_misses = float(stats["cond_misses"])
        number_of_instructions = float(stats["instruction_num"])
        number_of_conditionals = float(stats["lookup_num"])
        f.write("mpki: {0:.4f}\n".format((conditional_misses / number_of_instructions) * 1000))
        f.write("accuracy: {0:.4f}\n".format(1 - conditional_misses/number_of_conditionals))
    