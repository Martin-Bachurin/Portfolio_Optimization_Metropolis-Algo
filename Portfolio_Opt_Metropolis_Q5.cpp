// This function is found below.
void GetData ();

// "ticker" is a global variable.
char **ticker;

// The covariance matrix is a global variable.
double **V;
//Global Variables
double** Weights;
double** WeightsCopy;
double* Best;
double E, U, U1, U2;
int BuyPenny, SellPenny;

//Declared functions
void AllocateMemory();
void DeleteMemory();
void InitializePortfolio();
void NeighborProposal();
void Metropolis();
double Energy(double**);

#include <math.h>    // various math functions, such as exp()
#include <stdio.h>   // various "input/output" functions
#include <time.h>    // functions for timing computations
#include "Functions.h" //functions given
#include <iostream>

int main () {

   int i, seed;

   // Read in and display the tickers and covariance matrix.
   GetData ();

   // Show the 50 tickers.
   for (i = 1; i <= 50; i++) {
      printf (ticker[i]);
   }
   Pause ();

   // Seed the RNG.
   seed = GetInteger ("RNG Seed?... ");
   U = MTUniform (seed);

   /////////////////////////////////////////////////////////////////////////////
   // Your Metropolis algorithm goes here...

   //Function calls go here.
   AllocateMemory();

   InitializePortfolio();

   Metropolis();

   DeleteMemory();
   
   NeighborsEnergy();
   // Report the best-found portfolio and its variance here.
   
   // Pause so the execution window does not close.
   Exit ();

}



////////////////////////////////////////////////////////////////////////////////
// Allocate space for and read in covariance matrix and stock tickers.
////////////////////////////////////////////////////////////////////////////////
void GetData () {

   int i, j;
   double V0;
   char input[100];
   FILE *fp;

   // Allocate array space.
   V = Array (50, 50);

   // Allocate space to hold ticker names; "ticker" is a global variable.
   ticker = (char **) calloc (50+1, sizeof (char *));
   for (i = 0; i <= 50; i++) {
      ticker[i] = (char *) calloc (10, sizeof (char));
   }

   // Read in the data.
   fp = fopen ("V.txt", "r");
   for (i = 1; i <= 50; i++) {

      // Read in stock i's covariance data.

      // Name of the stock ticker.
      fgets (ticker[i], 9, fp);

      // The 50 covariances for stock "i".
      for (j = 1; j <= 50; j++) {

         // Read in V[i][j].
         fgets (input, 99, fp);
         sscanf (input, "%lf", &V0);

         // Put data into the V array.
         V[i][j] = V0;

      }

   }
   fclose (fp);

   return;

}
// create memory
void AllocateMemory(){
  // create a copy of weights for step reversion
  Weights = new double*[51];
  WeightsCopy = new double*[51];
  Best = new double[51];

  for(int i = 1; i<51; ++i){
    Weights[i] = new double[1];
    WeightsCopy[i] = new double[1];
  }

  Weights[0] = new double[2];
  Weights[0][0] = 50; Weights[0][1] = 1;

  WeightsCopy[0] = new double[2];
  WeightsCopy[0][0] = 50; WeightsCopy[0][1] = 1;
}
// delete memory
void DeleteMemory(){
  delete[] Best;
  for(int i = 0; i<51; ++i){
    delete[] Weights[i];
    delete[] WeightsCopy[i];
  }
  delete[] Weights;
  delete[] WeightsCopy;
}
// initaialize random portfolio
void InitializePortfolio(){
  // create variables
  int zeroes, zeroesLeft, a;
  double leftspace, p;
  // choose a random number of zero weight stocks to have in portfolio
  zeroes = int(U * 41);
  // dummy variable to know when to stop allocating money.
  zeroesLeft = zeroes;
  std::cout<<"Zeroes: "<<zeroes<<"\t Non-Zero Indexes: "<<50-zeroes<<"\n";
  a = 0; // counter
  while(a<50){ // while not at the end
    // calculate how much space is left in array
    leftspace = 50 - a;
    // the remaining zeroes that havent been allocated to a position
    // divided by the remaining space in the array will give a probability
    // for choosing a zero weight to allocate
    p = zeroesLeft / leftspace;
    // generate uniform
    U1 = MTUniform(0);
    // if less than that probability
    if(U1<p){
      // set weight to 0
      Weights[a+1][1] = 0;
      // take away zero to allocate somewhere
      --zeroesLeft;
      // increment a
      ++a;
    }
    else{
      // allocate an equal weight to the position
      Weights[a+1][1] = 100.0 / (50 - zeroes);
      // increment a
      ++a;
    }
  }
}

