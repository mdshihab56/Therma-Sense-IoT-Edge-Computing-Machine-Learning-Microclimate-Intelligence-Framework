# ThermaSense: IoT Edge Computing & Machine Learning Microclimate Intelligence Framework

ThermaSense is an integrated, full-stack Internet of Things (IoT) hardware platform and Machine Learning (ML) analytical pipeline. It is specifically engineered to address dense urban health challenges, microclimatic heat stress risks, and indoor thermal instability in high-density tropical regions like Dhaka, Bangladesh.

The platform bridges localized physical edge sensing with macro-climatic cloud meteorology via the OpenWeatherMap API. It leverages dual artificial intelligence models—an Artificial Neural Network (ANN) Multilayer Perceptron for classification and a Random Forest Regressor for temporal trend forecasting—to evaluate human health thresholds based on international occupational safety limits (ASHRAE Standard 55 / ISO 7730).



## End-to-End System Architecture

<img width="710" height="455" alt="Screenshot 2026-06-11 212315" src="https://github.com/user-attachments/assets/17e3e5a6-89a5-4601-935f-01b380700b28" />


## Scientific Methodology & Theoretical Framework

The ThermaSense framework operates as a decoupled edge-to-cloud predictive analytics pipeline. The underlying scientific methodology is divided into four distinct phases:

### Phase 1: Localized Physical Data Collection (The Edge Layer)
In high-density tropical cities like Dhaka, residential concrete structures act as thermal masses, trapping solar radiation during peak daylight hours and radiating heat inward long after sunset. Suboptimal cross-ventilation windows compound this effect, creating indoor microclimates that are often significantly more oppressive than outdoor ambient baselines. 

To quantify this, the edge hardware node captures localized physical parameters using a DHT11 sensor matrix, sampling two core variables:
* **Ambient Indoor Temperature ($T_{in}$):** Measures kinetic thermal energy inside the living space.
* **Relative Humidity ($H_{in}$):** Measures the percentage of water vapor present in the air relative to the maximum saturation point at that specific temperature. 

Instead of routing data over expensive cloud queues continuously, the edge layer hosts a local asynchronous network server. This layer is programmed with a non-blocking loop via the microchip CPU clock (`millis()`), ensuring telemetry updates every 5 seconds without blocking incoming network socket handshakes. 

The accompanying local interface relies on **AJAX (Asynchronous JavaScript and XML)**. A background routine polls a headless JSON API endpoint (`/liveData`) compiled directly from the chip's volatile RAM registers every 3 seconds. This isolates data mutations from structural page reloads, conserving local bandwidth and presenting a clean, flicker-free telemetry stream.

### Phase 2: Cloud Synchronization & Thermodynamic Gradients
Indoor microclimates do not exist in a vacuum; they interact dynamically with macro-environmental changes. To map these boundary layer interactions, the Python Flask core establishes a synchronous outbound HTTP GET request to the OpenWeatherMap API servers to retrieve regional outdoor meteorology for Dhaka:
* **Outdoor Temperature ($T_{out}$)**
* **Outdoor Relative Humidity ($H_{out}$)**

By pairing local telemetry with live regional weather, the system isolates the **Thermal Gradient Matrix** ($T_{delta} = T_{out} - T_{in}$). This gradient allows the machine learning algorithms to model the structural heat transfer efficiency of the building envelope, calculating whether the indoor space is acting as a heat sink or dissipating thermal loads efficiently.

### Phase 3: Machine Learning & Predictive Inference
Once the data vector is compiled into a standardized $1 \times 4$ array ($[T_{in}, H_{in}, T_{out}, H_{out}]$), it is passed into two separate machine learning models operating in parallel:

#### 1. The Classifier: Multilayer Perceptron (ANN)
The classification engine consists of a feedforward Artificial Neural Network (ANN) optimized via Scikit-Learn. The input layer distributes the 4-feature vector across densely connected hidden layers mapping non-linear mathematical combinations. Each artificial neuron computes a weighted sum of its inputs, adds a baseline bias parameter ($z = \sum w_i x_i + b$), and passes the result through a Rectified Linear Unit (ReLU) activation function:
$$\text{f}(z) = \max(0, z)$$

This structural mapping is critical for environmental health processing. The human body cools itself via latent heat of vaporization (sweating). When relative humidity crosses critical thresholds, sweat cannot efficiently evaporate into the saturated air, triggering rapid spikes in core body temperatures even at moderate ambient readings. 

The neural network maps these highly non-linear temperature-humidity dependencies, outputting a binary classification threshold: `0` for **Optimal Thermal Comfort** or 1 for **High Microclimatic Heat Stress Risk**.

#### 2. The Forecaster: Random Forest Regressor
While the neural network evaluates immediate biological threats, an ensemble Random Forest Regressor projects environmental shifts across a **+4-Hour Predictive Horizon**. 

