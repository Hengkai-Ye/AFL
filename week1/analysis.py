import os
import time
import shutil
from subprocess import *

def isInterest():
    target_str = "2 favored,"
    my_stdout = open("./stdout.txt", "w")
    p = Popen(['../afl-fuzz', '-i', '../input', '-o', '../output', '../fuzz-file/demo1'], stdout=my_stdout)
    time.sleep(0.5)
    p.kill()
    my_stdout.close()
    my_stdout = open("./stdout.txt", "r")
    for line in my_stdout:
        if target_str in line:
            return 1 #interest file
    return 0 #non-interest file

def main():
    chain_path = "./chain2/"
    new_chain_path = "./chain1/"
    input = "../input/"
    anls_rs = open("./analysis_result.txt", "w")
    output_files = os.listdir(chain_path)
    output_files.sort()
    anls_rs.write("Yes-")
    i = 0
    j = 1
    shutil.copyfile(chain_path + output_files[i], new_chain_path + output_files[i])
    while(j < len(output_files)):
        shutil.rmtree(input)
        os.mkdir(input)
        shutil.copyfile(chain_path + output_files[i], input + output_files[i])
        shutil.copyfile(chain_path + output_files[j], input + output_files[j])
        flag = isInterest()
        if(flag):
            shutil.copyfile(chain_path + output_files[j], new_chain_path + output_files[j])
            anls_rs.write("Yes-")
            i += 1
            j += 1
        else:
            j += 1
            anls_rs.write("NO-")

if __name__ == '__main__':
    main()

