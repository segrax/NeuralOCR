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

using namespace std;

#include "network.hpp"

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

	double			 mDelta;
	vector< double> mChange;

	double			 mError;

	string			 mName;

	cAction( const string &pName, const bool &pSigmoid, const size_t &pInputCount, const double pThreshold ) {
		mDelta = 0;
		mError = 0;

		mResult = 0;
		mWeight.reserve( pInputCount );
		mChange.reserve( pInputCount );
		mThreshold = pThreshold;

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
		mLearningRate	= 0.01;
		mErrorThresh	= 0.005;
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

	cConnection *AddLayer( size_t pInputs, size_t pOutputs ) {
		string Name = "";
		cConnection *Connection = new cConnection( pOutputs, pInputs, Name );

		mConnections.push_back( Connection );

		return Connection;
	}

	/**
	*
	**/
	cConnection *InsertNewLayer( const size_t pLayer ) {
		string Name = "";;

		cConnection *Layer = mConnections[pLayer];
		cConnection *LayerNext = mConnections[pLayer+1];

		cConnection *NewLayer= new cConnection( LayerNext->mActions.size() - 2, Layer->mActions.size(), Name );
		vector< cConnection* >::iterator LayerIT;

		size_t count = 0;

		for( LayerIT = mConnections.begin(); LayerIT != mConnections.end(); ++LayerIT, ++count ) {

			if( count == pLayer + 1 )
				break;
		}

		mConnections.insert( LayerIT, NewLayer );

		return NewLayer;
	}

	// 
	bool TrainNewLayer( size_t pLayer ) {

		cConnection *NewLayer = mConnections[ pLayer ];
		cConnection *PrevLayer = mConnections[ pLayer - 1 ];
		cConnection *NextLayer = mConnections[ pLayer + 1 ];

		for( size_t Node = 0; Node < NewLayer->mActions.size(); ++Node ) {

			// Every weight to the previous layer
			for( size_t NodeIn = 0; NodeIn < NewLayer->mActions[Node]->mWeight.size(); ++ NodeIn ) {

				if( NodeIn < NextLayer->mActions[Node]->mWeight.size() ) {

					NewLayer->mActions[Node]->mWeight[ NodeIn ] = NextLayer->mActions[Node]->mWeight[ NodeIn ];

					// TODO: Recalculate delta from here?

				} else
					NewLayer->mActions[Node]->mWeight[ NodeIn ] = RandomNumber( -1, 1 );
			}
			
		}

		return false;
	}

	void ErrorLayer( const double *pTarget, const size_t pTargets  ) {

		for( int Layer = mConnections.size() - 1; Layer >= 0; --Layer ) {
			cConnection *Connection = mConnections[Layer];

			for( size_t Node = 0; Node < Connection->mActions.size(); ++Node ) {
				double Error =0 ;

				// Output Layer
				if( Layer == mConnections.size() - 1 ) {

					Error = pTarget[ Node ] - Connection->mActions[ Node ]->mResult;

				} else {
					cConnection *ConnectionUp = mConnections[Layer+1];

					for( size_t LayerUpNode = 0; LayerUpNode < ConnectionUp->mActions.size(); ++LayerUpNode ) {

						cAction *ActionsUp = ConnectionUp->mActions[ LayerUpNode ];

						Error += ActionsUp->mWeight[ LayerUpNode ] * ActionsUp->mDelta;
					}


				}
				Connection->mActions[Node]->mError = Error;
				Connection->mActions[Node]->mDelta = Error * Connection->mActions[Node]->mResult * (1 - Connection->mActions[Node]->mResult);

			}
		}
	}

	void WeightAdjust() {

		for( size_t Layer = 2; Layer <= mConnections.size() - 1; ++Layer ) {

			cConnection *Connection = mConnections[Layer];
			cConnection *ConnectionUp = mConnections[Layer - 1];

			for( size_t Node = 0; Node < Connection->mActions.size(); ++Node ) {
				cAction *Action = Connection->mActions[ Node ];

				// k
				for( size_t NodeIn = 0; NodeIn < ConnectionUp->mActions.size(); ++NodeIn ) {

					double change = Action->mChange[ NodeIn ];

					change = ( mLearningRate * 
						Action->mDelta * 
						ConnectionUp->mActions[ NodeIn ]->mResult )

						+ (mMomentum * change);

					Action->mChange[ NodeIn ] = change;
					Action->mWeight[ NodeIn ] += change;
				}

				Action->mThreshold += mLearningRate * Action->mDelta;
			}
		}
	}

	double MeanSqueared( cConnection *pConnection ) {
		double sum = 0;

		for( size_t i = 0; i < pConnection->mActions.size(); ++i ) {

			sum += pow( pConnection->mActions[ i ]->mError, 2 );
		}

		return sum / pConnection->mActions.size();
	}

	double TrainPattern( const double *pInput, const size_t pInputs, const double *pTarget, const size_t pTargets ) {

		Forward( pInput, pInputs );

		ErrorLayer( pTarget, pTargets );
		WeightAdjust();

		double Error = MeanSqueared( mConnections[ mConnections.size() - 1 ] );

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

			AddLayer( pInputs, Sums / 1.2 );

			// Second last layer
			AddLayer( pInputs, Sums / 1.4 );
			// Output Layer
			AddLayer( pInputs, pTargets );

			// Correct still?
			mLayer = mConnections.size() - 2;

			CreateLayer = false;
		} 

		double Error = 0;

		// Train
		for( size_t Iteration = 0; Iteration < 100; ++Iteration ) {

			if( CreateLayer ) {
				CreateLayer = false;

				cConnection *NewLayer = InsertNewLayer( mLayer++ );

				CreateLayer = TrainNewLayer( mLayer );

				--Iteration;

				continue;
			}

			Error = TrainPattern( pInput, pInputs, pTarget, pTargets );
		}

		return Error;
	}

	cConnection *Forward( const double *pInputs, const size_t pInputCount ) {

		// Load inputs
		for( size_t Node = 0; Node < pInputCount; ++Node ) {

			
			mConnections[ 0 ]->mActions[Node]->mResult = pInputs[ Node ];
				
		}

		// Run the network forward
		for( size_t Layer = 1; Layer <= mConnections.size() - 1; ++Layer ) {

			cConnection *Connection = mConnections[ Layer ];
			cConnection *ConnectionUp = mConnections[ Layer - 1 ];

			
			for( vector<cAction*>::iterator ActIT = Connection->mActions.begin(); ActIT != Connection->mActions.end(); ++ActIT ) {

				double Result = (*ActIT)->mThreshold;

				for( size_t WeightUpNode = 0; WeightUpNode < ConnectionUp->mActions.size(); ++WeightUpNode ) {
					cAction *ActionUp = ConnectionUp->mActions[ WeightUpNode ];

					Result += ActionUp->mResult  * (*ActIT)->mWeight[ WeightUpNode ];
				}

	//			if( Action->mResult > Action->mThreshold )
					(*ActIT)->mResult = 1 / (1 + exp(-Result));
			}
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

			if( blue == 0xFF && red == 0xFF & green == 0xFF )
				Input[i] = 0;
			else
				Input[i] = 1;


			cout << Input[i];			
			++i;
		}
		cout << "\n";
	}

	pBytes = BufSize;
	return Input;
}

