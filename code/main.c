// ================= PIN DEFINITIONS =================
#define SDA_PIN PB_3
#define SCL_PIN PB_2
#define OLED_ADDR 0x3C

#define TRIG_PIN PB_5
#define ECHO_PIN PB_4
#define LASER_PIN PB_6

#define SERVO_PAN_PIN PF_1
#define SERVO_TILT_PIN PF_2

#define THRESHOLD 30.0   // Distance threshold (cm) for target locking


// ================= SERVO PARAMETERS =================
int panAngle = 0;
int tiltAngle = 90;

int panDirection = 1;
int tiltDirection = 1;

int lockedPanAngle = 0;
int lockedTiltAngle = 90;

unsigned long lastServoUpdate = 0;
bool motorsLocked = false;


// ===================================================
//               I2C BIT-BANG IMPLEMENTATION
// ===================================================

// Small delay for I2C timing
void i2c_delay() {
    delayMicroseconds(5);
}

// Set SDA HIGH (input mode)
void SDA_HIGH() {
    pinMode(SDA_PIN, INPUT);
}

// Set SDA LOW
void SDA_LOW() {
    pinMode(SDA_PIN, OUTPUT);
    digitalWrite(SDA_PIN, LOW);
}

// Set SCL HIGH
void SCL_HIGH() {
    pinMode(SCL_PIN, INPUT);
}

// Set SCL LOW
void SCL_LOW() {
    pinMode(SCL_PIN, OUTPUT);
    digitalWrite(SCL_PIN, LOW);
}

// Initialize I2C lines
void i2c_init() {
    pinMode(SDA_PIN, INPUT);
    pinMode(SCL_PIN, INPUT);
    SDA_HIGH();
    SCL_HIGH();
}

// Generate I2C START condition
void i2c_start() {
    SDA_HIGH();
    SCL_HIGH();
    i2c_delay();
    SDA_LOW();
    i2c_delay();
    SCL_LOW();
}

// Generate I2C STOP condition
void i2c_stop() {
    SDA_LOW();
    i2c_delay();
    SCL_HIGH();
    i2c_delay();
    SDA_HIGH();
}

// Write one byte over I2C
void i2c_write(uint8_t data) {
    for (int i = 0; i < 8; i++) {
        (data & 0x80) ? SDA_HIGH() : SDA_LOW();

        i2c_delay();
        SCL_HIGH();
        i2c_delay();
        SCL_LOW();

        data <<= 1;
    }

    // ACK bit (ignored)
    SDA_HIGH();
    i2c_delay();
    SCL_HIGH();
    i2c_delay();
    SCL_LOW();
}


// ===================================================
//                OLED DISPLAY FUNCTIONS
// ===================================================

// Send command to OLED
void oled_cmd(uint8_t cmd) {
    i2c_start();
    i2c_write(OLED_ADDR << 1);
    i2c_write(0x00);   // Command mode
    i2c_write(cmd);
    i2c_stop();
}

// Send data to OLED
void oled_data(const uint8_t *data, int n) {
    i2c_start();
    i2c_write(OLED_ADDR << 1);
    i2c_write(0x40);   // Data mode

    for (int i = 0; i < n; i++) {
        i2c_write(data[i]);
    }

    i2c_stop();
}

// Set cursor position
void oled_set(uint8_t col, uint8_t page) {
    oled_cmd(0x21);
    oled_cmd(col);
    oled_cmd(127);

    oled_cmd(0x22);
    oled_cmd(page);
    oled_cmd(3);
}

// Clear display
void oled_clear() {
    uint8_t blank[16] = {0};

    for (int p = 0; p < 4; p++) {
        oled_set(0, p);
        for (int c = 0; c < 128; c += 16) {
            oled_data(blank, 16);
        }
    }
}

// Initialize OLED
void oled_init() {
    oled_cmd(0xAE);  // Display OFF
    oled_cmd(0x20); oled_cmd(0x00);
    oled_cmd(0x40);
    oled_cmd(0xA0);
    oled_cmd(0xC0);
    oled_cmd(0xA8); oled_cmd(0x1F);
    oled_cmd(0xDA); oled_cmd(0x02);
    oled_cmd(0x81); oled_cmd(0x7F);
    oled_cmd(0xA4);
    oled_cmd(0xA6);
    oled_cmd(0x8D); oled_cmd(0x14);
    oled_cmd(0xAF);  // Display ON
}


// ===================================================
//              ULTRASONIC SENSOR FUNCTIONS
// ===================================================

// Initialize ultrasonic + laser pins
void us_init() {
    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);
    pinMode(LASER_PIN, OUTPUT);

    digitalWrite(TRIG_PIN, LOW);
    digitalWrite(LASER_PIN, LOW);
}

