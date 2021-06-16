import os
import shutil
output_path = "/home/hengkai/Desktop/output/queue/"
crash_path = "/home/hengkai/Desktop/output/crashes/"
chain_path = "./chain2/"
output_files = os.listdir(output_path)
output_files.sort()
crash_files = os.listdir(crash_path)
chain_ele = output_files[-1]
crash_ele = crash_files[-1]
shutil.copyfile(crash_path+crash_ele, chain_path+crash_ele.replace("000000", "999999", 1))
while(chain_ele[-5:] != "input"):
    id = chain_ele[3:9]
    src = chain_ele[14:20]
    shutil.copyfile(output_path+chain_ele, chain_path+chain_ele)
    for i in range(len(output_files)):
        if(output_files[i][3:9] == src):
            chain_ele = output_files[i]
            break
shutil.copyfile(output_path+chain_ele, chain_path+chain_ele)
