#define CLIENT

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include <iostream>
#include <vector>

#include "common.h"//配列の出力、未提出札の更新（後々整理）
#include "myfunc.h"//CardとCard、Cardと場札の大きさチェック（？）

#include "Game.h"//サーバとの通信など
#include "changeCards.h"//カード交換の仕方
#include "Player.h"//プレイヤのクラス
#include "Table.h"//場の状態のクラス
#include "Yaku.h"//役のクラス
#include "makeYaku.h"//合法手集合の操作

#include "Delusion.h"//妄想による提出方法
#include "default.h"//デフォルトクライアント的な提出方法

#include "debug.h"

using namespace std;

int main(int argc,char* argv[]){
    cout << "game start." << endl;
    //引数のチェック 引数に従ってサーバアドレス、接続ポート、クライアント名を変更
    Game game(argc, argv);
    
    //ゲームに参加
    game.connectToServer();
    
    while( !game.isAllGameEnd() ){//まだ全試合が終わっていないなら
        /* 新しい1ゲームの始まり */
        /* ここで1ゲームの間使う変数の準備を行う */
        cout << game.getGameCount() << " game start." <<endl;
        
        //tableや手札等もgameに含めてもいいのかもしれないが、
        //一応1試合中に継続して使うものは出しておいてみる
        
        Table table;            //場の情報用の構造体
        Players players;        //players
        players.setup();
        
        int hands[8][15]={{0}};       //手札をおさめる変数
        int bafuda[8][15]={{0}};      //場札をおさめる変数
        //int gomi[8][15]={{0}};  //すでに提出されたカード集合
        int64 gomi = 0ull;
        
        /*ここまで*/
        
        game.receiveCards( hands );//最初のカードを受け取る
        //ここで富豪・大富豪の場合、貧民・大貧民から強いカードを搾取した後の手札が来ている。
        //つまり本来の手札+1,2枚の札が来ている
        //ここから貧民・大貧民にあげるカードを選ばなければいけない
        //UECdaのプロトコルによれば、手札としてもらう配列の[5][1]に交換するカードの枚数が入っている
        int give_cards_num = hands[5][1];//あげなきゃいけないカードは何枚あるか
        if( give_cards_num > 0 ){//交換する必要がある
            int change[8][15]={{0}};
            selectChangeCards(change, hands, give_cards_num);//交換する（押し付ける）カードを選択する
            game.sendCard(change);      //いらないカードをサーバに提示する
        }else{//カードを交換する必要がない場合
            //貧民・大貧民の場合、交換されるカードは"サーバーに勝手にとられてしまう"が
            //この時点では交換される前の手札が来ている。
            //つまりこの後、ゲームでもらう手札とこの時点での手札を比べると
            //富豪・大富豪から何を押し付けられたのかが確認できる。
        }

        while( !game.isOneGameEnd() ){            //1ゲームが終わるまでの繰り返し
            
            game.receiveCards( hands );//そのターンの手札を受け取り
            
            if( game.isFirstTime() ){   //最初回であれば
                table.firstGame(hands);     //最初のセッティング
                players.setPlayers(hands);  //プレイヤーの情報を保存
                addCardToBit(&gomi, hands);       //相手の持ちえないカードに自分の手札を入れる
            }
            else{                       //次回からは更新していく
                table.setBaInfo(hands); //場の状況の更新
                players.update(hands);  //プレイヤーの情報の更新
            }
            
            cout << players.turnId() << " ";
            table.print3();
            cout << " ( ";
            for(int i=0;i<players.size();i++){
                cout << players.id[i].passed;
            }
            cout <<" ";
            for(int i=0;i<players.size();i++){
                cout << players.id[i].cards_num << " ";
            }
            cout <<" ) ";
            if( players.turnId() == game.myPlayerNum() ){//自分のターンであるなら提出するカードを選択する
                cout << endl;
                cout << "[HANDS] ";
                printCardStr(hands);//何を持っているか
                cout << endl;
                cout << "Input Yaku number. " << endl;
    
                int cards[8][15]={{0}};
                Yaku select_cards;
                vector<Yaku> myYaku, legalYaku;        //自分の手札から生成できる合法手の集合体
                makeAllYaku(&myYaku, hands);//合法手をリストアップする
                pickAllLegalYaku(&legalYaku, myYaku, table);//現在局面で提出可能なものに絞る

                //2種類の提出方法
                //selectSubmitCardsByDelusion( &select_cards, table, players, myYaku, gomi, hands);//妄想による選択
                //selectSubmitCardsLikeDefault( &select_cards, table, myYaku );//標準クライアントライクな選択
                
                vector<Yaku> kaidan, pair, tanki;
                pickKaidan( &kaidan, legalYaku );sortYakuByRank( &kaidan, table.isKakumei() );
                pickPair( &pair, legalYaku );sortYakuByRank( &pair, table.isKakumei() );
                pickTanki( &tanki, legalYaku );sortYakuByRank( &tanki, table.isKakumei() );
                copy(pair.begin(),pair.end(),back_inserter(kaidan));
                copy(tanki.begin(),tanki.end(),back_inserter(kaidan));

                for( int i=0; i<kaidan.size(); i++){
                    cout << "[" << i << "] " <<  kaidan[i].getStr() << " " ;
                }
                cout << "[p]" << "PASS ";
                cout << "[c]" << "check identified cards " <<endl;
                while(1){
                    string input;
                    getline(cin, input);
                    if( input == "p" ){
                        select_cards = myYaku[myYaku.size()-1];
                        break;
                    }else if( input == "c" ){
                        int card[8][15]={{0}};
                        convBitTo815( card, gomi );
                        printCards( card );
                        continue;
                    }
                    //stringstream ss(input);
                    int yakuNum = atoi( input.c_str() );
                    if( yakuNum >=0 && yakuNum<kaidan.size() ){
                        select_cards = kaidan[yakuNum];
                        break;
                    }
                }
                
                select_cards.setBitTo815( cards );//815配列に成形する
                game.sendCard( cards );//サーバに提示する
                bool is_passed = game.isPassed();//受理されたかどうか確認する
                
                if(is_passed && select_cards.mNum>0){//提出したつもりだったのに受理されなかった
                	cout << "sippai!" << endl;
                    table.print();
                	cout << "hands" << endl;
                    print815(hands);
                    cout << "bafuda" << endl;
                    print815(bafuda);
                    cout << "submit" << endl;
                    select_cards.printBit2();
                    cout << select_cards.isJTanki() << endl;
                	exit(1);
                }
                else{//提出した
                    #ifdef COMMENT
                    cout << "BFD:";
                    cout << table.mBafuda.getStr();//場札は何か？
                    cout << " TFD:";
                    printCardStr(hands);//何を持っているか
                    cout << "(" << myYaku.size()-1 << ")";//いくつの合法手が作れるか
                    cout <<  "=>";//そんな中
                    cout << select_cards.getStr();//提出した札は
                    cout << endl;
                    #endif
                }
                
            }
            
            cout << " => ";
            game.receiveCards(bafuda);          //場札を取得する
            Yaku yaku;
            yaku.set815ToYaku( bafuda );    //役形式に変換する
            if( table.mBafuda.getCardBit()==yaku.getCardBit() ){   //見たけど同じカード（ターンの人がパスをした）
                players.pass();
                cout << "PASS ";
            }else{
                table.setBafuda( yaku );        //役を取り換える
                cout << yaku.getStr() << " ";
            }
            
            cout << endl;
            //addCard2(gomi, bafuda);             //すでに提出された札の更新
            gomi |= table.mBafuda.getCardBit();//すでに提出された札の更新

            game.checkGameEnd();
        }//1ゲームが終わるまでの繰り返しここまで
        players.sekijun.clear();
        players.agari.clear();
    }
        
    game.disconnectToServer();

    return 0;
}
