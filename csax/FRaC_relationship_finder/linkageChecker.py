#! python

import csv
import re
import sys
import random


for arg in sys.argv:
  if(arg == "-h" or arg == "--help"):
    print("Linkage Disequilibrium Detector usage:")
    print("Please provide exactly two arguments, the chromosome lookup file and the FRaC relationship finder output graph.  The chromosome lookup file should be a TSV where the first column contains SNP names, and the second icontains chromosome numbers.")
    exit(0)

if(len(sys.argv) != 3):
  print("Error: Please provide exactly two arguments (SNP chromosome lookup and relationship file).")


snpInfoPath = sys.argv[1]
pairsPath = sys.argv[2]

chromosomeLookup = dict()
usedSNPs = []

nameCol = 0
chromosomeCol = 1

with open(snpInfoPath) as snpinfo:
  snpInfoReader = csv.reader(snpinfo, delimiter="\t")
  for row in snpInfoReader:
    if(len(row) == 0 or row[0][0] == "#"):
      continue
    chromosomeLookup[row[nameCol]] = row[chromosomeCol]

sames = 0
differents = 0
weightedSames = 0
weightedDifferents = 0

with open(pairsPath) as allpairs:
  pairReader = csv.reader(allpairs, delimiter="\t")
  for row in pairReader:
    if(len(row) == 2):
      #This is a node, take the SNP
      usedSNPs.append(row[0])
    if(len(row) == 3):
      #This is an edge, determine if the chromosomes are linked, count it, and take the weight.
      if(chromosomeLookup[row[0]] == chromosomeLookup[row[1]]):
        sames += 1
        weightedSames += float(row[2])
      else:
        differents += 1
        weightedDifferents += float(row[2])

total = sames + differents
weightedTotal = weightedSames + weightedDifferents

actualRatio = sames / float(total)
actualWeightedRatio = weightedSames / float(weightedTotal)

print("Found " + str(sames) + " / " + str(total) + " = " + str(actualRatio) + " identical chromosome matches.")
print("Found " + str(weightedSames) + " / " + str(weightedTotal) + " = " + str(actualWeightedRatio) + " identical chromosome match weight sum.")
print("Expected without linkage disequilibrium lower bound " + str(1.0/23.0))

def calcExpectedNoLinkage(snps, reps):
  sames = 0
  differents = 0
  
  for i in range(0,reps):
    a = random.choice(snps)
    b = random.choice(snps)
    while(b == a):
      b = random.choice(snps) #Sampling without replacement.

    if(chromosomeLookup[a] == chromosomeLookup[b]):
      sames += 1
    else:
      differents += 1
  total = sames + differents
  ratio = float(sames) / float(differents)
  return (sames, differents, total, ratio)

repCount = 10000
(sames, differents, total, randFlagRatio) = calcExpectedNoLinkage(usedSNPs, repCount)

print("Random choice (x" + str(repCount) + ") on flagged SNPs yields " + str(sames) + " / " + str(total) + " = " + str(randFlagRatio) + " identical chromosome matches.")

sames = 0
differents = 0

allSNPs = list(chromosomeLookup.keys())

(sames, differents, total, randAllRatio) = calcExpectedNoLinkage(allSNPs, repCount)

print("Random choice (x" + str(repCount) + ") on all SNPs yields (removes selection bias) " + str(sames) + " / " + str(total) + " = " + str(sames / float(total)) + " identical chromosome matches.")

print("To correct for linkage disequilibrium bias, multiply each linked edge by the expected match linked ratio over the actual weighted lined ratio (" + str(randFlagRatio) + " / " + str(actualWeightedRatio) + " = " + str(randFlagRatio / actualWeightedRatio) + ")")
