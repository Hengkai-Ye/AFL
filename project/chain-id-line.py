import os
import re
import shutil
import argparse

def gen_chain(c_path, q_path):
    chain_id = 1
    q_files = os.listdir(q_path)
    q_files.sort()
    q_list = [0] * len(q_files)
    max_id = len(q_files) - 1
    for i in range(max_id,1,-1): 
        if(q_list[i] != 1):
            chain_path = c_path + str(chain_id) + '/'
            os.mkdir(chain_path)
            id = int(q_files[i][3:9])
            src = int(q_files[i][14:20])
            shutil.copyfile(q_path + q_files[i], chain_path + q_files[i])
            q_list[i] = 1
            while(src != 0):
                shutil.copyfile(q_path + q_files[src + 1], chain_path + q_files[src + 1])
                q_list[src + 1] = 1
                id = int(q_files[src + 1][3:9])
                src = int(q_files[src + 1][14:20])
            shutil.copyfile(q_path + q_files[1], chain_path + q_files[1])
            chain_id += 1    

def print_chain(c_path, pdr2_path):
    target_id = []
    chain = []
    target_chain = {}
    log_file = open(pdr2_path, "r")
    for line in log_file:
        if(line.split(":")[1] == "No-New-Branch\n"):
            target_id.append('0' * (6 - len(line.split(":")[0])) + line.split(":")[0])

    for root, dirs, files in os.walk(c_path):
        for dir in dirs:
            sub_chain = os.path.join(root,dir)
            for sub_root, sub_dirs, sub_files in os.walk(sub_chain):
                for i in sorted(sub_files):
                    id = re.split('[:,]', i)[1]
                    if id in target_id:
                        chain.append(1)
                    else:
                        chain.append(0)
            if 1 in chain:
                target_chain[int(dir)] = chain
                chain = []
            else:
                chain = []
                
    return target_chain

def get_source(pdr1_path, inst_path, chain_path, target_chain):
    id_dict = {}
    branch_dict = {}
    code_dict = {}
    total_id_list = []
    pdr1_file = open(pdr1_path, "r")
    inst_file = open(inst_path, "r")
    for key in sorted(target_chain.keys()):
        chain_id_list = []
        for i, v in enumerate(target_chain[key]):
            if(v):
                c_files = os.listdir(chain_path + str(key) + '/')
                c_files.sort()
                chain_id_list.append(c_files[i][:9])
                total_id_list.append(c_files[i][:9])
                id_dict[key] = chain_id_list
    total_id_list.sort()
    count = 0
    while 1:
        line = pdr1_file.readline()
        target_str = total_id_list[count]
        if not line:
            break
        if target_str in line:
            branch_list = []
            line = pdr1_file.readline()
            end_str = ",src"
            while(end_str not in line):
                branch_list.append(line.split(":")[0])
                last_line = pdr1_file.tell()
                line = pdr1_file.readline()

            branch_dict[total_id_list[count]] = branch_list

            if count < len(total_id_list) - 1:
                count += 1
                if(int(total_id_list[count].split(":")[1]) == (int(total_id_list[count - 1].split(":")[1]) + 1)):
                    pdr1_file.seek(last_line)

    for key in sorted(id_dict.keys()):
        print(key, id_dict[key])

    for key in sorted(branch_dict.keys()):
        code_list = []
        for branch_id in branch_dict[key]:
            for line in inst_file:
                if(branch_id == line.split(":")[1]):
                    code_list.append(line[5:-1])
                    inst_file.seek(0,0)
                    break
            
        code_dict[key] = code_list

    for key in sorted(code_dict.keys()):
        print(key, code_dict[key])

    
def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('-chain', type=str, default='./chain/', help='chain path')
    parser.add_argument('-queue', type=str, default='./output/queue/', help='queue path')
    parser.add_argument('-pdr1', type=str, help='pdr path for the fuzzed program')
    parser.add_argument('-pdr2', type=str, help='pdr path for the tested program')
    parser.add_argument('-inst', type=str, help='instrument path for the fuzzed program')
    args = parser.parse_args()
    #shutil.rmtree(args.chain)
    os.mkdir(args.chain)
    target_chain = {}
    gen_chain(args.chain, args.queue)
    target_chain = print_chain(args.chain, args.pdr2)
    for key in sorted(target_chain.keys()):
        print(key, target_chain[key])    
    get_source(args.pdr1, args.inst, args.chain, target_chain)

if __name__ == '__main__':
    main()