#ifndef TESTFRAMEWORK_H
#define	TESTFRAMEWORK_H

#include "svm.h"
#include "sample.hpp"
#include "array.hpp"

class TestFramework{
  public:
 
  SampleStructure ss;

  Array<Sample> training;
  Array<Sample> test;
  Array<bool> testClasses; //True for normal samples
  
  TestFramework(SampleStructure ss, Array<Sample> test, Array<Sample> training, Array<bool> testClasses) : ss(ss), training(training), test(test), testClasses(testClasses) { }
};

TestFramework loadTestData(std::string testDirectory);

#endif