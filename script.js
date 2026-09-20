import {
    initializeApp
} from "https://www.gstatic.com/firebasejs/12.7.0/firebase-app.js";

import {
    getDatabase,
    ref,
    onValue
} from "https://www.gstatic.com/firebasejs/12.7.0/firebase-database.js";


// =========================================================
// CYRUS FIREBASE CONFIG
// =========================================================

const firebaseConfig = {

    apiKey:
        "AIzaSyAy89R7qsCggt1gHQdeh7ds35QieXaui6o",

    authDomain:
        "cyrus-ebb57.firebaseapp.com",

    databaseURL:
        "https://cyrus-ebb57-default-rtdb.europe-west1.firebasedatabase.app",

    projectId:
        "cyrus-ebb57",

    storageBucket:
        "cyrus-ebb57.firebasestorage.app",

    messagingSenderId:
        "850409218429",

    appId:
        "1:850409218429:web:7ec610a12e4fedd5d1ac14",

    measurementId:
        "G-SL2VN1J2S5"

};


// =========================================================
// INITIALIZE FIREBASE
// =========================================================

const firebaseApp =
    initializeApp(
        firebaseConfig
    );


// =========================================================
// DATABASE
// =========================================================

const database =
    getDatabase(
        firebaseApp
    );


// =========================================================
// MAIN DATABASE PATH
// =========================================================

const dataRef =
    ref(
        database,
        "ESP32_Data"
    );


// =========================================================
// VARIABLES
// =========================================================

let allSensorData = {};

let selectedDate = "";

let sensorChart = null;


// =========================================================
// DOM ELEMENTS
// =========================================================

const statusText =
    document.getElementById(
        "statusText"
    );


const statusDot =
    document.getElementById(
        "statusDot"
    );


const currentTemperature =
    document.getElementById(
        "currentTemperature"
    );


const currentHumidity =
    document.getElementById(
        "currentHumidity"
    );


// ONLY ONE DATE SELECTOR
// This controls BOTH graph and history.

const historyDate =
    document.getElementById(
        "historyDate"
    );


const historyBody =
    document.getElementById(
        "historyTableBody"
    );


const recordCount =
    document.getElementById(
        "recordCount"
    );


const toggleHistory =
    document.getElementById(
        "toggleHistory"
    );


const historyContent =
    document.getElementById(
        "historyContent"
    );


// =========================================================
// ONLINE STATUS
// =========================================================

function setOnlineStatus()
{
    if (statusText)
    {
        statusText.textContent =
            "SYSTEM ONLINE";
    }


    if (statusDot)
    {
        statusDot.classList.remove(
            "offline"
        );

        statusDot.classList.add(
            "online"
        );
    }
}


// =========================================================
// OFFLINE STATUS
// =========================================================

function setOfflineStatus(
    message = "SYSTEM OFFLINE"
)
{
    if (statusText)
    {
        statusText.textContent =
            message;
    }


    if (statusDot)
    {
        statusDot.classList.remove(
            "online"
        );

        statusDot.classList.add(
            "offline"
        );
    }
}


// =========================================================
// NUMBER HELPER
// =========================================================

function toNumber(value)
{
    const number =
        Number(value);


    if (
        Number.isFinite(
            number
        )
    )
    {
        return number;
    }


    return null;
}


// =========================================================
// FORMAT NUMBER
// =========================================================

function formatNumber(value)
{
    if (
        value === null ||
        value === undefined
    )
    {
        return "--";
    }


    return Number(value)
        .toFixed(1);
}


// =========================================================
// GET DATE LIST
// =========================================================

function getDateList(data)
{
    return Object.keys(
        data || {}
    )
    .filter(
        key =>
            data[key] &&
            typeof data[key] === "object"
    )
    .sort()
    .reverse();
}


// =========================================================
// GET LATEST READING
// =========================================================