void Metropolis () {
   // create variables
   double T, DeltaE, p, E_min;
   int k, n, AcceptTransition, NextReport;

   // Get the temperature.
   T = GetDouble ("What is the temperature?... ");
   E = Energy(Weights);
   E_min = 100000000000;
   for(int j = 0; j<50; ++j){
     std::cout<<"Weight for "<<ticker[j+1]<<": "<<Weights[j+1][1]<<"\n";
   }
   Pause();

   // Time the computations.
   Time ();

   // Simulate the Markov chain 100 million steps. The portfolio weights will
   //   always hold the current value of the Markov chain X0, X1, X2,... .
   for (n = 1; n <= 10000000; n++) { // sim loop
      // copy weights to revert back if needed
      for(int i = 1; i < 51; ++i){
        WeightsCopy[i][1] = Weights[i][1];
      }
      // Get a proposed random change.
      NeighborProposal ();
      // Compute the change in energy associated with reversing that portion
      //   of the portfolio.
      DeltaE =  Energy(Weights) - E;
      // See if the proposed transition is accepted.
      AcceptTransition = 0;

      if (DeltaE <= 0) { // if decrease in energy
         AcceptTransition = 1;
      }
      else if (T > 0) { // calculate mu(y)/mu(x)
         p = exp(-DeltaE / T);
         U = MTUniform(0);
         if (U <= p) {
            AcceptTransition = 1;
         }
      }

      // Effect the transition. If the new energy is a best-so-far, record the
      //    portfolio (in best[*]) and the new minimal energy (Emin).
      if (AcceptTransition==1) {
         // Update the variance of the current portfolio (the portfolio's "energy").
         E += DeltaE;
         // Record data for the best portfolio, if appropriate.
         if (E < E_min) {
            E_min = E;
            for (k = 0; k < 50; k++) {
               Best[k] = Weights[k+1][1];
            }
         }
      } // End of "if" statement.
      else{
        // else revert back to old
        for(int i = 1; i < 51; ++i){
          Weights[i][1] = WeightsCopy[i][1];
        }
      }

      if(n == 1000000){
        printf ("Should be done in %.1f seconds.\n", (10)*Time());
      }
      // Show progress to the screen.
      if (n % 100000 == 0) {
         //Pause();
         printf (". ");
      }
   } // This ends the Markov chain simulation loop.

   // Report the best route found throughout the Markov chain.
   E = E_min;

   // Finish up; report best-found route length to the screen.
   printf ("\n\n");
   for(int j = 0; j<50; ++j){
     if(Best[j]!=0){
       std::cout<<"Weight for "<<ticker[j+1]<<": "<<Best[j]<<"\n";
     }
   }
   int zeroes = 0;
   // counting zeroes
   for(int i = 1; i<51; ++i){
     if(Best[i]==0){
       ++zeroes;
     }
   }
   std::cout<<"Zeroes: "<<zeroes<<"\t Non-Zero Indexes: "<<50-zeroes<<"\n";
   std::cout<<"The minimum variance is "<<E<<".\n";
   printf ("\n\n");
   printf ("100 hundred million Markov chain steps completed in %.1f seconds.\n", Time ());
   //printf ("View the solution with ShowRoutes.tex.\n");
}

