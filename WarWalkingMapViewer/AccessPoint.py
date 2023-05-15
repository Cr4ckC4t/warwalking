class AccessPoint:
	def __init__(self, lat, lng, bssid, ssid, enc, rssi, ch):
		self.lat = lat
		self.lng = lng
		self.bssid = bssid
		self.ssid = ssid
		self.enc = enc
		self.rssi = rssi
		self.ch = ch

	def __str__(self):
		strrepr = f'{"="*32}\n'
		strrepr += f'{self.ssid}\n'
		strrepr += f'BSSID: {self.bssid}\n'
		strrepr += f'ENCRYPTION: {self.enc}\n'
		strrepr += f'SIGNAL: {self.rssi}\n'
		strrepr += f'CHANNEL: {self.ch}\n'
		strrepr += f'{"="*32}\n'
		return strrepr