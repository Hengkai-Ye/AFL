import pickle
import argparse
from subprocess import *
from collections import defaultdict

def llvm_compile(code1, code2, branch_path1, branch_path2):
    prog1 = code1[:-2]
    prog2 = code2[:-2]
    my_stdout = open(branch_path1, "w")
    call(['../afl-clang-fast', code1, '-o', prog1], stderr=my_stdout)
    my_stdout.close()
    my_stdout = open(branch_path2, "w")
    call(['../afl-clang-fast', code2, '-o', prog2], stderr=my_stdout)
    my_stdout.close()

def log_to_pickle(branch_path):
    branch_dict = defaultdict(list)
    my_stdout = open(branch_path, "r")
    for line in my_stdout:
        branch_id = line.split(":")[0]
        src_line = line.split(":")[1]
        dst_line = line.split(":")[2]
        branch_dict[branch_id].append(src_line)
        branch_dict[branch_id].append(dst_line)
    my_stdout.close()
    with open(branch_path+'.pkl', 'wb') as p:
        pickle.dump(branch_dict, p)



def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('-code1', type=str, default='./source-code/demo1.c', help='path to the first source code')
    parser.add_argument('-code2', type=str, default='./source-code/demo2.c', help='path to the second source code')
    args = parser.parse_args()
    branch_path1 = "./info/branch-info-1"
    branch_path2 = "./info/branch-info-2"
    llvm_compile(args.code1, args.code2, branch_path1, branch_path2)
    log_to_pickle(branch_path1)
    log_to_pickle(branch_path2)
if __name__=='__main__':
    main()