import os
log_path = "/home/hengkai/Desktop/AFL/llvm_mode/trace-coreutils.log"
cxx_path = "/home/hengkai/Desktop/coreutils-8.32/install/chain2-1.log"
total = 0
hit = 0
log_list = []
log_file = open(log_path, "r")
hit_list = []
for line in log_file:
    log_list.append(line.split(":")[1])
cxx_file = open(cxx_path, "r")
for line in cxx_file:
    if(line.split(":")[1] == "NB\n"):
        total += 1
        id = line.split(":")[0]
        if id in log_list:
            hit += 1
            hit_list.append(id)
print(hit)
print(total)

