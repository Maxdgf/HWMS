'''
=====================================================================
----------------|Simple HWMS🌤️ Desktop Parser App|------------------
----------------| by https://github.com/Maxdgf |---------------------
=====================================================================
|    Shows current weather data and device ip from HWMS web page.   |
'''

from tkinter import*
import threading
import time

from Modules.constants import*
from Modules.HWMS_api import*
import Modules.HWMS_api as hwms

root = Tk()
root.title(NAME)
root.geometry("900x500")
root.configure(bg=MAIN_COLOR)

#thread of get main data (weather data)
def get_data_from_server():
    def main_task():
        while True:
            weather_data = hwms.get_weather_data()
            current_ip = hwms.get_current_ip()
            root.after(0, update_data_label, current_ip, current_ip_label)
            root.after(0, update_data_label, weather_data, main_data_label)
            time.sleep(REFRESH_DELAY)

    thread = threading.Thread(target=main_task, daemon=True)
    thread.start()        

#update data in label
def update_data_label(data, data_label):
    data_label.configure(text=data)

#UI
name_label = Label(root, text=NAME, font=("Segoe UI, Tahoma, Geneva, Verdana, sans-serif", 16, "bold"), bg=MAIN_COLOR, fg="white")
name_label.pack(pady=10)
description_label = Label(root, text="control center", font=("Segoe UI, Tahoma, Geneva, Verdana, sans-serif", 10, "italic"), bg=MAIN_COLOR, fg="white")
description_label.pack(pady=10)
main_data_label = Label(root, text="Loading data...", font=("Segoe UI, Tahoma, Geneva, Verdana, sans-serif", 10, "bold"), bg=MAIN_COLOR, fg="white")
main_data_label.pack(expand=True)
current_ip_label = Label(root, text="Loading ip...", font=("Segoe UI, Tahoma, Geneva, Verdana, sans-serif", 12, "italic"), bg=MAIN_COLOR, fg="white")
current_ip_label.pack(side="bottom", pady=10)

#launch data threads functions
get_data_from_server()

root.mainloop()