During execution, the input feature vector is introduced to hundreds of independent decision trees compiled via bootstrap aggregation (bagging). Each tree steps down conditional mathematical split points based on historical trends (e.g., assessing if $H_{in} > 75\%$). The individual regression trees output an independent numerical scalar, and the ensemble engine averages the results across the entire forest structure:
$$y = \frac{1}{N}\sum_{i=1}^{N} T_i(x)$$

This ensemble average represents the projected +4-Hour future temperature value. Averaging out independent branch variance makes the system highly resilient against noisy data artifacts or physical sensor inaccuracies.

### Phase 4: Biometric Health Baselines & Thresholds
The diagnostic outputs are actively mapped against international occupational safety limits and comfort envelopes, specifically **ASHRAE Standard 55** (Thermal Environmental Conditions for Human Occupancy) and **ISO 7730**:

| Metric Vector | Ideal Biological Range | Threat Threshold Trigger | Physiological Risk Analysis |
| :--- | :--- | :--- | :--- |
| **Indoor Temperature** | $20.0^\circ\text{C} - 25.0^\circ\text{C}$ | $\ge 30.0^\circ\text{C}$ | Cardiovascular Strain, Dehydration, & Cognitive Fatigue |
| **Indoor Relative Humidity** | $40\% - 60\%$ | $\ge 70\%$ | Reduced Sweat Evaporation Efficiency & Rapid Heat Shock Risks |


## Code Architecture: Functional Module Breakdown

### 1. The Edge Layer (`hardware_node.ino`)
* **`setup()` Module:** Initializes the ESP8266 serial register clock ($115200\text{ Baud}$), starts the physical DHT sensor interface, coordinates the Wi-Fi connection loop, and spins up the native network server.
* **`loop()` Module:** Implements a non-blocking execution block using millis(). Every 5 seconds, it reads raw physical signals from the DHT11 sensor, updates internal telemetry states, and appends values to a persistent string buffer (`csvDataLog`).

  <img width="1920" height="1080" alt="Screenshot (4564)" src="https://github.com/user-attachments/assets/2efae393-8651-4c74-811e-a7b16127d432" />

* **`handleLiveDataRoute()` Endpoint (`/liveData`):** Compiles ongoing environmental measurements into a lightweight JSON payload. This allows background queries to access live numbers directly from chip memory.
* **`handleRootRoute()` Module:** Serves the client-side user interface. It embeds an automated **JavaScript AJAX** polling routine that queries the `/liveData` API endpoint every 3 seconds, updating frontend elements dynamically without page reloads. It also runs a native clock widget with an interactive 12-hour/24-hour display toggle.

### 2. The Analytics & Inference Gateway (`app.py`)
* **`upload_telemetry_file()` Route (`/api/upload`):** Handles telemetry file uploads. It includes a `utf-8-sig` encoding filter to remove hidden Byte Order Marks (BOM) caused by Windows text editors, preventing array parsing failures.
* **`parse_csv_line()` Route (`/api/predict`):** The primary data processing and inference engine. It executes the following sequence:
    * Reads the uploaded dataset and computes bulk statistical metrics (like `df['indoor_temp'].mean()`) to handle thousands of rows simultaneously.
    * Employs an out-of-bounds safety fallback: if an invalid dataset index is requested, it automatically snaps to the most recent real-time entry line.
    * Queries live external weather conditions for Dhaka via an HTTP `requests` call to the OpenWeatherMap cloud platform.
    * Feeds the 4-feature vector into the pre-loaded Scikit-Learn binary model pickles (`ann_model.pkl` and `regressor.pkl`).
    * Triggers the **Matplotlib Analytics Engine**. This compiles a dual-axis metric bar matrix alongside a line chart tracking the 4-hour predictive forecast horizon. The combined plot is exported at a high-resolution **180 DPI** to a static asset directory.

### 3. The Analytics Frontend Portal Component (`index.html`)
* **File Upload Component:** Features an asynchronous event handler that takes the `telemetry.txt` file and sends it as a raw multi-part form data stream directly into the Flask backend endpoint.
* **Dynamic Canvas Hub:** Renders the 180 DPI Matplotlib charts inside an HTML anchor tag (`target="_blank"`). This allows professors or researchers to click anywhere on the data graph to seamlessly scale it up in a brand-new, isolated browser tab for close evaluation.





<img width="1920" height="1080" alt="Screenshot (4565)" src="https://github.com/user-attachments/assets/36951c17-2487-4b0a-a72d-9ca85b44d94c" />


## Fixed Parameters

1. Medical Confort Threshold (20-25°C)
2. Clinical Heat Stress Alert(30°C)
3. Extreme Clinical Hyperthermia Boundary (35°C)
4. Upper Normal Physiological Bounds (25°C)
---

## Repository Asset Layout

