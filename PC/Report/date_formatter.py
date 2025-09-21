"""
Date Formatting Module
Supports various date format patterns as specified in requirements
"""

import datetime
import calendar
from typing import Dict, Union


class DateFormatter:
    """Format dates according to specified patterns"""
    
    def __init__(self, locale: str = 'en'):
        """
        Initialize date formatter
        
        Args:
            locale: Locale for month/day names ('en' for English, 'ru' for Russian, 'he' for Hebrew)
        """
        self.locale = locale
        
        # Month names in different languages
        self.month_names = {
            'en': {
                'full': ['January', 'February', 'March', 'April', 'May', 'June',
                        'July', 'August', 'September', 'October', 'November', 'December'],
                'short': ['Jan', 'Feb', 'Mar', 'Apr', 'May', 'Jun',
                         'Jul', 'Aug', 'Sep', 'Oct', 'Nov', 'Dec']
            },
            'ru': {
                'full': ['Январь', 'Февраль', 'Март', 'Апрель', 'Май', 'Июнь',
                        'Июль', 'Август', 'Сентябрь', 'Октябрь', 'Ноябрь', 'Декабрь'],
                'short': ['Янв', 'Фев', 'Мар', 'Апр', 'Май', 'Июн',
                         'Июл', 'Авг', 'Сен', 'Окт', 'Ноя', 'Дек']
            },
            'he': {
                'full': ['ינואר', 'פברואר', 'מרץ', 'אפריל', 'מאי', 'יוני',
                        'יולי', 'אוגוסט', 'ספטמבר', 'אוקטובר', 'נובמבר', 'דצמבר'],
                'short': ['ינו', 'פבר', 'מרץ', 'אפר', 'מאי', 'יונ',
                         'יול', 'אוג', 'ספט', 'אוק', 'נוב', 'דצמ']
            }
        }
        
        # Day names in different languages
        self.day_names = {
            'en': {
                'full': ['Monday', 'Tuesday', 'Wednesday', 'Thursday', 'Friday', 'Saturday', 'Sunday'],
                'short': ['Mon', 'Tue', 'Wed', 'Thu', 'Fri', 'Sat', 'Sun']
            },
            'ru': {
                'full': ['Понедельник', 'Вторник', 'Среда', 'Четверг', 'Пятница', 'Суббота', 'Воскресенье'],
                'short': ['Пн', 'Вт', 'Ср', 'Чт', 'Пт', 'Сб', 'Вс']
            },
            'he': {
                'full': ['שני', 'שלישי', 'רביעי', 'חמישי', 'שישי', 'שבת', 'ראשון'],
                'short': ['ב׳', 'ג׳', 'ד׳', 'ה׳', 'ו׳', 'ש׳', 'א׳']
            }
        }
    
    def format_date(self, date: Union[datetime.date, datetime.datetime], pattern: str) -> str:
        """
        Format date according to pattern
        
        Supported patterns:
        - yyyy: 4-digit year (2024)
        - yy: 2-digit year (24)
        - MMMM: Full month name (January)
        - MMM: Short month name (Jan)
        - MM: 2-digit month (01)
        - M: Month number (1)
        - dddd: Full day name (Monday)
        - ddd: Short day name (Mon)
        - dd: 2-digit day (01)
        - d: Day number (1)
        - ww: ISO week number (35)
        
        Args:
            date: Date to format
            pattern: Format pattern
            
        Returns:
            Formatted date string
        """
        if isinstance(date, datetime.datetime):
            date = date.date()
        
        result = pattern
        
        # Year patterns
        result = result.replace('yyyy', f'{date.year:04d}')
        result = result.replace('yy', f'{date.year % 100:02d}')
        
        # Month patterns (process longer patterns first)
        if 'MMMM' in result:
            month_name = self.month_names[self.locale]['full'][date.month - 1]
            result = result.replace('MMMM', month_name)
        elif 'MMM' in result:
            month_name = self.month_names[self.locale]['short'][date.month - 1]
            result = result.replace('MMM', month_name)
        elif 'MM' in result:
            result = result.replace('MM', f'{date.month:02d}')
        elif 'M' in result:
            result = result.replace('M', str(date.month))
        
        # Day of week patterns (process longer patterns first)
        weekday = date.weekday()  # 0=Monday, 6=Sunday
        if 'dddd' in result:
            day_name = self.day_names[self.locale]['full'][weekday]
            result = result.replace('dddd', day_name)
        elif 'ddd' in result:
            day_name = self.day_names[self.locale]['short'][weekday]
            result = result.replace('ddd', day_name)
        
        # Day of month patterns (process longer patterns first)
        # First handle 'dd' pattern (two-digit day with leading zero)
        if 'dd' in result and 'dddd' not in result and 'ddd' not in result:
            result = result.replace('dd', f'{date.day:02d}')
        # Then handle single 'd' pattern (day without leading zero)
        elif 'd' in result and 'dd' not in result and 'ddd' not in result and 'dddd' not in result:
            result = result.replace('d', str(date.day))
        
        # Week number pattern
        if 'ww' in result:
            year, week, _ = date.isocalendar()
            result = result.replace('ww', f'{week:02d}')
        
        return result
    
    def format_filename(self, date: Union[datetime.date, datetime.datetime], 
                       pattern: str = "MMM_yyyy") -> str:
        """
        Format filename with date
        
        Args:
            date: Date to format
            pattern: Format pattern for filename
            
        Returns:
            Formatted filename
        """
        formatted = self.format_date(date, pattern)
        # Replace spaces with underscores for filename
        return formatted.replace(' ', '_')
    
    def get_month_name(self, month: int, short: bool = False) -> str:
        """
        Get month name in current locale
        
        Args:
            month: Month number (1-12)
            short: Whether to return short name
            
        Returns:
            Month name
        """
        if month < 1 or month > 12:
            raise ValueError("Month must be between 1 and 12")
        
        if short:
            return self.month_names[self.locale]['short'][month - 1]
        else:
            return self.month_names[self.locale]['full'][month - 1]
    
    def get_day_name(self, weekday: int, short: bool = False) -> str:
        """
        Get day name in current locale
        
        Args:
            weekday: Weekday number (0=Monday, 6=Sunday)
            short: Whether to return short name
            
        Returns:
            Day name
        """
        if weekday < 0 or weekday > 6:
            raise ValueError("Weekday must be between 0 and 6")
        
        if short:
            return self.day_names[self.locale]['short'][weekday]
        else:
            return self.day_names[self.locale]['full'][weekday]


if __name__ == "__main__":
    # Test the date formatter
    formatter = DateFormatter('en')
    
    test_date = datetime.date(2025, 8, 21)
    
    # Test various patterns
    patterns = [
        'yyyy',
        'yy', 
        'MMMM',
        'MMM',
        'MM',
        'M',
        'dddd',
        'ddd',
        'dd',
        'd',
        'ww',
        'ddd dd/MM/yy',
        'MMMM yyyy',
        'dd MMM yyyy'
    ]
    
    print(f"Test date: {test_date}")
    print("=" * 40)
    
    for pattern in patterns:
        result = formatter.format_date(test_date, pattern)
        print(f"{pattern:15} -> {result}")
    
    print("\nFilename examples:")
    print(f"MMM_yyyy: {formatter.format_filename(test_date, 'MMM_yyyy')}")
    print(f"yyyy_MM: {formatter.format_filename(test_date, 'yyyy_MM')}")
    
    # Test Russian locale
    print("\nRussian locale:")
    ru_formatter = DateFormatter('ru')
    print(f"MMMM yyyy: {ru_formatter.format_date(test_date, 'MMMM yyyy')}")
    print(f"ddd dd MMM: {ru_formatter.format_date(test_date, 'ddd dd MMM')}")