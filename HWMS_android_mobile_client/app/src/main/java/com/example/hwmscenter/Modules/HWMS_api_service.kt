package com.example.hwmscenter.Modules

import org.jsoup.Jsoup

/*
=====================================================================
----------------|Mini-Extraction HWMS🌤️ Api Service|-----------------
----------------| by https://github.com/Maxdgf |---------------------
=====================================================================
|         Extracts weather data and the current device IP.           |
 */

private const val URL = "http://192.168.0.44:80/" //replace with your esp 01 ip address

class HWMS_api_service {
    //fetching current device ip
    fun fetchHWMScurrentIp(): String {
        var currentIp = ""

        try {
            val document = Jsoup.connect(URL).get()
            currentIp = document.select("h4").text()
        } catch (e: Exception) {
            //if fetching is not successful, returns undefined data
            e.printStackTrace()
            currentIp = "current ip: ?"
        }

        return currentIp
    }

    //fetching main sensors data
    fun fetchHWMSvalues(): String {
        var result = ""

        try {
            val document = Jsoup.connect(URL).get()
            val mainData = document.select("h1")

            for (data in mainData) {
                result += data.text() + "\n"
            }
        } catch (e: Exception) {
            //if fetching is not successful, returns warning message
            e.printStackTrace()
            result = "Null data!\nCheck wifi connection."
        }

        return result
    }
}