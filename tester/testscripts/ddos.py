import traceback
import requests
import threading
import time

def thread_get_request(name):
	i = 0
	for i in range (2000):
		try:
			r = requests.get('http://localhost:1000/')
			print(r.status_code)
			print(r.text)
		except Exception:
			traceback.print_exc()
			exit()

if __name__ == "__main__":
	threads = list()
	for index in range(50):
		x = threading.Thread(target=thread_get_request, args=(index,))
		threads.append(x)
		x.start()

	for index, thread in enumerate(threads):
		thread.join()