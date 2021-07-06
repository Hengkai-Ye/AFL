import os
import shutil
#q_path = "/home/hengkai/Desktop/libming-CVE-2018-8807/install/queue/"
#c_path = "/home/hengkai/Desktop/libming-CVE-2018-8807/install/chain/"
q_path = "/home/hengkai/Desktop/coreutils-8.32/install/queue2/"
c_path = "/home/hengkai/Desktop/coreutils-8.32/install/chain2/"
def main():
    chain_id = 1
    q_list = [0] * 400
    q_files = os.listdir(q_path)
    q_files.sort()
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
if __name__ == '__main__':
    main()