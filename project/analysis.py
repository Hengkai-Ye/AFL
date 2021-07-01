import os
import time
import shutil
import argparse
from subprocess import *
op_path = "../output/queue/"
ip_path = "../input/"

def fuzz_one(prog, t, chain_path): #fuzz one program and generate a chain
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

def comparision(prog, result, chain_path):
    target_str = "no-new-path"
    my_result = open(result, "a")
    ip_files = os.listdir(chain_path)
    ip_files.sort()
    index_record = []
    shutil.rmtree(ip_path)
    os.mkdir(ip_path)
    for ip_file in ip_files:
        shutil.copyfile(chain_path+ip_file, ip_path+ip_file)
    my_stdout = open("./info/stdout", "w")
    call(['../afl-fuzz', '-r', '-i', '../input', '-o', '../output', prog], stdout=my_stdout)
    my_stdout.close()
    my_stdout = open("./info/stdout", "r")
    for line in my_stdout:
        if target_str in line:
            index_record.append(int(line.split(':')[0]))
    for i in range(len(ip_files)):
        if i in index_record:
            my_result.write("N-")
            #print("No-", end='')
        else:
            my_result.write("Y-")
            #print("Yes-", end='')
    my_result.write("\n")
    my_result.close()

def refresh_input(ip_seed):
    shutil.rmtree(ip_path)
    os.mkdir(ip_path)
    shutil.copyfile(ip_seed, ip_path+'input')

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('-prog1', type=str, default='./source-code/demo1', help='path to the first program')
    parser.add_argument('-prog2', type=str, default='./source-code/demo2', help='path to the second program')
    parser.add_argument('-chain1', type=str, default='./chain1/', help='path to chain of first program')
    parser.add_argument('-chain2', type=str, default='./chain2/', help='path to chain of second program')
    parser.add_argument('-t', type=int, default=5, help='how many minutes to fuzz')
    parser.add_argument('-s', type=str, default='./info/input-seed', help='input-seed')
    parser.add_argument('-r', type=str, default='./info/result', help='result')
    args = parser.parse_args()

    my_result = open(args.r, "w")
    my_result.write("Performance of Chain1 in Program2\n")
    my_result.close()
    refresh_input(args.s)
    fuzz_one(args.prog1, args.t, args.chain1)
    comparision(args.prog2, args.r, args.chain1)

    my_result = open(args.r, "a")
    my_result.write("Performance of Chain2 in Program1\n")
    my_result.close()
    refresh_input(args.s)
    fuzz_one(args.prog2, args.t, args.chain2)
    comparision(args.prog1, args.r, args.chain2)
    


if __name__ == '__main__':
    main()
