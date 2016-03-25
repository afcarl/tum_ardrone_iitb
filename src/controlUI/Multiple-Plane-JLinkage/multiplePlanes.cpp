/*
 *   File Name: main.cpp
 *     Project: Multiple Plane JLinkage
 *  Created on: 20-Mar-2016
 *      Author: Sona Praneeth Akula
 *     Details:
 *   TodoNotes: TODO
 */

#include "allHeaders.hpp"
#include "readingData.hpp"
#include "additionalSteps.hpp"
#include "utilities.hpp"
#include "makeBoundingRects.hpp"
#include "JLinkage.h"
#include "RandomSampler.h"
#include "PrimitiveFunctions.h"
#include <opencv2/core/core.hpp>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <list>


using namespace std;
using namespace cv;


// The main file
void callJLinkage(const vector<Point3d> &locations, vector<int> &labels)
{
	vector<vector<float> *> * mDataPoints;
    mDataPoints = new vector<vector<float> *>(locations.size());
    for(int i=0; i<locations.size(); i++)
    {
       (*mDataPoints)[i] = new vector<float>(3);
       (*(*mDataPoints)[i])[0] = locations[i].x;
       (*(*mDataPoints)[i])[1] = locations[i].y;
       (*(*mDataPoints)[i])[2] = locations[i].z;
    }
	int numModels = 10000;
	float mKdTreeCloseProb = 0.8, mKdTreeFarProb = 0.2;
	int mKdTreeRange =10;

    RandomSampler mRandomSampler(GetFunction_Plane, DistanceFunction_Plane, (int)(*(*mDataPoints)[0]).size()-1, 3, (int)mDataPoints->size(),true);
    mRandomSampler.SetPoints(mDataPoints);
    mRandomSampler.SetNFSamplingTypeNN(mKdTreeRange, (float)mKdTreeCloseProb, (float)mKdTreeFarProb, true);
    vector<vector<float> *> *mModels = mRandomSampler.GetNSample(numModels, 0, NULL, NULL);

    float mInlierThreshold = 0.1;
    JLinkage mJLinkage(DistanceFunction_Plane, mInlierThreshold, mModels->size(), true, ((*mDataPoints)[0])->size(), mKdTreeRange);
    vector<const sPtLnk *> mPointMap(mDataPoints->size());
    list<sClLnk *> mClustersList;

    for(unsigned int nModelCount = 0; nModelCount < mModels->size(); nModelCount++){
        mJLinkage.AddModel(((*mModels)[nModelCount]));
    }

    int counter=0;
    for(std::vector<std::vector<float> *>::iterator iterPts = mDataPoints->begin(); iterPts != mDataPoints->end(); ++iterPts ){
        mPointMap[counter] = mJLinkage.AddPoint(*iterPts);
        counter++;
    }

    mClustersList = mJLinkage.DoJLClusterization(NULL);

	labels.resize(locations.size());
    int counterCl =0;
    for(std::list<sClLnk *>::iterator iterCl = mClustersList.begin(); iterCl != mClustersList.end(); ++iterCl){
        for(std::list<sPtLnk *>::iterator iterPt = (*iterCl)->mBelongingPts.begin(); iterPt != (*iterCl)->mBelongingPts.end(); ++iterPt){
            unsigned int counterPt = 0;
            for(std::vector<const sPtLnk *>::iterator iterPtIdx = mPointMap.begin(); iterPtIdx != mPointMap.end(); ++iterPtIdx){
                if((*iterPt) == (*iterPtIdx)){
                    labels[counterPt] = counterCl;
                    break;
                }
                ++counterPt;
            }
        }
        ++counterCl;
    }			
}


int findMultiplePlanes(const vector<Point3d> &points, vector< vector<double> > &sortedPlaneParameters,  vector< vector<Point3d> > & continuousBoundingBoxPoints){ 

	vector<Point3d> newPoints;
	// Step 1: Performing JLinkage to find multiple models
	// This vector describes to which plane does point i belong to
	// This is an output from JLinkage
	vector<int> planeIndices;
	// Plane Parameters (a, b, c, d) for plane i
	vector< vector<double> > planeParameters;


	callJLinkage(points, planeIndices);
	// planeParameters = fitting_fn(points);

	// Step 2
	map<int, int> freqOfSelectedPlanes;
	int minPointsPerPlane = 50;
	int numberOfPointsInData = points.size();
	// int numberOfUniquePlanes = numberOfUniquePlanes( planeIndices );
	map<int, int> numberOfPointsPerPlane;
	vector<int> newPlaneIndices;
	int numberOfPlanes;
	removeUnnecessaryPlanes( points, planeIndices, minPointsPerPlane,
			numberOfPointsPerPlane, newPoints, newPlaneIndices, numberOfPlanes);

	// Step 3
	// Generate parameters from the n:w
	// ew set of points
	// planeParameters = fitting_fn(points);

	// Step 4
	// Perform K-means
	Mat pointsMatrix = Mat(points);
	// Reference: http://docs.opencv.org/2.4/modules/core/doc/clustering.html
	// kmeans(pointsMatrix, numberOfPlanes, planeIndices, TermCriteria::epsilon, clustersCenters);
	// planeParameters = fitting_fn(points);

	// Step 5
	// Remove points which far from the estimated plane after performing k-means
	// Get 3D Projection of points onto the plane
	vector<double> distanceMatrix;
	vector< vector<int> > planePointsIndexMapping;
	vector<Point3d> newSortedPoints;
	vector< vector<double> > newPlaneParameters;
	map<int, pair<int, int> > planeIndexBounds;
	vector<Point3d> projectionsOf3DPoints;
	calculateDistanceFromPlane( newPoints, planeParameters, planeIndices,
			distanceMatrix, planePointsIndexMapping);
	removePointsFarFromPlane( newPoints, planeParameters, distanceMatrix, planePointsIndexMapping,
			newSortedPoints, newPlaneParameters, planeIndexBounds);
	get3DPlaneProjectionsOfPoints ( newSortedPoints, newPlaneParameters, planeIndexBounds,
						projectionsOf3DPoints	);


	// Step 6
	vector<Point3d> sortedProjectionsOf3DPoints;
	map<int, pair<int, int> > sortedPlaneIndexBounds;
	vector< vector<Point3d> > boundingBoxPoints;
	orderPlanePointsByCentroids( projectionsOf3DPoints, planeParameters, planeIndexBounds,
			sortedProjectionsOf3DPoints, sortedPlaneParameters, sortedPlaneIndexBounds);
	getBoundingBoxCoordinates ( sortedProjectionsOf3DPoints, sortedPlaneParameters,
			sortedPlaneIndexBounds, boundingBoxPoints );
	getContinuousBoundingBox ( boundingBoxPoints, sortedPlaneParameters,
							continuousBoundingBoxPoints);
	return 0;
}