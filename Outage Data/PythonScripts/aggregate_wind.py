import sys
import csv
import math
from datetime import datetime

print ("This is the name of the script: ", sys.argv[0])
print ("Number of arguments: ", len(sys.argv))
print ("The arguments are: ", str(sys.argv))

class DailyData:
    station = 0;
    stationName = 0;
    latitude = 0;
    longitude = 0;
    year = 0;
    month = 0;
    day = 0;
    reportType = 0;
    windSpeedLow = 0;
    windSpeedHigh = 0;
    windSpeedAvg = 0;
    windSpeedStdDev = 0;
    pressureLow = 0;
    pressureHigh = 0;
    pressureAvg = 0;
    pressureStdDev = 0;


class ClimateData:
    station = 0         # 0
    stationName = 0     # 1
    latitude = 0        # 3
    longitude = 0       # 4
    date = 0            # 5
    reportType = 0      # 6
    windSpeed = 0       # 17
    windDirection = 0   # 18
    windGust = 0        # 19
    pressure = 0        # 20
			
    def __init__(self, station, stationName, latitude, longitude,
		date, reportType, windSpeed, windDirection, windGust, pressure):
				
        self.station = station
        self.stationName = stationName
        self.latitude = latitude
        self.longitude = longitude
        self.date = date
        self.reportType = reportType
        self.windSpeed = windSpeed
        self.windDirection = windDirection
        self.windGust = windGust
        self.pressure = pressure

    def getDateTime(self):
        return datetime.strptime(self.date, "%m/%d/%y %H:%M:%S")

    def getDay(self):
        return datetime.strptime(self.data, "%m/%d/%y")

if (len(sys.argv) < 3):
    print("Error: Improper format!")
    print("aggregate_wind.py <input_file> <output_file>");
    exit()
else:

    inputFile = sys.argv[1]
    outputFile = sys.argv[2]
    
    print("Files: ")
    print("Input: ", inputFile)
    print("Output: ", outputFile)

    try:
        print("Opening input file.")
        with open(inputFile, newline='') as csvFile:

            windData = []
            
            print("Reading input file.");
            inputReader = csv.reader(csvFile, delimiter=',')
            for row in inputReader:
                windData.append(ClimateData(row[0],	row[1], row[3],	row[4],	row[5],	row[6],	row[17], row[18], row[19], row[20]))
                    #print(', '.join(row).)

            print("Aggregating data.");
            for wd in windData:
                print(wd.station, end=',')
                print(wd.stationName, end=',')
                print(wd.latitude, end=',')
                print(wd.longitude, end=',')
                print(wd.date, end=',')
                print(wd.reportType, end=',')
                print(wd.windSpeed, end=',')
                print(wd.windDirection, end=',')
                print(wd.windGust, end=',')
                print(wd.pressure)
				
            for wd in windData:
                print(wd.getDateTime())
                
    except OSError:
        print("Unable to open input file: ", inputFile)
        exit();

def aggregateData(weatherData):
    daily = []
    stations = []

    # Categorize Stations
    for wd in windData:
        skipFlag = False
        for s in stations:
            if (wd.station == s) :
                skipFlag = True
                break
        if skipFlag == False:
            stations.append(wd.station)

    # Categorize Days
    for wd in windData:
        skipFlag = False
        for d in daily:
            if (wd.getDay() == d):
                skipFlag = True;
                break
        if (skipFlag == False):
            daily.append(wd.getDay())

    dailyData = []
## TODO Fix this
    for s in stations:
        for d in daily:
            myDay = []
            minWindSpeed = 999999
            maxWindSpeed = -1
            avgWindSpeed = 0
            stdDevWindSpeed = 0

            minPressure = 999999
            maxPressure = -999999
            avgPressure = 0
            stdDevPressure = 0

            for wd in windData:
                if (d == wd.getDay()):
                    myDay.append(wd)

                    minWindSpeed = min(minWindSpeed, wd.windSpeed)
                    maxWindSpeed = max(maxWindSpeed, wd.windSpeed)
                    avgWindSpeed += wd.windSpeed

                    minPressure = min(minPressure, wd.pressure)
                    maxPressure = max(maxPressure, wd.pressure)
                    avgPressure += wd.pressure

            if (len(myDay) > 0):
                avgWindSpeed /= len(myDay)
                avgPressure /= len(myDay)
            else:
                avgWindSpeed = 0
                avgPressure = 0

            for wd in myDay:
                stdDevWindSpeed += abs(wd.windSpeed*wd.windSpeed - avgWindSpeed*avgWindSpeed)
                stdDevPressure += abs(wd.pressure*wd.pressure - avgPressure*avgPressure)

            if ((len(myDay) - 1) > 0):
                stdDevWindSpeed = math.sqrt(stdDevWindSpeed / len(myDay) - 1)
                stdDevPressure = math.sqrt(stdDevPressure / len(myDay) - 1)
            else:
                stdDevWindSpeed = 0
                stdDevPressure = 0

            dayData = dailyData()

            dailyData.station = myDay

            class DailyData:
                station = 0;
                stationName = 0;
                latitude = 0;
                longitude = 0;
                year = 0;
                month = 0;
                day = 0;
                reportType = 0;
                windSpeedLow = 0;
                windSpeedHigh = 0;
                windSpeedAvg = 0;
                windSpeedStdDev = 0;
                pressureLow = 0;
                pressureHigh = 0;
                pressureAvg = 0;
                pressureStdDev = 0;


