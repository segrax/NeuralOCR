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

using namespace std;

double RandomNumber(double pMin, double pMax) {

	return pMin + ((double)rand() / RAND_MAX) * (pMax - pMin);
}

void Zero( double *pVals, size_t pCount ) {

	for( size_t count = 0; count < pCount; ++count ) {

		pVals[count] = 0;
	}
}

class cAction {
public:
	vector< double > mWeight;
	double			 mThreshold;
	double			 mResult;
	double			 mInput;

	double			 mDelta;
	vector< double> mChange;

	double			 mError;

	string			 mName;

	cAction( const string &pName, const bool &pSigmoid, const size_t &pInputCount, const double pThreshold ) {
		mDelta = 0;
		mError = 0;
		mInput = 0;

		mResult = 0;
		mThreshold = pThreshold;

		mWeight.reserve( pInputCount );
		mChange.reserve( pInputCount );

		for( unsigned int x = 0; x < pInputCount; ++x )  {
			mWeight.push_back( RandomNumber( -1, 1 ) );
			mChange.push_back( 0 );
		}
	}
};

class cConnection {
public:
	vector< cAction* >  mActions;

	size_t              mActionsIn;
	string              mName;

	cConnection( const size_t &pActions, const size_t &pActionsIn, const string &pName ) {
		bool sigmoid = false;
		mActions.resize( pActions );

		mActionsIn = pActionsIn;

		for( size_t Node = 0; Node < pActions; ++Node ) {

			mActions[ Node ] = new cAction( pName, sigmoid, pActionsIn, RandomNumber(-1,1) );
		}
	}

	void EraseInputs() {

		for( size_t Node = 0; Node < mActions.size(); ++Node ) {
			mActions[Node]->mInput = 0;
		}
	}

	cAction *AddAction( bool &pSigmoid, string &pName ) {
		cAction *Action = new cAction( pName, pSigmoid, mActionsIn, RandomNumber(-1,1) );

		mActions.push_back( Action );

		return Action;
	}
};

class cNetwork {
public:
	double mErrorThresh;
	double mLearningRate;
	double mMomentum;

	double  *mSum;
	size_t   pInputs;

	vector< cConnection* >   mConnections;
	size_t                   mConnection;

	size_t mLayer;

public:

	/**
	* 
	* @param pSum  Pointer to the current node values
	* @param pSums Number of nodes
	**/
	cNetwork() {

		mLayer = 0;

		mMomentum		= 0.01;
		mLearningRate	= 0.1;
		mErrorThresh	= 0.005;
	}

	double activationFunction(double x) {

		return 1/(1 + exp(-1*x));
	}

	double activationFunctionPrimed(double x) {

		return activationFunction(x)*(1 - activationFunction(x));
	}

	bool Load( string pFile ) {
		std::ifstream in(pFile,std::ios::binary);
		if(in.is_open() == false )
			return false;

		cout << "Loading...";
		size_t Layers = 0;
		in.read( (char*) &Layers, sizeof( size_t) );

		// Each Layer
		for( size_t Layer = 0; Layer < Layers; ++Layer ) {
			size_t Actions = 0;
			size_t ActionsIn = 0;

			in.read( (char*) &Actions, sizeof( size_t ) );
			in.read( (char*) &ActionsIn, sizeof( size_t ) );

			cConnection *Connection = AddLayer( ActionsIn, Actions );

			// Each Node
			for( size_t Action = 0; Action < Actions; ++Action ) {
				size_t Weights = 0;

				in.read( (char*) &Weights, sizeof( size_t ) );

				// Each Weight
				for( size_t Weight = 0; Weight < Weights; ++Weight ) {
					double Val = 0;
					in.read( (char*) &Val, sizeof( double ) );

					Connection->mActions[ Action ]->mWeight[ Weight ] = Val;
				}
			}
		}
		in.close();
		cout << "Done\n";
		return true;
	}

