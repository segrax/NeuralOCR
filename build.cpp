/**
*          Network Builder
*
* A self organizing, hand/auto trained
*  auto growing with memory, and stuff
*
* In other words... a growing learner                         
* 
* Copyright (c) 2013 Robert Crossfield
*
*              Strobs Canardly Systems
*
*                                  ScS
**/
#include<iostream>
#include<vector>
#include<map>
#include<time.h>
#include <fstream>
#include <direct.h>
#include <sstream>
#include <iomanip>
#include<algorithm>

using namespace std;

double RandomNumber(double pMin, double pMax) {

	return pMin + ((double)rand() / RAND_MAX) * (pMax - pMin);
}

void Zero( double *pVals, size_t pCount ) {

	for( size_t count = 0; count < pCount; ++count ) {

		pVals[count] = 0;
	}
}

#include "network.hpp"

#include "source/bitmap_image.hpp"

#define PATH_MAX 4096

std::string getcwd_string( void ) {
	char buff[PATH_MAX];
	_getcwd( buff, PATH_MAX );
	std::string cwd( buff );
	return cwd;
}

double *InputBmp( string pFile, size_t &pBytes ) {
	size_t Length = 0;
	stringstream file;

	file << getcwd_string();
	file << "\\" << pFile;

	bitmap_image Bitmap( file.str() );

	size_t BufSize = Bitmap.width() * Bitmap.height();
	size_t PaletteSize = 0;

	double *Input = new double[ BufSize ];

	unsigned int i = 0;

	for( unsigned int y = 0; y < Bitmap.height(); ++y ) {
		for( unsigned int x = 0; x < Bitmap.width(); ++x ) {

			unsigned char red, green, blue;

			Bitmap.get_pixel( x, y, red, green, blue );

			if( blue == 0xFF && red == 0xFF && green == 0xFF ) {
				Input[i] = 0;

			} else {

				if( red == 0xFF && blue == 0 && green == 0 ) {
					Input[ i ] = 2;

				} else {

					Input[i] = 1;
				}
			}

			cout << Input[i];			
			++i;
		}
		cout << "\n";
	}

	pBytes = BufSize;
	return Input;
}

cNetwork *Network = 0;
size_t Inputs = 0, Targets = 5;
double *Input__ = InputBmp( "Pics\\24x12\\__.bmp", Inputs);
double *InputA_ = InputBmp( "Pics\\24x12\\A_.bmp", Inputs);
double *InputB_ = InputBmp( "Pics\\24x12\\B_.bmp", Inputs);
double *InputC_ = InputBmp( "Pics\\24x12\\C_.bmp", Inputs);

double *Input_A = InputBmp( "Pics\\24x12\\_A.bmp", Inputs);
double *Input_B = InputBmp( "Pics\\24x12\\_B.bmp", Inputs);
double *Input_C = InputBmp( "Pics\\24x12\\_C.bmp", Inputs);
double *InputAB = InputBmp( "Pics\\24x12\\AB.bmp", Inputs);
double *InputBA = InputBmp( "Pics\\24x12\\BA.bmp", Inputs);
//double *Input_A_Red = InputBmp( "Pics\\24x12\\_A_Red.bmp", Inputs );

/*
double *Input__ = InputBmp( "Pics\\96x12\\________.bmp", Inputs );
double *InputA_ = InputBmp( "Pics\\96x12\\A_______.bmp", Inputs );
double *InputB_ = InputBmp( "Pics\\96x12\\B_______.bmp", Inputs );

double *Input_A = InputBmp( "Pics\\96x12\\_______A.bmp", Inputs );
double *Input_B = InputBmp( "Pics\\96x12\\_______B.bmp", Inputs );

double *InputBA = InputBmp( "Pics\\96x12\\B_______A_.bmp", Inputs);*/

double *Target = new double[Targets];

