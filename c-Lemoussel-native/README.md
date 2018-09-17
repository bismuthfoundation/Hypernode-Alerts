# C code to send email when your Hypernode is down

## Installing

1. Install [libcurl](http://curl.haxx.se/libcurl/c).
     * `sudo apt install libcurl4-gnutls-dev`
2. Install [libconfig](https://github.com/hyperrealm/libconfig)
     * `sudo apt install libconfig-dev`
3. Clone this [repository](https://github.com/bismuthfoundation/Hypernode-Alerts.git) using Git or download the [zip file](https://github.com/bismuthfoundation/Hypernode-Alerts/archive/master.zip) from Github.
4. Change to directory `c-Lemoussel-native` and do `make`

## Usage

This tool use [Bismuth.live API](https://hypernodes.bismuth.live/status.json)
This API lists all registered Hypernodes with their current PoS height, or -1 if inaccessible. Do not hammer this url, it only updates every 3 minutes.

Modify configuration file `HNAnative.cfg`

```shell
// How often to check your HN status, minimum 180 sec (3 minutes).
// Do not hammer this url, it only updates every 3 minutes.
timeSupervision=180
// IP of Hypernode
nodeIpAddress=["192.99.248.44", "192.99.248.45", "192.99.248.46", "192.99.248.47"]
// Where to send email
toAddress="lemoussel@testemail.com"
// SMTP server
smtpServer="smtpserver.com" 
// SMTP port
smtpPort=587 or 465
// SMTP protocol
smtpProtocol="smtp" or "smtps"
// SMTP account (email account)
smtpUser="lemoussel@testsmtpserver.com"
// SMTP password (mail password)
smtpPasswd="password"
```

and, basically, run `HNAnative`

Every `timeSupervision` seconds, it checks if your HN is accessible. If your Hypernode is inaccessible, an email is sent with this information :

```shell
Subject: BISMUTH NODE ALERT
Email content: Hypernode IP: [HN IP adress] OFF Line Status: -1
```

## Google configuration

```shell
// SMTP server
smtpServer="smtp.gmail.com"
smtpPort=587
smtpProtocol="smtp"
// SMTP account (email account)
smtpUser="GmailAccount@gmail.com"
// SMTP password (mail password)
smtpPasswd="password of GmailAccount"
```

You will most likely need to go here and allow unsecure apps: [Google Less secure application access](https://www.google.com/settings/security/lesssecureapps)