	void Save( string pFile ) {
		std::ofstream out(pFile,std::ios::binary);
		size_t Layers = mConnections.size();
		cout << "Saving...";
		out.write( (const char*) &Layers, sizeof( size_t) );

		for( vector<cConnection*>::iterator LayerIT = mConnections.begin(); LayerIT != mConnections.end(); ++LayerIT ) {
			size_t Actions = (*LayerIT)->mActions.size();

			out.write( (const char*) &Actions, sizeof( size_t ) );
			out.write( (const char*) &(*LayerIT)->mActionsIn, sizeof( size_t ) );

			for( vector<cAction*>::iterator ActionIT = (*LayerIT)->mActions.begin(); ActionIT != (*LayerIT)->mActions.end(); ++ActionIT ) {
				size_t Weights = (*ActionIT)->mWeight.size();

				//out.write(  (const char*) (*ActionIT)->
				out.write( (const char*) &Weights, sizeof( size_t ) );

				for( vector<double>::iterator WeightIT = (*ActionIT)->mWeight.begin(); WeightIT != (*ActionIT)->mWeight.end(); ++WeightIT ) {

					out.write( (const char*) &(*WeightIT), sizeof( double ) );
				}
			}
		}
		cout << "Done\n";
		out.close();
	}


	void Randomize( double *pVals, size_t pCount ) {

		for( size_t count = 0; count < pCount; ++count ) {

			pVals[count] = RandomNumber( -1, 1 );
		}
	}

	cConnection *AddLayer( size_t pInputs, size_t pOutputs) {
		string Name = "";
		cConnection *Connection = new cConnection( pOutputs, pInputs, Name );

		mConnections.push_back( Connection );

		return Connection;
	}

	void ErrorLayer( const double *pTarget, const size_t pTargets  ) {

		for( int Layer = mConnections.size() - 1; Layer >= 0; --Layer ) {
			cConnection *Connection = mConnections[Layer];

			for( size_t Node = 0; Node < Connection->mActions.size(); ++Node ) {
				cAction *Action = Connection->mActions[ Node ];

				double Error =0 ;

				// Output Layer
				if( Layer == mConnections.size() - 1 ) {

					Error = activationFunctionPrimed( Connection->mActions[ Node ]->mInput )
						* (pTarget[ Node ] - Connection->mActions[ Node ]->mResult);

				} else {

					cConnection *ConnectionUp = mConnections[Layer+1];


					for( size_t LayerUpNode = 0; LayerUpNode < ConnectionUp->mActions.size(); ++LayerUpNode ) {

						cAction *ActionsUp = ConnectionUp->mActions[ LayerUpNode ];

						Error += Action->mWeight[ LayerUpNode ] * ActionsUp->mDelta;
					}


				}
				Connection->mActions[Node]->mError = Error;
				Connection->mActions[Node]->mDelta = activationFunctionPrimed( Connection->mActions[Node]->mInput ) * Error ;

			}
		}
	}

	void WeightAdjust( const double *pInput, const size_t pInputs ) {

		for( int Layer = mConnections.size() - 1; Layer >= 1; --Layer ) {
			cConnection *Connection = mConnections[Layer];
			cConnection *ConnectionUp = mConnections[Layer - 1];

			for( size_t Node = 0; Node < Connection->mActions.size(); ++Node ) {
				cAction *Action = Connection->mActions[ Node ];

				// k
				for( size_t NodeIn = 0; NodeIn < ConnectionUp->mActions.size(); ++NodeIn ) {

					Action->mWeight[ NodeIn ] += ( mLearningRate * Action->mDelta * ConnectionUp->mActions[ NodeIn ]->mResult );

					//+ (mMomentum * change);
				}
			}
		}

		// Input layer
		cConnection *Connection = mConnections[0];

		for( size_t Node = 0; Node < Connection->mActions.size(); ++Node ) {
			cAction *Action = Connection->mActions[ Node ];

			for( size_t NodeIn = 0; NodeIn < pInputs; ++NodeIn ) {
				Action->mWeight[ Node ] += ( mLearningRate *  Action->mDelta * pInput[NodeIn] );
			}
		}
	}

	double MeanSqueared( cConnection *pConnection, const double *pTarget, const size_t pTargets ) {
		double sum = 0;

		for( size_t i = 0; i < pConnection->mActions.size(); ++i ) {

			//sum += pow( pConnection->mActions[ i ]->mError, 2 );
			sum += (pTarget[i] - pConnection->mActions[ i ]->mResult )

				*  (pTarget[i] - pConnection->mActions[ i ]->mResult);

		}

		return sum;
	}

	double TrainPattern( const double *pInput, const size_t pInputs, const double *pTarget, const size_t pTargets ) {

		Forward( pInput, pInputs );

		ErrorLayer( pTarget, pTargets );
		WeightAdjust( pInput, pInputs );

		double Error = MeanSqueared( mConnections[ mConnections.size() - 1 ], pTarget, pTargets  );

		return Error;
	}

