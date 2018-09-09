# Hypernode-Alerts
Various apps from Bismuth users to get alerted when your Hypernode is down.

Apps are named as follow:  
Language-User

Get in touch on discord or open a PR to add your own app.

## NodeJS-Olajosadri

Checks status of a Hypernode, and sends a mail when it's down.


# Docs

Some related docs

## Bismuth.live HTML status

Available at:  
https://hypernodes.bismuth.live/?page_id=163

## Bismuth.live API

`https://hypernodes.bismuth.live/status.json` is updated every 3 minutes and lists all registered Hypernodes with their current PoS height, or -1 if inaccessible.

The format is a Json dict, keys being Hypernodes IP and value, the height (or -1)

Exemple:  
`{"51.15.95.155":7581,"51.15.228.170":7581,"163.172.222.163":7581,"163.172.143.181":7581}`

Do **not** hammer this url, it only updates every 3 minutes.

## IFTTT maker webhook

Allows to send SMS or mobile notification from a simple HTTP call.

TBD


