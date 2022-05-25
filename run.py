import os
import itertools

# SimObject('BranchPredictor.py', sim_objects=[
#     'IndirectPredictor', 'SimpleIndirectPredictor', 'BranchPredictor',
#     'LocalBP', 'TournamentBP', 'BiModeBP', 'TAGEBase', 'TAGE', 'LoopPredictor',
#     'TAGE_SC_L_TAGE', 'TAGE_SC_L_TAGE_64KB', 'TAGE_SC_L_TAGE_8KB',
#     'LTAGE', 'TAGE_SC_L_LoopPredictor', 'StatisticalCorrector', 'TAGE_SC_L',
#     'TAGE_SC_L_64KB_StatisticalCorrector',
#     'TAGE_SC_L_8KB_StatisticalCorrector',
#     'TAGE_SC_L_64KB', 'TAGE_SC_L_8KB', 'MultiperspectivePerceptron',
#     'MultiperspectivePerceptron8KB', 'MultiperspectivePerceptron64KB',
#     'MPP_TAGE', 'MPP_LoopPredictor', 'MPP_StatisticalCorrector',
#     'MultiperspectivePerceptronTAGE', 'MPP_StatisticalCorrector_64KB',
#     'MultiperspectivePerceptronTAGE64KB', 'MPP_TAGE_8KB',
#     'MPP_LoopPredictor_8KB', 'MPP_StatisticalCorrector_8KB',
#     'MultiperspectivePerceptronTAGE8KB',
#     'AlwaysBP'])
BP_NAMES = [
    'Perceptron', 'PathPerceptron'
]
# BP_NAMES = [
#     'AlwaysBP','Perceptron', 'PathPerceptron', 'BiModeBP', 'LTAGE', 'LocalBP', 'MultiperspectivePerceptron64KB', 'MultiperspectivePerceptron8KB', 'MultiperspectivePerceptronTAGE64KB', 'MultiperspectivePerceptronTAGE8KB', 'TAGE', 'TAGE_SC_L_64KB', 'TAGE_SC_L_8KB', 'TournamentBP'
# ]
BENCHMARKS = ["Bubblesort", "IntMM", "Oscar", "Perm", "Puzzle", "Queens", "Quicksort", "RealMM", "Towers", "Treesort"]
STATS_FILE = "m5out/stats.txt"
COMMAND = "./build/X86/gem5.opt ./configs/example/se.py --bp-type={} -c ./tests/test-progs/test-suite/SingleSource/Benchmarks/Stanford/{}"

STATS_MAPPING = {"cond_misses": "system.cpu.branchPred.condIncorrect", 
                 "indirect_misses": "system.cpu.branchPred.indirectMispredicted",
                 "host_seconds": "hostSeconds",
                 "sim seconds": "simSeconds",
                 "cond_num": "system.cpu.branchPred.condPredicted",
                 "indirect_num": "system.cpu.branchPred.indirectLookups",
                 "instruction_num": "simInsts",
                 "lookup_num": "system.cpu.branchPred.lookups"}


if __name__ == "__main__":
    for bp, benchmark in itertools.product(BP_NAMES, BENCHMARKS):
        os.system(COMMAND.format(bp, benchmark))
        stats_file = open(STATS_FILE, "r").readlines()

        stats = {name:[l.strip() for l in stats_file if stat in l][0].split()[1] for name, stat in STATS_MAPPING.items()}
    
        print(f"===============================================")
        print(f"Completed {bp} for {benchmark}")
        print(f"===============================================")        
        print(stats)
        with open("results/{}_{}.txt".format(bp, benchmark), "w") as f: 
            for stat, value in stats.items():
                f.write("{} : {}\n".format(stat, value)) 
            conditional_misses = float(stats["cond_misses"])
            number_of_instructions = float(stats["instruction_num"])
            number_of_conditionals = float(stats["lookup_num"])
            f.write("mpki: {0:.4f}\n".format((conditional_misses / number_of_instructions) * 1000))
            f.write("accuracy: {0:.4f}\n".format(1 - conditional_misses/number_of_conditionals))
        