	/**
	* Start with the result
	*
	* @param pSource The values which the network has to output, based on its current values
	* @param pCount  Number of values in pSource
	**/
	double Backward( double *pTarget, size_t pTargets, double *pInput, size_t pInputs ) {

		bool CreateLayer = false;

		if( mConnections.size() == 0 ) {
			size_t Sums = pInputs;

			// Input Layer
			AddLayer( pInputs, Sums );
			// First Hidden
			AddLayer( pInputs, Sums );

			/*double Targets = pTargets;
			size_t DecSize = (Sums - pTargets) / 4;

			while( (Targets / Sums) * 100 < 40 ) {

			Sums -= DecSize;

			if(Sums < Targets) {
			Sums = Targets;
			break;
			}

			AddLayer( pInputs, Sums );
			}*/

			AddLayer( pInputs, Sums );

			// Second last layer
			AddLayer( pInputs, Sums / 1.4 );
			// Output Layer
			AddLayer( pInputs, pTargets );

			// Correct still?
			mLayer = mConnections.size() - 2;
		} 

		double Error = 0;

		// Train
		for( size_t Iteration = 0; Iteration < 100; ++Iteration ) {

			if( CreateLayer ) {
				CreateLayer = false;

				//cConnection *NewLayer = InsertNewLayer( mLayer++ );

				//CreateLayer = TrainNewLayer( mLayer );

				--Iteration;

				continue;
			}

			Error = TrainPattern( pInput, pInputs, pTarget, pTargets );
		}

		return Error;
	}

	cConnection *Forward( const double *pInputs, const size_t pInputCount ) {

		cConnection *Connection = mConnections[ 0 ];
		cConnection *ConnectionUp = 0;

		Connection->EraseInputs();

		// Load inputs
		for( size_t Node = 0; Node < pInputCount; ++Node ) {

			cAction *Action = Connection->mActions[ Node ];

			for( size_t WeightUpNode = 0; WeightUpNode < Connection->mActions.size(); ++WeightUpNode ) {

				Connection->mActions[Node]->mInput += 
					pInputs[ WeightUpNode ] * 
					Connection->mActions[Node]->mWeight[ WeightUpNode ] ;
			}

		}

		// Run the network forward
		for( size_t Layer = 0; Layer < mConnections.size() - 1; ++Layer ) {

			Connection = mConnections[ Layer ];
			ConnectionUp = mConnections[ Layer + 1 ];

			ConnectionUp->EraseInputs();

			for( size_t Node = 0 ; Node < Connection->mActions.size(); ++ Node ) {
				cAction *Action = Connection->mActions[ Node ];

				Action->mResult = activationFunction( Action->mInput );
				
				for( size_t WeightUpNode = 0; WeightUpNode < ConnectionUp->mActions.size(); ++WeightUpNode ) {
					cAction *ActionUp = ConnectionUp->mActions[ WeightUpNode ];

					ActionUp->mInput += Action->mResult  * Action->mWeight[ WeightUpNode ];
				}
			}
		}

		// Set outputs
		for( size_t Node = 0; Node < mConnections[  mConnections.size() -1 ]->mActions.size() ; ++Node ) {

			cAction *Action = mConnections[  mConnections.size() - 1 ]->mActions[ Node ];

			Action->mResult = activationFunction( Action->mInput );	
		}

		return mConnections[ mConnections.size() -1] ;
	}