// Measure distance (in cm)
float us_distance() {
    unsigned long t1, t2, timeout;

    // Trigger pulse
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);

    // Wait for echo HIGH
    timeout = 0;
    while (digitalRead(ECHO_PIN) == LOW) {
        timeout++;
        if (timeout > 10000) return -1.0;
        delayMicroseconds(1);
    }

    t1 = micros();

    // Measure echo duration
    timeout = 0;
    while (digitalRead(ECHO_PIN) == HIGH) {
        timeout++;
        if (timeout > 30000) break;
        delayMicroseconds(1);
    }

    t2 = micros();

    unsigned long duration = t2 - t1;

    if (duration < 100 || duration > 25000) return -1.0;

    // Convert to cm
    return duration / 58.0;
}


// ===================================================
//                SERVO CONTROL FUNCTIONS
// ===================================================

// Convert angle to pulse width (µs)
int angleToPulse(int angle) {
    return map(angle, 0, 180, 500, 2500);
}

// Generate servo pulse
void sendServoPulse(int pin, int angle) {
    int pulseWidth = angleToPulse(angle);

    digitalWrite(pin, HIGH);
    delayMicroseconds(pulseWidth);
    digitalWrite(pin, LOW);
}

// Initialize servo position
void servo_init() {
    pinMode(SERVO_PAN_PIN, OUTPUT);
    pinMode(SERVO_TILT_PIN, OUTPUT);

    for (int i = 0; i < 50; i++) {
        sendServoPulse(SERVO_PAN_PIN, panAngle);
        delay(20);

        sendServoPulse(SERVO_TILT_PIN, tiltAngle);
        delay(20);
    }
}

// Update servo scanning motion
void updatePanServo() {
    unsigned long currentTime = micros();

    // Update every 20 ms
    if (currentTime - lastServoUpdate >= 20000) {

        if (!motorsLocked) {
            // Sweep pan
            panAngle += 2 * panDirection;

            if (panAngle >= 180) {
                panAngle = 180;
                panDirection = -1;
            } else if (panAngle <= 0) {
                panAngle = 0;
                panDirection = 1;
            }

            // Sweep tilt
            tiltAngle += tiltDirection;

            if (tiltAngle >= 130) {
                tiltAngle = 130;
                tiltDirection = -1;
            } else if (tiltAngle <= 90) {
                tiltAngle = 90;
                tiltDirection = 1;
            }

        } else {
            // Lock servo at detected position
            panAngle = lockedPanAngle;
            tiltAngle = lockedTiltAngle;
        }

        lastServoUpdate = currentTime;
    }

    sendServoPulse(SERVO_PAN_PIN, panAngle);
    delayMicroseconds(100);
    sendServoPulse(SERVO_TILT_PIN, tiltAngle);
}


// ===================================================
//                    SETUP FUNCTION
// ===================================================

void setup() {
    i2c_init();

    delay(50);
    oled_init();
    delay(50);

    oled_clear();

    us_init();
    servo_init();

    lastServoUpdate = micros();
}


// ===================================================
//                      MAIN LOOP
// ===================================================

void loop() {

    updatePanServo();

    static unsigned long lastDistanceCheck = 0;
    static float dist = 0;

    // Moving average filter
    static float readings[5] = {0};
    static int readIndex = 0;

    // Check distance every 50 ms
    if (millis() - lastDistanceCheck >= 50) {

        float newDist = us_distance();

        if (newDist > 2 && newDist < 400) {
            readings[readIndex] = newDist;
            readIndex = (readIndex + 1) % 5;

            float sum = 0;
            int validCount = 0;

            for (int i = 0; i < 5; i++) {
                if (readings[i] > 2 && readings[i] < 400) {
                    sum += readings[i];
                    validCount++;
                }
            }

            if (validCount > 0) {
                dist = sum / validCount;  // Filtered distance
            }
        }

        lastDistanceCheck = millis();

        bool isLocked = (dist <= THRESHOLD && dist > 0);

        // Store locked position
        if (isLocked && !motorsLocked) {
            lockedPanAngle = panAngle;
            lockedTiltAngle = tiltAngle;
        }

        motorsLocked = isLocked;

        // Control laser
        digitalWrite(LASER_PIN, isLocked ? HIGH : LOW);
    }


    // OLED update every 200 ms
    static unsigned long lastDisplayUpdate = 0;

    if (millis() - lastDisplayUpdate >= 200) {

        bool isLocked = (dist <= THRESHOLD && dist > 0);

        char line1[16];
        char line2[16];

        if (isLocked) {
            sprintf(line1, "TARGET:");
            sprintf(line2, "LOCKED");
        } else {
            sprintf(line1, "DISTANCE:");
            sprintf(line2, "%d CM", (int)dist);
        }

        oled_clear();
        drawText(line1, 10, 0);
        drawText(line2, 25, 2);

        lastDisplayUpdate = millis();
    }
}
