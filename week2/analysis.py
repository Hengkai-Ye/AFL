import os
import time
import shutil
import argparse
from subprocess import *
op_path = "../output/queue/"
ip_path = "../input/"
chain_path = "./chain/"

def fuzz_one(prog, t): #fuzz one program and generate a chain
    shutil.rmtree(chain_path)
    os.mkdir(chain_path)
    p = Popen(['../afl-fuzz', '-i', '../input', '-o', '../output', prog])
    time.sleep(t*60)
    p.kill()
    op_files = os.listdir(op_path)
    op_files.sort()
    chain_ele = op_files[-1]
    while(chain_ele[-5:] != "input"):
        id = chain_ele[3:9]
        src = chain_ele[14:20]
        shutil.copyfile(op_path+chain_ele, chain_path+chain_ele)
        for i in range(len(op_files)):
            if(op_files[i][3:9] == src):
                chain_ele = op_files[i]
                break
    shutil.copyfile(op_path+chain_ele, chain_path+chain_ele)

def comparision(prog, result):
    target_str = "no-new-path"
    my_result = open(result, "a")
    ip_files = os.listdir(chain_path)
    ip_files.sort()
    index_record = []
    shutil.rmtree(ip_path)
    os.mkdir(ip_path)
    for ip_file in ip_files:
        shutil.copyfile(chain_path+ip_file, ip_path+ip_file)
    my_stdout = open("./stdout.txt", "w")
    call(['../afl-fuzz', '-r', '-i', '../input', '-o', '../output', prog], stdout=my_stdout)
    my_stdout.close()
    my_stdout = open("./stdout.txt", "r")
    for line in my_stdout:
        if target_str in line:
            index_record.append(int(line.split(':')[0]))
    for i in range(len(ip_files)):
        if i in index_record:
            my_result.write("No-")
            #print("No-", end='')
        else:
            my_result.write("Yes-")
            #print("Yes-", end='')
    my_result.write("\n")
    my_result.close()

def refresh_input(ip_seed):
    shutil.rmtree(ip_path)
    os.mkdir(ip_path)
    shutil.copyfile(ip_seed, ip_path+'input')

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('-prog1', type=str, default='../fuzz-file/demo2', help='path to first program')
    parser.add_argument('-prog2', type=str, default='../fuzz-file/demo3', help='path to second program')
    parser.add_argument('-t', type=int, default=5, help='how many minutes to fuzz')
    parser.add_argument('-s', type=str, default='./input_seed', help='input_seed')
    parser.add_argument('-r', type=str, default='./result', help='result')
    args = parser.parse_args()

    my_result = open(args.r, "w")
    my_result.write("Performance of Chain from Program1\n")
    my_result.close()
    refresh_input(args.s)
    fuzz_one(args.prog1, args.t)
    comparision(args.prog2, args.r)

    my_result = open(args.r, "a")
    my_result.write("Performance of Chain from Program2\n")
    my_result.close()
    refresh_input(args.s)
    fuzz_one(args.prog2, args.t)
    comparision(args.prog1, args.r)
    


if __name__ == '__main__':
    main()
