//#define USE_OLED

#ifdef USE_OLED
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
#endif

#include <arduinoFFT.h>

#define INTERVAL 100
#define FEATURE_NUM 32
#define MIC A0
#define THRESHOLD 11
#define SAMPLE_NUM 40

float features[FEATURE_NUM];
double featuresForFFT[FEATURE_NUM];
int randomClass;

int sumPos[FEATURE_NUM];
int sumNeg[FEATURE_NUM];

float uP = 0;
float uN = 0;

byte posCount = 0;
byte negCount = 0;

byte dataset[SAMPLE_NUM][FEATURE_NUM];
float weights[FEATURE_NUM];

float score;

float tNeg;
float tPos;

float Bias;

byte predCount = 0;
arduinoFFT fft;

void setup() {
  // put your setup code here, to run once:
  pinMode(MIC, INPUT);

  begin(115200);
#ifdef USE_OLED
  println("use oled");
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3c)) {
    println(F("Could not connect to the display"));
    while (true) {};
  }
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);

  println("use oled");
  display.println(F("T Hansda"));
  display.display();
  delay(2000);
  display.clearDisplay();
  display.println(F("Collecting Data ..."));
  display.display();
  delay(2000);
  collectSamples();

  display.clearDisplay();
  display.setCursor(0, 0);
  display.println(F("Training ..."));
  display.display();

  delay(2000);
  train();
#else
  println("t: train, p: predict");
  while (!available() > 0) {
  }
  switch (read()) {
    case 116:
      println("starting training");
      collectSamples();
      train();
      saveWeights();
      break;
    case 112:
      println("Prediction mode");
      predict();
      break;
  }
  println("Prediction mode");
#endif

}

void train() {
  for (byte i = 0; i < SAMPLE_NUM; i++)
  {
    // count N_pos and N_neg
    if (dataset[i][0] == 1) posCount++;
    if (dataset[i][0] == 0) negCount++;

    //    eq 3 and 4
    for (byte j = 1; j < FEATURE_NUM; j++)
    {

      if (dataset[i][0] == 1)
      {
        sumPos[j] += dataset[i][j];
      }

      if (dataset[i][0] == 0)
      {
        sumNeg[j] += dataset[i][j];
      }
    }
  }

  //  eq 5, calculate the weights
  for (byte j = 1; j < FEATURE_NUM; j++)
  {
    uP = sumPos[j] / posCount;
    uN = sumNeg[j] / negCount;
    if (uP + uN == 0) weights[j] = 0;
    else
      weights[j] = (uP - uN) / (uP + uN);
  }


  //eq 7 and 8
  for (byte ii = 0; ii < SAMPLE_NUM; ii++)
  {
    score = 0;
    for (byte j = 1; j < FEATURE_NUM; j++)
    {
      score += weights[j] * dataset[ii][j];
    }
    (dataset[ii][0] == 0) ? tNeg += score : tPos += score;

  }

  //  eq 9
  Bias = (negCount * (tPos / posCount) + posCount * (tNeg / negCount)) / (negCount + posCount);
  //  println("Training complete");
#ifdef USE_OLED
  display.clearDisplay();
#endif
}

void saveWeights() {

}

void predict() {

}

void collectSamples() {
  for (int j = 0; j < SAMPLE_NUM; j++) {
    randomClass = random(2);

#ifdef USE_OLED
    display.setCursor(0, 0);
    display.clearDisplay();
    display.print(F("No. "));
    display.println(j);
    if (randomClass == 1) {
      display.println(F("class Pos"));
    } else {
      display.println(F("class Neg"));
    }
    display.display();
#else
    print("provide sample no ");
    print(j);
    println(randomClass ? " class True" : " class False");
#endif
    dataset[j][0] = randomClass;

    while (!(readData() > THRESHOLD)) {
    }

    for (int i = 0; i < FEATURE_NUM; i++) {
      //      println(readData());
      featuresForFFT[i] = readData() - THRESHOLD;

    }

    fft.Windowing(featuresForFFT, FEATURE_NUM, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
    for (int i = 0; i < FEATURE_NUM; i++) {

      //      features[i] = abs(featuresForFFT[i]);
      dataset[j][i + 1] = min(abs(featuresForFFT[i]) * 100, 255);
      //      println(dataset[j][i + 1]);
    }

    delay(INTERVAL);
  }

#ifdef USE_OLED
  display.clearDisplay();
  display.println(F("Sample collected"));
  display.display();
  delay(2000);
#else
  println("Sample Collection complete");
#endif
}

void loop() {

#ifdef USE_OLED
  println("predicting");
  display.clearDisplay();

  display.setCursor(0, 0);
  display.print(F("Pred no."));
  display.println(predCount);
  display.display();
#else
  print("Predicted Class: ");
#endif
  while (!(readData() >  THRESHOLD)) {}
  score = 0;
  for (int i = 0; i < FEATURE_NUM; i++) {
    //      println(readData());
    featuresForFFT[i] = readData() - THRESHOLD;

  }

  fft.Windowing(featuresForFFT, FEATURE_NUM, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  for (int i = 0; i < FEATURE_NUM; i++) {
    //      features[i] =
    score += weights[i] * min(abs(featuresForFFT[i]) * 100, 255);
    //      println(abs(featuresForFFT[i]));

  }


#ifdef USE_OLED
  display.clearDisplay();
  //  display.setCursor(0, 0);
  if (score >= Bias) {
    display.print(F("Positive"));
  } else {
    display.print(F("Negative"));
  }
  //  display.print(score >= Bias ?  : );
  //  display.println(predCount);
  display.display();
  delay(1500);
#else
  println(score >= Bias ? "True" : "False");
  delay(INTERVAL);
#endif

  predCount++;
}

int16_t readData() {
  return ((analogRead(MIC) - 512) >> 2);
}

void begin(unsigned int boudrate) {
  unsigned int ubrr = F_CPU / 16 / boudrate - 1;

  UBRR0H = (unsigned char)(ubrr >> 8);
  UBRR0L = (unsigned char)(ubrr);

  UCSR0B = (1 << RXEN0) | (1 << TXEN0);
  UCSR0C = (1 << USBS0) | (3 << UCSZ00);
}

void transmit(char data) {
  while (!(UCSR0A & (1 << UDRE0)))
    ;
  UDR0 = data;
}

void print(char *data) {
  while (data != "") {
    transmit(*data);
    data++;
  }
}

void println(char *data) {
   while (*data != '\0') {
    transmit(*data);
    data++;
  }
  transmit('\n');
  transmit('\r');
}

char read(){
  return UDR0;
}

bool available(){
  return !(UCSR0A & (1<<RXC0));
}
