import requests
url = 'http://10.11.5.9:7000'
myobj = 'mykey:myvalue&password=test'

x = requests.post(url, myobj)

print(x.text)
