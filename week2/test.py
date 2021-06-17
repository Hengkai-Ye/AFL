result = "./result"
my_result = open(result, "a")
my_result.write("No-")
my_result.write("Yes-")
my_result.write("\n")
my_result.close()
my_result = open(result, "a")
my_result.write("Yes-")