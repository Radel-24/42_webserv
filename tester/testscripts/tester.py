import requests
url = 'http://10.11.5.15:7000'
myobj = {'mykey': 'myvalue'}

x = requests.post(url, data = myobj)

print(x.text)
