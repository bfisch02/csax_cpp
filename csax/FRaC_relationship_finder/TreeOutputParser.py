#! python3

import sys
import re
import csv
import statistics

#Constant definition

normalClassName = "norm"
featurePrefix = "SNP_A-"
aggregatorFunction = statistics.median
filterNonpositive = False
precision = "%0.6f"

def printerr(*args):
  sys.stderr.write(' '.join(map(str,args)) + '\n')

#Read command line arguments

givenPath = None
for arg in sys.argv[1:]:
  if(arg[0] == "-"):
    if(arg == "-F"):
      filterNonpositive = True
    elif(arg == "--median"):
      aggregatorFunction = statistics.median
    elif(arg == "--mean"):
      aggregatorFunction = statistics.mean
    elif(arg == "-q"):
      printerr = lambda *a: None
    elif(arg == "-h" or arg == "--help"):
      print("-F Filter nonpositive features and edges.  Most negative results are caused by sampling effects, and can thus be considered equivalent to 0 and ignored.")
      print("--median Use median surprisal values for each feature's overall surprisal (default).")
      print("--mean Use mean surprisal values for each feature's overall surprisal.")
      print("-q Don't log progress and other information to stderr.")
      print("--normal=ARG Set the normal class name to ARG.  All other classes are considered anomalous.")
      print("--prefix=ARG Set the feature prefix to ARG.  Features without this prefix will not be recognized by the graph builder.")
    elif(arg.startswith("--normal=")):
      normalClassName = arg.split("=", 1)[1]
    elif(arg.startswith("--prefix=")):
      featurePrefix = arg.split("=", 1)[1]
    else:
      sys.stderr.write("Error: Can't recognize option \"" + arg + "\".\n")
      exit(1)
  elif givenPath == None:
    givenPath = arg
  else:
    sys.stderr.write("Error: Can't handle argument \"" + arg + "\".  Make sre to provide only one path argument.\n")
    exit(1)

if(givenPath == None):
  sys.stderr.write("Please provide a path argument (May not start with \"-\").\n")
  exit(1)

#Resolve filepaths
dirPath = "./" + givenPath + "/"

metadataPath = dirPath + "metadata"
testsetPath = dirPath + "testset"
surprisalScoresPath = dirPath + "frac_out_termwise"
treeLogPath = dirPath + "frac_log"

#Read Inputs

#Process Metadata file

#Declare data structures to convert between feature numbers and names
rowNumberToName = []
rowNameToNumber = dict()
featureCount = 0

with open(metadataPath, "r") as metadata:
  metadataReader = csv.reader(metadata, delimiter='\t')
  rowIndex = 0
  for row in metadataReader:
    if row[1] == "nominal":
      rowNumberToName.append(row[0])
      rowNameToNumber[row[0]] = rowIndex
      rowIndex = rowIndex + 1

printerr("Read " + str(rowIndex) + " features in metadata (from \"" + metadataPath +"\").")

featureCount = rowIndex


#Process the testset to find which samples are anomalies

sampleClasses = []
with open(testsetPath, "r") as testset:
  testsetReader = csv.reader(testset, delimiter='\t')
  for row in testsetReader:
    sampleClasses.append(row[len(row)-1])

#Read the termwise surprisal scores.

normSurprisalsAvg = []
anomSurprisalsAvg = []
adjustedSurprisalsAvg = []

def columnAverages(table):
  transpose = zip(*table)
  avgs = map(aggregatorFunction, transpose)
  return avgs

