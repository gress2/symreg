import glob
import sys

if len(sys.argv) < 3:
    print("Requires two command line arguments")
    exit()

masters = sorted(glob.glob(sys.argv[1] + "_rankings_*"))
students = sorted(glob.glob(sys.argv[2] + "_rankings_*"))

sum_tau = 0

for z in range(len(masters)):
    master_file = masters[z]
    student_file = students[z]

    with open(master_file, 'r') as f:
        master_rankings = [int(x) for x in f.readlines()]

    with open(student_file, 'r') as f:
        student_rankings = [int(x) for x in f.readlines()]

    s = []

    for i in range(len(student_rankings)):
        s.append((student_rankings[i], i))

    s.sort(key=lambda elem: master_rankings[elem[1]])

    s = [x[0] for x in s]

    concordant = 0
    discordant = 0

    for i in range(len(s)):
        cur = s[i]
        for j in range(i + 1, len(s)):
            if s[j] >= cur:
                concordant += 1
            else:
                discordant += 1

    kendalls_tau = (concordant - discordant) / (concordant + discordant)
    sum_tau += kendalls_tau
    print(kendalls_tau)

print("Average: " + str(sum_tau / len(masters)))
