#include <algorithm>

using namespace std;

int main(){
	int n = 40000;
	int matrix[n][n];
	for(int j = 0; j < n; j++){
		for(int k =0; k < n; k++){
			matrix[j][k] = 0;
 	}
}
	int matb[n][n];
	int bsize = 16;
	for(int i = 0; i < n; i+=bsize){
		for(int j = 0; j < n; j+=bsize){
			for(int i1 = i; i1 <= min(i+bsize-1,n); i1++){
				for(int j1 = j; j1 <= min(j+bsize-1,n); j1++){
					matb[i1][j1] = matrix[j1][i1];
					
				}		
			}
		}
	}	
}
