#include <arduinoFFT.h>

#define INTERVAL 100
#define FEATURE_NUM 64
#define MIC A0
#define THRESHOLD 11
#define SAMPLE_NUM 50

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

arduinoFFT fft;

void setup() {
  // put your setup code here, to run once:
  pinMode(MIC, INPUT);
  Serial.begin(115200);
  Serial.println("t: train, p: predict");
  while (!Serial.available() > 0) {
  }
  switch (Serial.read()) {
    case 116:
      Serial.println("starting training");
      collectSamples();
      train();
      saveWeights();
      break;
    case 112:
      Serial.println("Prediction mode");
      predict();
      break;
  }

  Serial.println("Prediction mode");
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
  Serial.println("Training complete");
}

void saveWeights() {

}

void predict() {

}
void collectSamples() {
  for (int j = 0; j < SAMPLE_NUM; j++) {
    randomClass = random(2);

    Serial.print("provide sample no ");
    Serial.print(j);
    Serial.println(randomClass ? " class True" : " class False");
    dataset[j][0] = randomClass;

    while (!(readData() > THRESHOLD)) {
    }

    for (int i = 0; i < FEATURE_NUM; i++) {
      //      Serial.println(readData());
      featuresForFFT[i] = readData() - THRESHOLD;

    }

    fft.Windowing(featuresForFFT, FEATURE_NUM, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
    for (int i = 0; i < FEATURE_NUM; i++) {

      //      features[i] = abs(featuresForFFT[i]);
      dataset[j][i + 1] = min(abs(featuresForFFT[i]) * 100, 255);
      //      Serial.println(dataset[j][i + 1]);
    }

    delay(INTERVAL);
  }

  Serial.println("Sample Collection complete");
}

void loop() {

  Serial.print("Predicted Class: ");
  while (!(readData() >  THRESHOLD)) {}
  score = 0;
  for (int i = 0; i < FEATURE_NUM; i++) {
    //      Serial.println(readData());
    featuresForFFT[i] = readData() - THRESHOLD;

  }

  fft.Windowing(featuresForFFT, FEATURE_NUM, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  for (int i = 0; i < FEATURE_NUM; i++) {
    //      features[i] =
    score += weights[i] * min(abs(featuresForFFT[i]) * 100, 255);
    //      Serial.println(abs(featuresForFFT[i]));

  }

  Serial.println(score >= Bias ? "True" : "False");
  delay(INTERVAL);

}

int16_t readData() {
  return ((analogRead(MIC) - 512) >> 2);
}
