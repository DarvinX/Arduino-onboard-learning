#define INTERVAL 200
#define SAMPLING_INTERVAL 64  // 15.6KHz
#define MIC A0
#define THRESHOLD 3
#define SAMPLE_NUM 100
#define FEATURE_NUM 64

int randomClass;
double featuresForFFT[FEATURE_NUM];
int8_t dataset[SAMPLE_NUM][FEATURE_NUM + 1];

void setup() {
  pinMode(MIC, INPUT);
  Serial.begin(9600);

  collectSamples();
  for (int sampleIndex = 0; sampleIndex < SAMPLE_NUM; sampleIndex++) {
    for (int dataPointIndex = 0; dataPointIndex < FEATURE_NUM; dataPointIndex++) {
      Serial.print(dataset[sampleIndex][dataPointIndex]);
      Serial.print(",");
    }
    Serial.println();
  }
}

void loop() {
  // Serial.println(readData());
}

void collectSamples() {
  for (int j = 0; j < SAMPLE_NUM; j++) {
    randomClass = random(2);

    Serial.print("provide sample no ");
    Serial.print(j);
    Serial.println(randomClass ? " class True" : " class False");
    dataset[j][0] = randomClass;

    while (!(readData() > THRESHOLD)) {}

    for (int i = 0; i < FEATURE_NUM; i++) {
      //      Serial.println(readData());
      dataset[j][i + 1] = readData();
      delayMicroseconds(SAMPLING_INTERVAL);
    }

    delay(INTERVAL);
  }
}

int16_t readData() {
  return ((analogRead(MIC) - 512) >> 2);
}
