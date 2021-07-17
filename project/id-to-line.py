logo0_path = "/home/hengkai/Desktop/file-FILE5_32/install/o3/fuzz/file-o3.log"
logo3_path = "/home/hengkai/Desktop/file-FILE5_32/install/o0/fuzz/file-o3.log"
logtrace_path = "/home/hengkai/Desktop/AFL/llvm_mode/trace/file-o3.log"
id_list = []
branch_list = []
line_list = []
logo3_file = open(logo3_path, "r")
logo0_file = open(logo0_path, "r")
logtrace_file = open(logtrace_path, "r")
for line in logo3_file:
    if(line.split(":")[1] == "No-New-Branch\n"):
        id_list.append(line.split(":")[0])

count = 0
while 1:
    line = logo0_file.readline()
    target_str = "0" + id_list[count] + ",src"
    if not line:
        break
    if target_str in line:
        line = logo0_file.readline()
        #branch_list.append(line.split(":")[0])
        end_str = ",src"
        while(end_str not in line):
            branch_list.append(line.split(":")[0])
            last_line = logo0_file.tell()
            line = logo0_file.readline()
            
        if count < len(id_list) - 1:
            count += 1
            if(int(id_list[count]) == (int(id_list[count - 1]) + 1)):
                logo0_file.seek(last_line)

print(id_list)
print(branch_list)
branch_list = list(set(branch_list))
print(branch_list)
for line in logtrace_file:
    if(line.split(":")[1] in branch_list):
        line_list.append(line)
print(sorted(line_list))
print(len(line_list))
