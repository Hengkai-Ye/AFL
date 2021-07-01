import pickle
pklfile = open('./info/branch-info-2.pkl', 'rb')
test = pickle.load(pklfile)
print(test.get("58835")[0])