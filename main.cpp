//
//  main.cpp
//  CS109 Project
//
//  Created by Cwagon on 3/10/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include <iostream>
#include <fstream>
#include <string>
#include <stdio.h>
#include <vector>
#include <math.h>
using namespace std;

int laplace = 0;

typedef struct NB{
    int numZeros;
    int numOnes;
    int numVectors;
    int numValsPerLine;
    int total;
    double logtotal;
    double logPrior0;
    double logPrior1;
    int correct0;
    int correct1;
    vector<vector<int> > vects;

    
}NB;
struct NB Parsefile(ifstream &infile);
void PrintNB(struct NB &bayes);

typedef struct Pgiven {
    vector<double> probOfValAndOne;
    vector<double> probOfValAndZero;
}Pgiven;

//CalculateProbs
struct Pgiven getPstruct(int numVals);
void CalcProbOfZeroForFeature(NB bayes, int feature, Pgiven &given);
void CalcProbOfOneForFeature(NB bayes, int feature, Pgiven &given);
void SetVectorsOfProbabilities(NB &bayes, Pgiven &given1, Pgiven &given0);
void ComputeResults(NB &testData, NB &bayes, Pgiven &given1, Pgiven &given0);
void ComputeLLH(NB &testData, Pgiven &given1, Pgiven &given0, double &prob1, double &prob0, int X);
void SetPriors(NB &bayes);

void NBClassify(ifstream &infile, ifstream &testfile);
double GetZ(NB &data, vector<double> &betas, int vectorIndex){
  double sum = 0.0;  
  for (int i = 0; i < data.numValsPerLine; i++)
         sum+= betas[i]*(data.vects[vectorIndex][i]);
  cout << sum << "\n";
  return sum;
}
int main (int argc, const char * argv[])
{
 /* Input Files */
    ifstream infile;
    ifstream testfile;
    infile.open("simple-train.txt");
    if (infile.fail()){
        cout << "Input file failed\n";
    }
    testfile.open("simple-test.txt");
    if (testfile.fail()){
        cout << "Input file failed\n";
    }
  //NBClassify(infile, testfile);
  NB data = Parsefile(infile);
  NB testData = Parsefile(testfile);
  //* TRAIN *//
  int epochs = 10000;
  double learningRate = 0.0001;
  vector<double> betas(50);
  for (int i = 0; i < 50; i++)
    betas[i] = 0.0;

  /**/
    int m = data.numValsPerLine;
    for (int e = 0; e < epochs; e++){
        if (e > 999 && e%1000 == 0) cout << ".";
        vector<double> gradient(50, 0.0);
        for (int vectorIndex = 0; vectorIndex < data.numVectors; vectorIndex++){
            double z = GetZ(data, betas, vectorIndex);
            int y = data.vects[vectorIndex][m+1];
            for (int j = 0; j < m; j++){
                double eZ = exp(z);
                /*Note (1/ (1+e to the -z)) == eZ/(eZ+1) */
                gradient[j] += data.vects[vectorIndex][j] * (y-(1/(1+(1/eZ))) );//(y - (eZ/(eZ+1)));  
            }
        }
       cout << "\n"; 
    for (int k = 0; k < m; k++){
      betas[k] += (learningRate * gradient[k]);
//      cout << "beta[" << k<< "] is " << betas[k] << "  ";
    }
 }

    //*Classify test data **/
    for (int vectorInTestData = 0; vectorInTestData < testData.numVectors; vectorInTestData++){
      double probOfOne = 0.0;
      double Z = GetZ(data, betas, vectorInTestData);
      double eZ = exp(Z);
      cout << "Z = " << Z << " eZ = " << eZ << "\n";
      probOfOne = 1/(1+(1/eZ));
      cout << "ProbOfOne = " << probOfOne << "  " ;
      cout << "Vector " << vectorInTestData << " has Y= ";
      if (probOfOne > 0.5)
          cout << 1;
      else 
        cout << 0;
      cout << "\n";
    }
return 0;
}

void NBClassify(ifstream &infile, ifstream &testfile){
    /* Get number of entries and number of lines */
    NB bayes = Parsefile(infile);
    //PrintNB(bayes);
    /* Get probs */
    SetPriors(bayes);
    /* Intialize and set Vectors of probabilties for 11,10,00,01 */
    Pgiven given1 = getPstruct(bayes.numValsPerLine);
    Pgiven given0 = getPstruct(bayes.numValsPerLine);
    SetVectorsOfProbabilities(bayes, given1, given0);
    NB testData = Parsefile(testfile);
    ComputeResults(testData, bayes, given1, given0);
    infile.close();
    testfile.close();
}