cNetwork *Network = 0;
size_t Inputs = 0, Targets = 26;
double *Input__ = InputBmp( "Pics\\24x12\\__.bmp", Inputs);
double *InputA_ = InputBmp( "Pics\\24x12\\A_.bmp", Inputs);
double *InputB_ = InputBmp( "Pics\\24x12\\B_.bmp", Inputs);
	
double *Input_A = InputBmp( "Pics\\24x12\\_A.bmp", Inputs);
double *Input_B = InputBmp( "Pics\\24x12\\_B.bmp", Inputs);
double *InputAB = InputBmp( "Pics\\24x12\\AB.bmp", Inputs);
double *InputBA = InputBmp( "Pics\\24x12\\BA.bmp", Inputs);

double *Target = new double[Targets];

void TrainInitial( double *pInput, size_t pInputs, double *pTarget, size_t pTargets ) {
	
		//mMomentum = 0.001;
		//mLearningRate = 0.003;
	Network->mMomentum		= 0.01;
	Network->mLearningRate	= 0.01;
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
	
	Network->mLearningRate += 0.00001;

	for( size_t Iteration = 0; Iteration < 3; ++Iteration ) {

		Error = Network->Backward( pTarget, pTargets, pInput, pInputs  );

		//cout << " Iteration: " << Iteration << " Error: " << Error << "\n";
	}

}