<img width="384" height="272" alt="Screenshot 2026-06-11 212348" src="https://github.com/user-attachments/assets/60b9676e-1a4a-4bdc-b9d9-6409818e2e6f" />



## Quick-Start Deployment Guide

### Hardware Integration Section: Power Distribution & Single-Bus Data Ingestion Protocol

This section details the physical layer engineering, power architecture, and low-level serialized data transfer protocol implemented between the environmental telemetry edge sensor and the microcontroller unit.

### Step 1: Power Distribution Architecture
The ESP8266 microchip operates strictly on a 3.3V Logic Level. Supplying 5V directly to the microchip's GPIO data pins or its native VCC lines will cause thermal runaway and permanently damage the silicon wafer layout of the chip.

### Powering the Microcontroller Node: 
Via USB (Development Stage): When your ESP8266 board (like a NodeMCU or WeMos D1 Mini) is plugged into a laptop via a micro-USB cable, it draws 5V from the USB port. Onboard the chip is a Low-Dropout (LDO) voltage regulator (typically the AMS1117 chip) that steps down the incoming 5V to a clean, stable 3.3V.

Via External Battery/Adapter (Deployment Stage): You can feed an external power source (such as a 5V power bank or a 4V Lithium-Ion cell) directly into the 5V/VIN pin of the board, which routes through the onboard regulator to power the system safely.

### Step 2: The Physical Hardware Interconnect (Pin Mapping)
Your hardware configuration requires exactly 3 jumper wires to link the edge processor to the physical environment sensor:

| DHT11 Pin Label | Wire Function | ESP8266 Hardware Target Pin |
|-----------------|---------------|-----------------------------|
| **VCC (Power Input)** | Connects the positive power rail | **3V3 Pin (3.3V Output)** |
| **GND (Ground Reference)** | Connects the negative ground return path | **GND Pin (Common Ground)** |
| **DATA (Signal Out)** | Transmits serialized data pulses | **D2 (GPIO 4)** |

If you are using a bare, raw DHT11 sensor module instead of a pre-assembled 3-pin breakout PCB, you must place a 4.7kΩ to 10kΩ pull-up resistor bridging the space between the DHT11 VCC line and the DATA line. Pre-assembled breakout boards usually have this resistor already soldered onto their tiny PCB layout.

### Step 3: How the Data Transfer Works Mechanically
When professors ask, "How does a single data pin send both temperature and humidity information over a single wire?" explain it using Single-Bus Time-Domain Serial Protocols.

1. The Handshake Protocol (Start Signal)
The data line is normally held high (3.3V) by the pull-up resistor. When your code triggers a read, the ESP8266 pulls the D2 pin low for at least 18 milliseconds, then pulls it high and waits.
This serves as a wake-up call to the DHT11 sensor

2. Sensor Response
The DHT11 catches this pulse and pulls the line low for 80 microseconds, then releases it high for another 80 microseconds. This is the hardware confirmation handshake.

3. The 40-Bit Binary Stream Packet Transfer
The DHT11 then transmits exactly 40 bits (5 bytes) of data sequentially down the wire. To distinguish between a binary 0 and a binary 1, the protocol relies on pulse width timing:

Every bit starts with a 50-microsecond low voltage signal.
Binary 0: The low signal is followed by a high voltage signal lasting only 26 to 28 microseconds.
Binary 1: The low signal is followed by a high voltage signal lasting 70 microseconds.

4. Structuring the Payload & Checksum Integrity
The 40 bits arriving at your ESP8266 microcontroller are organized as follows:
Byte 1: Integral Relative Humidity data (8 bits)
Byte 2: Decimal Relative Humidity data (8 bits - always zero on DHT11)
Byte 3: Integral Temperature data (8 bits)
Byte 4: Decimal Temperature data (8 bits - always zero on DHT11)
Byte 5: The Checksum Byte (8 bits). 

5. Mathematical Checksum Verification
To ensure the data wasn't corrupted by electrical noise along the wire, the ESP8266 running your firmware runs a verification check:
$$\text{Byte 1} + \text{Byte 2} + \text{Byte 3} + \text{Byte 4} = \text{Byte 5 (Checksum)}$$

### AI Ingestion Portal Setup
1.  Clone this repository asset matrix onto your local execution path:
    ```bash
    git clone [https://github.com/YOUR_USERNAME/ThermaSense-IoT-Microclimate-Intelligence.git](https://github.com/YOUR_USERNAME/ThermaSense-IoT-Microclimate-Intelligence.git)
    cd ThermaSense-IoT-Microclimate-Intelligence
    ```
2.  Set up your execution dependencies:
    ```bash
    pip install flask flask-cors pandas numpy requests matplotlib scikit-learn
    ```
3.  Insert your private token key string into the OpenWeatherMap API initialization variable inside `app.py`.
4.  Run your Flask processing environment:
    ```bash
    python app.py
    ```
5.  Access the master analytics view panel at `http://127.0.0.1:5000`