struct NB Parsefile(ifstream &infile)
{
    NB bayes;
    bayes.numOnes = 0;
    bayes.numZeros = 0;
    string temp;
    getline(infile, temp);
    int numValsPerLine;
    sscanf(temp.c_str(), "%d", &numValsPerLine);
    getline(infile, temp);
    int numVectors;     
    sscanf(temp.c_str(), "%d", &numVectors);
    /* *****************************************/
    
    vector<vector<int> > vects(numVectors);
    for (int i = 0; i < numVectors; i++){
        vector<int> newVect(numValsPerLine+1);
        getline(infile, temp);
        //cout <<"NEW LINE: " << temp << "\n";
        for (int j =0; j < numValsPerLine+1; j++){
            if (temp[j] == 32 || temp[0] == 32)
                temp = temp.substr(1);
            if (temp[0] == ':')
                temp = temp.substr(1);
           // cout << "Get %d from "<< temp;
            sscanf(temp.c_str(), "%d", &newVect[j]);
            temp = temp.substr(1);
           // cout << " got: " << newVect[j] << "\n";
        }
        vects[i] = newVect;
        
        //cout << vects[i][0] <<","<< vects[i][1]<<"," << vects[i][2]<<"|" << "\n";
    }
    cout << "\n";
    bayes.numVectors = numVectors;
    bayes.numValsPerLine = numValsPerLine;
    bayes.vects = vects;
    
    
    
    for (int i = 0; i < numVectors; i++){
        (vects[i][numValsPerLine] == 0) ? bayes.numZeros++:bayes.numOnes++;
    }
    cout << "bayes.numZeros : " << bayes.numZeros << " bayes.numones: " << bayes.numOnes << "\n";
    return bayes;
}

void PrintNB(struct NB &bayes)
{
    for(int p = 0; p < bayes.numVectors; p++){
        for (int p2 = 0; p2 < bayes.numValsPerLine; p2++){
            cout << bayes.vects[p][p2];
        }
        cout << ":" << bayes.vects[p][bayes.numValsPerLine] << "\n";
    }
    
}



void ComputeResults(NB &testData, NB &bayes, Pgiven &given1, Pgiven &given0){
    testData.correct0 = 0;
    testData.correct1 =0;
    
    for (int i = 0; i < testData.numVectors; i++){
        double prob0 = 0;
        double prob1 = 0;
        ComputeLLH(testData,given1, given0, prob1, prob0, i);
        
        double logPY0 = log(bayes.numZeros)-log(bayes.numVectors);
        double logPY1 = log(bayes.numOnes)-log(bayes.numVectors);
        
        double PROB0 = logPY0 + prob0;
        double PROB1 = logPY1 + prob1;
        
        if (PROB0 > PROB1 && testData.vects[i][testData.numValsPerLine] == 0){
            testData.correct0 +=1;
            
        }
        else if (PROB1 > PROB0 && testData.vects[i][testData.numValsPerLine] == 1)
            testData.correct1 +=1;
        
        //cout << "Prior0:  "<< (double)bayes.numZeros/bayes.numVectors <<" prob0: "<< prob0 << "\n";
        //cout << "Prior1:  "<< (double)bayes.numOnes/bayes.numVectors <<" prob0: "<< prob1 << " ";
        //cout << "\n\t\t\t\t\t\tProb 0: "<< PROB0 << " Prob 1: " << PROB1 << "\n";
        
    }
    cout << "Class 0: tested " << testData.numZeros << ", correctly classified " << testData.correct0 << "\n";
    cout << "Class 1: tested " << testData.numOnes << ", correctly classified " << testData.correct1 << "\n";
    double overall = testData.correct0+testData.correct1;
    cout << "Overall: "<< (overall/testData.numVectors)*100 << "% testesd: " << testData.numVectors << " correctly classified " << overall << "\n";
    
}

