#ifndef BUBBLE_H
#define BUBBLE_H

#include"compress.h"
#include <iostream>
#include <string>
#include<stdlib.h>
#include<stdio.h>
#include"readConfig.h"
#include <math.h>

using namespace std;

class BubbleType
{
	int bubbleNumber;
	int numberBubbles;
	double meanBubbles;
	double sdBubbles;
	double sizeBubbles;
	double densityBubbles;
	int * positions;
  	static const char* FOLDER_NAME;
  	static const char* TOTALTYPES_NAME;
  	static const char* BUBBLE_NAME;
  	static const char* NUMBERBUBBLES_NAME;
  	static const char* MEANBUBBLE_NAME;
  	static const char* SDBUBBLE_NAME;
  	static const char* SIZEBUBBLE_NAME;
  	static const char* DENSITYBUBBLE_NAME;

	double randn (double mu, double sigma);
public:
//	BubbleType(int numberBubbles,double meanBubbles,double sdBubbles,double sizeBubbles,double densityBubbles) : numberBubbles(this.numberBubbles), meanBubbles(this.meanBubbles), 
//		sdBubbles(this.sdBubbles), sizeBubbles(this.sizeBubbles), densityBubbles(this.densityBubbles) {}
	int InitializeBubbles(unsigned compressArea);
	BubbleType(int);
	int getBubbleSize();
	int getBubblePosition(int i);
	double getBubbleDensity();
	int getBubbleNumber();
	static char* getBubbleSections(int * blocks);
};


#endif