void TrainInitial( const double *pInput, const size_t pInputs, double *pTarget, size_t pTargets ) {

	//mMomentum = 0.001;
	//mLearningRate = 0.003;
	Network->mMomentum		= 0.01;
	Network->mLearningRate	= 0.01;
	Network->mErrorThresh	= 0.005;

	double Error = 0;

	cout << "Initial Training\n";

	for( size_t Iteration = 0; Iteration < 5; ++Iteration ) {

		Error = Network->Backward( pInput, pInputs, pTarget, pTargets  );

		cout << " Iteration: " << Iteration << " Error: " << Error << "\n";
	}
}

void Train( const double *pInput, const size_t pInputs, double *pTarget, size_t pTargets ) {
	double Error = 0;

	//Network->mMomentum		= 0.001;
	//Network->mLearningRate	= 0.002;
	Network->mErrorThresh	= 0.005;

	//Network->mLearningRate -= 0.000001;

	for( size_t Iteration = 0; Iteration < 7; ++Iteration ) {

		Error = Network->Backward( pInput, pInputs, pTarget, pTargets  );

		//cout << " Iteration: " << Iteration << " Error: " << Error << "\n";
	}

}

void TestRunExpected(  double *pInput, size_t pInputs, size_t pOutputNumber, size_t pOutputNumberMax, double pValue, string pName ) {
	cConnection *Outputs = 0;
	cout << "testing " << pName << "\n";

	Outputs = Network->Forward( pInput, pInputs );

	for( size_t pOutput = pOutputNumber; pOutput < pOutputNumberMax; ++pOutput ) 
		cout << " Node: " << pOutput << " : "  << Outputs->mActions[ pOutput ]->mResult << "\n";

	cout << "\n";

}

void TestRun( double *pInput, size_t pInputs, string pName ) {
	cConnection *Outputs = 0;

	cout << "\ntesting " << pName << "\n";

	Outputs = Network->Forward( pInput, pInputs );
	for( int x = 0 ; x < 4; ++x ) {

		cout << " " <<  x << ": " << Outputs->mActions[ x ]->mResult << "  ";
	}
	cout << "\n";
}

void TrainRun( const double *pInput, const size_t pInputs, size_t pOutput, double pOutputValue, string pName ) {

	Zero(Target, Targets );
	Target[pOutput] = pOutputValue;

	cout << "training " << pName << "\n";
	Train( pInput, pInputs, Target, Targets );

	//TestRun( pInput, pInputs, pName );
}


void Run() {

	Network = new cNetwork();
	Network->Load("net.bin");

	Zero( Target, Targets );
	// Train Blank
	TrainInitial( Input__, Inputs, Target, Targets );

	for(;;) {
		TrainRun( Input__, Inputs, 0, 0.0, "__" );

		// Train A_
		TrainRun( InputA_, Inputs, 0, 1, "A_" );			// Node 0

		// Train B_
		TrainRun( InputB_, Inputs,	1,	1, "B_" );		// Node 1

		// Train C_
		TrainRun( InputB_, Inputs,	2,	1, "C_" );		// Node 2
		//TrainRun( Input_A_Red, Inputs, 3, 1, "_A_Red" );
		// Train _A
		//TrainRun( Input_A, Inputs, 0, 1, "_A" );

		// Train _B
		//TrainRun( Input_B, Inputs,	1,	1, "_B" );		// Node 1
		Network->Save("net.bin");
		TestRun( Input__, Inputs, "__");

		//TestRunExpected( Input_A_Red, Inputs, 3, 4, 1, "_A (Red)"); 
		
		TestRunExpected( InputA_, Inputs, 0, 1, 1, "A_");
		TestRunExpected( Input_A, Inputs, 0, 1, 1, "_A" );

		TestRunExpected( InputB_, Inputs, 1, 2, 1, "B_");
		TestRunExpected( Input_B, Inputs, 1, 2, 1, "_B");

		TestRunExpected( InputC_, Inputs, 2, 3, 1, "C_");
		TestRunExpected( Input_C, Inputs, 2, 3, 1, "_C");

		TestRunExpected( InputBA, Inputs, 0, 2, 1, "AB");
		//( InputBA, Inputs, "BA");

		//Network->ShowHow( InputA_, Inputs, 0 );

	}

}

int main() {		
	srand (time(NULL));
	Run();

}