function getLatestReading(data)
{
    let latest = null;


    const dates =
        Object.keys(
            data || {}
        ).sort();


    for (
        const date of dates
    )
    {
        const times =
            Object.keys(
                data[date] || {}
            ).sort();


        for (
            const time of times
        )
        {
            const reading =
                data[date][time];


            if (
                !reading ||
                typeof reading !== "object"
            )
            {
                continue;
            }


            const temperature =
                toNumber(
                    reading.temperature
                );


            const humidity =
                toNumber(
                    reading.humidity
                );


            if (
                temperature === null &&
                humidity === null
            )
            {
                continue;
            }


            latest =
            {
                date:
                    date,

                time:
                    time,

                temperature:
                    temperature,

                humidity:
                    humidity
            };
        }
    }


    return latest;
}


// =========================================================
// GET READINGS FOR DATE
// =========================================================

function getReadingsForDate(
    date
)
{
    const result = [];


    if (!date)
    {
        return result;
    }


    const dayData =
        allSensorData[date];


    if (
        !dayData ||
        typeof dayData !== "object"
    )
    {
        return result;
    }


    const times =
        Object.keys(
            dayData
        )
        .filter(
            time =>
                dayData[time] &&
                typeof dayData[time] === "object"
        )
        .sort();


    times.forEach(
        time =>
        {
            const reading =
                dayData[time];


            const temperature =
                toNumber(
                    reading.temperature
                );


            const humidity =
                toNumber(
                    reading.humidity
                );


            if (
                temperature === null &&
                humidity === null
            )
            {
                return;
            }


            result.push(
            {
                time:
                    time,

                temperature:
                    temperature,

                humidity:
                    humidity
            });
        }
    );


    return result;
}


// =========================================================
// POPULATE ONLY HISTORY DATE
// =========================================================

function populateDateSelect()
{
    if (!historyDate)
    {
        return;
    }


    const dates =
        getDateList(
            allSensorData
        );


    historyDate.innerHTML =
        "";


    if (
        dates.length === 0
    )
    {
        const option =
            document.createElement(
                "option"
            );


        option.value =
            "";


        option.textContent =
            "No dates available";


        historyDate.appendChild(
            option
        );


        return;
    }


    dates.forEach(
        date =>
        {
            const option =
                document.createElement(
                    "option"
                );


            option.value =
                date;


            option.textContent =
                date;


            historyDate.appendChild(
                option
            );
        }
    );


    if (
        selectedDate &&
        dates.includes(
            selectedDate
        )
    )
    {
        historyDate.value =
            selectedDate;
    }
    else
    {
        selectedDate =
            dates[0];

        historyDate.value =
            selectedDate;
    }
}


// =========================================================
// UPDATE CURRENT READING
// =========================================================

function updateCurrentReading()
{
    const latest =
        getLatestReading(
            allSensorData
        );


    if (!latest)
    {
        if (currentTemperature)
        {
            currentTemperature.textContent =
                "-- °C";
        }


        if (currentHumidity)
        {
            currentHumidity.textContent =
                "-- %";
        }


        return;
    }


    if (currentTemperature)
    {
        currentTemperature.textContent =
            formatNumber(
                latest.temperature
            ) +
            " °C";
    }


    if (currentHumidity)
    {
        currentHumidity.textContent =
            formatNumber(
                latest.humidity
            ) +
            " %";
    }


    console.log(
        "LATEST DHT11 READING:",
        latest
    );
}


// =========================================================
// UPDATE GRAPH
// =========================================================

