#ifndef _PvAlign_h
#define _PvAlign_h

//This version use no table, since the limited Sram, 
//the drawback is the limited possible dp size (<=20?)

class PvAlign
{
  private :
    //float table[20][20];
    float *p,*q;
    int m,n;
  public :
    
    float dist(int i, int j){
      if (j == 1 and i == 1) return abs(p[i-1] - q[j-1]);
      
      else if( i > 1 and j >= 1){
                /*if (table[i-1][j] == -1)
                    table[i-1][j] = dist(i - 1, j);
                
                float x = table[i - 1][j];

                if (table[i - 1][j-1] == -1)
                    table[i - 1][j-1] = dist(i - 1, j - 1);
                
                
                float y = table[i - 1][j-1];*/

                float x = dist(i - 1, j);
                float y = dist(i - 1, j - 1);
                if (x < y)
                    return abs(p[i - 1] - q[j - 1]) + x;
                else
                    return abs(p[i - 1] - q[j - 1]) + y;
            }
      else return 999999;
    }
    
    float evaluate(float *p,float *q,int m,int n){
    this->m=m;
    this->n=n;
    /*
    for (int i=0 ;i<100;i++){
      for(int j=0 ; j<100; j++){
        this->table[i][j] = -1;
      }
    }*/
    
    this->p = p;
    this->q = q;
    float res = 9999991;
    for (int k=1;k<n+1;k++){
      float cur = dist(m, k);
      if(cur < res) res = cur;
    }
    return res;
    }

};


#endif
