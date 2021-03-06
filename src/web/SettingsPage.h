#include <Arduino.h>

const char settingsPage[] PROGMEM = R"=====(
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

      h1 {
        margin-top: 10px;
      }

      h2 {
        padding-top: 20px;
        margin-bottom: 3px;
      }

      .textInput {
        border-radius: 5px;
        border-style: solid;
        border-width: 1px;
        border-color: darkgray;
        height: 25px;
        max-width: 300px;
        min-width: 150px;
        width: 100%;
        padding-left: 5px;
      }

      ::placeholder {
        font-style: italic;
        color: #bbbbbb;
      }

      .textInputShort {
        border-radius: 5px;
        border-style: solid;
        border-width: 1px;
        border-color: darkgray;
        height: 25px;
        max-width: 80px;
        min-width: 30px;
      }

      .inputLabel {
        width: 135px;
        font-size: 1.3em;
        display: block;
        margin-bottom: 10px;
      }

      .inputLabelNotWrapping {
        width: 135px;
        font-size: 1.3em;
        display: inline-block;
        margin-bottom: 10px;
      }

      .inputRow {
        width: 100%;
        padding-left: 10px;
        margin-top: 20px;
      }

      .submitButton {
        height: 40px;
        border-radius: 5px;
        border-width: 5px;
        border-style: hidden;
        width: 80px;
        font-size: 1.1em;
      }

      .checkboxInput {
        width: 20px;
        height: 20px;
      }

      @media (min-width: 576px) {
        .inputLabel {
          display: inline-block;
          margin-bottom: 0;
        }
      }
    </style>

    <script>
      var submitButton;
      var nameInput;
      var activateReportingInput;
      var editAddressInput;
      var intervalSecsInput;
      var activatePassiveInput;
      var activateReportingBatteryInput;
      var editAddressBatteryInput;

      window.onload = () => {
        determineHtmlObjects();

        loadAndFillSettings();

        submitButton.addEventListener("click", () => {
          settingsResult = checkSettings();

          if (!settingsResult.success) {
            window.alert(settingsResult.message);
          } else {
            xhr = new XMLHttpRequest();

            xhr.open("POST", "/settings", true);
            xhr.setRequestHeader("Content-Type", "application/json");

            xhr.onreadystatechange = function() {
              if (xhr.readyState === 4) {
                if (xhr.status === 200) {
                  window.alert("Settings updated successfully! ESP is going to restart...");

                  location.href = "/";
                } else {
                  window.alert("Something went wrong!");
                }
              }
            };

            xhr.send(JSON.stringify(settingsResult.settings));
          }
        });
      };

      function checkSettings() {
        checkResult = {};
        checkResult.settings = {};
        checkResult.success = false;
        checkResult.message = "";

        nameSetting = nameInput.value;
        activateReportingSetting = activateReportingInput.checked;
        editAddressSetting = editAddressInput.value;
        intervalSecsSetting = intervalSecsInput.value;
        activatePassiveSetting = activatePassiveInput.checked;
        activateBatteryReportingSetting = activateReportingBatteryInput.checked;
        editAddressBatterySetting = editAddressBatteryInput.value;

        if (nameSetting.length == 0) {
          checkResult.message = "Field 'Name' must not be empty!";
        } else if (activateReportingSetting && editAddressSetting.length == 0) {
          checkResult.message = "Field 'Address Temperature' must not be empty!";
        } else if (activateBatteryReportingSetting && editAddressBatterySetting.length == 0) {
          checkResult.message = "Field 'Address Battery' must not be empty!";
        } else if (
          activateReportingSetting &&
          (intervalSecsSetting.length == 0 || intervalSecsSetting !== parseInt(intervalSecsSetting, 10).toString())
        ) {
          checkResult.message = "Field 'Interval secs' must not be empty!";
        } else {
          checkResult.success = true;

          checkResult.settings.name = nameSetting;
          checkResult.settings.activateRep = activateReportingSetting;
          checkResult.settings.editAddress = editAddressSetting;
          checkResult.settings.intervalSecs = intervalSecsSetting;
          checkResult.settings.passive = activatePassiveSetting;
          checkResult.settings.activateRepBat = activateBatteryReportingSetting;
          checkResult.settings.editAddressBat = editAddressBatterySetting;
        }

        return checkResult;
      }

      function determineHtmlObjects() {
        submitButton = document.getElementById("submitButton");
        nameInput = document.getElementById("nameInput");
        activateReportingInput = document.getElementById("activateReportingInput");
        editAddressInput = document.getElementById("editAddressInput");
        intervalSecsInput = document.getElementById("intervalSecsInput");
        activatePassiveInput = document.getElementById("activatePassiveInput");
        activateReportingBatteryInput = document.getElementById("activateReportingBatteryInput");
        editAddressBatteryInput = document.getElementById("editAddressBatteryInput");
      }

      function loadAndFillSettings() {
        xhr = new XMLHttpRequest();

        xhr.open("GET", "/settings", true);
        xhr.setRequestHeader("Content-Type", "application/json");

        xhr.onreadystatechange = function() {
          if (xhr.readyState === 4) {
            if (xhr.status === 200) {
              settingsObject = JSON.parse(xhr.responseText);

              nameInput.value = settingsObject.name;
              activateReportingInput.checked = settingsObject.activateRep;
              editAddressInput.value = settingsObject.editAddress;
              intervalSecsInput.value = settingsObject.intervalSecs;
              activatePassiveInput.checked = settingsObject.passive;
              activateReportingBatteryInput.checked = settingsObject.activateRepBat;
              editAddressBatteryInput.value = settingsObject.editAddressBat;

              // Setting title
              window.document.title = "Thermometer-" + settingsObject.name;
            } else {
              window.alert("Could not retrieve settings!");
            }
          }
        };

        xhr.send(null);
      }
    </script>
  </head>

  <body>
    <h1 style="font-size: 3em">Settings</h1>
    <hr />
    <div class="inputRow">
      <h2>General</h2>
    </div>
    <div class="inputRow">
      <span class="inputLabel">Name </span>
      <input type="text" class="textInput" id="nameInput" />
    </div>
    <div class="inputRow">
      <h2>Reporting</h2>
    </div>
    <div class="inputRow">
      <span class="inputLabelNotWrapping">Activate</span>
      <input class="checkboxInput" type="checkbox" id="activateReportingInput" />
    </div>
    <div class="inputRow">
      <span class="inputLabel">Address Temperature</span>
      <input
        type="text"
        class="textInput"
        id="editAddressInput"
        placeholder="e.g. http://openhab/api/items/temperature/state"
      />
    </div>
    <div class="inputRow">
      <span class="inputLabel">Interval secs</span>
      <input type="text" class="textInputShort" id="intervalSecsInput" />
    </div>
    <div class="inputRow">
      <span class="inputLabelNotWrapping">Battery reporting</span>
      <input class="checkboxInput" type="checkbox" id="activateReportingBatteryInput" />
    </div>
    <div class="inputRow">
      <span class="inputLabel">Address Battery</span>
      <input
        type="text"
        class="textInput"
        id="editAddressBatteryInput"
        placeholder="e.g. http://openhab/api/items/sensor_battery/state"
      />
    </div>
    <div class="inputRow">
      <span class="inputLabelNotWrapping">Passive</span>
      <input class="checkboxInput" type="checkbox" id="activatePassiveInput" />
    </div>
    <div class="inputRow">
      <button class="submitButton" id="submitButton">Submit</button>
    </div>
  </body>
</html>

)=====";