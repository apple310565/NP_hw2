# NP_hw2
### 題目要求
下OX棋的網路程式
請設計一對client--server的OX棋下棋程式，具有下列幾個特性：

1. 允許多個client同時登錄至server。

2. client的使用者可以列出所有已登入的使用者名單。

3. client的使用者可以選擇要跟哪一個使用者下棋，並請求對方的同意。

4. 若對方同意後，開始進入棋局。

5. 雙方可輪流下棋，直到分出勝負或平手。

6. 登入的使用者可選擇登出。



ps.

1. 前述未詳盡指定的規格，同學可以用對自己最方便的方式自行定義。

2. 指令或執行畫面(例如棋盤...)，同學可自行定義。

3. 為方便起見，使用者的帳號與密碼可事先內建在server中，不用提供註冊功能。

### 程式流程
#### Server端
先設好socket後，每當有client要連線就創造一個新thread去處理它，並把client的sockfd放到fd陣列中。
和client端連線成功後會先要求client端登入，若登入成功並開始不段監聽client的要求。

(1)如果收到ls便列出所有的在線名單的帳號和對應的fd。

(2)若收到@fd，即表示想和哪個client發起對戰，server會發出CONNECT訊號去詢問被邀請的client端，若client端同意，回傳AGREE訊號，雙方建立連線開始遊戲。

(3)若收到#(0~8)代表想把圈圈叉叉下在哪個位置，server端會直接傳給對手的client端。

(4) 當成公聊訊息，傳給所有連線的client

#### Client端
先設好socket後連上server，要求使用者輸入帳密登入，成功後使用者便進入遊戲間。
client不停等著使用著的輸入和server傳來訊息。
遊戲開始後，提醒玩家到誰下，並每一輪都印出棋盤，不斷確認是否有分出輸贏或是平手。
##### 使用者輸入處理
(1)ls => 列出在線名單(除自己外)

(2)bye => 登出

(3)@fd => 發出邀請並建立連線

(4)#(0~8) => 下棋(要確認遊戲開始並且是輪到自己下時才能使用，否則會被擋掉)

(5)其餘=>在公頻上發出訊息
##### Server傳訊處理
(1)CONNECT => 輸出詢問是否想和某某某一起玩遊戲

(2)AGREE => 對方答應一起玩遊戲，正式開啟遊戲

(3)server-req-name ? => 登入

(4)#(0~8) => 對手要把棋下在哪裡

(5)其餘 => 直接印出來

以上是大致的流程，希望除了規定的ox遊戲外還有聊天功能在，可以加分。
