/**
*          Network Builder
* 
* Copyright (c) 2013 Robert Crossfield
*
*              Strobs Canardly Systems
*
*                                  ScS
**/

class cAction {
public:
	vector< double > mWeight;

	double			 mThreshold;
	double			 mResult;
	double			 mInput;

	vector<double>	 mChange;
	double			 mError;
	double			 mDelta;

	string			 mName;

	/**
	 * Constructor
	 * 
	 * @param pName       Name of the action
	 * @param pInputCount Number of inputs to this node (action)
	 * @param pThreshold  Threshold value for this node
	 **/
	cAction( const string &pName, const size_t &pInputCount, const double pThreshold ) {
		mDelta = 0;
		mInput = 0;
		mError = 0;

		mResult = 0;
		mThreshold = pThreshold;
		mChange.reserve( pInputCount );
		mWeight.reserve( pInputCount );


		for( unsigned int x = 0; x < pInputCount; ++x )  {
			mWeight.push_back( RandomNumber( -1 , 1 ) );
			mChange.push_back(0);
		}
	}
};

class cConnection {
public:
	vector< cAction* >  mActions;

	size_t              mActionsIn;
	string              mName;

	/**
	 * Constructor
	 * 
	 * @param pActions   Number of nodes (actions) in the layer
	 * @param pActionsIn Number of inputs to the layer
	 * @param pName      Name of the layer
	 **/
	cConnection( const size_t &pActions, const size_t &pActionsIn, const string &pName ) {
		mActions.resize( pActions );

		mActionsIn = pActionsIn;

		for( size_t Node = 0; Node < pActions; ++Node ) {

			mActions[ Node ] = new cAction( pName, pActionsIn, RandomNumber(-1,1) );
		}
	}
	
	/**
	 * Reset the inputs
	 **/
	void EraseInputs() {

		for( size_t Node = 0; Node < mActions.size(); ++Node ) {
			mActions[Node]->mInput = 0;
		}
	}
	
	/**
	 * Add an action to the network
	 * 
	 * @param pName Name of the layer
	 * 
	 * @return The new node (action)
	 **/
	cAction *AddAction( string &pName ) {
		cAction *Action = new cAction( pName, mActionsIn, RandomNumber(-1,1) );

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
	 * Constructor
	 *  Setup some basic values
	 **/
	cNetwork() {

		mLayer = 0;

		mMomentum		= 0.01;
		mLearningRate	= 0.01;
		mErrorThresh	= 0.005;
	}

	/**
	 * Randomize an array with randoms between -1 and 1
	 * 
	 * @param pVals  Array to fill with random numbers
	 * @param pCount Number of randoms to generate
	 * 
	 * @return void
	 **/
	void Randomize( double *pVals, size_t pCount ) {

		for( size_t count = 0; count < pCount; ++count ) {

			pVals[count] = RandomNumber( -1, 1 );
		}
	}

	/**
	 * Calculate the error value for each layer, starting at the output layer,
	 * moving backwards towards the input layer
	 * 
	 * @param pTarget  Expected values for the output layer
	 * @param pTargets Number of target values
	 * 
	 * @return void
	 **/ 
	void ErrorLayer( const double *pTarget, const size_t pTargets  ) {

		for( int Layer = mConnections.size() - 1; Layer >= 0; --Layer ) {
			cConnection *Connection = mConnections[Layer];

			for( size_t Node = 0; Node < Connection->mActions.size(); ++Node ) {
				cAction *Action = Connection->mActions[Node];
				double Error =0 ;

				// Output Layer
				if( Layer == mConnections.size() - 1 ) {

					Error = pTarget[ Node ] - Action->mResult;

				} else {
					cConnection *ConnectionUp = mConnections[Layer+1];

					for( size_t LayerUpNode = 0; LayerUpNode < ConnectionUp->mActions.size(); ++LayerUpNode ) {

						cAction *ActionsUp = ConnectionUp->mActions[ LayerUpNode ];

						Error += ActionsUp->mWeight[ LayerUpNode ] * ActionsUp->mDelta;
					}


				}
				Action->mError = Error;
				Action->mDelta = Error * Action->mResult * (1 - Action->mResult);

			}
		}
	}

	/**
	 * Adjust the weights in each layer towards the required weight to achieve the 'target' 
	 * which was previously run backward through the network
	 **/
	void WeightAdjust() {

		for( size_t Layer = 1; Layer <= mConnections.size() - 1; ++Layer ) {

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

				//Action->mThreshold += mLearningRate * Action->mDelta;
			}
		}
	}

	/**
	 * Calculate the mean squeared value of the error for a layer
	 * 
	 * @param pConnection The layer to calculate from
	 * 
	 * @return The sum of the errors
	 **/ 
	double MeanSqueared( cConnection *pConnection ) {
		double sum = 0;

		for( size_t i = 0; i < pConnection->mActions.size(); ++i ) {

			sum += pow( pConnection->mActions[ i ]->mError, 2 );
		}

		return sum / pConnection->mActions.size();
	}

