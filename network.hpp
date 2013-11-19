struct sHistory {

	double mWeight;
	double mInputResult;
	size_t mNode;

	sHistory( size_t pNode, double pWeight, double pResult ) {
		mNode = pNode;

		mWeight = pWeight;
		mInputResult = pResult;

	}

};

class cAction {
public:
	vector< double > mWeight;
	vector< sHistory > mStrengths;

	double			 mThreshold;
	double			 mResult;
	double			 mInput;

	vector<double>	 mChange;
	double			 mError;
	double			 mDelta;

	string			 mName;

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

	cConnection( const size_t &pActions, const size_t &pActionsIn, const string &pName ) {
		mActions.resize( pActions );

		mActionsIn = pActionsIn;

		for( size_t Node = 0; Node < pActions; ++Node ) {

			mActions[ Node ] = new cAction( pName, pActionsIn, RandomNumber(-1,1) );
		}
	}

	void EraseInputs() {

		for( size_t Node = 0; Node < mActions.size(); ++Node ) {
			mActions[Node]->mInput = 0;
			mActions[Node]->mStrengths.clear();
		}
	}

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

	void Randomize( double *pVals, size_t pCount ) {

		for( size_t count = 0; count < pCount; ++count ) {

			pVals[count] = RandomNumber( -1, 1 );
		}
	}

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
	void CreateGroup( size_t pLayer, string pName, size_t pNodes ) {

		while( pNodes-- ) {

			mConnections[pLayer]->AddAction( pName );
		}
	}
	
	cConnection *AddLayer( size_t pInputs, size_t pOutputs) {
		string Name = "";
		cConnection *Connection = new cConnection( pOutputs, pInputs, Name );

		mConnections.push_back( Connection );

		return Connection;
	}

	/**
	 *
	 * @param pLayers  Number of hidden layers
	 * @param pInputs  Number of nodes on the input layer
	 * @param pOutputs Number of nodes on the output layer
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
