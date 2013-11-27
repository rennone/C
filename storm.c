//ツリーのノードのクラス型
class Node {
  string URL;                 //このノードのURL
  unsigned int access;    //このノードにアクセスした回数
  Node *next[20] = {NULL}; //このノード次のURL 0:URL0へのポインタ, 1: ~ 

  //コンストラクタ
  Node(string url){
    URL = url;  //URL名を受け取る
    access = 1;
  }

  //アクセススを増やす
  void countUp(){
    access++;
  }

}

//各URLに応じて配列のインデックスをつける
enum {
  URL0 = 0,
  URL1,
  //... 
  URL19
};

//各URLを始点とした場合の, Rootへのポインタ
Node *Root[20] = {NULL}; 



//Boltでの処理
void bolt1(TUPLE tuple){
  int userID = tuple.value(0);  //ユーザIDを取ってくる
  string url = tuple.value(1);  //アクセスしたURLを取ってくる
  
  int urlNum = //enumからURLを配列番号に変換
  Node *prev[20] = //DBからuserIDをつかって,(各URL機銃んと下)一つ前にアクセスしたノードへのポインタを取ってくる

    //まだこのURLにアクセスした事が無い
    if (prev[urlNum] == NULL){
      if(Root[urlNum] == NULL) {
	//そもそもこのURLにアクセスしたのがこいつが最初の場合
	Root[urlNum] = prev[urlNum] = new Node(url);
      }
      else{
	//Rootを取ってくる
	Root[urlNum]->CountUp();
	prev = Root[urlNum];
      }
         /*  DBのuserIDの部分に 一つ前にアクセスしたノードを更新    */
    }

  for(int i=0;i<20;i++){
    if(prev[i] == NULL) continue;  //まだこのURLにアクセスした事が無いのは切る

    if (prev[i]->next[urlNum] == NULL) {      //この流れで,アクセスした人は過去にいない

      if(Root[urlNum] == NULL){ 	//そもそもこのURLにアクセスしたのがこいつが最初の場合
	Root[urlNum] = prev[i]->next[urlNum] = new Node(url);
      }
      else{
	prev[i]->next[urlNum] = new Node(url);
      }

    }
    else {
      prev[i]->next[urlNum]->countUp();  //アクセス数を増やす
    }
    prev[i] = prev[i]->next[url];

    /*  DBのuserIDの部分に 一つ前にアクセスしたノードを更新    */
    
  }

}