with open(surprisalScoresPath, "r") as surprisals:
  normSurprisals = []
  anomSurprisals = []
  surprisalsReader = csv.reader(surprisals, delimiter='\t')
  rowIndex = 0
  for row in surprisalsReader:
    vals = []
    for i in range(0,featureCount):
      vals.append(float(row[i]))
    if(sampleClasses[rowIndex] == normalClassName):
      normSurprisals.append(vals)
    else:
      anomSurprisals.append(vals)
    rowIndex += 1
  
  printerr("Read " + str(len(normSurprisals)) + " normal samples, and " + str(len(anomSurprisals)) + " anomalous.")
  #Calculate the average surprisal values for normals and anomalies, then subtract normal from abnormal
  
  normSurprisalsAvg = columnAverages(normSurprisals)
  anomSurprisalsAvg = columnAverages(anomSurprisals)
  
  adjustedSurprisalsAvg = [b - a for (a, b) in zip(normSurprisalsAvg, anomSurprisalsAvg)]

#Now place feature surprisals by name into a dict
surprisalLookup = dict()
for i in range(0, featureCount):
  surprisalLookup[rowNumberToName[i]] = adjustedSurprisalsAvg[i]

treeEntryRegex = re.compile("\\{Waffles Decision Tree Classifier \\{feature: \"([a-zA-Z0-9]+?)\"\\} \\{ \\[\\|(.+?)\\|\\] \\} \\}", re.MULTILINE|re.DOTALL)

nameRegex = re.compile(featurePrefix + "\\d+")

#Load the model printouts
rawData = ""
with open(treeLogPath, "r") as modelData:
  rawData = modelData.read()


def processTree(featureName, treeData, graph):
  #mainFeature = re.match(nameRegex, featureName).group(1)
  if featureName.isdigit():
    #If a number was provided instead of a name, look up the number
    mainFeature = rowNumberToName[int(featureName)]
  else:
    mainFeature = featureName
  
  associatedFeatures = set(re.findall(nameRegex, treeData)) #Convert to set to remove duplicates

  associatedFeatures.remove(mainFeature)  

  adjustedSurprisal = float(surprisalLookup[mainFeature]) / len(associatedFeatures)
  
  for associatedFeature in associatedFeatures:
    pair = (mainFeature, associatedFeature)
    if mainFeature > associatedFeature:
      pair = (associatedFeature, mainFeature)
    
    if not pair in graph:
      graph[pair] = adjustedSurprisal
    else:
      graph[pair] = graph[pair] + adjustedSurprisal
    

allMatches = re.finditer(treeEntryRegex, rawData)

graph = dict()

for m in allMatches:
  processTree (m.group(1), m.group(2), graph)

print("#Adjusted average surprisal values (anomaly average - normal average)")
#Make a list of (name, adjusted surprisal) pairs.
namedSurprisals = []
for i in range(0, featureCount):
  if filterNonpositive and adjustedSurprisalsAvg[i] <= 0:
    continue
  namedSurprisals.append((rowNumberToName[i], adjustedSurprisalsAvg[i]))

namedSurprisalsSorted = sorted(namedSurprisals, key=lambda tup : -tup[1])

for tup in namedSurprisalsSorted:
  print(tup[0] + '\t' + precision % tup[1])

if filterNonpositive:
  print("#Dropped " + str(featureCount - len(namedSurprisals)) + " nonpositive features.  There are circumstances in which this may happen consistently, such as when an abnormal class follows a pattern more closely than the normal, however these are quite unusual.  To see the dropped instances, rerun without the -F flag.")

print()
print("#Relationship Graph")

graphData = graph.items()
if filterNonpositive:
  graphData = list(filter(lambda tup : tup[1] > 0, graph.items()))

sortedGraph = sorted(graphData, key=lambda tup : -tup[1])
for item in sortedGraph:
  #print(str(item[0]) + '\t' + precision % item[1]) #Tuple style printing
  print(item[0][0] + '\t' + item[0][1] + '\t' + precision % item[1]) #TSV style printing
if filterNonpositive:
  print("#Dropped " + str(len(graph.items()) - len(graphData)) + " nonpositive edges.  These are very likely the result of a lack of statistical power, and probably do not represent a statistically sinficant phenomenon.  To see the dropped edges, rerun without the -F flag.")