function updateChart()
{
    if (
        typeof Chart === "undefined"
    )
    {
        console.error(
            "Chart.js is not loaded."
        );

        return;
    }


    const canvas =
        document.getElementById(
            "sensorChart"
        );


    if (!canvas)
    {
        console.error(
            "sensorChart canvas not found."
        );

        return;
    }


    // =====================================================
    // IMPORTANT:
    // USE THE SAME DATE SELECTOR AS HISTORY
    // =====================================================

    const date =
        historyDate
            ? historyDate.value
            : selectedDate;


    const readings =
        getReadingsForDate(
            date
        );


    const labels =
        readings.map(
            item =>
                item.time
        );


    const temperatures =
        readings.map(
            item =>
                item.temperature
        );


    const humidities =
        readings.map(
            item =>
                item.humidity
        );


    if (sensorChart)
    {
        sensorChart.destroy();

        sensorChart =
            null;
    }


    sensorChart =
        new Chart(
            canvas,
            {

                type:
                    "line",


                data:
                {
                    labels:
                        labels,


                    datasets:
                    [

                        {
                            label:
                                "Temperature (°C)",

                            data:
                                temperatures,

                            yAxisID:
                                "temperature",

                            borderColor:
                                "#38bdf8",

                            backgroundColor:
                                "rgba(56,189,248,.12)",

                            tension:
                                0.3,

                            borderWidth:
                                3,

                            pointRadius:
                                4
                        },


                        {
                            label:
                                "Humidity (%)",

                            data:
                                humidities,

                            yAxisID:
                                "humidity",

                            borderColor:
                                "#22c55e",

                            backgroundColor:
                                "rgba(34,197,94,.10)",

                            tension:
                                0.3,

                            borderWidth:
                                3,

                            pointRadius:
                                4
                        }

                    ]
                },


                options:
                {
                    responsive:
                        true,

                    maintainAspectRatio:
                        false,

                    animation:
                    {
                        duration:
                            300
                    },

                    interaction:
                    {
                        mode:
                            "index",

                        intersect:
                            false
                    },


                    plugins:
                    {
                        legend:
                        {
                            labels:
                            {
                                color:
                                    "#cbd5e1"
                            }
                        }
                    },


                    scales:
                    {

                        x:
                        {
                            ticks:
                            {
                                color:
                                    "#64748b"
                            },

                            grid:
                            {
                                color:
                                    "#1e293b"
                            }
                        },


                        temperature:
                        {
                            type:
                                "linear",

                            position:
                                "left",

                            ticks:
                            {
                                color:
                                    "#38bdf8"
                            },

                            title:
                            {
                                display:
                                    true,

                                text:
                                    "Temperature (°C)",

                                color:
                                    "#38bdf8"
                            },

                            grid:
                            {
                                color:
                                    "#1e293b"
                            }
                        },


                        humidity:
                        {
                            type:
                                "linear",

                            position:
                                "right",

                            ticks:
                            {
                                color:
                                    "#22c55e"
                            },

                            title:
                            {
                                display:
                                    true,

                                text:
                                    "Humidity (%)",

                                color:
                                    "#22c55e"
                            },

                            grid:
                            {
                                drawOnChartArea:
                                    false
                            }
                        }

                    }
                }
            }
        );


    console.log(
        "GRAPH UPDATED:",
        date,
        readings.length,
        "records"
    );
}


// =========================================================
// UPDATE HISTORY
// =========================================================

function updateHistory()
{
    if (!historyBody)
    {
        console.error(
            "historyTableBody was not found in HTML."
        );

        return;
    }


    const date =
        historyDate
            ? historyDate.value
            : selectedDate;


    const readings =
        getReadingsForDate(
            date
        );


    historyBody.innerHTML =
        "";


    // =====================================================
    // NO DATA
    // =====================================================

    if (
        readings.length === 0
    )
    {
        const row =
            document.createElement(
                "tr"
            );


        const cell =
            document.createElement(
                "td"
            );


        cell.colSpan =
            3;


        cell.textContent =
            "No sensor data available.";


        row.appendChild(
            cell
        );


        historyBody.appendChild(
            row
        );


        if (recordCount)
        {
            recordCount.textContent =
                "0 records";
        }


        return;
    }


    // =====================================================
    // DISPLAY RECORDS
    // =====================================================

    readings
        .slice()
        .reverse()
        .forEach(
            reading =>
            {
                const row =
                    document.createElement(
                        "tr"
                    );


                const timeCell =
                    document.createElement(
                        "td"
                    );


                const temperatureCell =
                    document.createElement(
                        "td"
                    );


                const humidityCell =
                    document.createElement(
                        "td"
                    );


                timeCell.textContent =
                    reading.time;


                temperatureCell.textContent =
                    formatNumber(
                        reading.temperature
                    ) +
                    " °C";


                humidityCell.textContent =
                    formatNumber(
                        reading.humidity
                    ) +
                    " %";


                row.appendChild(
                    timeCell
                );


                row.appendChild(
                    temperatureCell
                );


                row.appendChild(
                    humidityCell
                );


                historyBody.appendChild(
                    row
                );
            }
        );


    if (recordCount)
    {
        recordCount.textContent =
            readings.length +
            (
                readings.length === 1
                    ? " record"
                    : " records"
            );
    }


    console.log(
        "HISTORY UPDATED:",
        date,
        readings.length,
        "records"
    );
}


