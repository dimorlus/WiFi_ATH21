"""
Main Application Script
Temperature and Humidity Report Generator

Usage:
    python main.py <csv_file_path> <month> <year> [options]

Examples:
    python main.py data.csv 8 2025
    python main.py sensor_data.csv 12 2024 --output custom_report.docx
    python main.py data.csv 8 2025 --template my_template.docx --output Aug_2025.docx
"""

import os
import sys
import argparse
import datetime
from typing import List, Optional

from csv_parser import CSVParser
from data_processor import DataProcessor
from docx_generator import DOCXGenerator
from date_formatter import DateFormatter


class ReportGenerator:
    """Main report generation orchestrator"""
    
    def __init__(self, template_path: str = "template.docx"):
        """
        Initialize report generator
        
        Args:
            template_path: Path to DOCX template file
        """
        self.template_path = template_path
        self.csv_parser = CSVParser()
        self.data_processor = DataProcessor()
        self.date_formatter = DateFormatter('en')
        
        # Validate template exists
        if not os.path.exists(template_path):
            raise FileNotFoundError(f"Template file not found: {template_path}")
    
    def generate_report(self, csv_file_path: str, month: int, year: int, 
                       output_path: Optional[str] = None) -> bool:
        """
        Generate monthly temperature and humidity report
        
        Args:
            csv_file_path: Path to CSV data file
            month: Month number (1-12)
            year: Year
            output_path: Output file path (optional)
            
        Returns:
            True if successful, False otherwise
        """
        try:
            # Validate inputs
            if not os.path.exists(csv_file_path):
                print(f"Error: CSV file not found: {csv_file_path}")
                return False
            
            if month < 1 or month > 12:
                print(f"Error: Invalid month {month}. Must be 1-12.")
                return False
            
            if year < 1900 or year > 2100:
                print(f"Error: Invalid year {year}. Must be reasonable value.")
                return False
            
            print(f"Processing CSV file: {csv_file_path}")
            print(f"Generating report for: {self.date_formatter.get_month_name(month)} {year}")
            
            # Step 1: Parse CSV data
            print("Step 1: Parsing CSV data...")
            readings = self.csv_parser.parse_csv_file(csv_file_path)
            if not readings:
                print("Error: No valid readings found in CSV file")
                return False
            
            print(f"Loaded {len(readings)} sensor readings")
            
            # Step 2: Process monthly data
            print("Step 2: Processing monthly data...")
            daily_data = self.data_processor.process_month_readings(readings, year, month)
            if not daily_data:
                print("Error: No data found for specified month")
                return False
            
            print(f"Processed {len(daily_data)} working days")
            
            # Step 3: Generate output filename if not provided
            if output_path is None:
                month_name = self.date_formatter.get_month_name(month, short=True)
                output_path = f"{month_name}_{year}.docx"
            
            print(f"Output file: {output_path}")
            
            # Extract CSV filename without extension for title
            csv_filename = os.path.splitext(os.path.basename(csv_file_path))[0]
            
            # Step 4: Generate DOCX report
            print("Step 3: Generating DOCX report...")
            docx_generator = DOCXGenerator(self.template_path)
            success = docx_generator.generate_report(daily_data, month, year, output_path, csv_filename)
            
            if success:
                print(f"✓ Report generated successfully: {output_path}")
                self._print_summary(daily_data)
            else:
                print("✗ Failed to generate report")
            
            return success
            
        except Exception as e:
            print(f"Error generating report: {e}")
            return False
    
    def _print_summary(self, daily_data: List[dict]) -> None:
        """Print summary statistics"""
        total_days = len(daily_data)
        days_with_morning = sum(1 for d in daily_data if d['morning'] is not None)
        days_with_evening = sum(1 for d in daily_data if d['evening'] is not None)
        
        morning_compliant = sum(1 for d in daily_data 
                              if d['morning'] and d['morning']['fully_compliant'])
        evening_compliant = sum(1 for d in daily_data 
                              if d['evening'] and d['evening']['fully_compliant'])
        
        print("\n" + "="*50)
        print("SUMMARY STATISTICS")
        print("="*50)
        print(f"Total working days processed: {total_days}")
        print(f"Days with morning data (08:00): {days_with_morning}")
        print(f"Days with evening data (16:00): {days_with_evening}")
        print(f"Morning readings fully compliant: {morning_compliant}/{days_with_morning}")
        print(f"Evening readings fully compliant: {evening_compliant}/{days_with_evening}")
        
        if days_with_morning > 0:
            morning_compliance_rate = (morning_compliant / days_with_morning) * 100
            print(f"Morning compliance rate: {morning_compliance_rate:.1f}%")
        
        if days_with_evening > 0:
            evening_compliance_rate = (evening_compliant / days_with_evening) * 100
            print(f"Evening compliance rate: {evening_compliance_rate:.1f}%")
        
        print("\nCompliance criteria:")
        print("- Temperature: 20.0°C - 26.0°C")
        print("- Humidity: 40% - 60%")
        print("- Working days: Sunday - Thursday")
        print("- Target times: 08:00 and 16:00")


def main():
    """Main entry point"""
    parser = argparse.ArgumentParser(
        description="Generate monthly temperature and humidity compliance reports",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  python main.py data.csv 8 2025
  python main.py sensor_data.csv 12 2024 --output custom_report.docx
  python main.py data.csv 8 2025 --template my_template.docx --output Aug_2025.docx
        """
    )
    
    parser.add_argument('csv_file', help='Path to CSV data file')
    parser.add_argument('month', type=int, help='Month number (1-12)')
    parser.add_argument('year', type=int, help='Year')
    parser.add_argument('--template', default='template.docx',
                       help='Path to DOCX template file (default: template.docx)')
    parser.add_argument('--output', help='Output file path (default: auto-generated)')
    parser.add_argument('--verbose', '-v', action='store_true',
                       help='Enable verbose output')
    
    args = parser.parse_args()
    
    try:
        # Create report generator
        generator = ReportGenerator(args.template)
        
        # Generate report
        success = generator.generate_report(
            args.csv_file, 
            args.month, 
            args.year, 
            args.output
        )
        
        # Exit with appropriate code
        sys.exit(0 if success else 1)
        
    except FileNotFoundError as e:
        print(f"Error: {e}")
        sys.exit(1)
    except Exception as e:
        print(f"Unexpected error: {e}")
        if args.verbose:
            import traceback
            traceback.print_exc()
        sys.exit(1)


if __name__ == "__main__":
    main()