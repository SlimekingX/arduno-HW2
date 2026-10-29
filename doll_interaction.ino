// -------------------------------------------------------------
// HW2: Interactive Toy - 即按即震限制於 HAPPY 狀態版
// -------------------------------------------------------------

const int BTN_PIN = 2;     // 按鈕模組[cite: 5]
const int VIB_PIN = 5;     // 震動馬達 (PWM 腳位)[cite: 4, 7]
const int LED_R   = 9;     // RGB LED 紅光[cite: 6]
const int LED_B   = 10;    // RGB LED 藍光[cite: 6]
const int LED_G   = 11;    // RGB LED 綠光[cite: 6]

const bool IS_COMMON_ANODE = true; 

int hitCount = 0;
int score = 10;
unsigned long windowStartTime = 0;
unsigned long lastTouchTime = 0;
bool lastBtnState = LOW;

enum MoodState { SAD, HAPPY, SUPER_HAPPY, ANGRY };
MoodState currentMood = HAPPY;

void setRGB(int r, int g, int b) {
  if (IS_COMMON_ANODE) {
    r = 255 - r;
    g = 255 - g;
    b = 255 - b;
  }
  analogWrite(LED_R, r);
  analogWrite(LED_G, g);
  analogWrite(LED_B, b);
}

void setup() {
  Serial.begin(9600);
  
  pinMode(BTN_PIN, INPUT);
  pinMode(VIB_PIN, OUTPUT);
  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);

  windowStartTime = millis();
  lastTouchTime = millis();
  
  // 開機馬達測試[cite: 4, 7]
  digitalWrite(VIB_PIN, HIGH);
  delay(200);
  digitalWrite(VIB_PIN, LOW);
  
  Serial.println("Interactive Toy Initialized!");
}

void loop() {
  unsigned long now = millis();

  // 1. 讀取按鈕[cite: 5]
  bool currentBtnState = digitalRead(BTN_PIN);
  if (currentBtnState == HIGH && lastBtnState == LOW) {
    hitCount++;
    score++;
    lastTouchTime = now;
    
    // 限制：只有在 HAPPY 狀態下按壓才會有即按即震[cite: 4]
    if (currentMood == HAPPY) {
      digitalWrite(VIB_PIN, HIGH);
      delay(120);
      digitalWrite(VIB_PIN, LOW);
    }
    
    Serial.print("Touched! Hits: ");
    Serial.print(hitCount);
    Serial.print(" | Score: ");
    Serial.println(score);
  }
  lastBtnState = currentBtnState;

  // 2. 每 5 秒結算與切換情緒[cite: 1, 2]
  if (now - windowStartTime >= 5000) {
    if (now - lastTouchTime > 5000) {
      currentMood = SAD;          // 超過 5 秒未摸 -> 寂寞傷心[cite: 1]
      score = max(0, score - 2);
    } else if (hitCount > 20) {
      currentMood = ANGRY;        // 5 秒按 > 20 次 -> 生氣抗議[cite: 2]
      score = max(0, score - 5);
    } else if (hitCount > 10) {
      currentMood = SUPER_HAPPY;  // 5 秒按 11~20 次 -> 極度開心[cite: 2]
    } else if (hitCount >= 1) {
      currentMood = HAPPY;        // 5 秒按 1~10 次 -> 平常開心撫摸[cite: 2]
    }

    hitCount = 0;
    windowStartTime = now;
  }

  // 3. 執行該情緒專屬的燈光與馬達效果[cite: 1, 4]
  updateEffects(now);
}

void updateEffects(unsigned long now) {
  switch (currentMood) {
    case SAD:
      // 藍光呼吸燈 + 每 3 秒寂寞發抖（不被按壓點擊干擾）[cite: 1, 4]
      {
        int breath = (exp(sin(now / 1000.0 * M_PI)) - 0.36787944) * 108.0;
        setRGB(0, 0, breath);
        
        if (now % 3000 < 200) {
          analogWrite(VIB_PIN, 200);
        } else {
          analogWrite(VIB_PIN, 0);
        }
      }
      break;

    case HAPPY:
      // 常亮綠光[cite: 1]
      setRGB(0, 255, 0);
      break;

    case SUPER_HAPPY:
      // 紫粉光閃爍 + 連續咕嚕震動[cite: 1, 4]
      setRGB(255, 20, 147);
      if ((now / 250) % 2 == 0) {
        analogWrite(VIB_PIN, 220);
      } else {
        analogWrite(VIB_PIN, 0);
      }
      break;

    case ANGRY:
      // 紅光爆閃 + 強烈連續抗議震動[cite: 1, 4]
      if ((now / 100) % 2 == 0) {
        setRGB(255, 0, 0);
        digitalWrite(VIB_PIN, HIGH);
      } else {
        setRGB(0, 0, 0);
        digitalWrite(VIB_PIN, LOW);
      }
      break;
  }
}