	/**
	 * Train the network towards a certain set of targets, based on a certain set of inputs
	 * 
	 * @param pInput   The input data
	 * @param pInputs  The number of inputs
	 * @param pTarget  The Target data
	 * @param pTargets The number of targets
	 * 
	 * @return The error (difference between expected and actual result)
	 **/ 
	double TrainPattern( const double *pInput, const size_t pInputs, const double *pTarget, const size_t pTargets ) {

		Forward( pInput, pInputs );

		ErrorLayer( pTarget, pTargets );
		WeightAdjust();

		double Error = MeanSqueared( mConnections[ mConnections.size() - 1 ] );

		return Error;
	}

	/**
	 * Run the target data backwards through the network
	 * 
	 * @param pInput   The input data
	 * @param pInputs  The number of inputs
	 * @param pTarget  The Target data
	 * @param pTargets The number of targets
	 * 
	 * @return The error (difference between expected and actual result)
	 **/
	double Backward( const double *pInput, const size_t pInputs, const double *pTarget, const size_t pTargets ) {

		if( mConnections.size() == 0 )
			CreateLayers( 2, pInputs, pTargets );

		double Error = 0;

		// Train
		for( size_t Iteration = 0; Iteration < 3; ++Iteration ) {

			Error = TrainPattern( pInput, pInputs, pTarget, pTargets );
		}

		return Error;
	}

	/**
	 * Run a set of inputs through the network
	 * 
	 * @param pInputs     Input Data
	 * @param pInputCount Number of bytes in the input 
	 * 
	 * @return Output Layer
	 **/
	cConnection *Forward( const double *pInputs, const size_t pInputCount ) {

		// Load inputs
		for( size_t Node = 0; Node < pInputCount; ++Node ) {

			
			mConnections[ 0 ]->mActions[Node]->mResult = pInputs[ Node ];
				
		}

		// Run the network forward
		for( size_t Layer = 1; Layer <= mConnections.size() - 1; ++Layer ) {

			cConnection *Connection = mConnections[ Layer ];
			cConnection *ConnectionUp = mConnections[ Layer - 1 ];

			
			for( size_t Node = 0; Node < Connection->mActions.size(); ++Node ) {
				cAction *Action = Connection->mActions[ Node ];

				double Result = 0;

				for( size_t WeightUpNode = 0; WeightUpNode < ConnectionUp->mActions.size(); ++WeightUpNode ) {
					cAction *ActionUp = ConnectionUp->mActions[ WeightUpNode ];

					Result += ActionUp->mResult  * Action->mWeight[ WeightUpNode ];
				}

	//			if( Action->mResult > Action->mThreshold )
					Action->mResult = 1 / (1 + exp(-Result));
			}
		}

		return mConnections[ mConnections.size() -1] ;
	}
	
	/**
	 * Load a saved network
	 * 
	 * @param pFile A saved network to be loaded
	 * 
	 * @return True if file was loaded
	 **/ 
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

	/**
	 * Save the network to a file
	 * 
	 * @param pFile File to save the network to
	 * 
	 * @return void
	 **/
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
	
	/**
	 * Create a layer in the network
	 * 
	 * @param pLayer Layer number
	 * @param pName  Name of the layer
	 * @param pNodes Number of nodes in the layer
	 * 
	 * @return void
	 **/
	void CreateGroup( size_t pLayer, string pName, size_t pNodes ) {

		while( pNodes-- ) {

			mConnections[pLayer]->AddAction( pName );
		}
	}
	
	/**
	 * Add a new layer to the network
	 * 
	 * @param pInputs  Number of inputs to the layer
	 * @param pOutputs Number of outputs from the layer
	 * 
	 * @return The new layer
	 **/ 
	cConnection *AddLayer( size_t pInputs, size_t pOutputs) {
		string Name = "";
		cConnection *Connection = new cConnection( pOutputs, pInputs, Name );

		mConnections.push_back( Connection );

		return Connection;
	}

	/**
	 * Create a set of layers
	 * 
	 * @param pHiddenLayers Number of hidden layers
	 * @param pInputs       Number of nodes on the input layer
	 * @param pOutputs      Number of nodes on the output layer
	 * 
	 * @return void
	 **/
	void CreateLayers( const size_t pHiddenLayers, const size_t pInputs, const int pOutputs ) {
		int Outputs = pInputs;

		// Input Layer
		AddLayer( pInputs, Outputs );

		int MinLayer = pHiddenLayers;
		int DecSize = (pInputs - pOutputs) / (pOutputs/2);

		while( (pOutputs / Outputs) * 100 < 40 ) {

			// Ensure minimum layers
			if(MinLayer <= 1) {

				// Enough outputs by two?
				if( (Outputs - (DecSize * 2)) >= pOutputs )
					Outputs -= DecSize;
				else
					break;
			} else
				--MinLayer;

			AddLayer( pInputs, Outputs );
		}

		// Output Layer
		AddLayer( pInputs, pOutputs );

		cConnection *Connection = mConnections[0];

		for( size_t Node = 0; Node < Connection->mActions.size(); ++Node ) {
			cAction *Action = Connection->mActions[Node];

			for( size_t Weight = 0; Weight < Action->mWeight.size(); ++Weight ) 
				Action->mWeight[ Weight ] = 0;
		}
		
	}
};
