'use strict';

/*
 * App ID for the skill
 */
var APP_ID = "REPLACE__ALEXA_APP_ID;
var SKILL_NAME = 'REPLACE_ALEXA_SKILL_NAME';

/*
 * Alexa SDK
 */
var Alexa = require('alexa-sdk');

/*
* HTTP/HTTPS
*/
var https = require('https');

/*
 * Thinger.io device
 */
const ti_user = "REPLACE_ME__THINGER_IO_USER";
const ti_device = "REPLACE_ME__THINGER_IO_DEVICE_ID";
const ti_token = "REPLACE_ME__THINGER_IO_ACCESS_TOKEN";
const ti_api_host = "api.thinger.io"
const ti_api_port = 443;
const ti_api_base_path = "/v2/users/" + ti_user + "/devices/" + ti_device + "/";

/*
 * Register handlers
 */

exports.handler = function (event, context, callback) {
    var alexa = Alexa.handler(event, context);
    alexa.appId = APP_ID;
    alexa.registerHandlers(handlers);
    alexa.execute();
};

var handlers = {

    /*
     * The "ShowStatus" intent:
     */
    "ShowStatus": function () {
        var myHandler = this;
        var speechOutput;
        var cardTitle;
        var statusSlot = this.event.request.intent.slots.Status;
        var status = "open";
        var imageNumber = 0;

        // Get slot(s):
        if (statusSlot && statusSlot.value) {
            status = statusSlot.value.toLowerCase();
        }

        // Determine image number from status:
        switch (status) {
            case "open":
                imageNumber = 0;
                break;
            case "closed":
                imageNumber = 2;
                break;
            case "away":
                imageNumber = 1;
                break;
            default:
                imageNumber = 0;
                break;
        }

        // Build path:
        var ti_api_path = ti_api_base_path + "image";

        // Build request body:
        var ti_input = { in: {
                clean: true,
                number: imageNumber
            }
        };

        // Build POST request:
        var request_body = JSON.stringify(ti_input);
        var request_headers = {
            "Authorization": "Bearer " + ti_token,
            "Content-Type": "application/json",
            "Content-Length": Buffer.byteLength(request_body)
        }
        var request_options = {
            host: ti_api_host,
            port: ti_api_port,
            path: ti_api_path,
            method: "POST",
            headers: request_headers
        }

        console.log("REQUEST - HEAD:" + JSON.stringify(request_options));
        console.log("REQUEST - BODY:" + JSON.stringify(request_body));

        // Handle POST request:
        var request = https.request(request_options, function (r) {
            console.log("RESPONSE - STATUS:" + r.statusCode);
            r.on('data', function (d) {
                console.log("RESPONSE:" + d);
                var d_json = JSON.parse(d);
            });
            r.on('end', function () {
                console.log("END: returning speech output ...");
                speechOutput = "The door sign has been updated successfully! The new status shown is '"+ status+"'.";
                cardTitle = "Success";
                myHandler.emit(':tellWithCard', speechOutput, cardTitle, speechOutput);
            });
            r.on('error', function (e) {
                console.log("ERROR:");
                console.error(e);
                speechOutput = "Sorry, there was problem - I could not update the door sign!";
                cardTitle = "Error";
                myHandler.emit(':tellWithCard', speechOutput, cardTitle, speechOutput);
            });
        });

        // Send POST request:
        request.write(request_body);
        request.end();
    },

    /*
     * Built-in intents:
     */
    "AMAZON.HelpIntent": function () {
        this.emit(':ask', "You can say tell door sign 'we are open', or, you can say exit... What can I help you with?", "What can I help you with?");
    },
    "AMAZON.StopIntent": function () {
        var speechOutput = "OK";
        this.emit(':tell', speechOutput);
    },
    'Unhandled': function () {
        this.emit(':ask', "What can I do for you?", "What can I do for you?");
    }
};