# Smart-Room-Environment-and-Occupancy-Monitor

## Overview
The **Smart Room Environment and Occupancy Monitor** is an IoT-based system designed to optimize energy efficiency and occupant comfort by dynamically adjusting room conditions based on real-time sensor data. The system integrates PIR motion sensors, DHT22 temperature/humidity sensors, and LDR light sensors with an ESP32 microcontroller to monitor occupancy, temperature, humidity, and light levels. Data is displayed locally on an OLED screen and transmitted to the ThingSpeak cloud platform for remote monitoring and analysis.

## Key Features
- **Real-time monitoring** of temperature, humidity, light intensity, and occupancy.
- **Automated lighting control**: LED turns on only when the room is occupied and ambient light is insufficient.
- **Local display**: OLED screen provides instant feedback on environmental conditions.
- **Cloud integration**: Data is uploaded to ThingSpeak for remote access and historical analysis.
- **Energy efficiency**: Reduces energy waste by adjusting lighting based on actual usage.
- **Scalability**: Modular design allows for future expansion (e.g., adding air quality sensors or HVAC control).

## Hardware Components
| Component          | Functionality                          | Connection to ESP32       |
|--------------------|----------------------------------------|---------------------------|
| DHT22 Sensor       | Measures temperature and humidity      | GPIO 15                   |
| PIR Sensor         | Detects human presence (occupancy)     | GPIO 14                   |
| LDR Sensor         | Measures ambient light intensity       | GPIO 34 (Analog)          |
| OLED Display       | Displays real-time sensor readings     | GPIO 21 (SDA), GPIO 22 (SCL) |
| LED                | Provides visual feedback (lighting)    | GPIO 2                    |

## Software Setup
1. **Microcontroller Programming**: The ESP32 is programmed using MicroPython to collect sensor data, execute control logic, and manage outputs.
2. **Cloud Integration**: Data is transmitted to ThingSpeak via Wi-Fi for remote monitoring.
3. **Simulation**: The system is tested using the Wokwi IoT simulator to validate functionality before physical deployment.

## Installation Steps
1. **Hardware Setup**: Connect all sensors and components to the ESP32 as per the pin configuration table.
2. **Sensor Calibration**: Test and calibrate each sensor to ensure accurate readings.
3. **Upload Code**: Flash the MicroPython script to the ESP32.
4. **Configure ThingSpeak**: Set up a ThingSpeak channel and update the ESP32 code with your API key and Wi-Fi credentials.
5. **Test the System**: Verify sensor readings, OLED display, LED control, and cloud data upload.

## Expected Outcomes
- **Temperature/Humidity**: Monitored in real-time and displayed on ThingSpeak (Fields 1 and 2).
- **Occupancy**: Binary status (occupied/unoccupied) shown on ThingSpeak (Field 4).
- **Light Intensity**: Ambient light levels tracked and displayed (Field 3).
- **LED Control**: Status (ON/OFF) logged on ThingSpeak (Field 5) based on occupancy and light thresholds.

## Future Enhancements
- Integrate HVAC control for automated temperature regulation.
- Add mobile app support for real-time alerts and control.
- Implement machine learning for predictive environmental adjustments.
- Expand with additional sensors (e.g., CO2, air quality).

## References
- Das, S. K., & Mukherjee, A. (2017). IoT-based smart home management. *ICMDCS*.
- Mahdavinejad, M. S., et al. (2018). Machine learning for IoT data analysis. *arXiv:1809.00847*.
- ThingSpeak Documentation: [https://thingspeak.com](https://thingspeak.com)
  
## Wokwi working model link
- https://wokwi.com/projects/428341005411164161

## Contributors
- Sri Krishna Chaitanya Chivatam
- Rohit Damarla
- Venkata Amardeep Marpina
- Dileep Kumar Perala

## License
This project is open-source and available for modification and distribution under the MIT License.
