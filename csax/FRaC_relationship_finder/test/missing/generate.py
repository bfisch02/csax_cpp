import random
import os
import math

centers = []
stdevs = []

dimensions = 32
distributions = 8

groupSize = 4


squareSize = 32
stdevMax = 16

fmt = "{0:.3f}"

def init():
	for i in range(0,distributions):
		centers.append([])
		stdevs.append([])
		for j in range(0,dimensions):
			centers[i].append(random.uniform(-squareSize,squareSize))
			stdevs[i].append(random.uniform(0, stdevMax))


#Writers
def writeSchema(f):
	for i in range(0, dimensions):
		f.write(str(i + 1) + "\t" + "real" + "\t" + str(-squareSize) + "," + str(squareSize) + "\n") #Note: Min/max are just a guess.
	f.write(str(dimensions + 1) + "\t" + "class" + "\t" + "0,1,2" + "\n")

def writeCatSchema(f, groupSize):
	nameIndex = 0

	distcatnames = str(0)
	for i in range(1, distributions):
		distcatnames += "," + str(i)

	for i in range(0, dimensions / groupSize):
		nameIndex += 1
		f.write(str(nameIndex) + "\t" + "nominal" + "\t" + distcatnames + "\n")
		for j in range(0, groupSize):
			nameIndex += 1
			f.write(str(nameIndex) + "\t" + "real" + "\t" + str(-squareSize) + "," + str(squareSize) + "\n")
	nameIndex += 1
	f.write(str(nameIndex) + "\t" + "class" + "\t" + "0,1" + "\n")

def writeData(f, data, className):
	for sample in data:
		for feature in sample:

			f.write((fmt.format(feature) if type(feature) is float else str(feature)) + "\t")
		f.write(str(className) + "\n")

#Generators
def genNormal(count, unknownCount):
	samples = []
	for i in range(0, count):
		thisSample = []
		dist = i % distributions
		for j in range(0, dimensions):
			thisSample.append(random.gauss(centers[dist][j], stdevs[dist][j]))
		samples.append(thisSample)

	applyDropSamples(samples, unknownCount)
	return samples

def genAbnormal(count, unknownCount):
	samples = []
	for i in range(0, count):
		thisSample = []
		for j in range(0, dimensions):
			thisSample.append(random.uniform(-squareSize, squareSize))
		samples.append(thisSample)

	applyDropSamples(samples, unknownCount)
	return samples


#Functional noise applicators
def applyUniformNoise(samples, magnitude):
	return map(lambda sample: map(lambda i: i if type(i) is str else i + random.uniform(-magnitude, magnitude), sample), samples)

def applyGaussianNoise(samples, magnitude):
	return map(lambda sample: map(lambda i: i if type(i) is str else i + random.gauss(0, magnitude), sample), samples)

#In place, sorry.
def applyDropSamples(samples, unknownCount):
	allIndices = range(0, len(samples[0]))
	for sample in samples:
		random.shuffle(allIndices)
		unknownIndices = allIndices[0:unknownCount]
		for i in unknownIndices:
			sample[i] = "?"


#Convenience functions to generate and write
def writeTraining(f, count, noise):
	data = genNormal(count, 0)
	data = applyUniformNoise(data, noise)
	writeData(f, data, 0)

def writeTestNormal(f, count, unknowns, noise):
	data = genNormal(count, unknowns)
	data = applyUniformNoise(data, noise)
	writeData(f, data, 0)

def writeTestSeminormal(f, count, unknowns, noise):
	data = genNormal(count, unknowns)
	data = applyUniformNoise(data, noise)
	writeData(f, data, 1)

def writeTestAbnormal(f, count, unknowns):
	data = genAbnormal(count, unknowns)
	writeData(f, data, 2)

sampleCount = 256

normalNoise = 1
seminormalNoise = 5

def createTest1(dirName):
	os.system("rm -rf " + dirName)
	os.mkdir(dirName)

	trainset = genNormal(sampleCount, 0)
	trainset = applyUniformNoise(trainset, normalNoise)

	for i in range(0, dimensions):
		dpath = dirName + str(i) + "/"
		os.mkdir(dpath)
		with open(dpath + "metadata", "w") as f:
			writeSchema(f)

		with open(dpath + "trainset", "w") as f:
			writeData(f, trainset, 0)

		with open(dpath + "testset", "w") as f:
			writeTestNormal(f, sampleCount, i, normalNoise)
			writeTestAbnormal(f, sampleCount, i)

def createTest2(dirName):
	os.system("rm -rf " + dirName)
	os.mkdir(dirName)

	trainset = genNormal(sampleCount, 0)
	trainset = applyUniformNoise(trainset, normalNoise)

	for i in range(0, dimensions):
		dpath = dirName + str(i) + "/"
		os.mkdir(dpath)
		with open(dpath + "metadata", "w") as f:
			writeSchema(f)

		with open(dpath + "trainset", "w") as f:
			writeData(f, trainset, 0)

		with open(dpath + "testset", "w") as f:
			writeTestNormal(f, sampleCount, i, normalNoise)
			writeTestSeminormal(f, sampleCount, i, seminormalNoise)

def genNormalWithCats(count, groupSize, unknownCount, noise):
	samples = []
	for i in range(0, sampleCount):
		sample = []
		for g in range(0, dimensions / groupSize):
			dist = random.randrange(distributions)
			sample.append(str(dist))
			for h in range(0, groupSize):
				index = groupSize * g + h
				sample.append(random.gauss(centers[dist][index], stdevs[dist][index]))
		samples.append(sample)

	applyDropSamples(samples, unknownCount)
	samples = applyUniformNoise(samples, noise)
	return samples


def createTest3(dirName):
	os.system("rm -rf " + dirName)
	os.mkdir(dirName)

	trainset = genNormalWithCats(sampleCount, groupSize, 0, normalNoise)

	for i in range(0, dimensions):
		dpath = dirName + str(i) + "/"
		os.mkdir(dpath)
		with open(dpath + "metadata", "w") as f:
			writeCatSchema(f, groupSize)

		with open(dpath + "trainset", "w") as f:
			writeData(f, trainset, 0)

		with open(dpath + "testset", "w") as f:
			data = genNormalWithCats(sampleCount, groupSize, i, normalNoise)
			writeData(f, data, 0)
			data = genNormalWithCats(sampleCount, groupSize, i, seminormalNoise)
			writeData(f, data, 1)

init()
createTest1("test1/")
createTest2("test2/")
createTest3("test3/")

