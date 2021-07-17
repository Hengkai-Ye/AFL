import os

log_path = "/home/hengkai/Desktop/AFL/llvm_mode/trace/file-o3.log"
cxx_path = "/home/hengkai/Desktop/file-FILE5_32/install/o3/fuzz/file-o3.log"
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


'''
flag = 1
while 1:
    line = cxx_file.readline()
    if not line:
        break
    if(line.split(":")[0] == "id"):
        if(flag == 0):
            print(line.split(":")[1])
        if(flag == 1):
            hit += 1
        total += 1
        flag = 0
    if(line.split(":")[1] == "NB\n" or line.split(":")[1] == "HC\n"):
        id = line.split(":")[0]
        if id in log_list:
            flag = 1
'''
print(hit)
print(total)