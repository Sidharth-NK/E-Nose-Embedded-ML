import serial
import time
import csv
import os


BAUD_RATE = 9600  
SERIAL_PORT = "/dev/ttyACM0"  # Change for Windows: "COMx"


odor_label = input("Enter odor name: ").strip().lower()

# Directory for storing different odor data
LOG_DIR = "odor_dataset"
os.makedirs(LOG_DIR, exist_ok=True)

# Generate filename using the label and timestamp
timestamp = time.strftime("%Y%m%d_%H:%M:%S")
CSV_FILE = os.path.join(LOG_DIR, f"{odor_label}_{timestamp}.csv")

# Open serial connection
try:
    sr = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)  
    time.sleep(2)  # Wait for connection to establish
except serial.SerialException as error:
    print("Error: Could not open serial port:", error)
    exit()  

# Open CSV file for writing sensor data
with open(CSV_FILE, mode='w', newline="") as file:
    writer = csv.writer(file)
    writer.writerow(['Timestamp', 'Humidity', 'Temperature', 'NO2', 'Ethanol', 'VOC', 'CO', 'Location', 'Label'])

    print(f"Logging '{odor_label}' sensor data to '{CSV_FILE}'... Press Ctrl+C to stop.")

    try:
        while True:
            # Read a line from serial, decode it, and strip whitespace
            line = sr.readline().decode("utf-8").strip()
            if line:
                try:
                    # Split the line into parts (humidity, temp, NO2, Ethanol, VOC, CO, location, placeholder_label)
                    parts = line.split(',')
                    
                    # Convert numerical values to floats
                    values = list(map(float, parts[:7]))  # First 7 values are numbers
                    placeholder_label = parts[7]  # Last value is the placeholder "label"

                    # Ensure exactly 8 values are received (7 numbers + 1 string)
                    if len(parts) != 8:
                        raise ValueError("Incorrect Number of Values Received!")

                    humid, temp, no2, ethanol, voc, co, location = values
                    timestamp = time.strftime("%Y-%m-%d %H:%M:%S")

                    writer.writerow([timestamp, humid, temp, no2, ethanol, voc, co, location, odor_label])
                    file.flush()  # Save data immediately

                    print(f"{timestamp} | {odor_label.upper()} | Humid: {humid} | Temp: {temp} | NO2: {no2} | Ethanol: {ethanol} | VOC: {voc} | CO: {co} | Location: {int(location)}")

                except ValueError as e:
                    print(f"Error: {e} | Invalid data received:", line)

            time.sleep(1)  # Wait 1 second before next reading

    except KeyboardInterrupt:
        print("\nData Collection Stopped")

    finally:
        sr.close()
        file.close()
        print("Serial connection and file closed.")