void ComputeLLH(NB &testData, Pgiven &given1, Pgiven &given0, double &prob1, double &prob0, int X)
{
    for (int feature = 0; feature < testData.numValsPerLine; feature++){
        if (testData.vects[X][feature] == 0){
            prob0 += log(given0.probOfValAndZero[feature]);
            prob1 += log(given0.probOfValAndOne[feature]);
        }
        if (testData.vects[X][feature]==1){
            prob0 += log(given1.probOfValAndZero[feature]);
            prob1 += log(given1.probOfValAndOne[feature]);
        }
    }
}

void CalcProbOfZeroForFeature(NB bayes, int feature, Pgiven &given){
    
    
    for (int i = 0; i < bayes.numVectors; i++){
        if (bayes.vects[i][bayes.numValsPerLine] == 0){
            if (bayes.vects[i][feature] == 0) {
                given.probOfValAndZero[feature]+=1;
            }
        }
        else if (bayes.vects[i][bayes.numValsPerLine] == 1){
            if (bayes.vects[i][feature] == 0){
                given.probOfValAndOne[feature] +=1;
            }
        }
    }
    double denom = bayes.numVectors + (laplace * bayes.numVectors);
    given.probOfValAndZero[feature] = given.probOfValAndZero[feature]/denom;
    given.probOfValAndOne[feature] = given.probOfValAndOne[feature]/denom;
    // cout << "Probability for feature " << feature << " being 0 with y=1 is: " << given.probOfValAndOne[feature] << "\n";
    //  cout << "Probability for feature " << feature << " being 0 with y=0 is: " << given.probOfValAndZero[feature] << "\n";
}
void CalcProbOfOneForFeature(NB bayes, int feature, Pgiven &given){
    for (int i = 0; i < bayes.numVectors; i++){
        if (bayes.vects[i][bayes.numValsPerLine] == 0){
            if (bayes.vects[i][feature] == 1) {
                given.probOfValAndZero[feature]+=1;
            }
        }
        else if (bayes.vects[i][bayes.numValsPerLine] == 1){
            if (bayes.vects[i][feature] == 1){
                given.probOfValAndOne[feature] += 1;
            }
        }
;
    }
    double denom = bayes.numVectors+(laplace * bayes.numVectors);
    given.probOfValAndZero[feature] = given.probOfValAndZero[feature]/denom;
    given.probOfValAndOne[feature] = given.probOfValAndOne[feature]/denom;
    // cout << "Probability for feature " << feature << " being 1 with y=1 is: " << given.probOfValAndOne[feature] << "\n";
    // cout << "Probability for feature " << feature << " being 1 with y=0 is: " << given.probOfValAndZero[feature] << "\n";
}

void SetVectorsOfProbabilities(NB &bayes, Pgiven &given1, Pgiven &given0)
{
    for (int feature = 0; feature < bayes.numValsPerLine; feature++){
        //cout << "Checking fexture X" << feature << ":\n";
        CalcProbOfZeroForFeature(bayes, feature, given0);
        CalcProbOfOneForFeature(bayes, feature, given1);
    }
}
void SetPriors(NB &bayes)
{
    bayes.total = bayes.numOnes + bayes.numZeros;
    bayes.logtotal = log(bayes.total); 
    bayes.logPrior0 = log(bayes.numZeros)-log(bayes.numVectors);
    bayes.logPrior1 = log(bayes.numOnes)-log(bayes.numVectors);
}

struct Pgiven getPstruct(int numVals){
    Pgiven newPgiven;
    newPgiven.probOfValAndOne = vector<double>(50);
    newPgiven.probOfValAndZero = vector<double>(50);
    for (int i = 0; i < numVals; i++){
        newPgiven.probOfValAndOne[i] = laplace;
        newPgiven.probOfValAndZero[i] = laplace;
    }
    return newPgiven;
}

        
        /*
         for (int i = 0; i < bayes.numVectors; i++){
         cout << " \t\t\t\t for vector " << i << "\n";
         if (bayes.vects[i].back() == 0){ //if the class is zero
         if (bayes.vects[i][col] == 0){
         zerosgiven0[col]+=1.0;
         //cout << " \nin col " << col << " gives " <<zerosgiven0[col] << "\n";
         }
         else{
         onesgiven0[col]+=1.0;   
         }
         }
         else if (bayes.vects[i].back() == 1){//if the class is one
         if (bayes.vects[i][col] == 0){
         zerosgiven1[col]+=1.0;
         }
         else{
         onesgiven1[col]+=1.0;
         }
         }

         */

