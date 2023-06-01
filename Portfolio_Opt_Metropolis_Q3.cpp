// This function is found below.
void GetData ();

// "ticker" is a global variable.
char **ticker;

// The covariance matrix is a global variable.
double **V;
//Global Variables
double** Weights;
double* Best;
double E, U, U1, U2;
int BuyPenny, SellPenny;

//Declared functions
void AllocateMemory();
void DeleteMemory();
void InitializePortfolio();
void NeighborProposal(double**);
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
// Create Memory for the weights of Portfolio
// and the best Portfolio
void AllocateMemory(){
  Weights = new double*[51];
  Best = new double[51];

  for(int i = 1; i<51; ++i){
    Weights[i] = new double[1];
  }

  Weights[0] = new double[2];
  Weights[0][0] = 50; Weights[0][1] = 1;
}
// Delete the memory allocated for calculations
void DeleteMemory(){
  delete[] Best;
  for(int i = 0; i<51; ++i){
    delete[] Weights[i];
  }
  delete[] Weights;
}
//Random generate a initial Portfolio
void InitializePortfolio(){
  // create needed variables
  double allocation, sum, total;
  int a;
  a = 1; // counter
  sum = 0; // total allocation
  total = 10000; // total dollars available * 100
  while(1){ // while on
    // choose a random amount of dollars
    allocation = int(total * U);
    // subtract allocation from total available money
    total -= allocation;
    // put into dollars
    allocation = allocation/100.0;
    // if at last stock, i will put a penny in it.
    // due to the interger method on the allocation
    if(a==50){
      Weights[a][1]= 100 - sum;
      break;
    }
    sum += allocation; // keep running total of allocation
    // set weight
    Weights[a][1]=allocation;
    //Generate next uniform
    U = MTUniform(0);
    // increment a
    ++a;
  }
}

void Metropolis () {
  //create variables
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

   // Simulate the Markov chain 100 million steps. The portfolio  will
   //   always hold the current value of the Markov chain X0, X1, X2,... .
   for (n = 1; n <= 10000000; n++) {

      // Get a proposed random change.
      NeighborProposal (Weights);

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
        // if we didnt accept the transition,
        // reverse changes done to the pointer
        Weights[BuyPenny][1] = Weights[BuyPenny][1]-0.01;
        Weights[SellPenny][1] = Weights[SellPenny][1]+0.01;
      }

      if(n == 100000){
        printf ("Should be done in %.1f seconds.\n", (100)*Time());

      }
      // Show progress to the screen.
      if (n % 10000 == 0) {
         printf (". ");
      }

   } // This ends the Markov chain simulation loop.

   // Report the best portfolio found throughout the Markov chain.
   E = E_min;

   printf ("\n\n");
   for(int j = 0; j<50; ++j){
     std::cout<<"Weight for "<<ticker[j+1]<<": "<<Best[j]<<"\n";
   }
   std::cout<<"The minimum variance is "<<E<<".\n";
   printf ("\n\n");
   printf ("100 hundred million Markov chain steps completed in %.1f seconds.\n", Time ());
   //printf ("View the solution with ShowRoutes.tex.\n");
}

////////////////////////////////////////////////////////////////////////////////
// Determine the proposed transition. //////////////////////////////////////////
void NeighborProposal (double** CurrentWeights) {
   // generate a uniform
   U1 = MTUniform(0);
   //pick an index to buy a penny
   BuyPenny = int(U1*50)+1;
   while(1){//while on
     // generate another uniform
     U2 = MTUniform(0);
     // pick that index to sell a penny
     SellPenny = int(U2*50)+1;
     // if it is different, then select it.
     if(BuyPenny != SellPenny){
       break;
     }
   }
   // Increment weights by a penny
   Weights[BuyPenny][1] = CurrentWeights[BuyPenny][1] + 0.01;
   Weights[SellPenny][1] = CurrentWeights[SellPenny][1] - 0.01;
 }


////////////////////////////////////////////////////////////////////////////////
// Determine Energy of New Portfolio
// function is a variation of the given variance function.
double Energy(double** Portfolio){
   double energy;
   // check if any stocks are negative
   for(int i = 1; i<51; ++i){
     if(Portfolio[i][1]<-0.005){
       // if negative then set energy to something unreasonable
       energy = 100000;
       return energy;
     }
   }
   // otherwise same function
   double** IntermediateProduct;
   double** Product;
   double** TransposePortfolio;
   TransposePortfolio = Transpose(Portfolio);
   IntermediateProduct = Multiply(V, Portfolio);
   Product = Multiply(TransposePortfolio, IntermediateProduct);
   energy = Product[1][1];
   delete[] IntermediateProduct;
   delete[] Product;
   delete[] TransposePortfolio;
   return energy;
 }
