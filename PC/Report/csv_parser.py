"""
CSV Data Parser Module
Парses CSV files with TIME;TEMP;HUM;DEV format
"""

import pandas as pd
import datetime
from typing import List, Dict, Optional, Tuple
from dataclasses import dataclass


@dataclass
class SensorReading:
    """Data class for sensor reading"""
    timestamp: datetime.datetime
    temperature: float
    humidity: float
    device_id: str


class CSVParser:
    """Parser for CSV files with sensor data"""
    
    def __init__(self):
        pass
    
    def parse_csv_file(self, file_path: str) -> List[SensorReading]:
        """
        Parse CSV file and return list of sensor readings
        
        Args:
            file_path: Path to CSV file
            
        Returns:
            List of SensorReading objects
        """
        try:
            # Read CSV with semicolon separator
            df = pd.read_csv(file_path, sep=';', encoding='utf-8')
            
            readings = []
            for _, row in df.iterrows():
                try:
                    # Parse timestamp - format: "Thu, Aug 21 2025 14:27"
                    timestamp_str = row['TIME']
                    timestamp = self._parse_timestamp(timestamp_str)
                    
                    # Parse temperature and humidity
                    temperature = float(row['TEMP'])
                    humidity = float(row['HUM'])
                    device_id = str(row['DEV'])
                    
                    reading = SensorReading(
                        timestamp=timestamp,
                        temperature=temperature,
                        humidity=humidity,
                        device_id=device_id
                    )
                    readings.append(reading)
                    
                except (ValueError, KeyError) as e:
                    print(f"Error parsing row: {row}, error: {e}")
                    continue
                    
            return readings
            
        except Exception as e:
            print(f"Error reading CSV file {file_path}: {e}")
            return []
    
    def _parse_timestamp(self, timestamp_str: str) -> datetime.datetime:
        """
        Parse timestamp string to datetime object
        
        Format: "Thu, Aug 21 2025 14:27"
        """
        try:
            # Remove day name and comma
            cleaned = timestamp_str.split(', ')[1]
            
            # Parse the datetime
            return datetime.datetime.strptime(cleaned, "%b %d %Y %H:%M")
            
        except Exception as e:
            raise ValueError(f"Cannot parse timestamp '{timestamp_str}': {e}")
    
    def filter_by_date_range(self, readings: List[SensorReading], 
                           start_date: datetime.date, 
                           end_date: datetime.date) -> List[SensorReading]:
        """
        Filter readings by date range
        
        Args:
            readings: List of sensor readings
            start_date: Start date (inclusive)
            end_date: End date (inclusive)
            
        Returns:
            Filtered list of readings
        """
        filtered = []
        for reading in readings:
            reading_date = reading.timestamp.date()
            if start_date <= reading_date <= end_date:
                filtered.append(reading)
        return filtered
    
    def filter_working_days(self, readings: List[SensorReading]) -> List[SensorReading]:
        """
        Filter readings to include only working days (Sunday-Thursday)
        
        Args:
            readings: List of sensor readings
            
        Returns:
            Filtered list of readings (working days only)
        """
        filtered = []
        for reading in readings:
            # weekday() returns 0=Monday, 1=Tuesday, ..., 6=Sunday
            # Working days: Sunday(6), Monday(0), Tuesday(1), Wednesday(2), Thursday(3)
            weekday = reading.timestamp.weekday()
            if weekday in [6, 0, 1, 2, 3]:  # Sunday to Thursday
                filtered.append(reading)
        return filtered
    
    def group_by_date(self, readings: List[SensorReading]) -> Dict[datetime.date, List[SensorReading]]:
        """
        Group readings by date
        
        Args:
            readings: List of sensor readings
            
        Returns:
            Dictionary with date as key and list of readings as value
        """
        grouped = {}
        for reading in readings:
            date = reading.timestamp.date()
            if date not in grouped:
                grouped[date] = []
            grouped[date].append(reading)
        return grouped


if __name__ == "__main__":
    # Test the parser
    parser = CSVParser()
    
    # Create test CSV content
    test_csv_content = """TIME;TEMP;HUM;DEV
Thu, Aug 21 2025 14:27;24.4;64.3;HT_3C71BF28FBD8
Thu, Aug 21 2025 14:37;24.5;66.5;HT_3C71BF28FBD8
Thu, Aug 21 2025 14:47;26.4;58.9;HT_3C71BF28FBD8"""
    
    # Save test file
    with open('test_data.csv', 'w', encoding='utf-8') as f:
        f.write(test_csv_content)
    
    # Test parsing
    readings = parser.parse_csv_file('test_data.csv')
    print(f"Parsed {len(readings)} readings")
    for reading in readings:
        print(f"{reading.timestamp}: T={reading.temperature}°C, H={reading.humidity}%, Dev={reading.device_id}")