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

	double			 mDelta;

	string			 mName;

	cAction( const string &pName, const size_t &pInputCount, const double pThreshold ) {
		mDelta = 0;
		mInput = 0;

		mResult = 0;
		mThreshold = pThreshold;

		mWeight.reserve( pInputCount );


		for( unsigned int x = 0; x < pInputCount; ++x )  {
			mWeight.push_back( RandomNumber( -1, 1 ) );
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
		mLearningRate	= 0.0005;
		mErrorThresh	= 0.005;
	}

	double activationFunction(double x) {

		return 1/(1 + exp(-1*x));
	}

	double activationFunctionPrimed(double x) {

		return activationFunction(x)*(1 - activationFunction(x));
	}
	
	void Randomize( double *pVals, size_t pCount ) {

		for( size_t count = 0; count < pCount; ++count ) {

			pVals[count] = RandomNumber( -1, 1 );
		}
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

	cConnection *AddLayer( size_t pInputs, size_t pOutputs) {
		string Name = "";
		cConnection *Connection = new cConnection( pOutputs, pInputs, Name );

		mConnections.push_back( Connection );

		return Connection;
	}

	void ErrorLayer( const double *pTarget, const size_t pTargets  ) {

		cConnection *Connection = mConnections[ mConnections.size() - 1 ];
		cConnection *ConnectionUp = 0;

		// Output Layer first
		for( size_t Node = 0; Node < Connection->mActions.size(); ++Node ) {
			cAction *Action = Connection->mActions[ Node ];

			Action->mDelta = activationFunctionPrimed( Connection->mActions[ Node ]->mInput )
				* (pTarget[ Node ] - Connection->mActions[ Node ]->mResult);
		}



		for( int Layer = mConnections.size() - 2; Layer >= 0; --Layer ) {
			Connection = mConnections[Layer];
			ConnectionUp = mConnections[Layer+1];

			for( size_t Node = 0; Node < Connection->mActions.size(); ++Node ) {
				cAction *Action = Connection->mActions[ Node ];
				double Sum = 0;

				for( size_t LayerUpNode = 0; LayerUpNode < ConnectionUp->mActions.size(); ++LayerUpNode ) {

					cAction *ActionsUp = ConnectionUp->mActions[ LayerUpNode ];

					Sum += ActionsUp->mDelta * Action->mWeight[ LayerUpNode ];
				}

				//Connection->mActions[Node]->mError = Sum;
				Connection->mActions[Node]->mDelta = activationFunctionPrimed( Connection->mActions[Node]->mInput ) * Sum ;

			}
		}
	}

	void WeightAdjust( const double *pInput, const size_t pInputs ) {

		// Layers
		for( int Layer = mConnections.size() - 2; Layer >= 1; --Layer ) {
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
				Action->mWeight[ NodeIn ] += ( mLearningRate *  Action->mDelta * pInput[NodeIn] );
			}
		}
	}

	double MeanSqueared( cConnection *pConnection, const double *pTarget, const size_t pTargets ) {
		double sum = 0;

		for( size_t i = 0; i < pConnection->mActions.size(); ++i ) {


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
	 *
	 * @param pLayers  Number of hidden layers
	 * @param pInputs  Number of nodes on the input layer
	 * @param pOutputs Number of nodes on the output layer
	 **/
	void CreateLayers( const size_t pHiddenLayers, const size_t pInputs, const int pOutputs ) {
		int Outputs = pInputs;

		// Input Layer
		AddLayer( pInputs, Outputs );
		// First Hidden
		AddLayer( pInputs, Outputs );

		size_t MinLayer = pHiddenLayers;
		size_t DecSize = (pInputs - pOutputs) / 3;

		while( (pOutputs / Outputs) * 100 < 40 ) {

			// Ensure minimum layers
			if(MinLayer > 1) {

				// Enough outputs by two?
				if( (Outputs - (DecSize * 2)) >= pOutputs )
					Outputs -= DecSize;
			} else
				--MinLayer;

			if(Outputs < pOutputs) {
				Outputs = pOutputs;
				break;
			}

			AddLayer( pInputs, Outputs );
		}

		// Output Layer
		AddLayer( pInputs, pOutputs );
	}

	/**
	* Start with the result
	*
	**/
	double Backward( const double *pInput, const size_t pInputs, const double *pTarget, const size_t pTargets ) {

		if( mConnections.size() == 0 )
			CreateLayers( 7, pInputs, pTargets );

		double Error = 0;

		// Train
		for( size_t Iteration = 0; Iteration < 100; ++Iteration ) {

			Error = TrainPattern( pInput, pInputs, pTarget, pTargets );
		}

		return Error;
	}

	cConnection *Forward( const double *pInputs, const size_t pInputCount, bool pTrackOutputs = false ) {

		cConnection *Connection = mConnections[ 0 ];
		cConnection *ConnectionUp = 0;

		Connection->EraseInputs();

		// Load inputs
		for( size_t Node = 0; Node < pInputCount; ++Node ) {

			cAction *Action = Connection->mActions[ Node ];

			for( size_t UpNode = 0; UpNode < Connection->mActions.size(); ++UpNode ) {

				Action->mInput += pInputs[ UpNode ] * Action->mWeight[ UpNode ];

				if( pTrackOutputs ) {
					sHistory History( UpNode, Action->mWeight[ UpNode ], pInputs[ UpNode ] );

					Action->mStrengths.push_back( History );
				}
			}

			Action->mResult = activationFunction( Action->mInput );
		}

		// Run the network forward
		for( size_t Layer = 1; Layer < mConnections.size(); ++Layer ) {

			Connection = mConnections[ Layer ];
			ConnectionUp = mConnections[ Layer - 1 ];

			Connection->EraseInputs();

			// Each node on this layer
			for( size_t Node = 0 ; Node < Connection->mActions.size(); ++ Node ) {
				cAction *Action = Connection->mActions[ Node ];

				// Each input to this layer
				for( size_t UpNode = 0; UpNode < ConnectionUp->mActions.size(); ++UpNode ) {
					cAction *ActionUp = ConnectionUp->mActions[ UpNode ];

					Action->mInput += ActionUp->mResult * Action->mWeight[ UpNode ] ;

					if( pTrackOutputs ) {
						sHistory History( UpNode, Action->mWeight[ UpNode ], pInputs[ UpNode ] );

						Action->mStrengths.push_back( History );
					}
				}

				Action->mResult = activationFunction( Action->mInput );
			}
		}

		return mConnections[ mConnections.size() -1] ;
	}

	void ShowHow( const double *pInput, size_t pInputs, size_t pOutputNode ) {

		Forward( pInput, pInputs, true );

		for( size_t Layer = mConnections.size() - 1; Layer > 0 ; --Layer ) {
			cConnection *CurrentLayer = mConnections[Layer];

			for( size_t Node = 0; Node < CurrentLayer->mActions.size(); ++Node ) {

				// Only show one node on the output layer
				if( Node != pOutputNode && Layer == mConnections.size() - 1 ) 
					continue;

				cAction *CurrentNode = CurrentLayer->mActions[ Node ];

				cout << "Layer: " << Layer << " Node: " << Node << "\n";
				cout << "Influence from the nodes on Layer " << Layer - 1 << "\n";

				cout << "Input Node - Added Value\n";

				map< double, size_t > Results;

				// Go through all the results, and add them to the Results map
				for( vector< sHistory >::iterator StrIT = CurrentNode->mStrengths.begin(); StrIT != CurrentNode->mStrengths.end(); ++StrIT ) {

					double val = (*StrIT).mInputResult * (*StrIT).mWeight;

					Results.insert( make_pair( val, (*StrIT).mNode ) );
				}

				size_t count = 0;
				size_t total = Results.size();

				// Each connection which made this so
				for( map< double, size_t >::iterator MapIT = Results.begin(); MapIT != Results.end(); ++MapIT ) {

					if( count < 10 || count > (total - 10) )
						cout << ": N-" << (*MapIT).second << " Added-Val: " << (*MapIT).first << "\n";
					else
						if( count == 11 )
							cout << "\n";
				}
				cout << "\n";
			}

		}
	}

	void CreateGroup( size_t pLayer, string pName, size_t pNodes ) {

		while( pNodes-- ) {

			mConnections[pLayer]->AddAction( pName );
		}
	}
};