// =========================================================
// UPDATE DASHBOARD
// =========================================================

function updateDashboard()
{
    const dates =
        getDateList(
            allSensorData
        );


    console.log(
        "AVAILABLE DATES:",
        dates
    );


    // =====================================================
    // NO FIREBASE DATA
    // =====================================================

    if (
        dates.length === 0
    )
    {
        if (currentTemperature)
        {
            currentTemperature.textContent =
                "-- °C";
        }


        if (currentHumidity)
        {
            currentHumidity.textContent =
                "-- %";
        }


        if (historyBody)
        {
            historyBody.innerHTML =
                "";


            const row =
                document.createElement(
                    "tr"
                );


            const cell =
                document.createElement(
                    "td"
                );


            cell.colSpan =
                3;


            cell.textContent =
                "No sensor data available.";


            row.appendChild(
                cell
            );


            historyBody.appendChild(
                row
            );
        }


        if (recordCount)
        {
            recordCount.textContent =
                "0 records";
        }


        populateDateSelect();

        updateChart();

        return;
    }


    // =====================================================
    // SET DEFAULT DATE
    // =====================================================

    if (
        !selectedDate ||
        !dates.includes(
            selectedDate
        )
    )
    {
        selectedDate =
            dates[0];
    }


    // =====================================================
    // POPULATE ONE DATE SELECTOR
    // =====================================================

    populateDateSelect();


    // =====================================================
    // UPDATE EVERYTHING
    // =====================================================

    updateCurrentReading();

    updateChart();

    updateHistory();
}


// =========================================================
// FIREBASE REALTIME LISTENER
// =========================================================

console.log(
    "================================="
);

console.log(
    "CYRUS ACTIVITY 4"
);

console.log(
    "CONNECTING TO FIREBASE"
);

console.log(
    "Database path: /ESP32_Data"
);

console.log(
    "================================="
);


setOfflineStatus(
    "CONNECTING..."
);


onValue(

    dataRef,

    (snapshot) =>
    {
        console.log(
            "================================="
        );


        console.log(
            "FIREBASE DATA RECEIVED"
        );


        const value =
            snapshot.val();


        console.log(
            "FIREBASE VALUE:",
            value
        );


        allSensorData =
            value || {};


        setOnlineStatus();


        updateDashboard();


        console.log(
            "Dashboard updated successfully."
        );


        console.log(
            "================================="
        );
    },


    (error) =>
    {
        console.error(
            "FIREBASE READ ERROR:",
            error
        );


        setOfflineStatus(
            "DATABASE ERROR"
        );
    }

);


// =========================================================
// ONLY HISTORY DATE CHANGE
// =========================================================

if (historyDate)
{
    historyDate.addEventListener(
        "change",
        function()
        {
            selectedDate =
                this.value;


            console.log(
                "DATE SELECTED:",
                selectedDate
            );


            // UPDATE GRAPH
            updateChart();


            // UPDATE HISTORY
            updateHistory();
        }
    );
}


// =========================================================
// SHOW / HIDE HISTORY
// =========================================================

if (toggleHistory)
{
    toggleHistory.addEventListener(
        "click",
        function()
        {
            if (
                historyContent &&
                historyContent.classList.contains(
                    "hidden"
                )
            )
            {
                historyContent.classList.remove(
                    "hidden"
                );


                toggleHistory.textContent =
                    "HIDE HISTORY";


                // Make sure current date data
                // is displayed immediately.
                updateHistory();
            }
            else if (
                historyContent
            )
            {
                historyContent.classList.add(
                    "hidden"
                );


                toggleHistory.textContent =
                    "SHOW HISTORY";
            }
        }
    );
}


// =========================================================
// INITIAL STATUS
// =========================================================

console.log(
    "CYRUS SCRIPT LOADED SUCCESSFULLY"
);