void TestRun( double *pInput, size_t pInputs, string pName ) {
	cConnection *Outputs = 0;

	cout << "testing " << pName << "\n";
	
	Outputs = Network->Forward( pInput, pInputs );
	for( int x = 0 ; x < 4; ++x ) {

		cout << x << ": " << Outputs->mActions[ x ]->mResult << "  ";
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
		TrainRun( Input__, Inputs, 0, 0.0, "__" );

		// Train A_
		TrainRun( InputA_, Inputs, 0, 1, "A_" );			// Node 0

		// Train B_
		TrainRun( InputB_, Inputs,	1,	1, "B_" );		// Node 1

		Network->Save("net.bin");
		// Train _A
		//TrainRun( Input_A, Inputs, 0, 0.2, "_A" );

		// Train _B
		//TrainRun( Input_B, Inputs,	1,	0.2, "_B" );		// Node 1

		TestRun( Input__, Inputs, "__");

		TestRun( InputA_, Inputs, "A_");
		//TestRun( Input_A, Inputs, "_A");

		TestRun( InputB_, Inputs, "B_");
		//TestRun( Input_B, Inputs, "_B");

		//TestRun( InputAB, Inputs, "AB");
		//( InputBA, Inputs, "BA");
	}



	/*cout << "testing _A\n";
	Outputs = Network->Forward( Input_A, Inputs );
	for( int x = 0 ; x < Outputs->mActions.size(); ++x ) {

		cout << x << ": " << Outputs->mActions[ x ]->mResult << "\n";
	}*/
}

int main() {		
	srand (time(NULL));
	Run();

	/*
	size_t InputsBl = 0;
	double *InputBl = InputBmp( "Pics\\_.bmp", InputsBl);
	size_t InputsA = 0;
	double *InputA = InputBmp( "Pics\\A.bmp", InputsA);
	size_t InputsB = 0;
	double *InputB = InputBmp( "Pics\\B.bmp", InputsB);
	size_t InputsC = 0;
	double *InputC = InputBmp( "Pics\\C.bmp", InputsC);
	*/

	//cNetwork Network( InputBl, InputsBl );
	/*
	size_t Results = 26;


	for( size_t it = 0; it < 100; ++it ) {
		cout << "Training Bl\n";
		Network.mInputSet( InputBl, InputsBl + 1);
		Result[0] = 0;Result[1] = 0;Result[2] = 0;
		Network.Backward( Result, Results );

		cout << "Training A\n";
		Network.mInputSet( InputA, InputsA + 1);
		Result[0] = 1;Result[1] = 0;Result[2] = 0;
		Network.Backward( Result, Results );

		cout << "Training B\n";
		Network.mInputSet( InputB, InputsB + 1);
		Result[0] = 0;Result[1] = 1;Result[2] = 0;
		Network.Backward( Result, Results );

		cout << "Training C\n";
		Network.mInputSet( InputC, InputsC + 1);
		Result[0] = 0;Result[1] = 0;Result[2] = 1;
		Network.Backward( Result, Results );

	cout << "testing B\n";
	cConnection *Outputs = Network.Forward( InputB, InputsB + 1);
	for( int x = 0 ; x < Outputs->mActions.size(); ++x ) {

		cout << x << ": " << Outputs->mActions[ x ]->mResult << "\n";
	}
	
	cout << "testing C\n";
	Outputs = Network.Forward( InputC, InputsC + 1);
	for( int x = 0 ; x < Outputs->mActions.size(); ++x ) {

		cout << x << ": " << Outputs->mActions[ x ]->mResult << "\n";
	}

	cout << "testing A\n";
	Outputs = Network.Forward( InputA, InputsA + 1);
	for( int x = 0 ; x < Outputs->mActions.size(); ++x ) {

		cout << x << ": " << Outputs->mActions[ x ]->mResult << "\n";
	}
	}*/
}
