#To generate an fzn file
minizinc --compile --solver org.minizinc.mzn-fzn  -O1 -I mznlib/ benchmark/problem1.mzn benchmark/data1.dzn --fzn benchmark/fzn-problem1-data1.fzn
#example
minizinc --compile --solver org.minizinc.mzn-fzn  -O1 -I mznlib/ benchmark/monomatch.mzn benchmark/mono1.dzn --fzn benchmark/fzn-mono1.fzn
