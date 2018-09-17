# C code to call an IFTTT webhook applet when your Hypernode is down

## Installing

1. Install [libcurl](http://curl.haxx.se/libcurl/c).  
     * `sudo apt install libcurl4-gnutls-dev`
2. Install [libconfig](https://github.com/hyperrealm/libconfig)  
     * `sudo apt install libconfig-dev`
3. Clone this [repository](https://github.com/bismuthfoundation/Hypernode-Alerts.git) using Git or download the [zip file](https://github.com/bismuthfoundation/Hypernode-Alerts/archive/master.zip) from Github.
4. Change to directory `c-Lemoussel-IFTTT` and do `make`


## Usage

This tool use [Bismuth.live API](https://hypernodes.bismuth.live/status.json).  
This API lists all registered Hypernodes with their current PoS height, or -1 if inaccessible.  
Do not hammer this url, it only updates every 3 minutes.

Modify configuration file `HNAifttt.cfg`  

```shell
// How often to check your HN status, minimum 180 sec (3 minutes).
// Do not hammer this url, it only updates every 3 minutes.
timeSupervision=180
// IP of Hypernode
nodeIpAddress=["192.99.248.44"]
// How often to check your HN status, minimum 180 sec (3 minutes).
timeSupervision=180
// IFTT Key
iftttKey="myIFTTTkey"
// IFFT Webhook Event
// Example: 
//   HNBIS_sms   : IFTTT Webhook applet to send a text message
//   HNBIS_email : IFTTT Webhook applet to send a email
iftttEvent=["HNBIS_sms", "HNBIS_email"]
```

and, basically, run `HNAifttt`

Every `timeSupervision` seconds, it checks if your HN is accessible.  
If your Hypernode is inaccessible, an HTTP call is made to execute IFTTT events (`iftttEvent`).

Look at [Trigger an HTTP request to Text Message from IFTTT](../IFTTT/webhook.md) to find out how to create an IFTTT Webhook applet that works with any device or application that can make a web request and that has triggered an IFTTT Webhook applet to send a text message (SMS).
