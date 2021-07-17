import os
import re
c_path = "/home/hengkai/Desktop/file-FILE5_32/install/o3/fuzz/chain/"
log_path = "/home/hengkai/Desktop/file-FILE5_32/install/o0/fuzz/file-o3.log"

def main():
    target_id = []
    chain = []
    target_chain = {}
    log_file = open(log_path, "r")
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
                        chain.append("N")
                    else:
                        chain.append("Y")
            if "N" in chain:
                target_chain[int(dir)] = chain
                chain = []
            else:
                chain = []
                
    for key in sorted(target_chain.keys()):
        print(key, target_chain[key])

if __name__ == '__main__':
    main()