////////////////////////////////////////////////////////////////////////////////
// Determine the proposed transition. //////////////////////////////////////////
void NeighborProposal () {
  // create variables
   int zeroes = 0;
   // count zeroes
   for(int i = 1; i<51; ++i){
     if(Weights[i][1]==0){
       ++zeroes;
     }
   }
   // if at boundary of 10 stocks only
   if(zeroes==40){
     // generate uniform
     U1 = MTUniform(0);
     // pick index
     BuyPenny = int(U1*50)+1;
     // if you pick an index that is allocated
     // force another index to be picked
     while(Weights[BuyPenny][1]!=0){
       U1 = MTUniform(0);
       BuyPenny = int(U1*50)+1;
     }
     // give that zero weight index some allocation
     Weights[BuyPenny][1] = 1;
     // one less zero weight index
     --zeroes;
     // reallocate portfolio
     for(int i = 1; i<51; ++i){
       if(Weights[i][1]!=0){
         Weights[i][1] = 100.0/(50-zeroes);
       }
     }
     //Pause();
   }
   // else at other boundary of every stock having allocation
   else if(zeroes==0){
     // generate uniform
     U1 = MTUniform(0);
     // pick an index
     BuyPenny = int(U1*50)+1;
     // take away its allocation
     Weights[BuyPenny][1] = 0;
     // one more zero weight index
     ++zeroes;
     // reallocate portfolio
     for(int i = 1; i<51; ++i){
       if(Weights[i][1]!=0){
         Weights[i][1] = 100.0/(50-zeroes);
       }
     }
   }
   else{
     // else in middle, pick randomly to add zero Indexes
     // or to take away a zero index
     U2 = MTUniform(0);
     // flip coin
     if(U2<0.5){
       // pick index to assign weight
       U1 = MTUniform(0);
       BuyPenny = int(U1*50)+1;
       while(Weights[BuyPenny][1]!=0){
         U1 = MTUniform(0);
         BuyPenny = int(U1*50)+1;
       }
       Weights[BuyPenny][1] = 1;
       --zeroes;
       // reallocate portfolio
       for(int i = 1; i<51; ++i){
         if(Weights[i][1]!=0){
           Weights[i][1] = 100.0/(50-zeroes);
         }
       }
     }
     else{
       // pick index to take allocation
       U1 = MTUniform(0);
       BuyPenny = int(U1*50)+1;
       while(Weights[BuyPenny][1]==0){
         U1 = MTUniform(0);
         BuyPenny = int(U1*50)+1;
       }
       Weights[BuyPenny][1] = 0;
       ++zeroes;
       //reallocate portfolio
       for(int i = 1; i<51; ++i){
         if(Weights[i][1]!=0){
           Weights[i][1] = 100.0/(50-zeroes);
         }
       }
     }
   }
 }


////////////////////////////////////////////////////////////////////////////////
// Determine Energy of New Portfolio
// variation of given variance function
double Energy(double** Portfolio){
   double** IntermediateProduct;
   double** Product;
   double** TransposePortfolio;
   TransposePortfolio = Transpose(Portfolio);
   double energy;
   IntermediateProduct = Multiply(V, Portfolio);
   Product = Multiply(TransposePortfolio, IntermediateProduct);
   energy = Product[1][1];
   delete[] IntermediateProduct;
   delete[] Product;
   delete[] TransposePortfolio;
   return energy;
 }

void NeighborsEnergy(){
   // create variable
   double OGenergy;
   int zeroesCalculation;
   int zeroes = 0;
   int done = 0;
   // store original portfolios energy
   OGenergy = Energy(Weights);
   std::cout<<"The Energy of the state is: "<<OGenergy<<"\n";
   
   // copy to revert back
   for(int i = 1; i < 51; ++i){
     WeightsCopy[i][1] = Weights[i][1];
   }
   // count zeroes
   for(int i = 1; i<51; ++i){
     if(Weights[i][1]==0){
       ++zeroes;
     }
   }
   // store dummy variable for calculations
   zeroesCalculation = zeroes;
   // go through each index 
   for(int j = 1; j<51; ++j){
     // if allocated, give it no allocation
     if(Weights[j][1]!=0){
       Weights[j][1] = 0;
       ++zeroesCalculation;
       // reallocate portfolio to get the neighbor
       for(int i = 1; i<51; ++i){
         if(Weights[i][1]!=0){
           Weights[i][1] = 100.0/(50-zeroesCalculation);
         }
       }
       // output neighboring portfolios energy
       std::cout<<"Neighoring State: "<<j<<" Energy: "<<Energy(Weights)<<"\n";
       // reset weights of portfolio
       for(int i = 1; i < 51; ++i){
         Weights[i][1] = WeightsCopy[i][1];
       }
       // reset zeroes
       zeroesCalculation = zeroes;
     }
     else{
       // else it doesnt have allocation,
       // so give it allocation
       Weights[j][1] = 1;
       --zeroesCalculation;
       // reallocate portfolio to get the neighbor
       for(int i = 1; i<51; ++i){
         if(Weights[i][1]!=0){
           Weights[i][1] = 100.0/(50-zeroesCalculation);
         }
       }
       // output neighboring portfolios energy
       std::cout<<"Neighoring State: "<<j<<" Energy: "<<Energy(Weights)<<"\n";
       // reset weights of portfolio
       for(int i = 1; i < 51; ++i){
         Weights[i][1] = WeightsCopy[i][1];
       }
       // reset zeroes
       zeroesCalculation = zeroes;
     }
   }
 }