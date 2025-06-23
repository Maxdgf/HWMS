'''
=====================================================================
----------------|Mini-Extraction HWMS🌤️ Api Service|----------------
----------------| by https://github.com/Maxdgf |---------------------
=====================================================================
|         Extracts weather data and the current device IP.           |
'''

from bs4 import BeautifulSoup
import requests
import re

URL = 'http://ip' #esp 01 ip (replace with your esp 01 address)

#gets current ip data from web page
def get_current_ip():
    try:
        response = requests.get(URL)
        soup = BeautifulSoup(response.text, 'html.parser')

        device_ip = soup.find('h4')

        if device_ip is not None:
            return device_ip.text
        else:
            return "Null data!"
    except:
        return "current ip: ?"    

#gets weather data (temperature, humidity, pressure) from web page
def get_weather_data():
    try:    
        response = requests.get(URL)
        response.encoding = 'UTF-8'
        soup = BeautifulSoup(response.text, 'html.parser')

        data_labels = soup.find_all('h1')

        if data_labels is not None:
            full_data = ""

            for data in data_labels:
                full_data += re.sub(r'\s+', ' ', data.text).strip() + ' ' #convert data to 1 row string
                    
            return full_data
        else:
            return "Null data!"  
    except:
        return "No data! Please check wifi connection or HWMS device."    
