# Hypernode-Alerts
Various apps from Bismuth users to get alerted when your Hypernode is down.

Apps are named as follow:  
Language-User

Get in touch on discord or open a PR to add your own app.

## NodeJS-Olajosadri

Checks status of a Hypernode, and sends a mail when it's down.


# Docs

Some related docs

## Hypernode native client

Pull the hypernode repo https://github.com/bismuthfoundation/hypernode , no need to install nor run node or Hypernode, you just need the hn_client.py helper app.  
https://github.com/bismuthfoundation/hypernode/blob/master/main/hn_client.py

Basically, run `python3 hn_client.py --action=status --ip=YOUR_HN_IP`  
and you'll get back either a timeout or closed connection (HN is stuck or down), either a json status.

Sample output:
```
{"config": {"address": "BLYkQwGZmwjsh7DY6HmuNBpTbqoRqX14ne", "ip": "127.0.0.1", "port": 6969, "verbose": 1},
"instance": {"version": "0.0.51", "hn_version": "0.0.75", "statustime": 1532851042, "localtime": 1532851046.703249},
"chain": {"block_height": 3631, "Genesis": "BLYkQwGZmwjsh7DY6HmuNBpTbqoRqX14ne", "height": 3631, "round": 6420, "sir": 1, "block_hash": "a7a4b32406584b54bd30bacbe0457583a2d84492", "uniques": 4, "uniques_round": 0, "forgers": 4, "forgers_round": 2},
"mempool": {"NB": 0, "SENDERS": 0, "RECIPIENTS": 0},
"peers": {"connected_count": 1, "outbound": ["127.0.0.1:06971"], "inbound": ["127.0.0.1:06971"],
"net_height": {"height": 3631, "round": 6420, "sir": 1, "block_hash": "a7a4b32406584b54bd30bacbe0457583a2d84492", "uniques": 0, "uniques_round": 0, "forgers": 4, "forgers_round": 2, "count": 1, "peers": ["127.0.0.1:06971"]}},
"state": {"state": "START", "round": 8693, "sir": 1, "forger": "BHbbLpbTAVKrJ1XDLMM48Qa6xJuCGofCuH"},
"client": {"version": "0.0.51", "lib_version": "0.0.33", "localtime": 1532851046.7035139}}
```

## Bismuth.live HTML status

Available at:  
https://hypernodes.bismuth.live/?page_id=163

## Bismuth.live API

https://hypernodes.bismuth.live/status.json  
Is updated every 3 minutes and lists all registered Hypernodes with their current PoS height, or -1 if inaccessible.

The format is a Json dict, keys being Hypernodes IP and value, the height (or -1)

Exemple:  
`{"51.15.95.155":7581,"51.15.228.170":7581,"163.172.222.163":7581,"163.172.143.181":7581}`

Do **not** hammer this url, it only updates every 3 minutes.

## IFTTT maker webhook

Allows to send SMS or mobile notification from a simple HTTP call.

TBD


