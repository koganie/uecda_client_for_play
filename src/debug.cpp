#include <iostream>
#include <string>
#include "debug.h"

using namespace std;

void print515(int data[5][15]){
    for(int i=0; i<5; i++){
        for(int j=0; j<15; j++){
            cout << data[i][j] << " ";
        }
        cout << endl;
    }
}

void printCards(int data[5][15]){
    std::string SUIT="SHDC";
    cout << "  3 4 5 6 7 8 9 X J Q K A 2"<< endl;
    for(int i=0; i<4; i++){
        cout << SUIT[i] << " ";
        for(int j=1; j<=13; j++){
            cout << data[i][j] << " ";
        }
        cout << endl;
    }
    if( data[4][1]!=0 ){
        cout << "J 1" << endl;
    }else{
        cout << "J 0" << endl;
    }
}


void print815(int data[8][15]){
    for(int i=0; i<8; i++){
        for(int j=0; j<15; j++){
            cout << data[i][j] << " ";
        }
        cout << endl;
    }
}

void printCardStr(int cards[8][15]){
    std::string SUIT="SHDC";
    std::string RANK="-3456789XJQKA2";
    for(int j=1;j<=13;j++){
        for(int i=0;i<4;i++){
            if( cards[i][j]!=0 ){
                std::cout << SUIT[i] << RANK[j] << " ";
            }
        }
    }
    if(cards[4][1]!=0){
        std::cout << "JR";
    }
}
