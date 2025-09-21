"""
Data Filtering and Processing Module
Filters sensor data for specific times and calculates averages
"""

import datetime
from typing import List, Optional, Tuple
from csv_parser import SensorReading


class DataProcessor:
    """Process and filter sensor data"""
    
    def __init__(self):
        # Temperature and humidity acceptable ranges
        self.temp_min = 20.0
        self.temp_max = 26.0
        self.humidity_min = 40.0
        self.humidity_max = 60.0
        
        # Target times for measurements
        self.target_times = [
            datetime.time(8, 0),   # 08:00
            datetime.time(16, 0),  # 16:00
        ]
        
        # Maximum time difference to search for nearby readings (in minutes)
        self.max_time_diff_minutes = 15
    
    def find_readings_around_time(self, readings: List[SensorReading], 
                                target_time: datetime.time, 
                                target_date: datetime.date) -> List[SensorReading]:
        """
        Find readings around target time on specific date
        
        Args:
            readings: List of sensor readings
            target_time: Target time to search around (e.g., 08:00)
            target_date: Target date
            
        Returns:
            List of readings closest to target time
        """
        # Create target datetime
        target_datetime = datetime.datetime.combine(target_date, target_time)
        
        # Filter readings for the specific date
        date_readings = [r for r in readings if r.timestamp.date() == target_date]
        
        if not date_readings:
            return []
        
        # Calculate time differences and sort by closest to target
        readings_with_diff = []
        for reading in date_readings:
            time_diff = abs((reading.timestamp - target_datetime).total_seconds() / 60)  # in minutes
            if time_diff <= self.max_time_diff_minutes:
                readings_with_diff.append((reading, time_diff))
        
        # Sort by time difference (closest first)
        readings_with_diff.sort(key=lambda x: x[1])
        
        # Return up to 3 closest readings
        return [reading for reading, _ in readings_with_diff[:3]]
    
    def calculate_average_reading(self, readings: List[SensorReading]) -> Optional[Tuple[float, float]]:
        """
        Calculate average temperature and humidity from readings
        
        Args:
            readings: List of sensor readings
            
        Returns:
            Tuple of (average_temperature, average_humidity) or None if no readings
        """
        if not readings:
            return None
        
        total_temp = sum(r.temperature for r in readings)
        total_humidity = sum(r.humidity for r in readings)
        count = len(readings)
        
        avg_temp = total_temp / count
        avg_humidity = total_humidity / count
        
        return (avg_temp, avg_humidity)
    
    def is_temperature_compliant(self, temperature: float) -> bool:
        """Check if temperature is within acceptable range (20-26°C)"""
        return self.temp_min <= temperature <= self.temp_max
    
    def is_humidity_compliant(self, humidity: float) -> bool:
        """Check if humidity is within acceptable range (40-60%)"""
        return self.humidity_min <= humidity <= self.humidity_max
    
    def is_compliant(self, temperature: float, humidity: float) -> bool:
        """Check if both temperature and humidity are compliant"""
        return self.is_temperature_compliant(temperature) and self.is_humidity_compliant(humidity)
    
    def process_day_readings(self, readings: List[SensorReading], 
                           date: datetime.date) -> dict:
        """
        Process readings for a specific day and return data for 08:00 and 16:00
        
        Args:
            readings: List of sensor readings
            date: Date to process
            
        Returns:
            Dictionary with processed data for the day
        """
        result = {
            'date': date,
            'morning': None,  # 08:00 data
            'evening': None,  # 16:00 data
        }
        
        # Process morning readings (08:00)
        morning_readings = self.find_readings_around_time(readings, datetime.time(8, 0), date)
        if morning_readings:
            avg_temp, avg_humidity = self.calculate_average_reading(morning_readings)
            result['morning'] = {
                'temperature': avg_temp,
                'humidity': avg_humidity,
                'temp_compliant': self.is_temperature_compliant(avg_temp),
                'humidity_compliant': self.is_humidity_compliant(avg_humidity),
                'fully_compliant': self.is_compliant(avg_temp, avg_humidity),
                'readings_count': len(morning_readings)
            }
        
        # Process evening readings (16:00)
        evening_readings = self.find_readings_around_time(readings, datetime.time(16, 0), date)
        if evening_readings:
            avg_temp, avg_humidity = self.calculate_average_reading(evening_readings)
            result['evening'] = {
                'temperature': avg_temp,
                'humidity': avg_humidity,
                'temp_compliant': self.is_temperature_compliant(avg_temp),
                'humidity_compliant': self.is_humidity_compliant(avg_humidity),
                'fully_compliant': self.is_compliant(avg_temp, avg_humidity),
                'readings_count': len(evening_readings)
            }
        
        return result
    
    def process_month_readings(self, readings: List[SensorReading], 
                             year: int, month: int) -> List[dict]:
        """
        Process readings for entire month
        
        Args:
            readings: List of sensor readings
            year: Year
            month: Month (1-12)
            
        Returns:
            List of daily processed data
        """
        # Get first and last day of month
        first_day = datetime.date(year, month, 1)
        if month == 12:
            last_day = datetime.date(year + 1, 1, 1) - datetime.timedelta(days=1)
        else:
            last_day = datetime.date(year, month + 1, 1) - datetime.timedelta(days=1)
        
        # Filter readings for the month and working days only
        from csv_parser import CSVParser
        parser = CSVParser()
        month_readings = parser.filter_by_date_range(readings, first_day, last_day)
        working_day_readings = parser.filter_working_days(month_readings)
        
        # Group by date
        grouped_readings = parser.group_by_date(working_day_readings)
        
        # Process each working day
        results = []
        current_date = first_day
        while current_date <= last_day:
            # Check if it's a working day
            weekday = current_date.weekday()
            if weekday in [6, 0, 1, 2, 3]:  # Sunday to Thursday
                day_readings = grouped_readings.get(current_date, [])
                day_result = self.process_day_readings(day_readings, current_date)
                results.append(day_result)
            
            current_date += datetime.timedelta(days=1)
        
        return results


if __name__ == "__main__":
    # Test the data processor
    from csv_parser import CSVParser, SensorReading
    import datetime
    
    # Create test data
    test_readings = [
        SensorReading(
            timestamp=datetime.datetime(2025, 8, 21, 8, 5),  # 08:05 (close to 08:00)
            temperature=24.0,
            humidity=55.0,
            device_id="TEST_DEVICE"
        ),
        SensorReading(
            timestamp=datetime.datetime(2025, 8, 21, 8, 7),  # 08:07
            temperature=24.2,
            humidity=56.0,
            device_id="TEST_DEVICE"
        ),
        SensorReading(
            timestamp=datetime.datetime(2025, 8, 21, 16, 2),  # 16:02 (close to 16:00)
            temperature=25.5,
            humidity=45.0,
            device_id="TEST_DEVICE"
        ),
    ]
    
    processor = DataProcessor()
    
    # Test finding readings around time
    target_date = datetime.date(2025, 8, 21)
    morning_readings = processor.find_readings_around_time(
        test_readings, 
        datetime.time(8, 0), 
        target_date
    )
    
    print(f"Found {len(morning_readings)} morning readings")
    
    # Test calculating average
    if morning_readings:
        avg_temp, avg_humidity = processor.calculate_average_reading(morning_readings)
        print(f"Average morning: T={avg_temp:.1f}°C, H={avg_humidity:.1f}%")
        print(f"Compliant: {processor.is_compliant(avg_temp, avg_humidity)}")
    
    # Test processing day
    day_result = processor.process_day_readings(test_readings, target_date)
    print(f"Day result: {day_result}")