// http://en.ilmatieteenlaitos.fi/open-data-manual
// Free registration required to get API-key
var myAPIKey = '';

// data.fmi.fi currently doesn't support HTTPS, so do not want to send GPS coordinates with API-key in cleartext
var weatherCity = '';

// Listen for when the watchface is opened
Pebble.addEventListener('ready', 
    function(e) {
        console.log('PebbleKit JS ready!');

        fetchWeather();
    }
);

// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
    function(e) {
        console.log('AppMessage received!');

        fetchWeather();
    }                     
);

// XMLHttpRequest function for easier reuse
var xhrRequest = function (url, type, callback) {
    var xhr = new XMLHttpRequest();
    xhr.onload = function () {
        callback(this.responseText);
    };
    xhr.open(type, url);
    xhr.send();
};

function fetchWeather() {
    // Finnish Meteorological Institute open data
    var url= 'http://data.fmi.fi/fmi-apikey/' + myAPIKey + '/wfs?request=getFeature&storedquery_id=fmi::observations::weather::multipointcoverage&place=' + weatherCity + '&parameters=temperature&maxlocations=1'; 

    // Send request to FMI
    xhrRequest(url, 'GET', 
        function(responseText) {
            // Pebble js doesn't support XML parse, so have to use regex
            var temperature = responseText.match(/<gml\:doubleOrNilReasonTupleList>[\s\n\r\d\.\-]*\s([\d\.\-]+)[^\d]*<\/gml\:doubleOrNilReasonTupleList>/i);
            var city = responseText.match(/\/region\">[\s\n\r]*([^\s\n\r]*)[\s\n\r]*<\/target\:region>/i);
        
            var cityvalue = false;
            var temperaturevalue = false;

            // Check if regex found anything
            try{
                cityvalue = city[1];
                temperaturevalue = temperature[1];
            } catch(err) {
                console.log(err.message);
            }

            // If values found, send them to watch
            if( cityvalue && temperaturevalue) {
                console.log(temperaturevalue);
                console.log(cityvalue);
                
                var dictionary = {
                    'WEATHER_TEMPERATURE_KEY': temperaturevalue + '\xB0C',
                    'WEATHER_CITY_KEY': cityvalue
                };

                // Send to Pebble
                Pebble.sendAppMessage(dictionary,
                    function(e) {
                        console.log('Weather info sent to Pebble successfully!');
                    },
                    function(e) {
                        console.log('Error sending weather info to Pebble!');
                    }
                );
            }
        }
    );
    
}

