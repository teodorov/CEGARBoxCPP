file = "k_d4_n.1360.intohylo"

with open("haskell_"+file,"w") as write_file:
  with open(file, "r") as read_file:
    write_file.write(read_file.read().replace("$",""))