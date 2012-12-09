// 本番用スケッチ
// 扉と部屋の中のセンサにより，人の出入りを検出
// サーバの動作としてフィードバック

// ライブラリ
#include <Servo.h>
#include <Bounce.h>

// サーボのインスタンス
Servo mServo;

// 各種入力ピン
const int SERVO  = 9;   // サーボ
const int DIST_S = 0;    // 距離センサ
const int DOOR_S = 3;    // ドアセンサ
const int ON_OFF = 4;    // オンオフスイッチ

// 各種定数
const int DIST_LIMIT = 50;   // 距離センサの閾値
const int TIME_LIMIT = 5;   // センサが反応してから待つ時間(sec)
const boolean IN_SIDE  = true;      // 距離センサが反応
const boolean OUT_SIDE = !IN_SIDE;  // ドアセンサが反応
const boolean GO_OUT = true;     // 外へ出た
const boolean INTO   = !GO_OUT;  // 中に入った

// チャタリング防止
// 5 msecの間余裕を持つ
Bounce bouncerOnOff = Bounce(ON_OFF, 5);
Bounce bouncerDoor = Bounce(DOOR_S, 5);

// 各種変数
int person = 0;   // 部屋の中の人数
boolean moveType = true;   // 動作モード
boolean doorLock = false;  // ドアが閉まるまで反応しないように

// 初期化
void initialize() {
  Serial.println("Initializing...");
  // 各種入出力
  pinMode(DOOR_S, INPUT);   // ドアセンサ
  pinMode(ON_OFF, INPUT);   // オン・オフスイッチ
  // 変数初期化
  person = 0;               // 人数
  // ドアセンサ読み取り
  if (digitalRead(DOOR_S) == LOW) doorLock = true;
  // サーボ初期化
  mServo.attach(SERVO);
  servoDemo();
  mServo.write(0);
  delay(2000);
  mServo.detach();
  // 初期化完了
  Serial.println("Initialize Done.");
}

// サーボのデモ (初期化したことを示すため)
void servoDemo() {
  Serial.println("Servo Demo...");
  int i;
  for(i = 0; i < 180; i++) {
    mServo.write(i);
    delay(15);
  }

  for (i = 180; i >= 1; i--) {
    mServo.write(i);
    delay(15);
  }
Serial.println("Servo Demo Done.");
}

// セットアップ
void setup() {
  // シリアル出力
  Serial.begin(9600);
  // 初期化
  initialize();
  // 初期化完了通知
  Serial.println("MODE: DEFAULT MODE");
}

// メインループ
void loop() {
  // チャタリングを防止しつつ値読み込み
  bouncerOnOff.update();
  bouncerDoor.update();
  int valueOnOff = bouncerOnOff.read();
  int valueDoor = bouncerDoor.read();

  // モード変更
  if (valueOnOff == HIGH) {
    String mode = "";
    moveType = !moveType;
    mode = moveType ? "MODE: DEFAULT MODE" : "MODE: GAME MODE";
    Serial.println("----- MODE CHANGE!!! -----");
    if (moveType) initialize();
    Serial.println(mode);
  }

  // 動作モードに応じて動作
  if (moveType) {
    // 距離センサが反応した時のイベント
    if (getDistance(DIST_S) < DIST_LIMIT) {
      if (!doorLock) {
        Serial.println("INSIDE Event");
        onSideEvent(IN_SIDE);
      }
    }
    // ドアが反応した時のイベント
    if (valueDoor == LOW) {
      if (!doorLock) {
        Serial.println("DOOR Event");
        onSideEvent(OUT_SIDE);
        doorLock = true;
      }
    }

    // 状態が初期化されるまで待つ
    while (getDistance(DIST_S) < DIST_LIMIT || doorLock) {
      bouncerDoor.update();
      valueDoor = bouncerDoor.read();
      if (valueDoor == HIGH) {
        doorLock = false;
      }
      delay(20);
    }
  }
  else {
    // TODO ゲームモード
  }
  // 安心のDelay
  delay(500);
}

// センサの結果から距離を返す変数
// @param  距離センサPinNo
// @return 距離 (cm)
int getDistance(int mSensor) {
  int temp = analogRead(mSensor);
  if (temp < 4) temp = 4;
  return (6787 / (temp - 3)) - 4;
}

// 距離センサ・ドアセンサが反応した時のイベント
boolean onSideEvent(boolean eventType) {
  // 距離センサ反応
  long start = millis();
  while (millis() - start < TIME_LIMIT * 1000) {
    if (eventType) {
      if (digitalRead(DOOR_S) == LOW) {
        onMoveEvent(eventType);
        doorLock = true;
        return true;
      }
    }
    else {
      if (getDistance(DIST_S) < DIST_LIMIT) {
        onMoveEvent(eventType);
        return true;
      }
    }
  }
  Serial.println("Event not happend.");
  return false;
}

// ドアが反応した時のイベント
void onMoveEvent(boolean eventType) {
  // 外に出た
  if (eventType) {
    if (person > 0) {
      person--;
    }
    Serial.println("Go Outside Person");
  }
  // 中に入った
  else {
    person++;
    Serial.println("Come Inside Person");
  }
  Serial.print("Person: ");
  Serial.println(person);
  moveServo(person);
}

// サーボの動作
void moveServo(int personSize) {
  int angle = personSize * 10;
  if (angle > 180) angle = 180;
  mServo.attach(SERVO);
  mServo.write(angle);
  delay(1500);
  mServo.detach();
}