	void CreateGroup( size_t pLayer, bool pName ) {
		string Name;
		size_t Number = 1;


		if( pName ) {  

			cout << "Group: (Provide a number to specify an amount of nodes)\n";
			cout << " Name: ";
			Name = cin.get();

			Number = atoi( Name.c_str() );
			if( Number > 0 ) {
				cout << " Name: ";
				Name = cin.get();
			}
		}

		if(Number == 0)
			Number = 1;

		bool sigmoid = false;

		while( Number-- ) {

			mConnections[pLayer]->AddAction( sigmoid, Name );
		}
	}
};


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

			if( blue == 0xFF && red == 0xFF & green == 0xFF ) {
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
double *Input_A_Red = InputBmp( "Pics\\24x12\\_A_Red.bmp", Inputs );

/*
double *Input__ = InputBmp( "Pics\\96x12\\________.bmp", Inputs );
double *InputA_ = InputBmp( "Pics\\96x12\\A_______.bmp", Inputs );
double *InputB_ = InputBmp( "Pics\\96x12\\B_______.bmp", Inputs );

double *Input_A = InputBmp( "Pics\\96x12\\_______A.bmp", Inputs );
double *Input_B = InputBmp( "Pics\\96x12\\_______B.bmp", Inputs );

double *InputBA = InputBmp( "Pics\\96x12\\B_______A_.bmp", Inputs);*/

double *Target = new double[Targets];

void TrainInitial( double *pInput, size_t pInputs, double *pTarget, size_t pTargets ) {

	//mMomentum = 0.001;
	//mLearningRate = 0.003;
	Network->mMomentum		= 0.01;
	Network->mLearningRate	= 0.1;
	Network->mErrorThresh	= 0.005;

	double Error = 0;

	cout << "Initial Training\n";

	for( size_t Iteration = 0; Iteration < 5; ++Iteration ) {

		Error = Network->Backward( pTarget, pTargets, pInput, pInputs  );

		cout << " Iteration: " << Iteration << " Error: " << Error << "\n";
	}
}

void Train( double *pInput, size_t pInputs, double *pTarget, size_t pTargets ) {
	double Error = 0;

	//Network->mMomentum		= 0.001;
	//Network->mLearningRate	= 0.002;
	Network->mErrorThresh	= 0.005;

	//Network->mLearningRate -= 0.00001;

	for( size_t Iteration = 0; Iteration < 7; ++Iteration ) {

		Error = Network->Backward( pTarget, pTargets, pInput, pInputs  );

		//cout << " Iteration: " << Iteration << " Error: " << Error << "\n";
	}

}

void TestRunExpected(  double *pInput, size_t pInputs, size_t pOutputNumber, size_t pOutputNumberMax, double pValue, string pName ) {
	cConnection *Outputs = 0;
	cout << "testing " << pName << "\n";

	Outputs = Network->Forward( pInput, pInputs );

	for( size_t pOutput = pOutputNumber; pOutput < pOutputNumberMax; ++pOutput ) 
		cout << " Node: " << pOutput << " : "  << std::setprecision(3) << Outputs->mActions[ pOutput ]->mResult << "\n";

	cout << "\n";

}

void TestRun( double *pInput, size_t pInputs, string pName ) {
	cConnection *Outputs = 0;

	cout << "\ntesting " << pName << "\n";

	Outputs = Network->Forward( pInput, pInputs );
	for( int x = 0 ; x < 4; ++x ) {

		cout << " " <<  x << ": " << std::setprecision(3) << Outputs->mActions[ x ]->mResult << "  ";
	}
	cout << "\n";
}

void TrainRun( double *pInput, size_t pInputs, size_t pOutput, double pOutputValue, string pName ) {

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
		//TrainRun( Input__, Inputs, 0, 0.0, "__" );

		// Train A_
		//TrainRun( InputA_, Inputs, 0, 1, "A_" );			// Node 0

		TrainRun( Input_A_Red, Inputs, 3, 1, "_A_Red" );


		// Train B_
		TrainRun( InputB_, Inputs,	1,	1, "B_" );		// Node 1

		// Train C_
		//TrainRun( InputB_, Inputs,	2,	1, "C_" );		// Node 2

		// Train _A
		//TrainRun( Input_A, Inputs, 0, 1, "_A" );

		// Train _B
		//TrainRun( Input_B, Inputs,	1,	1, "_B" );		// Node 1
		Network->Save("net.bin");
		TestRun( Input__, Inputs, "__");

		TestRunExpected( Input_A_Red, Inputs, 3, 4, 1, "_A (Red)"); 

		TestRunExpected( InputA_, Inputs, 0, 1, 1, "A_");
		TestRunExpected( Input_A, Inputs, 0, 1, 1, "_A" );

		TestRunExpected( InputB_, Inputs, 1, 2, 1, "B_");
		TestRunExpected( Input_B, Inputs, 1, 2, 1, "_B");

		TestRunExpected( InputC_, Inputs, 2, 3, 1, "C_");
		TestRunExpected( Input_C, Inputs, 2, 3, 1, "_C");

		TestRunExpected( InputBA, Inputs, 0, 2, 1, "AB");
		//( InputBA, Inputs, "BA");
	}

}

int main() {		
	srand (time(NULL));
	Run();

}
