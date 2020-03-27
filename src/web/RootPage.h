#include <Arduino.h>

const char rootPage[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
  <head>
    <meta name="viewport" content="width=device-width, initial-scale=1" />

    <title>Thermometer</title>

    <style>
      body {
        margin: 0;
        padding: 30px;
        background-color: white;
        color: #535353;
        font-family: "Trebuchet MS", "Lucida Sans Unicode", "Lucida Grande", "Lucida Sans", Arial, sans-serif;
      }

      body > h1 {
        margin-top: 10px;
      }

      .temperatureLabel {
        width: 120px;
        font-size: 4em;
        display: block;
        margin-bottom: 10px;
      }

      .row {
        width: 100%;
        padding-left: 10px;
        margin-top: 30px;
      }

      .openSettingsButton {
        height: 40px;
        border-radius: 5px;
        border-width: 5px;
        border-style: hidden;
        width: 150px;
        font-size: 1.1em;
      }

      @media (min-width: 576px) {
        .inputLabel {
          display: inline-block;
        }

        .inputLabel {
          margin-bottom: 0;
        }
      }
    </style>

    <script>
      var headingThermometer;

      function loadAndFillHeadings() {
        var xhr = new XMLHttpRequest();

        xhr.open("GET", "/settings", true);
        xhr.setRequestHeader("Content-Type", "application/json");

        xhr.onreadystatechange = function() {
          if (xhr.readyState === 4) {
            if (xhr.status === 200) {
              settingsObject = JSON.parse(xhr.responseText);

              // Setting title
              var name = "Thermometer-" + settingsObject.name;
              window.document.title = name;
              headingThermometer.innerText = name;
            } else {
              window.alert("Could not retrieve settings!");
            }
          }
        };

        xhr.send(null);
      }

      function updateTemperature() {
        var xhr = new XMLHttpRequest();

        xhr.open("GET", "/temperature", true);

        xhr.onreadystatechange = function() {
          if (xhr.readyState === 4) {
            if (xhr.status === 200) {
              var temp = parseFloat(xhr.responseText).toFixed(1);

              textTemperature.innerText = temp + "\u00b0C";
            } else {
              console.log("Could not retrieve temperature!");
              textTemperature.innerText = "NaN";
            }
          }
        };

        xhr.send(null);
      }

      window.onload = () => {
        headingThermometer = document.getElementById("headingThermometer");
        textTemperature = document.getElementById("textTemperature");

        loadAndFillHeadings();

        updateTemperature();
        window.setInterval(updateTemperature, 5000);
      };
    </script>
  </head>

  <body>
    <h1 style="font-size: 3em" id="headingThermometer">Thermometer</h1>
    <hr />
    <div class="row">
      <span class="temperatureLabel" id="textTemperature"></span>
    </div>
    <div class="row">
      <button class="openSettingsButton" id="openSettingsButton" onclick="location.href = '/settingsPage';">
        Open settings
      </button>
    </div>
  </body>
</html>


)=====";